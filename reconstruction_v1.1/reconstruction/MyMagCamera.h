#pragma once
#include <ctime>
#include <fstream>
#include "MagDevice.h"
#include "MagService.h"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>


class MyMagCamera
{
public:
	MyMagCamera();
	~MyMagCamera();
	bool InitialCamera(char* strIP);
	void SaveThermalBMP(char* saveThermalBMPPath);
	void StorageThermalAvi(const WCHAR* charFilename, UINT time, UINT intSamplePeriod);
	void StorageThermalMgs(const WCHAR* charFilename, UINT time, UINT intSamplePeriod);
	void StorageMgsStart(const WCHAR* charFilename, UINT intSamplePeriod);
	void StorageMgsStop();
	void StorageMgsPlay(const WCHAR* charFilename, MAG_FRAMECALLBACK funcFrame, void* pUserData);
	void saveTemperatureData(char* saveTemperatureDataPath);

private:
	CMagDevice* device;
	CMagService* service;
};

void CALLBACK NewFrame(UINT, int, DWORD, DWORD, DWORD, void*);

