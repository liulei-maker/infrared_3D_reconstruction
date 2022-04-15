#include "MagDevice.h"

#ifdef _WIN64
	#pragma comment(lib, "ThermoGroupSDKLib_x64.lib") 
#else
	#pragma comment(lib, "ThermoGroupSDKLib.lib")
#endif

#define MAX_CHANNELINDEX	(128)

CMagDevice::CMagDevice(HWND hWndMsg) : 
m_hWndMsg(hWndMsg)
,m_bInitialized(FALSE)
,m_intChannelIndex(-1)
,m_bIsRecordingAvi(FALSE)
,m_bIsRecordingMGS(FALSE)
,m_bIsRecordingLocalAvi(FALSE)
,m_bIsRecordingLocalMgs(FALSE)
,m_bIsPlayingLocalMgs(FALSE)
,m_intCamIPAddr(0)
{
	memset(&m_CamInfo, 0, sizeof(m_CamInfo));
	memset(&m_RegContent, 0, sizeof(m_RegContent));

	Initialize();
}

CMagDevice::~CMagDevice()
{
	if (MAG_IsProcessingImage(m_intChannelIndex))
	{
		MAG_StopProcessImage(m_intChannelIndex);
	}

	if (MAG_IsLinked(m_intChannelIndex))
	{
		DisLinkCamera();//include stop sd storage
	}

	if (MAG_IsInitialized(m_intChannelIndex))
	{
		MAG_Free(m_intChannelIndex);
		m_bInitialized = FALSE;
	}

	if (MAG_IsChannelAvailable(m_intChannelIndex))
	{
		MAG_DelChannel(m_intChannelIndex);
		m_intChannelIndex = -1;
	}
}

BOOL CMagDevice::Initialize()
{
	if (m_bInitialized)
	{
		return TRUE;
	}

	if (m_intChannelIndex<=0 || m_intChannelIndex>MAX_CHANNELINDEX)
	{
		for (int i=1; i<=MAX_CHANNELINDEX; i++)
		{
			if (!MAG_IsChannelAvailable(i))//find an unused channel
			{
				BOOL bSuccess = MAG_NewChannel(i);
				//ASSERT(bSuccess);

				m_intChannelIndex = i;

				break;
			}
		}
	}

	if (m_intChannelIndex>0 && m_intChannelIndex<=MAX_CHANNELINDEX)
	{
		m_bInitialized = MAG_Initialize(m_intChannelIndex, m_hWndMsg);
	}

	return m_bInitialized;
}

BOOL CMagDevice::IsLinked()
{
	return MAG_IsLinked(m_intChannelIndex);
}

BOOL CMagDevice::LinkCamera(const char * charIp, UINT intTimeoutMS)
{
	return LinkCamera(inet_addr(charIp), intTimeoutMS);
}

BOOL CMagDevice::LinkCameraEx(const char * charIp, USHORT shortCmdPort, USHORT shortImgPort, const char * charCloudUser, const char * charCloudPwd, UINT intCamSN, const char * charCamUser, const char * charCamPwd, UINT intTimeoutMS)
{
	return LinkCameraEx(inet_addr(charIp), shortCmdPort, shortImgPort, charCloudUser, charCloudPwd, intCamSN, charCamUser, charCamPwd, intTimeoutMS);
}

BOOL CMagDevice::LinkCamera(UINT intIP, UINT intTimeoutMS)
{
	if (MAG_LinkCamera(m_intChannelIndex, intIP, intTimeoutMS))
	{
		m_intCamIPAddr = intIP;
		MAG_GetCamInfo(m_intChannelIndex, &m_CamInfo, sizeof(m_CamInfo));
		MAG_ReadCameraRegContent(m_intChannelIndex, &m_RegContent, MAG_DEFAULT_TIMEOUT, FALSE);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CMagDevice::LinkCameraEx(UINT intIP, USHORT shortCmdPort, USHORT shortImgPort, const char * charCloudUser, const char * charCloudPwd, UINT intCamSN, const char * charCamUser, const char * charCamPwd, UINT intTimeoutMS)
{
#ifdef MAG_Linux_Not_Implemented

	return LinkCamera(intIP, intTimeoutMS);

#else

	if (MAG_LinkCameraEx(m_intChannelIndex, intIP, shortCmdPort, shortImgPort,
		charCloudUser, charCloudPwd, intCamSN, charCamUser, charCamPwd, intTimeoutMS))
	{
		m_intCamIPAddr = intIP;
		MAG_GetCamInfo(m_intChannelIndex, &m_CamInfo, sizeof(m_CamInfo));
		MAG_ReadCameraRegContent(m_intChannelIndex, &m_RegContent, MAG_DEFAULT_TIMEOUT, FALSE);

		return TRUE;
	}
	else
	{
		return FALSE;
	}

#endif
}

void CMagDevice::DisLinkCamera()
{
	//remember to stop sd storage before dislink
	if (m_bIsRecordingMGS)
	{
		SDCardStorage(SDFileMGS, 0);
	}

	if (m_bIsRecordingAvi)
	{
		SDCardStorage(SDFileAVI, 0);
	}

	m_intCamIPAddr = 0;

	MAG_DisLinkCamera(m_intChannelIndex);
}

const struct_CamInfoEx * CMagDevice::GetCamInfoEx()
{
#ifdef MAG_Linux_Not_Implemented

	return NULL;

#else

	MAG_GetCamInfoEx(m_intChannelIndex, &m_CamInfoEx, sizeof(m_CamInfoEx));

	return &m_CamInfoEx;

#endif
}

UINT CMagDevice::GetRecentHeartBeat()
{
	return MAG_GetRecentHeartBeat(m_intChannelIndex);
}

BOOL CMagDevice::SetReConnectCallBack(MAG_RECONNECTCALLBACK pCallBack, void * pUserData)
{
	return MAG_SetReConnectCallBack(m_intChannelIndex, pCallBack, pUserData);
}

BOOL CMagDevice::ResetCamera()
{
	//the user should stop image process before reset
	//if you forget, the sdk will call MAG_StopProcessImage()

	//remember to stop sd storage before reset
	if (m_bIsRecordingMGS)
	{
		SDCardStorage(SDFileMGS, 0);
	}

	if (m_bIsRecordingAvi)
	{
		SDCardStorage(SDFileAVI, 0);
	}

	if (MAG_ResetCamera(m_intChannelIndex))
	{
		//MAG_ResetCamera() will call MAG_Free() and MAG_DelChannel()
		//so the channel is invalid now
		m_bInitialized = FALSE;
		m_intChannelIndex = -1;

		//this object is reusable after call Initialize()

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CMagDevice::TriggerFFC()
{
	return MAG_TriggerFFC(m_intChannelIndex);
}

BOOL CMagDevice::AutoFocus()
{
	return MAG_SetPTZCmd(m_intChannelIndex, PTZFocusAuto, 0);
}

BOOL CMagDevice::SetIoAlarmState(BOOL bAlarm)
{
	return MAG_SetIoAlarmState(m_intChannelIndex, bAlarm);
}

BOOL CMagDevice::SetPTZCmd(enum PTZCmd cmd, DWORD dwPara)
{
	return MAG_SetPTZCmd(m_intChannelIndex, cmd, dwPara);
}

BOOL CMagDevice::QueryPTZState(enum PTZQuery query, int * intValue, UINT intTimeoutMS)
{
	return MAG_QueryPTZState(m_intChannelIndex, query, intValue, intTimeoutMS);
}

BOOL CMagDevice::SetSerialCmd(const BYTE * buffer, UINT intBufferLen)
{
	return MAG_SetSerialCmd(m_intChannelIndex, buffer, intBufferLen);
}

BOOL CMagDevice::SetSerialCallBack(MAG_SERIALCALLBACK pCallBack, void * pUserData)
{
	return MAG_SetSerialCallBack(m_intChannelIndex, pCallBack, pUserData);
}

BOOL CMagDevice::GetCameraTemperature(int intT[4], UINT intTimeoutMS)
{
	return MAG_GetCameraTemperature(m_intChannelIndex, intT, intTimeoutMS);
}

BOOL CMagDevice::SetCameraRegContent(const struct_CeRegContent * pContent)
{
	if (MAG_SetCameraRegContent(m_intChannelIndex, pContent))
	{
		MAG_ReadCameraRegContent(m_intChannelIndex, &m_RegContent, 1000 + MAG_DEFAULT_TIMEOUT, FALSE);//we need more time here
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CMagDevice::ReadCameraRegContent2(struct_CfgPara * pContent, UINT intTimeoutM)
{
	return MAG_ReadCameraRegContent2(m_intChannelIndex, pContent, intTimeoutM);
}

BOOL CMagDevice::SetCameraRegContent2(const struct_CfgPara * pContent)
{
	return MAG_SetCameraRegContent2(m_intChannelIndex, pContent);
}

BOOL CMagDevice::SetUserROIs(const struct_UserROIs * pROI)
{
	return MAG_SetUserROIs(m_intChannelIndex, pROI);
}

BOOL CMagDevice::SetUserROIsEx(const struct_RectROI * pROIs, UINT intROINum)
{
	return MAG_SetUserROIsEx(m_intChannelIndex, pROIs, intROINum);
}

BOOL CMagDevice::SetIrregularROIs(const struct_IrregularROI * pROIs, UINT intROINum)
{
	return MAG_SetIrregularROIs(m_intChannelIndex, pROIs, intROINum);
}

BOOL CMagDevice::SetROIReportCallBack(MAG_ROICALLBACK pCallBack, void * pUserData)
{
	return MAG_SetROIReportCallBack(m_intChannelIndex, pCallBack, pUserData);
}

BOOL CMagDevice::SetIrregularROIReportCallBack(MAG_ROICALLBACK pCallBack, void * pUserData)
{
	return MAG_SetIrregularROIReportCallBack(m_intChannelIndex, pCallBack, pUserData);
}

BOOL CMagDevice::SetIrregularROIReportExCallBack(MAG_IRREGULARROICALLBACK pCallBack, void * pUserData)
{
	return MAG_SetIrregularROIReportExCallBack(m_intChannelIndex, pCallBack, pUserData);
}

BOOL CMagDevice::IsProcessingImage()
{
	return MAG_IsProcessingImage(m_intChannelIndex);
}

BOOL CMagDevice::StartProcessImage(const OutputPara * paraOut, MAG_FRAMECALLBACK funcFrame, DWORD dwStreamType, void * pUserData)
{
	return MAG_StartProcessImage(m_intChannelIndex, paraOut, funcFrame, dwStreamType, pUserData);
}

BOOL CMagDevice::StartProcessPulseImage(const OutputPara * paraOut, MAG_FRAMECALLBACK funcFrame, DWORD dwStreamType, void * pUserData)
{
	return MAG_StartProcessPulseImage(m_intChannelIndex, paraOut, funcFrame, dwStreamType, pUserData);
}

BOOL CMagDevice::TransferPulseImage()
{
	return MAG_TransferPulseImage(m_intChannelIndex);
}

void CMagDevice::StopProcessImage()
{
	if (m_bIsRecordingLocalAvi)
	{
		LocalStorageAviStop();
	}

	if (m_bIsRecordingLocalMgs)
	{
		LocalStorageMgsStop();
	}

	MAG_StopProcessImage(m_intChannelIndex);
}

void CMagDevice::SetColorPalette(enum ColorPalette ColorPaletteIndex)
{
	MAG_SetColorPalette(m_intChannelIndex, ColorPaletteIndex);
}

BOOL CMagDevice::SetSubsectionEnlargePara(int intX1, int intX2, UCHAR byteY1, UCHAR byteY2)
{
	return MAG_SetSubsectionEnlargePara(m_intChannelIndex, intX1, intX2, byteY1, byteY2);
}

void CMagDevice::SetIsothermalPara(int intLowerLimit, int intUpperLimit)
{
	MAG_SetIsothermalPara(m_intChannelIndex, intLowerLimit, intUpperLimit);
}

void CMagDevice::SetIsothermalParaEx(int intLowerLimit, int intUpperLimit, BYTE R, BYTE G, BYTE B)
{
#ifdef MAG_Linux_Not_Implemented
	MAG_SetIsothermalPara(m_intChannelIndex, intLowerLimit, intUpperLimit);
#else
	MAG_SetIsothermalParaEx(m_intChannelIndex, intLowerLimit, intUpperLimit, R, G, B);
#endif
}

void CMagDevice::SetEnhancedROI(UINT intEnhancedRatio, UINT x0, UINT y0, UINT x1, UINT y1)
{
#ifdef MAG_Linux_Not_Implemented
	//
#else
	MAG_SetEnhancedROI(m_intChannelIndex, intEnhancedRatio, x0, y0, x1, y1);
#endif
}

void CMagDevice::SetAutoEnlargePara(DWORD dwAutoEnlargeRange, int intBrightOffset, int intContrastOffset)
{
	MAG_SetAutoEnlargePara(m_intChannelIndex, dwAutoEnlargeRange, intBrightOffset, intContrastOffset);
}

BOOL CMagDevice::GetApproximateGray2TemperatureLUT(int * pLut, UINT intBufferSize)
{
#ifdef MAG_Linux_Not_Implemented
	//
#else
	return MAG_GetApproximateGray2TemperatureLUT(m_intChannelIndex, pLut, intBufferSize);
#endif
}

void CMagDevice::SetEXLevel(enum EX ExLevel, int intCenterX, int intCenterY)
{
	MAG_SetEXLevel(m_intChannelIndex, ExLevel, intCenterX, intCenterY);
}

enum EX CMagDevice::GetEXLevel()
{
	return MAG_GetEXLevel(m_intChannelIndex);
}

void CMagDevice::SetDetailEnhancement(int intDDE, BOOL bQuickDDE)
{
	MAG_SetDetailEnhancement(m_intChannelIndex, intDDE, bQuickDDE);
}

BOOL CMagDevice::SetVideoContrast(int intContrastOffset)
{
	return MAG_SetVideoContrast(m_intChannelIndex, intContrastOffset);
}

BOOL CMagDevice::SetVideoBrightness(int intBrightnessOffset)
{
	return MAG_SetVideoBrightness(m_intChannelIndex, intBrightnessOffset);
}

void CMagDevice::GetFixPara(struct_FixPara * pPara)
{
	MAG_GetFixPara(m_intChannelIndex, pPara);
}

float CMagDevice::SetFixPara(const struct_FixPara * pPara, BOOL bEnableCameraCorrect)
{
	return MAG_SetFixPara(m_intChannelIndex, pPara, bEnableCameraCorrect);
}

int CMagDevice::FixTemperature(int intT, float fEmissivity, DWORD dwPosX, DWORD dwPosY)
{
	return MAG_FixTemperature(m_intChannelIndex, intT, fEmissivity, dwPosX, dwPosY);
}

const USHORT * CMagDevice::GetFilteredRaw()
{
	return MAG_GetFilteredRaw(m_intChannelIndex);
}

BOOL CMagDevice::GetOutputBMPdata(UCHAR const ** pData, BITMAPINFO const ** pInfo)
{
	return MAG_GetOutputBMPdata(m_intChannelIndex, pData, pInfo);
}

BOOL CMagDevice::GetOutputBMPdata_copy(UCHAR * pBmp, UINT intBufferSize)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_GetOutputBMPdata_copy(m_intChannelIndex, pBmp, intBufferSize);
#endif
}

BOOL CMagDevice::GetOutputBMPdataRGB24(UCHAR * pRGBBuffer, UINT intBufferSize, BOOL bOrderBGR)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_GetOutputBMPdataRGB24(m_intChannelIndex, pRGBBuffer, intBufferSize, bOrderBGR);
#endif
}

BOOL CMagDevice::GetOutputColorBardata(UCHAR const ** pData, BITMAPINFO const ** pInfo)
{
	return MAG_GetOutputColorBardata(m_intChannelIndex, pData, pInfo);
}

BOOL CMagDevice::GetOutputColorBardata_copy(UCHAR * pColorBar, UINT intBufferSize)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_GetOutputColorBardata_copy(m_intChannelIndex, pColorBar, intBufferSize);
#endif
}

BOOL CMagDevice::GetOutputVideoData(UCHAR const **  pData, BITMAPINFO const ** pInfo)
{
	return MAG_GetOutputVideoData(m_intChannelIndex, pData, pInfo);
}

BOOL CMagDevice::GetOutputVideoData_copy(UCHAR * pBmp, UINT intBufferSize)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_GetOutputVideoData_copy(m_intChannelIndex, pBmp, intBufferSize);
#endif
}

const UCHAR * CMagDevice::GetOutputVideoYV12()
{
	return MAG_GetOutputVideoYV12(m_intChannelIndex);
}

const struct_State * CMagDevice::GetFrameStatisticalData()
{
	return MAG_GetFrameStatisticalData(m_intChannelIndex);
}

BOOL CMagDevice::GetTemperatureData(int * pData, UINT intBufferSize, BOOL bEnableExtCorrect)
{
	return MAG_GetTemperatureData(m_intChannelIndex, pData, intBufferSize, bEnableExtCorrect);
}

BOOL CMagDevice::GetTemperatureData_Raw(int * pData, UINT intBufferSize, BOOL bEnableExtCorrect)
{
	return MAG_GetTemperatureData_Raw(m_intChannelIndex, pData, intBufferSize, bEnableExtCorrect);
}

int CMagDevice::GetTemperatureProbe(DWORD dwPosX, DWORD dwPosY, UINT intSize)
{
	return MAG_GetTemperatureProbe(m_intChannelIndex, dwPosX, dwPosY, intSize);
}

int CMagDevice::GetLineTemperatureInfo(int * buffer, UINT intBufferSizeByte, int info[3], UINT x0, UINT y0, UINT x1, UINT y1)
{
	return MAG_GetLineTemperatureInfo(m_intChannelIndex, buffer, intBufferSizeByte, info, x0, y0, x1, y1);
}

int CMagDevice::GetLineTemperatureInfo2(UINT x0, UINT y0, UINT x1, UINT y1, int info[5])
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_GetLineTemperatureInfo2(m_intChannelIndex, x0, y0, x1, y1, info);
#endif
}

BOOL CMagDevice::GetRectTemperatureInfo(UINT x0, UINT y0, UINT x1, UINT y1, int info[5])
{
	return MAG_GetRectTemperatureInfo(m_intChannelIndex, x0, y0, x1, y1, info);
}

BOOL CMagDevice::GetEllipseTemperatureInfo(UINT x0, UINT y0, UINT x1, UINT y1, int info[5])
{
	return MAG_GetEllipseTemperatureInfo(m_intChannelIndex, x0, y0, x1, y1, info);
}

BOOL CMagDevice::GetRgnTemperatureInfo(const UINT * Pos, UINT intPosNumber, int info[5])
{
	return MAG_GetRgnTemperatureInfo(m_intChannelIndex, Pos, intPosNumber, info);
}

BOOL CMagDevice::UseTemperatureMask(BOOL bUse)
{
	return MAG_UseTemperatureMask(m_intChannelIndex, bUse);
}

BOOL CMagDevice::IsUsingTemperatureMask()
{
	return MAG_IsUsingTemperatureMask(m_intChannelIndex);
}

BOOL CMagDevice::SaveBMP(DWORD dwIndex, const WCHAR * charFilename)
{
	return MAG_SaveBMP(m_intChannelIndex, dwIndex, charFilename);
}

BOOL CMagDevice::SaveMGT(const WCHAR * charFilename)
{
	return MAG_SaveMGT(m_intChannelIndex, charFilename);
}

BOOL CMagDevice::SaveDDT(const WCHAR * charFilename)
{
	return MAG_SaveDDT(m_intChannelIndex, charFilename);
}

int CMagDevice::SaveDDT2Buffer(void * pBuffer, UINT intBufferSize)
{
	return MAG_SaveDDT2Buffer(m_intChannelIndex, pBuffer, intBufferSize);
}

BOOL CMagDevice::LoadDDT(OutputPara * paraOut, const WCHAR * charFilename, MAG_FRAMECALLBACK funcFrame, void * pUserData)
{
	if (!MAG_IsLinked(m_intChannelIndex))
	{
		if(!MAG_LoadDDT(m_intChannelIndex, paraOut, charFilename, funcFrame, pUserData))
		{
			return FALSE;
		}
		
		MAG_GetCamInfo(m_intChannelIndex, &m_CamInfo, sizeof(m_CamInfo));
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CMagDevice::LoadBufferedDDT(OutputPara * paraOut, const void * pBuffer, UINT intBufferSize, MAG_FRAMECALLBACK funcFrame, void * pUserData)
{
	if (!MAG_IsLinked(m_intChannelIndex))
	{
		if(!MAG_LoadBufferedDDT(m_intChannelIndex, paraOut, pBuffer, intBufferSize, funcFrame, pUserData))
		{
			return FALSE;
		}
		
		MAG_GetCamInfo(m_intChannelIndex, &m_CamInfo, sizeof(m_CamInfo));
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CMagDevice::SetAsyncCompressCallBack(MAG_DDTCOMPRESSCALLBACK pCallBack, int intQuality)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SetAsyncCompressCallBack(m_intChannelIndex, pCallBack, intQuality);
#endif
}

BOOL CMagDevice::GrabAndAsyncCompressDDT(void * pUserData)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_GrabAndAsyncCompressDDT(m_intChannelIndex, pUserData);
#endif
}

BOOL CMagDevice::SDCardStorage(enum SDStorageFileType filetype, UINT para)
{
#ifdef MAG_Linux_Not_Implemented

	return FALSE;

#else

	BOOL bReturn = MAG_SDCardStorage(m_intChannelIndex, filetype, para);

	if (bReturn && filetype==SDFileMGS)
	{
		if (para==1)
		{
			m_bIsRecordingMGS = TRUE;
		}
		else
		{
			m_bIsRecordingMGS = FALSE;
		}
	}

	if (bReturn && filetype==SDFileAVI)
	{
		if (para==1)
		{
			m_bIsRecordingAvi = TRUE;
		}
		else
		{
			m_bIsRecordingAvi = FALSE;
		}
	}

	return bReturn;

#endif
}

BOOL CMagDevice::SDStorageMGT()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileMGT, 0);
#endif
}

BOOL CMagDevice::SDStorageBMP()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileBMP, 0);
#endif
}

BOOL CMagDevice::SDStorageDDT()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileDDT, 0);
#endif
}

BOOL CMagDevice::SDStorageMGSStart()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileMGS, 1);
#endif
}

BOOL CMagDevice::SDStorageMGSStop()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileMGS, 0);
#endif
}

BOOL CMagDevice::SDStorageAviStart()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileAVI, 1);
#endif
}

BOOL CMagDevice::SDStorageAviStop()
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	return MAG_SDCardStorage(m_intChannelIndex, SDFileAVI, 0);
#endif
}

BOOL CMagDevice::LocalStorageAviStart(const WCHAR * charFilename, UINT intSamplePeriod)
{
#ifdef MAG_Linux_Not_Implemented
	return FALSE;
#else
	m_bIsRecordingLocalAvi |= MAG_LocalStorageAviStart(m_intChannelIndex, charFilename, intSamplePeriod);
	return m_bIsRecordingLocalAvi;
#endif
}

void CMagDevice::LocalStorageAviStop()
{
#ifdef MAG_Linux_Not_Implemented
	return;
#else
	MAG_LocalStorageAviStop(m_intChannelIndex);
	m_bIsRecordingLocalAvi = FALSE;
#endif
}

BOOL CMagDevice::LocalStorageMgsRecord(const WCHAR * charFilename, UINT intSamplePeriod)
{
	m_bIsRecordingLocalMgs |= MAG_LocalStorageMgsRecord(m_intChannelIndex, charFilename, intSamplePeriod);
	return m_bIsRecordingLocalMgs;
}

int CMagDevice::LocalStorageMgsPlay(const WCHAR * charFilename, MAG_FRAMECALLBACK funcFrame, void * pUserData)
{
	int intTotalFrames = MAG_LocalStorageMgsPlay(m_intChannelIndex, charFilename, funcFrame, pUserData);
	if (intTotalFrames > 0)
	{
		m_bIsPlayingLocalMgs = TRUE;
	}

	if (m_bIsPlayingLocalMgs)
	{
		MAG_GetCamInfo(m_intChannelIndex, &m_CamInfo, sizeof(m_CamInfo));
	}

	return intTotalFrames;
}

BOOL CMagDevice::LocalStorageMgsPopFrame()
{
	return MAG_LocalStorageMgsPopFrame(m_intChannelIndex);
}

BOOL CMagDevice::LocalStorageMgsSeekFrame(UINT intFrameIndex)
{
	return MAG_LocalStorageMgsSeekFrame(m_intChannelIndex, intFrameIndex);
}

void CMagDevice::LocalStorageMgsStop()
{
	MAG_LocalStorageMgsStop(m_intChannelIndex);
	m_bIsRecordingLocalMgs = FALSE;
	m_bIsPlayingLocalMgs = FALSE;
}

void CMagDevice::Lock()
{
	MAG_LockFrame(m_intChannelIndex);
}

void CMagDevice::Unlock()
{
	MAG_UnLockFrame(m_intChannelIndex);
}

void CMagDevice::ConvertPos2XY(UINT intPos, UINT * pX, UINT * pY)
{
	UINT W = m_CamInfo.intFPAWidth;
	if (W && pX && pY)
	{
		*pY = intPos/W;
		*pX = intPos - (*pY)*W;
	}
}

UINT CMagDevice::ConvertXY2Pos(UINT X, UINT Y)
{
	return Y*m_CamInfo.intFPAWidth + X;
}
