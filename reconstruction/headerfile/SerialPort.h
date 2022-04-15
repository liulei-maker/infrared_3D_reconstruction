#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#include <windows.h>
#include <iostream>
#include <string>
#include <string>

using namespace std;

//string readMessageBuff;
//bool readComplication = false;
class SerialPort
{
public:
	SerialPort(void);
	~SerialPort(void);
	//打开串口
	
	bool SerialOpen(LPCSTR COMx, int BaudRate);
	//发送数据
	int SerialWriteString(string& Buf, int size);
	int SerialWriteChar(unsigned char* Buf, int size);
	//接收数据
	int SerialReadString(string &OutBuf,int maxSize);
	int SerialReadChar(unsigned char* OutBuf,int maxSize);
	//监听数据输入
	void ListenSerial();
private:
	HANDLE m_hComm;//串口句柄
};
#endif
