//-------------------------------------------------------------------------
// <copyright file="USBCode.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//
//    The use and distribution terms for this software are covered by the
//    Microsoft Limited Permissive License (Ms-LPL) 
//    http://www.microsoft.com/resources/sharedsource/licensingbasics/limitedpermissivelicense.mspx 
//    which can be found in the file MS-LPL.txt at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    USB support entry points for driver
// </summary>
//-------------------------------------------------------------------------
//======================================================================
// USBCode - USB support entry points for driver
//
// Author: Douglas Boling
//======================================================================
#include "stdafx.h"        
#include "driver.h"


#define VENDORID 0x1162
#define PRODID   0x0322


const static USB_DRIVER_SETTINGS usbDriverSettings = 
{
	        sizeof(USB_DRIVER_SETTINGS),  
            VENDORID,   
            PRODID,   
            USB_NO_INFO,   
            USB_NO_INFO,   
            USB_NO_INFO,   
            USB_NO_INFO,   
            USB_NO_INFO,   
            USB_NO_INFO,   
            USB_NO_INFO

};


//======================================================================
// USBDeviceAttach - Called when Host controller wants to load the driver
// 
BOOL USBDeviceAttach (USB_HANDLE hDevice, LPCUSB_FUNCS lpUsbFuncs,
                      LPCUSB_INTERFACE lpInterface, LPCWSTR szUniqueDriverId,
                      LPBOOL fAcceptControl,
                      LPCUSB_DRIVER_SETTINGS lpDriverSettings, DWORD dwUnused)
{
    

    DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBDeviceAttach++\r\n")));
	
	// Default is that we won't accept this device
	*fAcceptControl = FALSE;


	DEBUGMSG(ZONE_USBLOAD, (DTAG TEXT("Creating Driver")));
	
	DEBUGMSG(ZONE_USBLOAD, (DTAG TEXT("....")));

	Driver* pDrv				= Driver::Create(hDevice, lpUsbFuncs,
												lpInterface, szUniqueDriverId,
												lpDriverSettings); 
	

	DEBUGMSG(ZONE_USBLOAD, (DTAG TEXT("Driver ptr %08x"),pDrv));

	if (!pDrv)
	{
		
		DEBUGMSG(ZONE_USBLOAD, (DTAG TEXT("Unable to create the driver object")));
		DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBDeviceAttach--\r\n")));
		return FALSE;
	}	

	// Accept this device as our own
	*fAcceptControl = TRUE;

    DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBDeviceAttach--\r\n")));
	return TRUE;
}

//======================================================================
// USBInstallDriver - Called by Host controller to have the driver
// register itself.  This call is made in response to the user
// entering the driver's DLL name in the "Unknown Device" message box.
// 
BOOL USBInstallDriver (LPCWSTR szDriverLibFile)
{
    WCHAR wsUsbDeviceID[] = CLASS_NAME_SZ;
    WCHAR wsSubClassRegKey[] = CLIENT_REGKEY_SZ;
    
	BOOL bSuccess;
	LPREGISTER_CLIENT_DRIVER_ID lpfnRegisterClientDriverID;
	LPREGISTER_CLIENT_SETTINGS lpfnRegisterClientSettings;

    DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBInstallDriver++\r\n")));

	HINSTANCE hUSBD = LoadLibrary (TEXT("usbd.dll"));
	if (hUSBD == 0)
	{
	    DEBUGMSG (ZONE_ERROR, (DTAG TEXT("Can't load USBD.DLL ERR 0x%08x\r\n"),GetLastError()));
		return FALSE;
	}

	// Explicitly link to USBD DLL.  
	lpfnRegisterClientDriverID = (LPREGISTER_CLIENT_DRIVER_ID) GetProcAddress (hUSBD, TEXT("RegisterClientDriverID"));
	lpfnRegisterClientSettings = (LPREGISTER_CLIENT_SETTINGS) GetProcAddress (hUSBD, TEXT("RegisterClientSettings"));
	if ((lpfnRegisterClientDriverID == 0) || (lpfnRegisterClientSettings == 0))
	{
		FreeLibrary (hUSBD);
		return FALSE;
	}
	//
	// register with USBD
	//	 
	bSuccess = (lpfnRegisterClientDriverID) (wsUsbDeviceID);
	if (!bSuccess) 
	{
		DEBUGMSG (ZONE_ERROR, (TEXT("RegisterClientDriverID error:%d\n"), GetLastError()));
		FreeLibrary (hUSBD);
		return FALSE;
	}
	//
	// Call USBD to create registry entries for USB client registration
	//
	bSuccess = (lpfnRegisterClientSettings) (szDriverLibFile, wsUsbDeviceID, 
	                                         NULL, &usbDriverSettings);
	if (!bSuccess) 
	{
		DEBUGMSG (ZONE_ERROR, (TEXT("RegisterClientSettings error:%d\n"), GetLastError()));
		FreeLibrary (hUSBD);
		return FALSE;
	}

	//
	// The USB host controller driver has a utility function to make it
	// simple to create (and later query) the proper registery entries 
	// for loading a stream driver.  First, we create a table of registry 
	//values we want to set.
	//
    REG_VALUE_DESCR usbDevKeyValues[] = {
        (TEXT("Dll")),               REG_SZ,    0, (PBYTE)(DRIVER_NAME),
        (TEXT("Prefix")),            REG_SZ,    0, (PBYTE)(DEVICE_PREFIX),
        NULL, 0, 0, NULL
    };

    
	bSuccess = GetSetKeyValues (wsSubClassRegKey, usbDevKeyValues, SET, TRUE );
	if (!bSuccess)
	{
        DEBUGMSG (ZONE_ERROR, (TEXT("GetSetKeyValues failed!\n")));
		FreeLibrary (hUSBD);
		return FALSE;
    }
	

	FreeLibrary (hUSBD);
    DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBInstallDriver--\r\n")));
	return TRUE;
}



//======================================================================
// USBUnInstallDriver - Called to have the USB client clean up its
// registry entries.
// 
BOOL USBUnInstallDriver()
{
    WCHAR wsUsbDeviceID[] = CLASS_NAME_SZ;
   
	LPUN_REGISTER_CLIENT_SETTINGS lpfnUnRegisterClientSettings;
	BOOL bSuccess = FALSE;

    DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBUnInstallDriver++\r\n")));

	// Explicitly link to USBD DLL.  
	HINSTANCE hUSBD = LoadLibrary (TEXT("usbd.dll"));
	if (hUSBD == 0)
	{
	    DEBUGMSG (ZONE_ERROR, (DTAG TEXT("Can't load USBD.DLL\r\n")));
		return FALSE;
	}
	lpfnUnRegisterClientSettings = (LPUN_REGISTER_CLIENT_SETTINGS) GetProcAddress (hUSBD, TEXT("UnRegisterClientSettings"));
	// Clean up the registry
	if (lpfnUnRegisterClientSettings)
		bSuccess = (lpfnUnRegisterClientSettings) (wsUsbDeviceID, NULL, &usbDriverSettings);

	FreeLibrary (hUSBD);
    DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("USBUnInstallDriver--\r\n")));
	return bSuccess;
}

