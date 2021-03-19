#include "COM_SEND.h"
#define TARGET_X 0
#define TARGET_Y 30

//#include"CAM_driver.h"
#include"armor_detect_0.h"

#include <opencv2/opencv.hpp>
//#include <opencv2/tracking.hpp>
//#include <opencv2/tracking/tracker.hpp>
#include <opencv2/video.hpp>
#include <iostream>

#include <string>
#include <sstream>
#include <math.h>

using namespace std;
using namespace cv;

VideoCapture capture(0);//【未封装相机驱动类】
Mat origin_img;//【未封装相机驱动类】
int exposure = 0;//【未封装相机驱动类】
int brightness = 256;
float Gamma_value = 0.2;//【未封装相机驱动类】

void on_trackbar_mian(int, void*);
void onMouse(int event, int x, int y, int flags, void*);

int main()
{
//----------------------------------串口发送代码------------------
	//打开usb
	int fd = openusb();
	//无法打开usb
	if(fd==-1){
		cout<<"open usb fails!"<<endl;
		//return 0;
	}
//------------------------------------------------------------
	//HKVcam capture;
	//capture.openCamera();
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);//设置相机宽度
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 360);//设置相机高度

	capture >> origin_img;
	char c = 0;
	namedWindow("origin_img");
	namedWindow("像素操作调参窗口", CV_WINDOW_NORMAL);
	//createTrackbar("�ع�", "origin_img", &exposure, 10, on_trackbar_mian);//��δ��װ���������?
	//createTrackbar("����", "origin_img", &brightness, 512, on_trackbar_mian);
	ArmorDetector AD("D:\\Robomaster\\PROJECTS\\HNU-V-CODE\\HNU-VISION-0\\PIXEL_PARA.txt");

	//--------------------KCF测试代码--------------------
	/*FileStorage fs = FileStorage("D:\\Robomaster\\PROJECTS\\HNU-V-CODE\\HNU-VISION-0\\KCF.xml", cv::FileStorage::Mode::WRITE, "UTF-8 XML");
	Ptr<TrackerKCF> tracker = TrackerKCF::create();
	Rect2d targetArmor;//这个量必须放在循环外部
	int judge_Mouse_right = 0, judge_KCF_init = 0;
	setMouseCallback("origin_img", onMouse, &judge_Mouse_right);*/
	double t = 0, fps;//"显示实时fps"使用的t
	//----------------------------------------------------

	//-------------------串口通信-------------------------
	RotatedRect target;
	//----------------------------------------------------

	Mat AIM;
	while (1)
	{
		//origin_img = capture.getFlame();
		capture >> origin_img;
		//imshow("origin_img", origin_img);
		
		AIM = origin_img.clone();
		line(AIM,Point(315,180),Point(325,180),Scalar(0, 255, 0),1,8);
		line(AIM,Point(320,175),Point(320,185),Scalar(0, 255, 0),1,8);
		imshow("AIM", AIM);

		//-------像素处理函数------
		AD.PixelOperator(origin_img);
		
		AD.GetLightRRect();

		//AD.getLightRois();//获取灯条roi（废弃方案）

		AD.GetArmorFromLight();


		//-------------
		fps = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		fps = 1.0 / fps;
		t = getTickCount();//网上代码不靠谱,t应该必须取上一个循环时运行到此处的时间点,再使用函数获取当前时间做减法,这样可以判断"一个循环内"消耗了多长时间,然后再用这个时间去除以cpu频率
		printf("%.2f\n", fps);		//平均帧率
		//-------------

		//---------------------KCF测试代码---------------------
		/*Mat ArmorCenters = Mat::zeros(origin_img.size(), CV_8UC1);//用于画被追踪的圆点的图像
		
		for (int i = 0; i < AD.AllPossibleArmor.size(); i++)
		{
			Point armorCenP = AD.AllPossibleArmor[i].center;
			circle(ArmorCenters, armorCenP, 20, Scalar(255, 255, 255),-1);//这些画出来的circle是要用于追踪的
		}
		imshow("ArmorCenters", ArmorCenters);
		
		if (judge_Mouse_right == 1)		//鼠标右键按下
		{
			if (judge_KCF_init == 0)		//跟踪器未被初始化过
			{
				if (AD.ChooseArmorToTrack(targetArmor) == 1)	//当前能找到装甲板目标（这条语句同时也会把targetArmor设置为可框选的装甲板）
				{
					tracker->init(ArmorCenters, targetArmor);	//***进行追踪器初始化
					//tracker->write(fs);
					judge_KCF_init = 1;							//设置初始化判断符
				}
			}
			else if (judge_KCF_init == 1)	//跟踪器已经被初始化
			{
				bool isFound = tracker->update(ArmorCenters, targetArmor);
				if (isFound != 1) {								//如果丢失目标，则重新初始化追踪器
					judge_KCF_init = 0;
					cout << "目标丢失！" << endl;
				}
			}
		}
		else if (judge_Mouse_right == 0)	//鼠标右键松开
		{
			judge_KCF_init = 0;				//设置跟踪器初始化判断符为未初始化
		}
		Mat Track_Show = ArmorCenters.clone();
		rectangle(Track_Show, targetArmor, Scalar(255, 255, 255), 3);//可视化，记得删除
		imshow("Track_Show", Track_Show);//可视化，记得删除
		//-----------------------------------------------------*/

		//----------------------------串口通信-------------------------
		float xAngle, yAngle;
		int xAngle_int, yAngle_int;
		bool result;
		//如果没有错误，发送正确坐标
		if(AD.ChooseArmorToAttack(target, xAngle, yAngle)!=-1)
		{
			xAngle_int = (int)(xAngle * 100); yAngle_int = (int)(yAngle * 100);
			result=SendDate(xAngle_int, yAngle_int , fd);
			cout<<"xAngle_int:"<<xAngle_int<<" xAngle_int:"<<xAngle_int<<endl;

			if(!result)
			{
				//exit(-1);
			}
		}
		//如果出现错误，发送错误信号
		else
		{
			cout<<"no target"<<endl;
			result=SendDate_error(fd);
		//	cout<<"result :"<<result<<endl;
			if(!result)
			{
				
				//exit(-1);
			}
		}
		//-------------------------------------------------------------

		c = cvWaitKey(1);
		if (c == 27) {
			break;
		}
	}
	//capture.closeCamera();
}

void on_trackbar_mian(int, void*)//【未封装相机驱动类】
{
	int position_init = getTrackbarPos("�ع�", "video");//获得当前轨迹条所在位置的数值
	capture.set(CV_CAP_PROP_EXPOSURE, exposure - 20);
	capture.set(CV_CAP_PROP_BRIGHTNESS, brightness - 128);//亮度 1

	printf("exposure�ع� = %.2f\n", capture.get(CV_CAP_PROP_EXPOSURE));
}
void onMouse(int event, int x, int y, int flags, void* judge_Mouse_right)
{
	if (event == EVENT_RBUTTONDOWN)		//按下右键开始跟踪
	{
		*(int*)judge_Mouse_right = 1;
	}
	else if (event == EVENT_RBUTTONUP)	//释放右键停止跟踪
	{
		*(int*)judge_Mouse_right = 0;
	}
}
