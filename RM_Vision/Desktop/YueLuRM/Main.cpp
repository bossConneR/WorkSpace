#pragma once
#include"ThreadManager.h"
#include<thread>
#include<iostream>

int main()
{
	hnurm::ThreadManager _thread_manager;

	_thread_manager.InitAll();

	std::thread generate(&hnurm::ThreadManager::GenerateThread,std::ref(_thread_manager));
	std::thread process(&hnurm::ThreadManager::ProcessThread, std::ref(_thread_manager));
	std::thread receive(&hnurm::ThreadManager::ReceiveThread, std::ref(_thread_manager));

	generate.join();
	process.join();
	receive.join();

	return 0;
}
