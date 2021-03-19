#include <iostream>
#include "opencv2/opencv.hpp"
#include <vector>
#include <queue>
#include <thread>

using namespace std;
using namespace cv;

const string window_name = "用户界面";
/*
研究一下hx的代码时如何可以取出最后一帧的
*/
queue<Mat> frameBuf;
int bufsize = 2;
int FPS = 200;//100 fps

void consumer()
{
	int i;
	int rate = 0;
	for (i=0; ; i++)
	{
		double t = (double)cv::getTickCount();//开始计时

		if (frameBuf.empty()) { break; }
		
		imshow("BufReadTest", frameBuf.front());
		rate = (1000 * (1.0 / FPS));
		cv::waitKey(rate);
		//frameBuf.pop();
		cout << "BufsizeRemain:" << ' ' << frameBuf.size() << ' ' << endl;

		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();//结束计时
		int fps = int(1.0 / t);//转换为帧率
		cout << "FPS: " << fps << endl;//输出帧率
	}
}

void productor()
{
	Mat frame;
	VideoCapture capture(0);
	int i = 0;
	char text[10];
	Mat frame_copy;

	while (capture.isOpened())
	{
		i += 1;

		//double t = (double)cv::getTickCount();//开始计时
		capture >> frame;

		
		frame.copyTo(frame_copy);
		/*_itoa_s(i, text, 10);
		putText(frame_copy, text, Point(50, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 23, 0), 4, 8);*/

		frameBuf.push(frame_copy);


		if (i >= bufsize)
		{
			frameBuf.pop();
		}

		cout << frameBuf.size() << ' '<<endl;

			
		//elps_t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();//结束计时
		//int fps = int(1.0 / elps_t);//转换为帧率
		//if (i % 3 == 0)
		//{
		//	fps_sum = fps_sum / 2;
		//	cout << "FPS: " << fps_sum << endl;//输出帧率
		//	fps_sum = 0;
		//}
		//else
		//{
		//	fps_sum += fps;
		//}
			

		if (i == bufsize+1000) { break; }
	}


	cv::waitKey(0);
	capture.release();
}


int main()
{
	thread generator(productor);
	waitKey(3000);
	thread reader(consumer);

	generator.join();
	reader.join();
	
	return 0;
}