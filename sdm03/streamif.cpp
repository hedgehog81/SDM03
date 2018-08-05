#include "stdafx.h"
#include "streamif.h"
#include "driver.h"
#include "register.h"


static Register  s_Register;
 
static Driver* GetConfigData (DWORD dwContext)
{
	int nLen, rc;
	DWORD dwLen, dwType, dwSize = 0;
	HKEY hKey;
	Driver* pDrv = 0;

	DEBUGMSG (ZONE_FUNC, (DTAG TEXT("GetConfigData++\r\n")));
	nLen = 0;
	// If ptr < 65K, it's a value, not a pointer.  
	if (dwContext < 0x10000) {
		return 0; 
	} else {
		__try {
			nLen = lstrlen ((LPTSTR)dwContext);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			nLen = 0;
		}
	}
	if (!nLen) {
		DEBUGMSG (ZONE_ERROR, (DTAG TEXT("dwContext not a ptr\r\n")));
		return 0;
	}

	// Open the Active key for the driver.
	rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,(LPTSTR)dwContext,0, 0, &hKey);

	if (rc == ERROR_SUCCESS) {
		// Read the key value.
		dwLen = sizeof(pDrv);
		rc = RegQueryValueEx (hKey, TEXT("ClientInfo"), NULL, &dwType,
								   (PBYTE)&pDrv, &dwLen);

		RegCloseKey(hKey);
		if ((rc == ERROR_SUCCESS) && (dwType == REG_DWORD))
		{

/*
			__try {
				if (pDrv->dwSize != sizeof (DRVCONTEXT))
					pDrv = 0;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				pDrv = 0;
			}
*/
		}
		else 
		{
			DEBUGMSG (ZONE_ERROR, (DTAG TEXT("Error reading registry\r\n")));
			pDrv = 0;
		}
	} else
		DEBUGMSG (ZONE_ERROR, (DTAG TEXT("Error opening Active key\r\n")));

	DEBUGMSG (ZONE_FUNC, (DTAG TEXT("GetConfigData--\r\n")));
	return pDrv;
}


static Driver* GetDriver(DWORD dwContext)
{
	DEBUGMSG (ZONE_FUNC, (DTAG TEXT("GetDriver %08x"),dwContext));
	
	Driver* pDrv = 0;
	
	if (dwContext)
	{
		pDrv = s_Register.Get(dwContext);
		DEBUGMSG (ZONE_FUNC, (DTAG TEXT("s_Register.Get =  %08x"),pDrv));
	}
	
	return pDrv;

}

static void ReleaseDriver(Driver* pDrv)
{
	DEBUGMSG (ZONE_FUNC, (DTAG TEXT("ReleaseDriver pDrv  %08x"),pDrv));

	if (pDrv)
	{
		pDrv->Release();
	}
}


DWORD SDM_Init (DWORD dwContext)
{
	DEBUGMSG(ZONE_INIT,(DTAG TEXT("SDM_Init dwContext %08x"),dwContext));
	
	Driver* pDrv = GetConfigData(dwContext);
	DWORD dwRet = 0;

	if (pDrv)
	{
		dwRet = s_Register.Put(pDrv);	
	}

	return dwRet;
}

BOOL  SDM_Deinit (DWORD dwContext)
{
	DEBUGMSG(ZONE_INIT,(DTAG TEXT("AFR_Deinit dwContext %08x"),dwContext));
	
	return s_Register.Remove(dwContext);
}


DWORD SDM_Open (DWORD dwContext, DWORD dwAccess, DWORD dwShare)
{
	Driver* pDrv = GetDriver(dwContext);
	DWORD dwRet = 0;

	if (pDrv)
	{
		DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("AFR_Open this %08x"),pDrv));
		dwRet = pDrv->Open();

		ReleaseDriver(pDrv);
	}
	
	return dwRet;
}


BOOL  SDM_Close (DWORD dwOpen)
{
	BOOL ret = false;

	Driver* pDrv = GetDriver(dwOpen);

	if (pDrv)
	{
		ret = pDrv->Close();
		ReleaseDriver(pDrv);
	}

	return ret;
}

DWORD SDM_Read (DWORD dwOpen, LPVOID pBuffer, DWORD dwCount)
{
	SetLastError(ERROR_READ_FAULT);
	return 0;
}

DWORD SDM_Write (DWORD dwOpen, LPVOID pBuffer, DWORD dwCount)
{
	SetLastError(ERROR_WRITE_FAULT);
	return 0;
}

DWORD SDM_Seek (DWORD dwOpen, long lDelta, WORD wType)
{
	SetLastError(ERROR_GEN_FAILURE);
	return 0;
}



DWORD SDM_IOControl (DWORD dwOpen, DWORD dwCode, 
                     PBYTE pIn, DWORD dwIn,
                     PBYTE pOut, DWORD dwOut, 
                     DWORD *pdwBytesWritten)

{
	
	BOOL ret = false;

	Driver* pDrv = GetDriver(dwOpen);

	if (pDrv)
	{
		ret = pDrv->IOControl(dwCode,pIn,dwIn,pOut,dwOut,pdwBytesWritten);
		ReleaseDriver(pDrv);
	}

	return ret;

}

void SDM_PowerDown (DWORD dwContext)
{

}

void SDM_PowerUp (DWORD dwContext)
{

}