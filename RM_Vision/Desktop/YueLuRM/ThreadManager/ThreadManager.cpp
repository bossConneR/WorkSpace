#include"ThreadManager.h"
#include<iostream>
#include<exception>


namespace hnurm
{

void ThreadManager::InitAll()
{

}


void ThreadManager::GenerateThread()
{

}


void ThreadManager::ProcessThread()
{

}


void ThreadManager::ReceiveThread()
{
	Protocol::Vision_recv_data temp_data(Protocol::Self_color::none, 0, Protocol::Work_mode::manual, Protocol::Tracker_mode::off); 
	int dur = 10;   //接收的时限
	for (;;)
	{
		if (_serial_ptr->try_get_recv_data_for(temp_data, dur))
		{
			Wrapped<Protocol::Work_mode> _work_mode = temp_data.mode;
			taskmode_buff_ptr->update(_work_mode);
		}
		else
		{
			std::cout << "fail to update the taskmode buffer" << std::endl;
		}
	}
}





}