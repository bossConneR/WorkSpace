//#define MODIFY_PixelOperator_PARAMETERS
#define SHOW_PIXEL_OPERATOR_IMG_Level 0
#define SHOW_DATA_OPERATOR_IMG_Level 0

#pragma once
#ifndef armor_detect_0
#define armor_detect_0

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <vector>

using namespace std;
using namespace cv;

/////////////////////////////////////////////////
////////////////////【专用数据结构】//////////////////////
struct Armor {
	RotatedRect LeftLight;//装甲板左灯条的旋转矩阵
    RotatedRect RightLight;//装甲板右灯条的旋转矩阵
	RotatedRect ArmorRRect;//两个灯条拟合后形成一个装甲板
	Armor();
	Armor(RotatedRect LeftLight_, RotatedRect RightLight_, RotatedRect ArmorRRect_) {
		LeftLight = LeftLight_;
		RightLight = RightLight_;
		ArmorRRect = ArmorRRect_;
	}
};

class ArmorDetector
{
public:
	/////////////////////////////////////////////////
	////////////////////【参数区域】//////////////////////
	

	/////////////////////////////////////////////////
	////////////////////【变量区域】//////////////////////
	//####################-2.计时模块变量###################//
	double startT;
	double endT;
	double LastTime;
	//####################1.像素操作变量####################//（Img变量前的编号代表“被处理的顺序”）
	Mat origin_Img;					//1原始图像
	Mat GammaCorrected_Img;			//2【MyGammaCorrection】伽马矫正过后的图像
	Mat gblur_Img;					//3【GaussianBlur】高斯模糊后的图像
	vector<Mat> channels;			//4【split】分离通道后的图像
	Mat subtracted_Img;				//5【absdiff】储存红蓝通道相减得到的图像
	Mat binary_color_Img;			//6【threshold】颜色二值化图片（就是对红蓝相减后的图像做二值化）
	Mat binary_brightness_img;		//6【threshold】亮度二值化图片
	Mat binary_light_img;			//6【threshold】对 颜色二值化图片 与 亮度二值化图片 取取交集（这幅图像同时也用于下一步的灯条数据化）
	Mat eroded_Img;					//7【eroded】
	Mat dilated_Img;				//8【dilated】/
	//####################2.数据化操作变量####################//
	vector<vector<Point>> light_contours;			//1【findContours】储存找到的灯光的轮廓
	vector<Vec4i> light_hierachy;					//1【findContours】储存“发光源 的轮廓”的层级关系（findContours的mode为RETR_CCOMP）
	vector<vector<Point>> light_choosed_contours;	//1在【findContours】的基础上简单筛选出的“灯条”轮廓 <*筛选方法待完善*>
	
	vector<RotatedRect> AllrLightRect;				//2【minAreaRect】存储由轮廓拟合成的所有旋转矩形  如果此变量定义在循环外部就记得清零
	
	vector<RotatedRect> LeftLight;//注意区分，这里面存储的是light，不是light的roi！！！
	vector<RotatedRect> RightLight;
	
	vector<RotatedRect> AllPossibleArmor;			//3所有可能的装甲板
	vector<Armor> AllPossibleArmorCompleteInfo;		//3所有可能的装甲板的完整信息，包括这个装甲板的左右灯条
	/////////////////////////////////////////////////
	////////////////////【函数区域】//////////////////////
	//####################-2.计时模块函数###################//
	void start_count();
	void end_count(int type);
	//####################0.构造析构函数####################//
	ArmorDetector();
	ArmorDetector(string PIXEL_file_address);			//从一个文件里面初始化ArmorDetector的参数，如果开启了调参模式，在程序 正常结束 时还会把调整后的参数存入文件中
	~ArmorDetector();

	//####################1.像素操作函数####################//
	Mat PixelOperator(Mat origin_Img);

	//####################2.数据化操作函数####################//
	void GetLightRRect();						//从灯条二值图中获取所有灯条旋转矩形【minAreaRect】得到所有灯条的旋转矩形（未完善）
	void getLightRois();						//获取灯条roi区域，为“判别是左灯条还是右灯条”做准备
	void GetArmorFromLight();					//两两比对所有灯条旋转矩形，得到装甲板（仍需要大量测试与枚举）
	int ChooseArmorToTrack(Rect2d &target);		//（用于KCF）选择装甲板进行射击(输入为一个Rect2d矩形的引用，这个Rect2d会被修改)（其实这个Rect2d是用于初始化跟踪器的）
	int ChooseArmorToAttack(RotatedRect &target, float &xAngle, float &yAngle);	//直接用于发送给A型板的数据
private:
	/////////////////////////////////////////////////
	////////////////////【参数区域】//////////////////////
	//####################0.文件操作参数####################//
	string PixelFile_Address;
	//####################1.像素操作参数####################//
	int GammaCorrected_value = 10;		//2【MyGammaCorrection】伽马矫正值
	int GaussianBlur_value = 5;			//3【GaussianBlur】高斯模糊后的图像
	int color_threshold_value = 103;		//6【threshold】颜色 二值化阈值
	int brightness_threshold_value = 44;//6【threshold】亮度 二值化阈值

	int blue_threshold_value = 225;		//蓝色通道二值化阈值
	int blue_contrast_value_alpha = 1;	//蓝色通道对比度alpha值
	int blue_contrast_value_beta = 1;	//蓝色通道对比度beta值

	int ele_erode_x = 2, ele_erode_y = 8;//7【erode】【开启调参模式时】腐蚀操作结构元素的Size的x值与y值	<*-优化-*>由于每个循环都调用一次这个getStructuringElement函数，开销会比较大
	int ele_dilated_x = 2, ele_dilated_y = 8;//8【dilated】【开启调参模式时】膨胀操作
	Mat ele_erode = getStructuringElement(MORPH_RECT, Size(ele_erode_x, ele_erode_y));//7【erode】【关闭调参模式时】腐蚀操作结构元素
	Mat els_dilated = getStructuringElement(MORPH_RECT, Size(ele_dilated_x, ele_dilated_y));//8【dilated】【关闭调参模式时】膨胀操作结构元素
	//Mat element_erode = getStructuringElement(MORPH_RECT, Size(2, 8));
	/////////////////////////////////////////////////
	////////////////////【变量区域】//////////////////////
	//####################1.像素操作变量####################//
	
	/////////////////////////////////////////////////
	////////////////////【函数区域】//////////////////////
	//####################-1.绘图函数########################//
	void DrawRotatedRect(Mat& src, RotatedRect rect, Scalar color, int thickness);

	//####################1.像素操作函数####################//
	void MyGammaCorrection(Mat& src, Mat& dst, float fGamma);//伽马矫正函数（自编）

	//####################2.数据操作函数####################//
	RotatedRect boundingRRect(RotatedRect rRect1, RotatedRect rRect2);//把两个灯条拟合成装甲板
};

#endif
