#include "MagCamera.h"
#include <ctime>
#include <fstream>
using namespace std;

void CALLBACK NewFrame(UINT intChannelIndex, int intCameraTemperature, DWORD dwFFCCounterdown,
	DWORD dwCamState, DWORD dwStreamType, void* pUserData)
{
	CMagDevice* pDevice = (CMagDevice*)pUserData;

	if (dwStreamType == STREAM_TEMPERATURE)//temperature
	{
		const UCHAR* pIrData;
		const BITMAPINFO* pIrInfo;
		if (pDevice->GetOutputBMPdata(&pIrData, &pIrInfo))
		{
			//pIrData中保存的是BMP图像的实际数据
		}
	}
}

MagCamera::MagCamera()
{
	device = new CMagDevice;
	service = new CMagService;
}

MagCamera::~MagCamera()
{
	device->StopProcessImage();
	device->DisLinkCamera();
	std::cout << "DisLink Camera!" << std::endl;
	delete device;
	delete service;
}

bool MagCamera::InitialMagCamera(const char *strIP)
{
	service->EnableAutoReConnect(TRUE);//开启短线重连
	if (!device->IsInitialized())
	{
		printf("initial thermalcamera fail\r\n");
		return FALSE;
	}
	
	if (!device->LinkCamera(strIP))
	{
		printf("connect thermalcamera fail\r\n");
		return FALSE;
	}
	const struct_CamInfo* pCamInfo = device->GetCamInfo();
	if (!pCamInfo)
	{
		printf("get thermalcamera's parameter faile\r\n");
		return FALSE;
	}

	OutputPara paraOut;
	paraOut.dwFPAWidth = pCamInfo->intFPAWidth;
	paraOut.dwFPAHeight = pCamInfo->intFPAHeight;
	paraOut.dwBMPWidth = pCamInfo->intVideoWidth;
	paraOut.dwBMPHeight = pCamInfo->intVideoHeight;
	paraOut.dwColorBarWidth = 16;
	paraOut.dwColorBarHeight = pCamInfo->intVideoHeight;

	if (!device->StartProcessImage(&paraOut, NewFrame, STREAM_TEMPERATURE, &device))
	{
		printf("translate data faile\r\n");
		device->DisLinkCamera();
		return FALSE;
	}

	Sleep(100);
	return TRUE;
}

void MagCamera::SaveThermalBMP(const char* saveThermalBMPPath)
{
	WCHAR* saveThermalBMPPath_s;
	swprintf(saveThermalBMPPath_s, 100, L"%hs", saveThermalBMPPath);
	device->Lock();
	if (device->SaveBMP(0, saveThermalBMPPath_s))
	{
		//printf("保存BMP成功！\r\n");
	}
	else
	{
		//printf("保存BMP失败！\r\n");
	}
	device->Unlock();
}

void MagCamera::StorageThermalAvi(const char* saveThermalAviPath, UINT time, UINT intSamplePeriod)
{
	WCHAR* saveThermalAvoPath_s;
	swprintf(saveThermalAvoPath_s, 100, L"%hs", saveThermalAviPath);
	device->LocalStorageAviStart(saveThermalAvoPath_s, intSamplePeriod);
	Sleep(time);
	device->LocalStorageAviStart(saveThermalAvoPath_s, intSamplePeriod);
	device->LocalStorageAviStop();
}

void MagCamera::StorageThermalMgs(const char* saveThermalMgsPath, UINT time, UINT intSamplePeriod)
{
	WCHAR* saveThermalMgsPath_s;
	swprintf(saveThermalMgsPath_s, 100, L"%hs", saveThermalMgsPath);
	device->LocalStorageMgsRecord(saveThermalMgsPath_s, intSamplePeriod);
	Sleep(time);
	device->LocalStorageMgsStop();
}


void MagCamera::StorageAviStart(const char* saveThermalAviPath, UINT intSamplePeriod)
{
	WCHAR* saveThermalAviPath_s;
	swprintf(saveThermalAviPath_s, 100, L"%hs", saveThermalAviPath);
	device->LocalStorageAviStart(saveThermalAviPath_s, intSamplePeriod);
}
//void MagCamera::StorageAviStart(const char* saveThermalAviPath, UINT intSamplePeriod)
//{
//	WCHAR* saveThermalAviPath_s;
//	swprintf(saveThermalAviPath_s, 100, L"%hs", saveThermalAviPath);
//	device->LocalStorageAviStart(saveThermalAviPath_s, intSamplePeriod);
//}
void MagCamera::StorageAviStop()
{
	device->LocalStorageAviStop();

}

//void MagCamera::StorageMgsStart(char* saveThermalMgsPath, UINT intSamplePeriod)
//{
//	WCHAR* saveThermalMgsPath_s;
//	swprintf(saveThermalMgsPath_s, 100, L"%hs", saveThermalMgsPath);
//	device->LocalStorageMgsRecord(saveThermalMgsPath_s, intSamplePeriod);
//}
void MagCamera::StorageMgsStart(const char* saveThermalMgsPath, UINT intSamplePeriod)
{
	WCHAR* saveThermalMgsPath_s;
	swprintf(saveThermalMgsPath_s, 100, L"%hs", saveThermalMgsPath);
	device->LocalStorageMgsRecord(saveThermalMgsPath_s, intSamplePeriod);
}

void MagCamera::StorageMgsStop()
{
	device->LocalStorageMgsStop();
}


void MagCamera::StorageMgsPlay(const WCHAR * charFilename, MAG_FRAMECALLBACK funcFrame, void * pUserData)
{
	device->LocalStorageMgsPlay(charFilename, funcFrame, pUserData);
	
}

void MagCamera::SaveTemperatureData(const char* saveTemperatureDataPath)
{
	const struct_CamInfo* info;
	info = device->GetCamInfo();
	const UINT dataSize = info->intFPAWidth * info->intFPAHeight;

	/*测试采集单帧数据所需要的时间*/
	//clock_t startTime, endTime;
	// startTime = clock();//计时开始
	device->Lock();
	int* TemperatureData;
	TemperatureData = new int[dataSize];
	device->GetTemperatureData_Raw(TemperatureData, dataSize * sizeof(int), 1);
	device->Unlock();
	//endTime = clock();
	//cout << "time1:" << endTime - startTime << endl;
	FILE* stream;
	stream = fopen(saveTemperatureDataPath, "w+");
	fwrite(TemperatureData, sizeof(int), dataSize, stream);
	fclose(stream);
	delete[] TemperatureData;
	//endTime = clock();
	//cout << "time2:" << endTime - startTime << endl;
}
