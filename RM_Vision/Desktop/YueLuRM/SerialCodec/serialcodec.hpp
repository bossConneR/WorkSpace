/* kiko@idispace.com 2021.01 */

//#define SERIAL_CODEC_DEBUG

/* SerialCodec API
 * 1. SerialCodec(int usb_id);
 * @Brief: open serial_port "/dev/ttyUSB(usb_id)"
 *
 * 2. SerialCode(std::string device_name);
 * @Brief: open serial_port "device_name"
 *
 * 3. bool try_get_recv_data_for(Protocol::Vision_recv_data& recv_data, int milli_secs = 10);
 * @Brief: try to retrieve data from serial port within mill_secs milliseconds
 * @return: if data has been fetched and decoded properly, then return true, otherwise return false
 * @NOTICE: this method will block the process for at most mill_secs milliseconds
 *
 * 4. bool send_data(int cam_id, float pitch, float yaw, float distance);
 * @Brief: encode and send data to the serial port
 * @return: true if data sent succeed
*/

#ifndef SERIALCODEC_HPP
#define SERIALCODEC_HPP

#include "../Protocol/protocol.h"
#include "../Serial/serial.hpp"
#include <chrono>
#include <exception>

namespace hnurm {

// Read & Write from or to the serial port
// Ensure integrity of receieved data pack thourgh this
// wrappered class combined with Serial and Protocol
class SerialCodec: public Serial, public Protocol
{

public:
    SerialCodec(int usb_id) :
        Serial(usb_id)
    {
    }
    SerialCodec(std::string dev_name) :
        Serial(dev_name)
    {}

public:
    // may get broke msg or don't get enough msg to decode, then return false
    // if both retrieve data and decode work fine, then return true
    // by defualt wait for up to 10 milli-secs (tests shows that 3ms is enough to work)
    bool try_get_recv_data_for(Protocol::Vision_recv_data& recv_data, int milli_secs = 10);

    // encode and send data to the serial port
    bool send_data(int cam_id, float pitch, float yaw, float distance);

};

// encode and send data to the serial port
// return true if send succeedd, otherwise return false
bool SerialCodec::send_data(int cam_id, float pitch, float yaw, float distance)
{
    auto data = Protocol::Vision_send_data(cam_id, pitch, yaw, distance);
    auto data_str = Protocol::encode(data);
    return Serial::send(data_str) > 0;
}

// try receive a valid package within milli_secs
// and decode it to a Protocol::Vision_recv_data
bool SerialCodec::try_get_recv_data_for(Protocol::Vision_recv_data& recv_data, int milli_secs)
{
    try{
    auto start = clk::now();

    volatile static int da_len = 10;   // default 10 bits,
                                 // auto detect string stream from serialport and update
                                 // no need for handcraft modification

    static bool pack_start = false;
    static std::string recv_buf = "";  // receive buffer
    static std::string tmp_str;        // receive some data from each recv
    int find_pos = std::string::npos;  // not find by defualt

#ifdef SERIAL_CODEC_DEBUG
    static int i = 0;
#endif

    tmp_str.clear();

    while (1)
    {
        // step1: try to receive str from serial port for up to milli_secs milliseconds
        if (Serial::try_recv_for(tmp_str, milli_secs))
        {
            find_pos = tmp_str.find_first_of(static_cast<char>(PROTOCOL_CMD_ID));

            // step1: handle newly received data string

            // package has not started && find frame header : 0XA5
            if (!pack_start && find_pos != std::string::npos)
            {
                pack_start = true;
                recv_buf = tmp_str.substr(find_pos);
                // grab from the header identifier 0XA5 to the end
            }
            else if (!pack_start && find_pos == std::string::npos)
            {
                auto end = clk::now();
                auto gap = std::chrono::duration_cast<Ms>(end - start);
                if (gap.count() > milli_secs)
                {
                    break;
                }
                // package not started and didn't find identifier
                // just pass and try to get some new data
                continue;
            }
            else if(pack_start && find_pos == std::string::npos)
            {
                // pack started and received data of this pack, just append them
                recv_buf.append(tmp_str);
            }
            else if (pack_start && find_pos != std::string::npos)
            {
                // pack started and find another frame
                // just grab the front data before next frame
                recv_buf.append(tmp_str.substr(0, find_pos));
            }

            // step2: update data_len
            if (recv_buf.size() > 3)
            {
                // update data_len if valid
                auto new_data_len = static_cast<int>(recv_buf[2] | recv_buf[1]);
                auto new_full_pack_len = new_data_len + OFFSET_BYTE;
                if ((new_full_pack_len >= MIN_DATA_LENGTH) &&
                        (new_full_pack_len <= MAX_DATA_LENGTH))
                {

                    data_len = new_full_pack_len;
    #ifdef SERIAL_CODEC_DEBUG
                    std::cout << "new_full_pack_len" << new_full_pack_len << std::endl;
                    std::cout << "data_len = " << data_len << std::endl;
    #endif
                }
    #ifdef SERIAL_CODEC_DEBUG
                // debug
                std::cout << "outer data_len = " << data_len << std::endl;
    #endif
            }

            // step3: check data integrity
            if (recv_buf.size() >= data_len)
            {
                if (decode(recv_buf,recv_data))
                {
                    pack_start = false;
                    recv_buf.clear();
                    return true;
                }
                else
                {
                    pack_start = false;
                    recv_buf.clear();
                    auto end = clk::now();
                    auto gap = std::chrono::duration_cast<Ms>(end - start);
                    if (gap.count() > milli_secs)
                    {
                        break;
                    }
                }
            }

        }
        auto end = clk::now();
        auto gap = std::chrono::duration_cast<Ms>(end - start);
        if (gap.count() > milli_secs)
        {
#ifdef SERIAL_CODEC_DEBUG
            std::cout << "times:" << i++ << " gap.count(): " << gap.count() << std::endl;
#endif
            break;
        }
    }

    return false;

    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}


} // namespace hnurm

#endif // SERIALCODEC_HPP
