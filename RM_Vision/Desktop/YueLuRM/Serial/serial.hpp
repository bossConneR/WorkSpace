/* kiko@idiospace.com 2021.01 */

/* Serial API
*  1. int send(const std::string& s);
*  @Brief: Send 'send_msg' to serial port
*  @return: bytes sent to serial port (int Serial::serial_port)
*
*
*  2. void recv(std::string& s);
*  @Brief: Retrieve a at least one byte and store in string s
*  @return: none
*  @NOTICE: This method will block the process if reach no data!
*
*
*  3. bool try_recv_for(std::string& s, int milli_secs = 3);
*  @Brief:  wait up to mill_secs milliseconds (default 3 ms)
*  @return: if retrieved at least one byte, then return true, otherwise return false.
*  @Notice: This method does not block, good feature for games like RM.
*/

#ifndef __SERIAL_HPP__
#define __SERIAL_HPP__

// C library
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>    // Contains file controls like open(), fcntl(), O_RDWR
#include <errno.h>    // Error integer and strerror() function
#include <termios.h>  // Contains POSIX terminal control definitions
#include <unistd.h>   // write(), read(), close()
#include <string>     // Store the Msg
#include <iostream>
#include <chrono>

namespace hnurm {

class Serial
{
protected:
    using Ms = std::chrono::milliseconds;   // ms
    using clk = std::chrono::high_resolution_clock;  // clk

public:

    Serial() = delete;
    Serial(std::string device_name);
    Serial(int usb_id);
    ~Serial(); 
 
    int send(const std::string& s);     // Send 'send_msg' to serial port
    void recv(std::string& s);          // Retrieve at least one byte store in string s
    // Try to retrive str from serial port and store in s for up to milli_secs
    // return true if at least one byte has been retrieved, otherwise return false
    bool try_recv_for(std::string& s, int milli_secs);

private:
    int serial_port;                    // A file descriptor for the opened serial port
    struct termios tty;                 // Create new termios struct, call it 'tty' for convention
    
    bool open_port(std::string dev_name); // None block by default
    bool open_port(int id);             // None block by default

    void close_port();
    
    bool rm_init();                     // Set baud rate, parity, RTS/CTS, etc
    
    int raw_write(const std::string& s);// Write to STM32
    std::string raw_read();      // Read from STM32
    
    void clear_input_buffer();          // Clear read buffer
    void clear_output_buffer();         // Clear write buffer
    void clear_input_output_buffer();   // Clear input & output buffer  
    
};

void Serial::clear_input_buffer()
{
    tcflush(serial_port, TCIFLUSH);                
}

void Serial::clear_output_buffer()
{
    tcflush(serial_port, TCOFLUSH);        
}

void Serial::clear_input_output_buffer()
{
    tcflush(serial_port, TCIOFLUSH);
}

Serial::Serial(std::string device_name)
{
    while (!open_port(device_name)) std::cerr << "[ERROR] OPEN PORT ERROR" << std::endl;
    rm_init();
}

Serial::Serial(int usb_id)
{
    while (!open_port(usb_id)) std::cerr << "[ERROR] OPEN PORT ERROR" << std::endl;
    rm_init();
}

Serial::~Serial() 
{
    close_port();
}

int Serial::send(const std::string& s)
{       
    // clear_output_buffer();  lose data, don't do this.
    return raw_write(s);
}

void Serial::recv(std::string& s)
{
    s = raw_read();
}

bool Serial::rm_init()
{
    tty.c_cflag &= ~PARENB;  // Cleaer parity bit, disableing parity
    tty.c_cflag &= ~CSTOPB;  // Clear stop field, only use one stop bit, Otherwise two bits are used
    tty.c_cflag &= ~CSIZE;   // Clear all the size bits, then use one of the statements below
    tty.c_cflag |= CS8;      // Transmit 8 bits per byte across the serial port
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control to avoid infinite waiting, RTS - request transmit
    tty.c_cflag |= CREAD | CLOCAL;  // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;  // Disable canonical mode
    tty.c_lflag &= ~ECHO;    // Disable echo
    tty.c_lflag &= ~ECHOE;   // Disable erasure
    tty.c_lflag &= ~ECHONL;  // Disable new-line echo
    tty.c_lflag &= ~ISIG;    // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST;   // Prevent special interpretation of output bytes (eg: newline chars)
    tty.c_oflag &= ~ONLCR;   // Prevent conversion of newline to carriage return / line feed
    
    // READ SETTINGS: Only work at block mode
    tty.c_cc[VTIME] = 1;     // wait for up to 100ms between characters since first char has been reached
    tty.c_cc[VMIN] = 1;      // wait for at least one character 

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsattr: %s\n", errno, strerror(errno));
        return false;
    }

    // Clear serial IO buffers
    clear_input_output_buffer();

    return true;
}

// dev_name, eg: "/dev/ttyUSB0" or "/dev/ttyS0", etc
bool Serial::open_port(std::string dev_name)
{
    // Open the serial port. Change device path as needed
    // Currently set to an standard FTDI USB-UART cable type device
    std::string device_name = dev_name;
    serial_port = open(device_name.c_str(), O_RDWR | O_NONBLOCK);

    if (tcgetattr(serial_port, &tty) != 0)
    {
        // printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool Serial::open_port(int id)
{
    std::string device_name = "/dev/ttyUSB" + std::to_string(id); 
    return open_port(device_name);
}

void Serial::close_port()
{
    try{
        close(serial_port);
    } catch (...) {}
}

int Serial::raw_write(const std::string& s)
{
    return write(serial_port, s.c_str(), s.size());
}

bool Serial::try_recv_for(std::string& s, int milli_secs)
{
    auto start = clk::now();

    s.clear();
    char buf_[256];

    while (s.empty())
    {
        int num_bytes = read(serial_port, &buf_[0], sizeof(buf_));
        if (num_bytes > 0)
        {
            s = std::string(&buf_[0], num_bytes);
            return true;
        }

        auto end = clk::now();
        auto gap = std::chrono::duration_cast<Ms>(end - start);
        if(gap.count() > milli_secs)
        {
            break;
        }
    }
    return false;
}

std::string Serial::raw_read()
{
    static std::string s;

    s.clear();
    char buf_[256];
    while (s.empty())
    {
        int num_bytes = read(serial_port, &buf_[0], sizeof(buf_));
        // if (num_bytes > 0) printf("Read %i bytes. Received message: %s\n", num_bytes, buf_);
        // else printf("recv nothing\n");
        if (num_bytes > 0)
        {
            s = std::string(&buf_[0], num_bytes);
            break;
        }
    }
    return s;
}

}

#endif


