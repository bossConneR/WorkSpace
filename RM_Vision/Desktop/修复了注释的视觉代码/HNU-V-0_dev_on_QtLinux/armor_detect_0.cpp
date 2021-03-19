
#define BLUE
#include "armor_detect_0.h"

//----------------------------------*-2.计时函数*-------------------------------------------
void ArmorDetector::start_count() {
	startT = ((double)getTickCount() / getTickFrequency());
}
void ArmorDetector::end_count(int type) {
	endT = ((double)getTickCount() / getTickFrequency());
	LastTime = endT - startT;
	if(type==1)
		printf("%lf\n", LastTime);
}
//----------------------------------*-1.绘图函数*-------------------------------------------

void ArmorDetector::DrawRotatedRect(Mat& src, RotatedRect rect, Scalar color, int thickness)
{
	Point2f vertices[4];
	rect.points(vertices);
	for (int i = 0; i < 4; i++)
	line(src, vertices[i], vertices[(i + 1) % 4], color, thickness);
}

//---------------------------------*0.文件读写函数*-----------------------------------------

ArmorDetector::ArmorDetector() {

}
ArmorDetector::ArmorDetector(string PIXEL_file_address) {
	//----------检查像素操作参数文件合法性操作?------------//
	fstream fs(PIXEL_file_address);
	if (fs) {					//如果fs打开文件成功
		cout << "-打开 像素操作参数文件 成功" << endl;
		vector<string> PARAMETERS;
		string temp;
		while (fs >> temp) {
			PARAMETERS.push_back(temp);
		}
		bool in_legal = 1;		//合法性判符
		if (PARAMETERS.size() == 8) {
			for (int i = 0; i < PARAMETERS.size(); i++) {
				for (int j = 0; j < PARAMETERS[i].size(); j++) {
					if ('0' <= PARAMETERS[i][j] && PARAMETERS[i][j] <= '9') {//【合法性】只要每个字符串都仅仅由数字组成

					}
					else {
						in_legal = 0;
						break;
					}
				}
			}
			if (PARAMETERS[4] == "0" || PARAMETERS[5] == "0" || PARAMETERS[6] == "0" || PARAMETERS[7] == "0") {//【合法性】膨胀腐蚀结构元素不能为0，会报错
				in_legal = 0;
			}
		}
		else {
			in_legal = 0;
		}
		
		if (in_legal) {			//仅当所有参数都合法的时候才进行赋值
			stringstream ss;	//PIXEL_file像素操作参数文件说明：文件中目前共8个参数，参数之间用空格分隔
			ss << PARAMETERS[0]; ss >> GammaCorrected_value;	ss.clear();
			ss << PARAMETERS[1]; ss >> GaussianBlur_value;		ss.clear();
			ss << PARAMETERS[2]; ss >> color_threshold_value;	ss.clear();
			ss << PARAMETERS[3]; ss >> brightness_threshold_value;	ss.clear();
			ss << PARAMETERS[4]; ss >> ele_erode_x;				ss.clear();		ss << PARAMETERS[5]; ss >> ele_erode_y;		ss.clear();
			ss << PARAMETERS[6]; ss >> ele_dilated_x;			ss.clear();		ss << PARAMETERS[7]; ss >> ele_dilated_y;	ss.clear();
			
			cout << "-初始化 像素操作参数 成功" << endl;
#ifndef MODIFY_PixelOperator_PARAMETERS
			PixelFile_Address = "no_data";//如果没打开调参模式，则就算初始化成功也不会读写文件
#endif // !MODIFY_PixelOperator_PARAMETERS

#ifdef MODIFY_PixelOperator_PARAMETERS
			cout << "-您已成功开启 *调参模式*\n 将会在ArmorDetector对象 析构 时把当前调参的数据存入文件 请按Esc正常退出程序（这个esc结束其实是实现在主函数里的）" << endl;
			PixelFile_Address = PIXEL_file_address;
#endif // MODIFY_PixelOperator_PARAMETERS

		}
		else {
			cerr << "-初始化 像素操作参数 失败：参数不合法, 将使用默认参数" << endl;
			PixelFile_Address = "no_data";
		}
	}
	else {
		cerr << "-打开 像素操作参数文件 失败：找不到文件, 将使用默认参数"<<endl;
		PixelFile_Address = "no_data";
	}
	
}
ArmorDetector::~ArmorDetector() {
#ifdef MODIFY_PixelOperator_PARAMETERS
	if (PixelFile_Address != "no_data") {
		ofstream Out; 
		Out.open(PixelFile_Address);
		if (Out) {
			Out << GammaCorrected_value <<" ";
			Out << GaussianBlur_value << " ";
			Out << color_threshold_value << " ";
			Out << brightness_threshold_value << " ";
			Out << ele_erode_x << " ";			Out << ele_erode_y << " ";
			Out << ele_dilated_x << " ";		Out << ele_dilated_y << " ";
		}
	}
#endif // MODIFY_PixelOperator_PARAMETERS

}

void ArmorDetector::MyGammaCorrection(Mat& src, Mat& dst, float fGamma)//伽马矫正函数【私有】
{
	// build look up table  
	unsigned char lut[256];
	for (int i = 0; i < 256; i++)
	{
		lut[i] = saturate_cast<uchar>(pow((float)(i / 255.0), fGamma) * 255.0f);
	}

	dst = src.clone();
	const int channels = dst.channels();
	switch (channels)
	{
	case 1:   //灰度图的情况
	{

		MatIterator_<uchar> it, end;
		for (it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++)
			//*it = pow((float)(((*it))/255.0), fGamma) * 255.0;  
			*it = lut[(*it)];

		break;
	}
	case 3:  //彩色图的情况
	{

		MatIterator_<Vec3b> it, end;
		for (it = dst.begin<Vec3b>(), end = dst.end<Vec3b>(); it != end; it++)
		{
			//(*it)[0] = pow((float)(((*it)[0])/255.0), fGamma) * 255.0;  
			//(*it)[1] = pow((float)(((*it)[1])/255.0), fGamma) * 255.0;  
			//(*it)[2] = pow((float)(((*it)[2])/255.0), fGamma) * 255.0;  
			(*it)[0] = lut[((*it)[0])];
			(*it)[1] = lut[((*it)[1])];
			(*it)[2] = lut[((*it)[2])];
		}

		break;

	}
	}
}

//-------------------------------------1.像素操作函数---------------------------------------

void callback(int value, void* detector);

Mat ArmorDetector::PixelOperator(Mat Origin_Img)
{	//######################处理图像部分代码######################//
	origin_Img = Origin_Img;//将原图像赋值给ArmorDetector的成员

	//MyGammaCorrection(origin_Img, GammaCorrected_Img, GammaCorrected_value / 10.0);						//2���ɵ��Ρ�٤������			//��ʱ̫����ע�͵�
	if (GaussianBlur_value % 2 == 0) { GaussianBlur_value += 1; }//保证高斯核是合法的									
	//GaussianBlur(origin_Img, gblur_Img, Size(GaussianBlur_value, GaussianBlur_value), 4, 4);	//3【可调参】高斯模糊（调节高斯模糊size值）GammaCorrected_Img -> gblur_Img		耗时：0.00322001s		//耗时太长，注释掉
	
	//split(gblur_Img/*origin_Img*/, channels);																			//4 分离图像三通道函数，gblur_Img -> channel					耗时：0.000607516s	
	split(/*gblur_Img*/origin_Img, channels);
	
#ifdef BLUE
	subtract(channels.at(0), channels.at(2), subtracted_Img);											//5 这里是把伽马校正后的 蓝色或者红色 通道与 绿色通道 相减，可以只剩下蓝色或者红色的区域	channel -> subtracted_img		耗时：0.000176879s
#endif
#ifdef RED
	subtract(channels.at(2), channels.at(0), subtracted_Img);											//5 这里是把伽马校正后的 蓝色或者红色 通道与 绿色通道 相减，可以只剩下蓝色或者红色的区域	channel -> subtracted_img		耗时：0.000176879s
#endif
	threshold(subtracted_Img, binary_color_Img, color_threshold_value, 255, THRESH_BINARY);				//6【可调参】颜色二值化（就是对红蓝相减后的图像做二值化）		耗时：0.000585319s																									
	
	//-----------针对高亮度灯条的算法-----------

	//将蓝色通道先调节对比度（或许不止）后再进行二值化，滤去一大部分，再用蓝色通道二值化后的图片减去（蓝色减红色）的图片
/*	Mat blueContrast;//蓝色通道调节对比度后
	Mat blueThreshold;//蓝色通道二值化后

	channels.at(0).convertTo(blueContrast, channels.at(0).type(), blue_contrast_value_alpha, blue_contrast_value_beta);

	threshold(blueContrast, blueThreshold, blue_threshold_value, 255, THRESH_BINARY);
	Mat subtracted_Blue_Bc_Img;
	subtract(blueThreshold, binary_color_Img, subtracted_Blue_Bc_Img);
	imshow("(4)蓝色通道二值化减去binary_color_Img", subtracted_Blue_Bc_Img);*/
	//------------------------------------------


	Mat gray_img_;	cvtColor(/*gblur_Img*/origin_Img, gray_img_, cv::ColorConversionCodes::COLOR_BGR2GRAY);			//-6 亮度二值化的中间过程										��ʱ��0.000713704s
	
	threshold(gray_img_, binary_brightness_img, brightness_threshold_value, 255, CV_THRESH_BINARY);		//6 亮度二值化													��ʱ��0.000462663s

	//binary_light_img = subtracted_Blue_Bc_Img & binary_brightness_img;										//6 对 颜色二值化图片 与 亮度二值化图片 取交集					��ʱ��0.000187973s
	binary_light_img = binary_color_Img & binary_brightness_img;

#ifdef MODIFY_PixelOperator_PARAMETERS
	//erode(binary_light_img, eroded_Img, getStructuringElement(MORPH_RECT, Size(ele_erode_x, ele_erode_y)));		//7【可调参】腐蚀操作（每次循环都新建结构元素）			��ʱ��0.00202596s
	
	//dilate(eroded_Img, dilated_Img, getStructuringElement(MORPH_RECT, Size(ele_dilated_x, ele_dilated_y)));		//8【可调参】膨胀操作（每次循环都新建结构元素）			��ʱ��0.00160518s
	
#endif // MODIFY_PixelOperator_PARAMETERS
#ifndef MODIFY_PixelOperator_PARAMETERS

	//erode(binary_light_img, eroded_Img, ele_erode);														//7【关闭调参模式时】腐蚀操作									��ʱ��0.00190941s

	//dilate(eroded_Img, dilated_Img, els_dilated);														//8【关闭调参模式时】膨胀操作

#endif // !MODIFY_PixelOperator_PARAMETERS



	//######################调参部分代码######################//
#ifdef MODIFY_PixelOperator_PARAMETERS
	namedWindow("像素操作调参窗口",WINDOW_AUTOSIZE);
	createTrackbar("伽马矫正值", "像素操作调参窗口", &GammaCorrected_value, 10, NULL);
	createTrackbar("高斯模糊值", "像素操作调参窗口", &GaussianBlur_value, 15, NULL);
	createTrackbar("颜色二值化阈值", "像素操作调参窗口", &color_threshold_value, 255, NULL);
	createTrackbar("亮度二值化阈值", "像素操作调参窗口", &brightness_threshold_value, 255, NULL);
	//-----------针对高亮度灯条的算法?-----------
	//createTrackbar("蓝色通道对比度", "像素操作调参窗口", &blue_threshold_value, 255, NULL);
	//createTrackbar("blue_contrast_value_alpha", "像素操作调参窗口", &blue_contrast_value_alpha, 10, NULL);
	//createTrackbar("blue_contrast_value_beta", "像素操作调参窗口", &blue_contrast_value_beta, 50, NULL);
	//------------------------------------------
	namedWindow("膨胀腐蚀调参窗口", WINDOW_AUTOSIZE);
	createTrackbar("ele_erode_x", "膨胀腐蚀调参窗口", &ele_erode_x, 10, NULL);
	createTrackbar("ele_erode_y", "膨胀腐蚀调参窗口", &ele_erode_y, 10, NULL);
	createTrackbar("ele_dilated_x", "膨胀腐蚀调参窗口", &ele_dilated_x, 10, NULL);
	createTrackbar("ele_dilated_y", "膨胀腐蚀调参窗口", &ele_dilated_y, 10, NULL);
#endif // MODIFY_PixelOperator_PARAMETERS在armor_detect_0.h定义此宏后开启像素操作的调参功能


	//######################显示中间图像部分代码######################//
#ifdef SHOW_PIXEL_OPERATOR_IMG_Level
	if (SHOW_PIXEL_OPERATOR_IMG_Level >= 2)
	{
		//imshow("(2)GammaCorrected_Img", GammaCorrected_Img);	//耗时太长，注释掉
		//imshow("(3)gblur_Img", gblur_Img);					//耗时太长，注释掉
		imshow("(4)<1>蓝色通道图像", channels.at(0));
		imshow("(4)<2>绿色通道图像", channels.at(1));
		imshow("(4)<3>红色通道图像", channels.at(2));

		//imshow("(4)蓝色通道对比度后图像", blueContrast);
		//imshow("(4)蓝色通道二值化后图像", blueThreshold);

		imshow("(5)subtracted_img", subtracted_Img);
		imshow("(6)binary_color_Img", binary_color_Img);
		imshow("(6)binary_brightness_img", binary_brightness_img);
		imshow("(6)binary_light_img", binary_light_img);
		//imshow("(7)eroded", eroded_Img);
		//imshow("(8)dilated", dilated_Img);
	}
	if (SHOW_PIXEL_OPERATOR_IMG_Level == 1)
	{
		imshow("(6)binary_color_Img", binary_color_Img);
		imshow("(6)binary_brightness_img", binary_brightness_img);
		imshow("(6)binary_light_img", binary_light_img);
	}
#endif //显示像素操作中间图像	
	
	return binary_light_img;
}
/*void callback(int value, void* detector)
{
	ArmorDetector AD_temp = *(ArmorDetector*)detector;//等等，这是个问题，这里的“等于号”没有被重载过，也就是说新建的AD_temp仍然指向原来的对象的地址？！?
	AD_temp.GammaCorrected_value = value;
	AD_temp.MyGammaCorrection(AD_temp.origin_Img, AD_temp.GammaCorrected_Img, AD_temp.GammaCorrected_value/10.0);//【调参】伽马矫正值
}*/

//-----------------------------------------2.数据操作函数------------------------------------

RotatedRect adjustRect(RotatedRect rect)//调整灯条的RotatedRect 让一个RotatedRect必定是长的为size.height，短的为size.width
{
	if (rect.size.width > rect.size.height)//让长边必定是高，短边必定是宽
	{
		auto temp = rect.size.height;
		rect.size.height = rect.size.width;
		rect.size.width = temp;
		rect.angle += 90;
	}
	//把角度归一化到 -180到+180 之间（由于进行了rect.angle += 90，可能会超出0~360度的范围，索性直接弄成-180~180）
	int angle_diff = (int)rect.angle / 180;//c++整数除法会自动省去余数，比如 4 /3 = 1，这个性质可以用于归一化，先算出angle中最多有多少个“180”度（设为n个），然后把angle - n * 180
	rect.angle -= angle_diff * 180;
	//c++整数除法会自动省去余数，比如 4 /3 = 1，这个性质可以用于归一化，先算出angle中最多有多少个“180”度（设为n个），然后把angle - n * 180
	if (rect.angle < 0)
	{
		rect.angle += 180;
	}
	/*//把坐标系换为以0度为轴，逆时针的角度坐标系（这个转化是错误的，删除）
	if (rect.angle > 90) {
		rect.angle = 270 - rect.angle;
	}
	else if (rect.angle <= 90) {
		rect.angle = 90 - rect.angle;
	}
	auto temp = rect.size.height;
	rect.size.height = rect.size.width;
	rect.size.width = temp;
	*/
	return rect;
}

void ArmorDetector::GetLightRRect()
{
	findContours(binary_light_img, light_contours, light_hierachy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));	//1��������������binary_light_img�Ļ����Ϸ�������
	light_choosed_contours.clear();																					//1��ɸѡ������ÿһ֡��Ҫ���light_choosed_contours �����ڶ������ⲿ�����ÿ�����?�������㣨findContours���Զ���light_contours���㣩��

	for (int i = 0; i < light_contours.size(); i++) {
		if ((10 <= light_contours[i].size() && light_contours[i].size() <= 300)/* && light_hierachy[i][2] == -1*/) {	//1��ɸѡ����������һ��������С������̫��Ҳ����̫С�����������㼶��ϵ���������������ڲ�������û�������������ڼ������ɸѡ���ͱ��뱣֤������ֵ������ʵ�ĵģ���������������ô���
			light_choosed_contours.push_back(light_contours[i]);
		}
	}

	RotatedRect rRect_temp;						//储存临时的拟合矩形

	AllrLightRect.clear();						//此变量定义在循环外部,记得清零
	for (int i = 0; i < light_choosed_contours.size(); i++) {
		rRect_temp = minAreaRect(light_choosed_contours[i]);														//2【由轮廓拟合成旋转矩形】minAreaRect函数 输入轮廓，返回旋转矩形
		
		rRect_temp = adjustRect(rRect_temp);	//让灯条的矩形一定是“高为长边，宽为短边”

		AllrLightRect.push_back(rRect_temp);	//将灯的矩形送入“轮廓拟合成的所有旋转矩形”的vector中
		
		//cout << rRect_temp.angle << " ";
	}//cout << endl;

#ifdef SHOW_DATA_OPERATOR_IMG_Level
	if (SHOW_DATA_OPERATOR_IMG_Level >= 2)
	{
		Mat light_contours_img = Mat::zeros(origin_Img.size(), CV_8UC3);
		Mat light_contours_choosed_img = Mat::zeros(origin_Img.size(), CV_8UC3);
		drawContours(light_contours_img, light_contours, -1, Scalar(255, 255, 0));
		drawContours(light_contours_choosed_img, light_choosed_contours, -1, Scalar(255, 255, 0));
		imshow("light_contours_img", light_contours_img);
		imshow("light_contours_choosed_img", light_contours_choosed_img);

		Mat All_Rect_img = Mat::zeros(origin_Img.size(), CV_8UC3);
		for (int i = 0; i < AllrLightRect.size(); i++) {
			DrawRotatedRect(All_Rect_img, AllrLightRect[i], Scalar(0, 255, 0), 2);
		}
		imshow("All_Rect_img", All_Rect_img);
	}
	if (SHOW_DATA_OPERATOR_IMG_Level == 1)
	{
		Mat All_Rect_img = Mat::zeros(origin_Img.size(), CV_8UC3);
		for (int i = 0; i < AllrLightRect.size(); i++) {
			DrawRotatedRect(All_Rect_img, AllrLightRect[i], Scalar(0, 255, 0), 2);
		}
		imshow("All_Rect_img", All_Rect_img);
	}
#endif // SHOW_DATA_OPERATOR_IMG_Level

	return;
}

int judgeBoundingRectSafe_640360(Rect brect) {//此函数用于检查灯条的boundingrect是否安全，如果安全返回0，不安全则直接返回“这可能是哪一种灯条（左 1，右 2，左和右都有可能 3）”
	if (brect.x < 0 || brect.x + brect.width < 0) {
		return 1;//左出界，视作左灯条
	}
	if (brect.x >= 640 || brect.x + brect.width >= 640) {
		return 2;//右出界，视作右灯条
	}
	if (brect.y < 0 || brect.y + brect.height < 0) {
		return 3;//上下越界，既可能视作左灯条，也可视作右灯条?
	}
	if (brect.y>=360 || brect.y + brect.height >= 360) {
		return 3;//上下越界，既可能视作左灯条，也可视作右灯条?
	}
	return 0;//安全，返回0
}

/*void ArmorDetector::getLightRois() {
	vector<RotatedRect> LightRois;
	vector<Mat> lights_left_and_small;

	for (int i = 0; i < AllrLightRect.size(); i++) {

		RotatedRect temp_rRect(AllrLightRect[i].center, AllrLightRect[i].size, AllrLightRect[i].angle);
		temp_rRect.size.width *= 5;

		LightRois.push_back(temp_rRect);
	}

	for (int i = 0; i < LightRois.size(); i++) {
		Rect brect = LightRois[i].boundingRect();

		int roi_judge = judgeBoundingRectSafe_640360(brect);
		if (roi_judge == 1) {
			LeftLight.push_back(AllrLightRect[i]);//左出界，视作左灯条
			continue;
		}
		else if (roi_judge == 2) {
			RightLight.push_back(AllrLightRect[i]);//右出界，视作右灯条
			continue;
		}
		else if (roi_judge == 3) {
			LeftLight.push_back(AllrLightRect[i]);//上下越界，既可能视作左灯条，也可视作右灯条?
			RightLight.push_back(AllrLightRect[i]);
			continue;
		}
		
		Mat roi_img1, roi_img2, roted_roi1;
		//（1）裁剪出rRect的外接Rect
		channels.at(1)(brect).copyTo(roi_img1);
		//（2）获取旋转矩形
		Mat rot_mat = getRotationMatrix2D(Point2f(brect.width / 2, brect.height / 2), LightRois[i].angle, 1.0);
		//（3）仿射变换旋转roi_img1
		warpAffine(roi_img1, roted_roi1, rot_mat, brect.size());
		//（4）裁剪旋转后的roi1
		cout << "width:" << brect.width << " " << LightRois[i].size.width << "  heigth:" << brect.height << " " << LightRois[i].size.height<<endl;
		//Rect roi2 = Rect(brect.width / 2 - LightRois[i].size.width / 2, brect.height / 2 - LightRois[i].size.height / 2, LightRois[i].size.width, LightRois[i].size.height);//roi2���ڽ���ת���ͼ���еġ��?��ת���Ρ����ֲü�������10-5�η��룬ע�⣬��Ҫ�����ĵ�����ƽ��
		//（5）得到最终的roi2
		//roted_roi1(roi2).copyTo(roi_img2);
		//（6）进行判断，判别是左灯条还是右灯条?
		//if()

		lights_left_and_small.push_back(roted_roi1);
	}
	if (lights_left_and_small.size() >= 2) {
		imshow("lights_left_and_small-0", lights_left_and_small[0]);
		imshow("lights_left_and_small-1", lights_left_and_small[1]);
	}
	

#ifdef SHOW_DATA_OPERATOR_IMG_Level
	if (SHOW_DATA_OPERATOR_IMG_Level >= 2)
	{
		Mat LightRois_img = Mat::zeros(origin_Img.size(), CV_8UC3);
		for (int i = 0; i < LightRois.size(); i++) {
			DrawRotatedRect(LightRois_img, LightRois[i], Scalar(255, 255, 0), 2);
		}
		imshow("LightRois_img", LightRois_img);
	}
#endif // SHOW_DATA_OPERATOR_IMG_Level
}*/

RotatedRect ArmorDetector::boundingRRect(RotatedRect left_rRect, RotatedRect right_rRect)//������������ϳ�װ�װ�?
{
	Point pl = left_rRect.center, pr = right_rRect.center;
	Point2f center;
	center.x = (pl.x + pr.x) / 2.0;
	center.y = (pl.y + pr.y) / 2.0;
	cv::Size2f wh_l = left_rRect.size;
	cv::Size2f wh_r = right_rRect.size;

	float width= sqrt((left_rRect.center.x - right_rRect.center.x) * (left_rRect.center.x - right_rRect.center.x)
		+ (left_rRect.center.y - right_rRect.center.y) * (left_rRect.center.y - right_rRect.center.y));
	float height = std::max(wh_l.height, wh_r.height);
	
	float angle = min(left_rRect.angle, right_rRect.angle);//(left_rRect.angle + right_rRect.angle) / 2;			//装甲板角度直接设为和灯条平行即可，省略乱七八糟的转化

	return RotatedRect(center, Size2f(width, height), angle);
}

void ArmorDetector::GetArmorFromLight()
{
	AllPossibleArmor.clear();//定义在外部，每帧清零
	AllPossibleArmorCompleteInfo.clear();//定义在外部，每帧清零

	pair<float, float> rRect1, rRect2, height_s, width_s;
	double light_aspect_ratio1,light_aspect_ratio2, angle_diff,  height_ratio,  width_ratio, armor_ratio, armor_angle, armor_area;
	for (int i = 0; i < AllrLightRect.size(); i++)
	{
		for (int j = i + 1; j < AllrLightRect.size(); j++)
		{
			 rRect1 = std::minmax(AllrLightRect[i].size.width, AllrLightRect[i].size.height);	//灯条1自身比例(使用minmax可以很方便地做大小排序)
			 light_aspect_ratio1 = rRect1.second / rRect1.first;
			 rRect2 = std::minmax(AllrLightRect[j].size.width, AllrLightRect[j].size.height);	//灯条2自身比例
			 light_aspect_ratio2 = rRect2.second / rRect2.first;
			 angle_diff = abs(AllrLightRect[i].angle - AllrLightRect[j].angle);					//两灯条角度差值

			 height_s = std::minmax(AllrLightRect[i].size.height, AllrLightRect[j].size.height);//两灯条的高度比例
			 height_ratio = height_s.first / height_s.second;									
			 width_s = std::minmax(AllrLightRect[i].size.width, AllrLightRect[j].size.width);	//两灯条的宽度比例
			 width_ratio = width_s.first / width_s.second;										
			
			//装甲板的法线方向指向摄像机的右侧或左侧
			if (1 < light_aspect_ratio1 && light_aspect_ratio1 <= 12
				&& 1 < light_aspect_ratio2 && light_aspect_ratio2 <= 12
				&&( angle_diff < 30 || 170 < angle_diff)&& height_ratio > 0.6 && width_ratio > 0.6
				&& (AllrLightRect[i].angle <= 100 && AllrLightRect[j].angle <= 100 || AllrLightRect[i].angle >= 80 && AllrLightRect[j].angle >= 80 //有可能“同侧”的条件不能定的那么死，可能会超过90度，定95度左右可能差不多
				|| (((170 <= AllrLightRect[i].angle&&AllrLightRect[i].angle <= 180) || (0 <= AllrLightRect[i].angle&&AllrLightRect[i].angle <= 10)) && ((170 <= AllrLightRect[j].angle&&AllrLightRect[j].angle <= 180) || (0 <= AllrLightRect[j].angle&&AllrLightRect[j].angle <= 10)))))//两个装甲板灯条都几乎垂直
			{
				cv::RotatedRect possible_rect;
				if (AllrLightRect[i].center.x < AllrLightRect[j].center.x) {
					possible_rect = boundingRRect(AllrLightRect[i], AllrLightRect[j]);
				}
				else{
					possible_rect = boundingRRect(AllrLightRect[j], AllrLightRect[i]);
				}
				 armor_ratio = possible_rect.size.width / possible_rect.size.height;
				 armor_angle = possible_rect.angle;
				 armor_area = possible_rect.size.area();

				if ( /*5 < armor_area && armor_area < 5000			//装甲面积
					&&*/ armor_ratio < 7							//装甲板长宽比 经验参数
					/*&& abs(armor_angle) < 20*/)					//装甲板倾斜角度 经验参数（等待之后做了大量测试再填写这个条件）
				{
					//存入“可能的装甲板”vector
					AllPossibleArmor.push_back(possible_rect);
					//存入“可能的装甲板的完全信息”vector
					if (AllrLightRect[i].center.x < AllrLightRect[j].center.x) {
						AllPossibleArmorCompleteInfo.push_back(Armor(AllrLightRect[i], AllrLightRect[j], possible_rect));
					}
					else {
						AllPossibleArmorCompleteInfo.push_back(Armor(AllrLightRect[j], AllrLightRect[i], possible_rect));
					}
				}

			}
			//if (AllPossibleArmor.size() == 0)waitKey(0);
		}
	}
#ifdef SHOW_DATA_OPERATOR_IMG_Level
	if (SHOW_DATA_OPERATOR_IMG_Level >= 2) {
	Mat AllPossibleArmor_img = origin_Img.clone();
	for (int i = 0; i < AllPossibleArmor.size(); i++) {
		DrawRotatedRect(AllPossibleArmor_img, AllPossibleArmor[i], Scalar(0, 255, 255), 2);
	}
	imshow("AllPossibleArmor_img", AllPossibleArmor_img);
	}
#endif // SHOW_DATA_OPERATOR_IMG_Level
}

int ArmorDetector::ChooseArmorToTrack(Rect2d &target)
{
	int Choosed = -1;//被选择的装甲板下标
	int img_center_X = (origin_Img.size().width) / 2;
	int img_center_Y = (origin_Img.size().height) / 2;
	float distance = 10000000, MIN_distance = 10000000;
	for (int i = 0; i < AllPossibleArmor.size(); i++)
	{
		distance = sqrt((AllPossibleArmor[i].center.x - img_center_X) * (AllPossibleArmor[i].center.x - img_center_X)//计算距离
			+ (AllPossibleArmor[i].center.y - img_center_Y) * (AllPossibleArmor[i].center.y - img_center_Y));
		if (MIN_distance > distance) {
			MIN_distance = distance;
			Choosed = i;
		}
	}
	if (Choosed == -1) {
		return -1;
	}
	else {
		target = Rect2d(Point2f(AllPossibleArmor[Choosed].center.x - 20, AllPossibleArmor[Choosed].center.y - 20),
			Size2f(40,40));
		return 1;
	}

}

int ArmorDetector::ChooseArmorToAttack(RotatedRect &target, float &xAngle, float &yAngle)//ѡ�����Ŀ��?�ҽ���pnp���鲽�裿
{
	int Choosed = -1;//被选择的装甲板下标
	int img_center_X = (origin_Img.size().width) / 2;
	int img_center_Y = (origin_Img.size().height) / 2;
	float distance = 10000000, MIN_distance = 10000000;
	for (int i = 0; i < AllPossibleArmorCompleteInfo.size(); i++)
	{
		distance = sqrt((AllPossibleArmorCompleteInfo[i].ArmorRRect.center.x - img_center_X) * (AllPossibleArmorCompleteInfo[i].ArmorRRect.center.x - img_center_X)//�������?
			+ (AllPossibleArmorCompleteInfo[i].ArmorRRect.center.y - img_center_Y) * (AllPossibleArmorCompleteInfo[i].ArmorRRect.center.y - img_center_Y));
		if (MIN_distance > distance) {
			MIN_distance = distance;
			Choosed = i;
		}
	}
	if (Choosed == -1) {//如果不存在装甲板，直接返回-1不进行后续操作
		return -1;
	}
	//弄出四个标定点用于pnp解算
	//左灯条四个标定点
	float height_LeftLight = AllPossibleArmorCompleteInfo[Choosed].LeftLight.size.height;
	float angle_LeftLight = AllPossibleArmorCompleteInfo[Choosed].LeftLight.angle;
	Point2f center_LeftLight = AllPossibleArmorCompleteInfo[Choosed].LeftLight.center;
	float diff_X_L = (height_LeftLight / 2) * sin(angle_LeftLight * 3.1415 / 180);
	float diff_Y_L = (height_LeftLight / 2) * cos(angle_LeftLight * 3.1415 / 180);
	//右灯条四个标定点（从我自己造的数据结构里面提取东西真的挺烦的）
	float height_RightLight = AllPossibleArmorCompleteInfo[Choosed].RightLight.size.height;
	float angle_RightLight = AllPossibleArmorCompleteInfo[Choosed].RightLight.angle;
	Point2f center_RightLight = AllPossibleArmorCompleteInfo[Choosed].RightLight.center;
	float diff_X_R = (height_RightLight / 2) * sin(angle_RightLight * 3.1415 / 180);
	float diff_Y_R = (height_RightLight / 2) * cos(angle_RightLight * 3.1415 / 180);

	cout << "angle_LeftLight: " << angle_LeftLight << endl;
	//由于我自己坑爹的直角坐标系建系（这个angle我是以竖直轴为0或者180度的），导致这个坐标是挺难运算的，推导过程我拍下来放在文档里面
	Point2f point1_LeftLight(center_LeftLight.x - diff_X_L, center_LeftLight.y + diff_Y_L);
	Point2f point2_LeftLight(center_LeftLight.x + diff_X_L, center_LeftLight.y - diff_Y_L);
	Point2f point1_RightLight(center_RightLight.x - diff_X_R, center_RightLight.y + diff_Y_R);
	Point2f point2_RightLight(center_RightLight.x + diff_X_R, center_RightLight.y - diff_Y_R);

	//---------------------------pnp算法调用（草稿）----------------------------------------
	double cm[3][3] = { 1.0632012475881963e+03, 0, 320, 0, 1.0632012475881963e+03, 180, 0, 0,
    1 };//相机矩阵
	vector<Point3f> objectPoints;
	vector<Point2f> imagePoints;
	vector<Point2f> distCoeffs;
	Mat cameraMatrix(3, 3, CV_64F);//32位浮点数用来存相机内参矩阵看看ok不

	for (int i = 0; i < cameraMatrix.rows; i++) {
		for (int j = 0; j < cameraMatrix.cols; j++) {
			cameraMatrix.at<double>(i, j) = cm[i][j];
		}
		cout << endl;
	}
	Mat rvec1, tvec1;
	//将装甲板的四个 世界坐标系 下的点输入objectPoints,世界坐标系单位为厘米
	objectPoints.push_back(Point3f(-11, 0, 2.6));objectPoints.push_back(Point3f(-11, 0, -2.6));objectPoints.push_back(Point3f(11, 0, 2.6));objectPoints.push_back(Point3f(11, 0, 2.6));
	//将装甲板的四个 相机坐标系 下的点输入imagePoints
	//需要保证 objectPoints 的点的顺序与 imagePoints 的顺序一模一样，所以让第一个输入的必定是“左上”的点，第二个必定“左下”，第三个必定“右上”，第四个必定“右下”
	if (point1_LeftLight.y > point2_LeftLight.y) {
		imagePoints.push_back(point1_LeftLight);
		imagePoints.push_back(point2_LeftLight);
	}
	else {
		imagePoints.push_back(point2_LeftLight);
		imagePoints.push_back(point1_LeftLight);
	}
	if (point1_RightLight.y > point1_RightLight.y) {
		imagePoints.push_back(point1_RightLight);
		imagePoints.push_back(point2_RightLight);
	}
	else {
		imagePoints.push_back(point2_RightLight);
		imagePoints.push_back(point1_RightLight);
	}
	solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec1, tvec1);//使用SOLVEPNP_P3P会报异常？
	double rm[3][3];
	Mat rotMat(3, 3, CV_64FC1, rm);//使得rm与rotMat矩阵共享数据，但是这个该死的偷懒方法让我的程序报错了！！
	Rodrigues(rvec1, rotMat);
	
	float theta_z = atan2(rm[1][0], rm[0][0])*57.2958;
	float theta_y = atan2(-rm[2][0], sqrt(rm[2][1] * rm[2][1] + rm[2][2] * rm[2][2]))*57.2958;
	float theta_x = atan2(rm[2][1], rm[2][2])*57.2958;

	//cout << "theta_x: " << theta_x << " theta_y: " << theta_y << " theta_z: " << theta_z << endl;
	
	vector<Point3f> reference_ObjectPoint;
	vector<Point2f> reference_ImagePoint;

	//设置自定义3D空间坐标
	reference_ObjectPoint.push_back(Point3f(0.0, 0.0, 0.0));
	reference_ObjectPoint.push_back(Point3f(1.5, 0.0, 0.0));
	reference_ObjectPoint.push_back(Point3f(0.0, 1.5, 0.0));
	reference_ObjectPoint.push_back(Point3f(0.0, 0.0, 1.5));
	
	projectPoints(reference_ObjectPoint, rvec1, tvec1, cameraMatrix, distCoeffs, reference_ImagePoint);


	//------------------------------------------------------------------------------

	//-------------------不使用pnp计算出转角----------------------------------------
	float fy = 1.0632012475881963e+03;//相机的y轴焦距(y轴定义为竖直轴,单位为“像素”)
	float fx = 1.0632012475881963e+03;//相机的x轴焦距(x轴定义为水平轴,单位为“像素”（其实就是相机标定那个工程里面直接取出的相机矩阵的fx）)
	//Point2f target_center((imagePoints[0].x + imagePoints[3].x) / 2, (imagePoints[0].y + imagePoints[3].y) / 2);
	Point2f target_center(AllPossibleArmorCompleteInfo[Choosed].ArmorRRect.center.x, AllPossibleArmorCompleteInfo[Choosed].ArmorRRect.center.y);
	//X0和Y0为 S系原点 在S'系中的坐标
	int X0 = -320;
	int Y0 = 180;
	int x_in_camera_coordinate_system = target_center.x + X0;
	int y_in_camera_coordinate_system = Y0 - target_center.y;
	//计算x轴角度
	xAngle = atan(x_in_camera_coordinate_system / fx) * 180.0 / 3.1416;
	//计算y轴角度
	yAngle = atan(y_in_camera_coordinate_system / fy) * 180.0 / 3.1416;

	cout << "xAngle:" << xAngle << " yAngle:" << yAngle << endl;
	//------------------------------------------------------------------------------
	
	#ifdef SHOW_DATA_OPERATOR_IMG_Level
	if (SHOW_DATA_OPERATOR_IMG_Level >= 2) {
		//可视化，记得删除
		Mat PointsPNP = origin_Img.clone();
		circle(PointsPNP, point1_LeftLight, 4, Scalar(0, 255, 0), -1);
		circle(PointsPNP, point2_LeftLight, 4, Scalar(0, 255, 0), -1);
		circle(PointsPNP, point1_RightLight, 4, Scalar(255, 0, 255), -1);
		circle(PointsPNP, point2_RightLight, 4, Scalar(255, 0, 255), -1);
		//画出坐标轴
		line(PointsPNP, reference_ImagePoint[0], reference_ImagePoint[1], Scalar(0, 0, 255), 2);
		line(PointsPNP, reference_ImagePoint[0], reference_ImagePoint[2], Scalar(0, 255, 255), 2);
		line(PointsPNP, reference_ImagePoint[0], reference_ImagePoint[3], Scalar(255, 0, 255), 2);
		//画出中心点
		circle(PointsPNP, target_center, 4, Scalar(255, 255, 255), -1);
		imshow("PointsPNP", PointsPNP);
	}
#endif
	//返回击打的目标（这是用在老的电控算法上的，新算法直接位姿解算）
	target = AllPossibleArmorCompleteInfo[Choosed].ArmorRRect;
	return 1;
}
