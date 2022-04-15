#pragma once
#include "SerialPort.h"
#include <thread>
#include <string>
#include <mutex>

string sendMessageBuff;

class SerialListener
{
public:
	SerialListener();
	~SerialListener();
	void Run();
private:
	SerialPort* robotSerial;		//机械臂控制器串口创建实例化对象
	mutex _mutex;
};

