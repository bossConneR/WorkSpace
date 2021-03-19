#include <iostream>
#include "opencv2/opencv.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <exception>
#include "headers/DataBuf.hpp"
#include "headers/ImageData.hpp"

using namespace std;
using namespace cv;
using namespace hnurm;

DataBuf <Wrapped<ImageData>> w_img_db;

class VideoCapture2Buf
{
private:
	int camera_id = 0;//不给相机id默认为0
	Mat frame;
	Mat frame_copy;

public:
	VideoCapture2Buf() :
		camera_id(0) {}
	VideoCapture2Buf(int camera_id_) :
		camera_id(camera_id_) {}
public:
	void capture()
	{
		VideoCapture capture(camera_id);
		try {
			if (!capture.isOpened())
				throw "cam failed to open.Check your cam id.";
			while (1)
			{
				Wrapped<ImageData> w_img;
				capture >> frame;
				frame.copyTo(frame_copy);//不知道去掉这一步行不行
				w_img.raw_data.set(camera_id, frame_copy);
				w_img_db.update(w_img, nullptr);
			}
		}
		catch (char*&e) {
			std::cerr << "emmmm" << endl;
		}
	}
};

int main()
{
	int camera_id = 0;
	VideoCapture2Buf capcamera(camera_id);
	
	//ReadFrameBuf cambuf(150);
	//cambuf.set(1);

	std::thread camreadthd(&VideoCapture2Buf::capture, &capcamera);
	//cv::waitKey(3000);//先保证Rawbuf中有东西
	//cambuf.readbuf();

	camreadthd.join();
	return 0;
}
