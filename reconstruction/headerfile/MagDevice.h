#ifndef __MAGDEVICE_H__
#define __MAGDEVICE_H__

#include "windows.h"
#include "typedef.h"
#include "ThermoGroupSDK.h"

#ifndef MAG_DEFAULT_TIMEOUT
#define MAG_DEFAULT_TIMEOUT	(500)
#endif

class CMagDevice
{
public:
	CMagDevice(HWND hWndMsg=NULL);
	~CMagDevice();

	BOOL IsInitialized(){return m_bInitialized;}
	const struct_CamInfo * GetCamInfo() {return &m_CamInfo;}
	const struct_CamInfoEx * GetCamInfoEx();
	const struct_CeRegContent * GetRegContent() { return &m_RegContent; }

	void ConvertPos2XY(UINT intPos, UINT * pX, UINT * pY);
	UINT ConvertXY2Pos(UINT X, UINT Y);

	BOOL IsLinked();
	BOOL LinkCamera(const char * charIp, UINT intTimeoutMS=2*MAG_DEFAULT_TIMEOUT);
	BOOL LinkCamera(UINT intIP, UINT intTimeoutMS=2*MAG_DEFAULT_TIMEOUT);
	BOOL LinkCameraEx(const char * charIp, USHORT shortCmdPort=33596, USHORT shortImgPort=33597, 
		const char * charCloudUser="", const char * charCloudPwd="", UINT intCamSN=0,
		const char * charCamUser="", const char * charCamPwd="", UINT intTimeoutMS=2*MAG_DEFAULT_TIMEOUT);
	BOOL LinkCameraEx(UINT intIP, USHORT shortCmdPort=33596, USHORT shortImgPort=33597, 
		const char * charCloudUser="", const char * charCloudPwd="", UINT intCamSN=0,
		const char * charCamUser="", const char * charCamPwd="", UINT intTimeoutMS=2*MAG_DEFAULT_TIMEOUT);
	void DisLinkCamera();
	UINT GetRecentHeartBeat();
	BOOL SetReConnectCallBack(MAG_RECONNECTCALLBACK pCallBack, void * pUserData);

	BOOL ResetCamera();
	BOOL TriggerFFC();
	BOOL AutoFocus();
	BOOL SetIoAlarmState(BOOL bAlarm);
	BOOL SetPTZCmd(enum PTZCmd cmd, DWORD dwPara=0);
	BOOL QueryPTZState(enum PTZQuery query, int * intValue, UINT intTimeoutMS=MAG_DEFAULT_TIMEOUT);
	BOOL SetSerialCmd(const BYTE * buffer, UINT intBufferLen);
	BOOL SetSerialCallBack(MAG_SERIALCALLBACK pCallBack, void * pUserData);
	
	BOOL GetCameraTemperature(int intT[4], UINT intTimeoutMS=MAG_DEFAULT_TIMEOUT);

	BOOL SetCameraRegContent(const struct_CeRegContent * pContent);

	BOOL ReadCameraRegContent2(struct_CfgPara * pContent, UINT intTimeoutMS);
	BOOL SetCameraRegContent2(const struct_CfgPara * pContent);

	BOOL SetUserROIs(const struct_UserROIs * pROI);
	BOOL SetUserROIsEx(const struct_RectROI * pROIs, UINT intROINum);
	BOOL SetIrregularROIs(const struct_IrregularROI * pROIs, UINT intROINum);

	BOOL SetROIReportCallBack(MAG_ROICALLBACK pCallBack, void * pUserData);
	BOOL SetIrregularROIReportCallBack(MAG_ROICALLBACK pCallBack, void * pUserData);
	BOOL SetIrregularROIReportExCallBack(MAG_IRREGULARROICALLBACK  pCallBack, void * pUserData);

	BOOL IsProcessingImage();
	BOOL StartProcessImage(const OutputPara * paraOut, MAG_FRAMECALLBACK funcFrame, DWORD dwStreamType, void * pUserData);
	BOOL StartProcessPulseImage(const OutputPara * paraOut, MAG_FRAMECALLBACK funcFrame, DWORD dwStreamType, void * pUserData);
	BOOL TransferPulseImage();
	void StopProcessImage();

	void SetColorPalette(enum ColorPalette ColorPaletteIndex);
	BOOL SetSubsectionEnlargePara(int intX1, int intX2, UCHAR byteY1=0, UCHAR byteY2=255);
	void SetAutoEnlargePara(DWORD dwAutoEnlargeRange, int intBrightOffset=0, int intContrastOffset=0);
	void SetIsothermalPara(int intLowerLimit, int intUpperLimit);
	void SetIsothermalParaEx(int intLowerLimit, int intUpperLimit, BYTE R, BYTE G, BYTE B);
	void SetEnhancedROI(UINT intEnhancedRatio, UINT x0, UINT y0, UINT x1, UINT y1);
	BOOL GetApproximateGray2TemperatureLUT(int * pLut, UINT intBufferSize);
	void SetEXLevel(enum EX ExLevel, int intCenterX, int intCenterY);
	enum EX GetEXLevel();
	void SetDetailEnhancement(int intDDE, BOOL bQuickDDE=TRUE);
	BOOL SetVideoContrast(int intContrastOffset);
	BOOL SetVideoBrightness(int intBrightnessOffset);

	void GetFixPara(struct_FixPara * pPara);
	float SetFixPara(const struct_FixPara * pPara, BOOL bEnableCameraCorrect);
	int FixTemperature(int intT, float fEmissivity, DWORD dwPosX, DWORD dwPosY);

	const USHORT * GetFilteredRaw();

	BOOL GetOutputBMPdata(UCHAR const ** pData, BITMAPINFO const ** pInfo);
	BOOL GetOutputBMPdata_copy(UCHAR * pBmp, UINT intBufferSize);
	BOOL GetOutputColorBardata(UCHAR const ** pData, BITMAPINFO const ** pInfo);
	BOOL GetOutputColorBardata_copy(UCHAR * pColorBar, UINT intBufferSize);
	BOOL GetOutputBMPdataRGB24(UCHAR * pRGBBuffer, UINT intBufferSize, BOOL bOrderBGR);
	BOOL GetOutputVideoData(UCHAR const ** pData, BITMAPINFO const ** pInfo);
	BOOL GetOutputVideoData_copy(UCHAR * pBmp, UINT intBufferSize);

	const UCHAR * GetOutputVideoYV12();

	UINT GetDevIPAddress() const { return m_intCamIPAddr; }
	const struct_State * GetFrameStatisticalData();
	BOOL GetTemperatureData(int * pData, UINT intBufferSize, BOOL bEnableExtCorrect);
	BOOL GetTemperatureData_Raw(int * pData, UINT intBufferSize, BOOL bEnableExtCorrect);
	int GetTemperatureProbe(DWORD dwPosX, DWORD dwPosY, UINT intSize);
	int GetLineTemperatureInfo(int * buffer, UINT intBufferSizeByte, int info[3], UINT x0, UINT y0, UINT x1, UINT y1);
	int GetLineTemperatureInfo2(UINT x0, UINT y0, UINT x1, UINT y1, int info[5]);
	BOOL GetRectTemperatureInfo(UINT x0, UINT y0, UINT x1, UINT y1, int info[5]);
	BOOL GetEllipseTemperatureInfo(UINT x0, UINT y0, UINT x1, UINT y1, int info[5]);
	BOOL GetRgnTemperatureInfo(const UINT * Pos, UINT intPosNumber, int info[5]);	

	BOOL UseTemperatureMask(BOOL bUse);
	BOOL IsUsingTemperatureMask();

	BOOL SaveBMP(DWORD dwIndex=0, const WCHAR * charFilename=NULL);
	BOOL SaveMGT(const WCHAR * charFilename=NULL);
	BOOL SaveDDT(const WCHAR * charFilename=NULL);
	int SaveDDT2Buffer(void * pBuffer, UINT intBufferSize);
	BOOL LoadDDT(OutputPara * paraOut, const WCHAR * charFilename, MAG_FRAMECALLBACK funcFrame, void * pUserData);
	BOOL LoadBufferedDDT(OutputPara * paraOut, const void * pBuffer, UINT intBufferSize, MAG_FRAMECALLBACK funcFrame, void * pUserData);
	
	BOOL SetAsyncCompressCallBack(MAG_DDTCOMPRESSCALLBACK pCallBack, int intQuality);
	BOOL GrabAndAsyncCompressDDT(void * pUserData);

	BOOL SDCardStorage(enum SDStorageFileType filetype, UINT para=0);
	BOOL SDStorageMGT();//legacy, should be replaced by SDCardStorage()
	BOOL SDStorageBMP();//legacy, should be replaced by SDCardStorage()
	BOOL SDStorageDDT();//legacy, should be replaced by SDCardStorage()
	BOOL SDStorageMGSStart();//legacy, should be replaced by SDCardStorage()
	BOOL SDStorageMGSStop();//legacy, should be replaced by SDCardStorage()
	BOOL SDStorageAviStart();//legacy, should be replaced by SDCardStorage()
	BOOL SDStorageAviStop();//legacy, should be replaced by SDCardStorage()

	BOOL LocalStorageAviStart(const WCHAR * charFilename=NULL, UINT intSamplePeriod=1);
	void LocalStorageAviStop();
	BOOL IsLocalAviRecording() const {return m_bIsRecordingLocalAvi;}

	BOOL LocalStorageMgsRecord(const WCHAR * charFilename=NULL, UINT intSamplePeriod=1);
	int  LocalStorageMgsPlay(const WCHAR * charFilename, MAG_FRAMECALLBACK funcFrame, void * pUserData);
	BOOL LocalStorageMgsPopFrame();
	BOOL LocalStorageMgsSeekFrame(UINT intFrameIndex);
	void LocalStorageMgsStop();
	BOOL IsLocalMgsRecording() const {return m_bIsRecordingLocalMgs;}
	BOOL IsLocalMgsPlaying() const {return m_bIsPlayingLocalMgs;}

	//BOOL GetCurrentOffset(const WCHAR * charReferenceDDT, int * pOffsetX, int * pOffsetY);

	//BOOL CaptureVisibleJpeg(const WCHAR * charFileName);

	void Lock();
	void Unlock();

private:
	BOOL Initialize();

	BOOL m_bInitialized;
	HWND m_hWndMsg;

	UINT m_intChannelIndex;
	
	UINT m_intCamIPAddr;
	struct_CamInfo m_CamInfo;
	struct_CamInfoEx m_CamInfoEx;
	struct_CeRegContent m_RegContent;

	BOOL m_bIsRecordingAvi;
	BOOL m_bIsRecordingMGS;
	BOOL m_bIsRecordingLocalAvi;
	BOOL m_bIsRecordingLocalMgs;
	BOOL m_bIsPlayingLocalMgs;
};

#endif
