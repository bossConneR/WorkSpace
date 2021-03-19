/* kiko@idiospace.com 2021.01*/
#include <iostream>
#include <opencv.hpp>
#include <thread>
#include "/home/kiko/robomaster/YLENGINE/Components/DataBuf/SerialData/SerialData.hpp"
#include "/home/kiko/robomaster/YLENGINE/Components/DataBuf/ImageData/ImageData.hpp"
#include "/home/kiko/robomaster/YLENGINE/Components/DataBuf/DataBuf/DataBuf.hpp"

using namespace std;
using namespace cv;
using namespace hnurm;


DataBuf <Wrapped<ImageData>> w_img_db;
DataBuf <Wrapped<SerialData>> w_serial_db;

void show_serial_info(const SerialData& serial_data)
{
    std::cout << "cam_id = " << static_cast<int>(
                     static_cast<uchar>(serial_data.camera_id)) << std::endl
              << "pitch = " << serial_data.pitch << std::endl
              << "yaw = " << serial_data.yaw << std::endl
              << "distance = " << serial_data.distance << std::endl;
}

void insert_data()
{
    Mat mat1 = imread("/home/kiko/Pictures/cat.png");
    Mat mat2 = imread("/home/kiko/Pictures/cat.jpg");
    Mat mat3 = imread("/home/kiko/Pictures/ice-berg.jpg");

    for (uint i = 0; ;++i)
    {
        Wrapped<ImageData> w_img;   // create new wrapped img instance with new time_stamp
        Wrapped<SerialData> w_serial;  // create new wrapped serial instance with new time_stamp

        if (i % 3 == 0)
        {
            w_img.raw_data.set(1, mat1);
            w_serial.raw_data.set(1, 11, 21, 3001);
        }
        else if(i % 3 == 1)
        {
            w_img.raw_data.set(2, mat2);
            w_serial.raw_data.set(2, 12, 22, 3002);
        }
        else if(i % 3 == 2)
        {
            w_img.raw_data.set(3, mat3);
            w_serial.raw_data.set(3, 13, 23, 3003);
        }

        if (w_img_db.update(w_img, [&](const Wrapped<ImageData>& w_img_)->bool
        {
            return !w_img_.raw_data.mat.empty();
        }))
        {
            std::cout << "[update] times: " << i << " successed" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
        else
        {
            std::cout << "[update] failed, lock failed or data corrupted" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        if(w_serial_db.update(w_serial))
        {
            //std::cout << "[update] times: " << i << " successed" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        else
        {
            std::cout << "[update] times: " << i << " failed, lock failed or data corrupted" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

void get_data()
{
    Wrapped<ImageData> w_img;
    Wrapped<SerialData> w_serial;

    for(uint i = 0 ; ; ++i)
    {
        w_img_db.get(w_img);
        if (w_serial_db.get(w_serial))
        {
             std::cout << "[get] times: " << i << " succeed" << std::endl;
             show_serial_info(w_serial.raw_data);
             std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        else
        {
            //std::cout << "[get] time: " << i << " failed, lock failed or no new data to get" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}

int main()
{

//    single thread testing
//    DataBuf <Wrapped<SerialData>> serial_buf_1;  // For Output of Serial 1
//    DataBuf <Wrapped<SerialData>> serial_buf_2;  // For Output of Serial 2
//    DataBuf <Wrapped<SerialData>> serial_buf_3;  // For Output of Serial 3

//    DataBuf <Wrapped<ImageData>> image_buf_1;  // For Input of CAM 1
//    DataBuf <Wrapped<ImageData>> image_buf_2;  // For Input of CAM 2
//    DataBuf <Wrapped<ImageData>> image_buf_3;  // For Input of CAM 3

//    Wrapped <SerialData> wrapped_serial_data;
//    wrapped_serial_data.raw_data.set(1, 10, 20, 30);
//    serial_buf_1.update(wrapped_serial_data);
//    wrapped_serial_data.raw_data.set(2, 20, 30, 40);
//    serial_buf_2.update(wrapped_serial_data);
//    wrapped_serial_data.raw_data.set(3, 40, 50, 60);
//    serial_buf_3.update(wrapped_serial_data);

//    serial_buf_1.get(wrapped_serial_data);
//    show_serial_info(wrapped_serial_data.raw_data);

//    serial_buf_2.get(wrapped_serial_data);
//    show_serial_info(wrapped_serial_data.raw_data);

//    serial_buf_3.get(wrapped_serial_data);
//    show_serial_info(wrapped_serial_data.raw_data);

//    Mat mat1 = imread("/home/kiko/Pictures/cat.png");
//    Mat mat2 = imread("/home/kiko/Pictures/cat.jpg");
//    Mat mat3 = imread("/home/kiko/Pictures/ice-berg.jpg");

//    Wrapped<ImageData> wrapped_image_data;

//    wrapped_image_data.raw_data.set(1, mat1);
//    image_buf_1.update(wrapped_image_data);
//    wrapped_image_data.raw_data.set(2, mat2);
//    image_buf_2.update(wrapped_image_data);
//    wrapped_image_data.raw_data.set(3, mat3);
//    image_buf_3.update(wrapped_image_data);

//    image_buf_1.get(wrapped_image_data);
//    std::cout << "id = " << static_cast<int>(static_cast<uchar>(wrapped_image_data.raw_data.camera_id)) << std::endl;
//    imshow("img_data_1", wrapped_image_data.raw_data.mat);
//    waitKey(0);

//    image_buf_2.get(wrapped_image_data);
//    std::cout << "id = " << static_cast<int>(static_cast<uchar>(wrapped_image_data.raw_data.camera_id)) << std::endl;
//    imshow("img_data_2", wrapped_image_data.raw_data.mat);
//    waitKey(0);

//    image_buf_3.get(wrapped_image_data);
//    std::cout << "id = " << static_cast<int>(static_cast<uchar>(wrapped_image_data.raw_data.camera_id)) << std::endl;
//    imshow("img_data_3", wrapped_image_data.raw_data.mat);
//    waitKey(0);

    // multi-thread testing
    std::thread insert_thread1(insert_data);
    std::thread insert_thread2(insert_data);

    std::thread get_thread1(get_data);
    std::thread get_thread2(get_data);

    insert_thread1.join();
    insert_thread2.join();
    get_thread1.join();
    get_thread2.join();

    std::cout << "hi" << std::endl;
}
