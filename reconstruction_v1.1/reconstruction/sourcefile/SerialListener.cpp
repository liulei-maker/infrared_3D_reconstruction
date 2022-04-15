#include "SerialListener.h"

SerialListener::SerialListener()
{

}

SerialListener::~SerialListener()
{
}

void SerialListener::Run()
{
	this->robotSerial = new SerialPort;		//机械臂控制器串口创建实例化对象
	if (!this->robotSerial->SerialOpen("COM3", 115200))
		return;
	while (true)
	{
		_mutex.lock();
		if (sendMessageBuff.size())
		{
			robotSerial->SerialWriteString(sendMessageBuff, sendMessageBuff.size());
			sendMessageBuff.clear();
		}
		_mutex.unlock();
		robotSerial->SerialReadString()
	}
}