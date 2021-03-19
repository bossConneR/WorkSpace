#pragma once

#include"SerialCodec/serialcodec.hpp"
#include"DataBuf.hpp"
#include"SerialData.hpp"
#include"ImageData.hpp"
#include"TaskSwitcher.hpp"
#include"VideoCapture.hpp"
#include<iostream>
#include<memory>

namespace hnurm
{

class ThreadManager
{
public:
	ThreadManager();

	~ThreadManager();

	void InitAll();

	void GenerateThread();

	void ProcessThread();

	void ReceiveThread();

private:
	//∫‹∂‡÷∏’Î
	/*DataBuf<Wrapped<SerialData>> serial_data_buff;
	DataBuf<Wrapped<ImageData>> image_data_buff;
	DataBuf<Wrapped<int>> taskmode_buff;*/
	std::unique_ptr < DataBuf<Wrapped<SerialData>>> serial_databuf_ptr;

	std::unique_ptr < DataBuf<Wrapped<ImageData>>> image_databuf_ptr;

	std::unique_ptr	< DataBuf<Wrapped<Protocol::Work_mode>>> taskmode_buff_ptr;

	std::unique_ptr<SerialCodec> _serial_ptr;

	std::unique_ptr<TaskSwitcher> task_switcher_ptr;

	std::unique_ptr<VideoCapture> video_capture_ptr;

};

}
