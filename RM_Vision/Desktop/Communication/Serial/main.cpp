/* kiko@idiospace.com */
//#define SERIAL_MAIN_CPP_DEBUG

#include "/home/kiko/robomaster/YLENGINE/Components/Communication/Serial/serial.hpp"
#include "/home/kiko/robomaster/YLENGINE/Components/Protocol/protocol.h"
#include "/home/kiko/robomaster/YLENGINE/Components/Communication/SerialCodec/serialcodec.hpp"

#include <iostream>
#include <thread>

using namespace hnurm;

// Serial serial("/dev/ttyUSB0");
SerialCodec serial("/dev/ttyUSB0");

std::string get_mode_str(Protocol::Work_mode mode)
{
    std::string work_mode;
    switch(mode)
        {
        case Protocol::Work_mode::manual:
            work_mode = "manual"; break;

        case Protocol::Work_mode::auto_shoot:
            work_mode = "auto_shoot"; break;

        case Protocol::Work_mode::auto_windmill:
            work_mode = "auto_windmill"; break;

        case Protocol::Work_mode::auto_save:
            work_mode = "auto_save"; break;

        case Protocol::Work_mode::auto_grab:
            work_mode = "auto_grab"; break;
        }
    return work_mode;
}

void send()
{
    for(uint i = 0 ; ; ++i) {
        auto send_data = Protocol::Vision_send_data((1+i)%3, (10+i)%100, (20+i)%100, (3000+i)%10000);
        auto send_str = Protocol::encode(send_data);
        serial.send(send_str);

        std::cout << "[sender]: size: " << send_str.size() << " time: " << i << std::endl;
        std::cout << "content: ";
        for (const auto& e: send_str) std::cout << static_cast<uint>(static_cast<unsigned char>(e)) << " ";
        std::cout << std::endl;

        sleep(1);
    }
}

void receieve()
{
   //  std::string recv_str;
    Protocol::Vision_recv_data recv_data;

    for (unsigned int i = 0 ; ; ++i)
    {
        // serial.recv(recv_str); //  At least receive 1 character, namely this method may block if receieve nothing
        if (!serial.try_get_recv_data_for(recv_data, 3))
        {
#ifdef SERIAL_MAIN_CPP_DEBUG
            std::cerr << "data broke or out of time" << std::endl;
            std::cout << std::endl;
#endif

            continue;
        }

//        std::cout << "\n[receiever] size: " << recv_str.size() << " time: " << i << std::endl;
//        std::cout << "content :";
//        for (const auto& e: recv_str) std::cout << static_cast<uint>(static_cast<unsigned char>(e)) << " ";
//        std::cout << std::endl;

        // auto recv_data = Protocol::decode(recv_str);

        std::cout << "[RECV] " << i << " times" << std::endl;
        std::cout << "self_color: " << (recv_data.self_color == Protocol::Self_color::blue ? "blue" : "red") << std::endl;
        std::cout << "act_id: " << recv_data.actuator_id << std::endl;
        std::cout << "work mode: " << get_mode_str(recv_data.mode) << std::endl;
        std::cout << "trace_on: " << (recv_data.trace_flag == Protocol::Tracker_mode::on ? "on" : "off") << std::endl;

        std::cout << std::endl << std::endl;

    }
}

int main()
{
    // std::thread sender(send);
    // std::thread receiever(receieve);
    // sender.join();
    // receiever.join();
    SerialCodec serial_codec(0);
    serial_codec.send_data(1, 2, 3, 100);
    Protocol::Vision_recv_data recv_data;
    if (serial_codec.try_get_recv_data_for(recv_data, 1000))
    {
        std::cout << "[RECV, 1000ms] " << std::endl;
        std::cout << "self_color: " << (recv_data.self_color == Protocol::Self_color::blue ? "blue" : "red") << std::endl;
        std::cout << "act_id: " << recv_data.actuator_id << std::endl;
        std::cout << "work mode: " << get_mode_str(recv_data.mode) << std::endl;
        std::cout << "trace_on: " << (recv_data.trace_flag == Protocol::Tracker_mode::on ? "on" : "off") << std::endl;
    }
    else
    {
        std::cout << "[RECV, 1000ms] no data recv" << std::endl;
    }
    return 0;
}
