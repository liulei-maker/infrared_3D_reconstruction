#ifndef __MAGSERVICE_H__
#define __MAGSERVICE_H__

#include "windows.h"
#include "typedef.h"
#include "mag_errorcode.h"
#include "ThermoGroupSDK.h"

#ifndef MAG_DEFAULT_TIMEOUT
#define MAG_DEFAULT_TIMEOUT	(500)     
#endif

class CMagService
{
public:
	CMagService(HWND hWndMsg=NULL);
	~CMagService();

	BOOL IsInitialized()const {return m_bInitialized;}

	static BOOL IsLanConnected();
	static BOOL IsUsingStaticIp();
	DWORD GetLocalIp();

	BOOL IsDHCPServerRunning();
	BOOL StartDHCPServer();
	void StopDHCPServer();

	static void EnableAutoReConnect(BOOL bEnable);

	BOOL EnumCameras();
	UINT GetTerminalCount();
	UINT GetTerminalList(struct_TerminalList * pList, DWORD dwBufferSize);

	int CompressDDT(void * pDstBuffer, UINT intDstBufferSize, const void * pSrcBuffer, UINT intSrcBufferSize, UINT intQuality);
	int DeCompressDDT(void * pDstBuffer, UINT intDstBufferSize, const void * pSrcBuffer, UINT intSrcBufferSize);

private:
	BOOL Initialize();

	static BOOL m_bInitialized;
	HWND m_hWndMsg;
};

#endif