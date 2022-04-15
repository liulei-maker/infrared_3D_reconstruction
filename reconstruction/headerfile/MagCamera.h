#ifndef __MAGCAMERA_H__
#define __MAGCAMERA_H__

#include "MagDevice.h"
#include "MagService.h"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")	//以太网使用的库

class MagCamera
{
public:
	MagCamera();
	~MagCamera();
	bool InitialMagCamera(const char *strIP);

	void SaveThermalBMP(const char* saveThermalBMPPath);
	void StorageThermalAvi(const char* saveThermalAviPath, UINT time, UINT intSamplePeriod = 1);
	void StorageThermalMgs(const char* saveThermalMgsPath, UINT time, UINT intSamplePeriod = 1);

	void StorageAviStart(const char* saveThermalAviPath, UINT intSamplePeriod = 1);
	//void StorageAviStart(const char* saveThermalAviPath, UINT intSamplePeriod);
	void StorageAviStop();
	void StorageMgsStart(const char* saveThermalMgsPath, UINT intSamplePeriod = 1);
	//void StorageMgsStart(const char* saveThermalMgsPath, UINT intSamplePeriod = 1);
	void StorageMgsStop();

	void StorageMgsPlay(const WCHAR * charFilename, MAG_FRAMECALLBACK funcFrame, void * pUserData);
	void SaveTemperatureData(const char *saveTemperatureDataPath);
private:
	CMagDevice  *device;
	CMagService *service;
};

void CALLBACK NewFrame(UINT, int, DWORD, DWORD, DWORD, void *);

#endif // !__MAGCAMERA_H__


