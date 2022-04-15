#include "MyMagCamera.h"


MyMagCamera::MyMagCamera()
{
	device = new CMagDevice;
	service = new CMagService;
}

MyMagCamera::~MyMagCamera()
{
	device->StopProcessImage();
	device->DisLinkCamera();
	delete device;
	delete service;
	std::cout << "DisLink Camera!" << std::endl;
}

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

bool MyMagCamera::InitialCamera(char* strIP)
{
	service->EnableAutoReConnect(TRUE);//开启短线重连
	if (!device->IsInitialized())
	{
		printf("相机初始化失败！\r\n");
		return FALSE;
	}

	if (!device->LinkCamera(strIP))
	{
		printf("连接相机失败！\r\n");
		return FALSE;
	}
	const struct_CamInfo* pCamInfo = device->GetCamInfo();
	if (!pCamInfo)
	{
		printf("获取相机参数失败！\r\n");
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
		printf("传输数据失败！\r\n");
		device->DisLinkCamera();
		return FALSE;
	}

	Sleep(100);
	return TRUE;
}

void MyMagCamera::SaveThermalBMP(char* saveThermalBMPPath)
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

void MyMagCamera::StorageThermalAvi(const WCHAR* charFilename = NULL, UINT time = 1000, UINT intSamplePeriod = 1)
{
	device->LocalStorageAviStart(charFilename, intSamplePeriod);
	Sleep(time);
	device->LocalStorageAviStart(charFilename, intSamplePeriod);
	device->LocalStorageAviStop();
}

void MyMagCamera::saveTemperatureData(char* saveTemperatureDataPath)
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
void MyMagCamera::StorageThermalMgs(const WCHAR* charFilename = NULL, UINT time = 1000, UINT intSamplePeriod = 1)
{
	device->LocalStorageMgsRecord(charFilename, intSamplePeriod);
	Sleep(time);
	device->LocalStorageMgsStop();

}
void MyMagCamera::StorageMgsStart(const WCHAR* charFilename = NULL, UINT intSamplePeriod = 1)
{
	device->LocalStorageMgsRecord(charFilename, intSamplePeriod);
}
void MyMagCamera::StorageMgsStop()
{
	device->LocalStorageMgsStop();
}
void MyMagCamera::StorageMgsPlay(const WCHAR* charFilename, MAG_FRAMECALLBACK funcFrame, void* pUserData)
{
	device->LocalStorageMgsPlay(charFilename, funcFrame, pUserData);

}