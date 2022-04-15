//#include "stdafx.h"

#include "MagService.h"

#ifdef _WIN64
	#pragma comment(lib, "ThermoGroupSDKLib_x64.lib")
#else
	#pragma comment(lib, "ThermoGroupSDKLib.lib")
#endif

BOOL CMagService::m_bInitialized = FALSE;

CMagService::CMagService(HWND hWndMsg) : 
m_hWndMsg(hWndMsg)
{
	Initialize();
}

CMagService::~CMagService()
{
	if (MAG_IsDHCPServerRunning())
	{
		MAG_StopDHCPServer();
	}
	
	if (MAG_IsInitialized(0))
	{
		MAG_Free(0);
	}

	if (MAG_IsChannelAvailable(0))
	{
		MAG_DelChannel(0);
	}

	m_bInitialized = FALSE;
}

BOOL CMagService::Initialize()
{
	if (m_bInitialized)
	{
		//ASSERT(0);//I guess you must created more than one CMagService class
		return TRUE;
	}

	if (!MAG_IsChannelAvailable(0))
	{
		BOOL bSuccess = MAG_NewChannel(0);
		//ASSERT(bSuccess);
	}

	if (MAG_IsLanConnected())
	{
		m_bInitialized = MAG_Initialize(0, m_hWndMsg);
	}

	return m_bInitialized;
}

BOOL CMagService::IsLanConnected()
{
#ifdef MAG_Linux_Not_Implemented
	return TRUE;
#else
	return MAG_IsLanConnected();
#endif
}

BOOL CMagService::IsUsingStaticIp()
{
	return MAG_IsUsingStaticIp();
}

DWORD CMagService::GetLocalIp()
{
	if (!m_bInitialized)
	{
		Initialize();
	}

	return m_bInitialized ? MAG_GetLocalIp() : 0;
}

BOOL CMagService::IsDHCPServerRunning()
{
	return MAG_IsDHCPServerRunning();
}

BOOL CMagService::StartDHCPServer()
{
	return MAG_StartDHCPServer(m_hWndMsg);
}

void CMagService::StopDHCPServer()
{
	MAG_StopDHCPServer();
}

void CMagService::EnableAutoReConnect(BOOL bEnable)
{
	MAG_EnableAutoReConnect(bEnable);
}

BOOL CMagService::EnumCameras()
{
	if (!m_bInitialized)
	{
		Initialize();
	}

	return m_bInitialized ? MAG_EnumCameras() : FALSE;
}

UINT CMagService::GetTerminalList(struct_TerminalList * pList, DWORD dwBufferSize)
{
	return m_bInitialized ? MAG_GetTerminalList(pList, dwBufferSize) : 0;
}

UINT CMagService::GetTerminalCount()
{
	return m_bInitialized ? MAG_GetTerminalList(NULL, 0) : 0;
}

int CMagService::CompressDDT(void * pDstBuffer, UINT intDstBufferSize, const void * pSrcBuffer, UINT intSrcBufferSize, UINT intQuality)
{
	return MAG_CompressDDT(pDstBuffer, intDstBufferSize, pSrcBuffer, intSrcBufferSize, intQuality);
}

int CMagService::DeCompressDDT(void * pDstBuffer, UINT intDstBufferSize, const void * pSrcBuffer, UINT intSrcBufferSize)
{
	return MAG_DeCompressDDT(pDstBuffer, intDstBufferSize, pSrcBuffer, intSrcBufferSize);
}
