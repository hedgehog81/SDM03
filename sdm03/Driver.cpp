// AES2501Driver.cpp: implementation of the AES2501Driver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <new>
#include <usbclient.h>
#include "defines.h"
#include "Driver.h"
#include "drvproto.h"
#include "fp_internal.h"



#ifdef DEBUG

static int DumpEndpointDescriptor (const USB_ENDPOINT_DESCRIPTOR* lpepd)
{
	DEBUGMSG (1, (TEXT("Endpoint descriptor\r\n")));

	if (lpepd->bLength != sizeof (USB_ENDPOINT_DESCRIPTOR))
		DEBUGMSG (1, (TEXT("\t\t Unexpected structure length %d   ***********\r\n"), lpepd->bLength));

	if (lpepd->bDescriptorType != USB_ENDPOINT_DESCRIPTOR_TYPE)
		DEBUGMSG (1, (TEXT("\t\t Unexpected DescriptorType!  %02xh \r\n"), lpepd->bDescriptorType));

	DEBUGMSG (1, (TEXT("\t\t bEndpointAddress:  %02xh  %s\r\n"), (lpepd->bEndpointAddress &0x0f), (lpepd->bEndpointAddress &0x80) ? TEXT("In") : TEXT("Out")));
	TCHAR szType[32] = TEXT("unknown");

	switch (lpepd->bmAttributes & USB_ENDPOINT_TYPE_MASK)
	{
	case USB_ENDPOINT_TYPE_CONTROL:
		lstrcpy (szType, TEXT("Control"));
		break;
	case USB_ENDPOINT_TYPE_ISOCHRONOUS:
		lstrcpy (szType, TEXT("Isochronous"));
		break;
	case USB_ENDPOINT_TYPE_BULK:
		lstrcpy (szType, TEXT("Bulk"));
		break;
	case USB_ENDPOINT_TYPE_INTERRUPT:
		lstrcpy (szType, TEXT("Interrupt"));
		break;
	}
	DEBUGMSG (1, (TEXT("\t\t bmAttributes:      %s\r\n"), szType));
	DEBUGMSG (1, (TEXT("\t\t wMaxPacketSize:    %xh (%d)\r\n"), lpepd->wMaxPacketSize, lpepd->wMaxPacketSize));
	DEBUGMSG (1, (TEXT("\t\t bInterval:         %xh (%d)\n"), lpepd->bInterval, lpepd->bInterval));
	return 0;
}

#endif





const Driver::control_req_t Driver::init_script[] = 
{
	{0x22,0x11,0x01,0x19,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x02,0x10,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x03,0x08,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x14,0x01,FLAG_INPUT_BUFFER},	
	{0x22,0x11,0x15,0xe4,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x16,0x02,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x17,0x80,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x25,0x06,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x26,0x5b,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x27,0x9a,FLAG_INPUT_BUFFER},	
	{0x22,0x11,0x01,0x19,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x02,0x10,FLAG_INPUT_BUFFER},	
	{0x22,0x11,0x03,0x08,FLAG_INPUT_BUFFER},
	{0x11,0x00,0x00,0x00,0x00},/*No input data*/
	{0x22,0x11,0x30,0x09,FLAG_INPUT_BUFFER},
	{0x05,0x01,0x00,0x00,0x00}	
};

const Driver::control_req_t Driver::capture_script[] = 
{
	{0x11,0x01,0x00,0x00,0x00},
	{0x22,0x11,0x30,0x0F,FLAG_INPUT_BUFFER},	
	{0x22,0x11,0x02,0x18,FLAG_INPUT_BUFFER},
	{0x22,0x11,0x02,0x10,FLAG_INPUT_BUFFER},
	{0x03,0x54b,0x00,0x00,0x00}
};

const Driver::control_req_t Driver::stop_capture_script[] = 
{
	{0x04,0x00,0x00,0x00,0x00},	
	{0x22,0x11,0x30,0x09,FLAG_INPUT_BUFFER},
	{0x11,0x00,0x00,0x00,0x00}
};


const Driver::control_req_t Driver::uninit_script[] = 
{
	{0x04,0x00,0x00,0x00,0x00}
};




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Driver::Driver(	HANDLE hDevice,LPCUSB_FUNCS lpUsbFuncs,
								LPCUSB_INTERFACE lpInterface, LPCWSTR szUniqueDriverId,
								LPCUSB_DRIVER_SETTINGS lpDriverSettings)
{
		
	
	m_bOpen  = false;
	m_hStreamDevice = 0;
	m_lpUsbFuncs = lpUsbFuncs;
	m_hDevice = hDevice;
	m_bDetached = false;

	m_inpipe		= 0;
	m_lpInterface	= 0;
	m_hCompletion	= 0;



	
	
	m_RefCount = 1;
	
	m_hImageBuffer = 0;
	m_ImageBufferSize = 0;
	m_ImageBuffer = 0;
	m_bHaveImage  = false;

	memset(m_ImageBufferName,0,sizeof(m_ImageBufferName));

	InitializeCriticalSection(&m_Lock);
	
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("AES2501Driver this %08x"),this));
	
}



Driver::~Driver()
{
	
	
	Close();

	ClosePipe();

	DeleteCriticalSection(&m_Lock);
	
}



void Driver::AddRef()
{
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT( "Ref driver instance thrd %08x" ),GetCurrentThreadId()));
	InterlockedIncrement(&m_RefCount);
}

void Driver::Release()
{
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT( "Unref driver instance thrd %08x" ),GetCurrentThreadId()));

	if (InterlockedDecrement(&m_RefCount) == 0)
	{
		DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT( "Last ref on driver instance thrd %08x" ),GetCurrentThreadId()));
		delete this;
	}	
		
}


DWORD Driver::Open()
{
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Driver::Open" )));
	DWORD dwRet = 0;

	EnterCriticalSection(&m_Lock);
	
	if (!m_bDetached && !m_bOpen)
	{
		
		
		if ( 0 == RunScript(init_script,sizeof(init_script)/sizeof(init_script[0])) )
		{
			
			if (0 != (m_ImageBuffer = (unsigned char*)AllocImageBuffer(IMAGE_HEIGHT * IMAGE_WIDTH)))
			{
				m_bHaveImage = false;
				m_bOpen = true;
				dwRet = (DWORD)this;
			}
		
		}
		
		
	}
	else
		SetLastError(ERROR_ACCESS_DENIED);

	LeaveCriticalSection(&m_Lock);
	
	
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT("Driver::Open ret %08x"),dwRet));

	return dwRet;
}



DWORD Driver::Close()
{
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Driver::Close" )));
	
	DWORD dwRet = false;

	EnterCriticalSection(&m_Lock);
	
	if (m_bOpen)
	{
		
		RunScript(uninit_script,sizeof(uninit_script)/sizeof(uninit_script[0]));
		
		dwRet = true;
		m_bOpen = false;

		if (m_ImageBuffer)
		{
			FreeImageBuffer(m_ImageBuffer);
			m_ImageBuffer = 0;
		}
		
	}	
	else
		SetLastError(ERROR_INVALID_HANDLE);

	LeaveCriticalSection(&m_Lock);

	return dwRet;

}

DWORD Driver::IOControl(DWORD dwCode, PBYTE pIn, DWORD dwIn,PBYTE pOut, 
							   DWORD dwOut, DWORD *pdwBytesWritten)
{
	DWORD dwRet = ERROR_INVALID_FUNCTION;
	
	*pdwBytesWritten = 0;

	EnterCriticalSection(&m_Lock);

	if (!m_bOpen || m_bDetached)
	{
		dwRet = ERROR_GEN_FAILURE;
	}	
	else
	{
		__try 
		{
			switch (dwCode)
			{
				
				case IOCTRL_GET_BUFFER_NAME:
						dwRet = GetBufferName(pIn,dwIn,pOut,dwOut,pdwBytesWritten);
					break;
				case IOCTRL_CHK_FINGER:
						dwRet = IsFingerPresent(pIn,dwIn,pOut,dwOut,pdwBytesWritten);
					break;
				case IOCTRL_CAPTURE:
						dwRet = Capture(pIn,dwIn,pOut,dwOut,pdwBytesWritten);
					break;
				case IOCTRL_GET_IMAGE:
						dwRet = GetImage(pIn,dwIn,pOut,dwOut,pdwBytesWritten);
					break;
			
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			dwRet = ERROR_GEN_FAILURE;
		}
	}
	
	LeaveCriticalSection(&m_Lock);
	
	SetLastError(dwRet);

	return dwRet == ERROR_SUCCESS;
}



Driver* Driver::Create(	HANDLE hDevice,LPCUSB_FUNCS lpUsbFuncs,
										LPCUSB_INTERFACE lpInterface, LPCWSTR szUniqueDriverId,
										LPCUSB_DRIVER_SETTINGS lpDriverSettings)
{
	
	DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("Driver::Create")));

	Driver* pDrv = new(std::nothrow) 			Driver	(	hDevice, lpUsbFuncs,
															lpInterface,szUniqueDriverId,
															lpDriverSettings);
	
	DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("Driver::Create ptr %08x"),pDrv));

	if (pDrv)
	{
		if (!pDrv->DevInit())
		{
			delete pDrv;
			pDrv = 0;
		}
	
	}
	else
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);

	return pDrv;
	
}

void Driver::ResetImage()
{

}


DWORD     Driver::__IsFingerPresent(bool* pPresent)
{

	char buffer[4];
	memset(buffer,0,sizeof(buffer));

	int ret = ReadRegister(0x16, 0x01, 0, buffer, 4, 5000);
	
	*pPresent = (buffer[0] == 0x01);

	return ret;

}


	
DWORD Driver::IsFingerPresent(	PBYTE pIn, DWORD dwIn, PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten)
{
	
	DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("Driver::IsFingerPresent")));

	bool Present = false;

	DWORD dwRet = __IsFingerPresent(&Present);

	if (dwRet != 0)
		return ERROR_GEN_FAILURE;
	
	DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("Driver::Capture finger %d"),Present));
	*((int*)pOut) = Present;
	*pdwBytesWritten = 1;
	

	return ERROR_SUCCESS;
}

DWORD  Driver::Capture(	PBYTE pIn, DWORD dwIn,
								PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten)
{
	
	DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("Driver::Capture")));

	if (dwOut < sizeof(imageinfo_t))
	{
		return ERROR_INVALID_PARAMETER;
	}

	ResetImage();

	bool Present = false;

	DWORD dwRet = __IsFingerPresent(&Present);
	

	DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("__IsFingerPresent dwRet %08x Present %08x"),dwRet,Present));

	if (dwRet != 0)
		return ERROR_GEN_FAILURE;

	imageinfo_t info = {0,0,0,0};

	if (Present)
	{
		
		dwRet = RunScript(capture_script,sizeof(capture_script)/sizeof(capture_script[0]));	

		DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("Start capture script dwRet %08x"),dwRet));

		if (dwRet == ERROR_SUCCESS)
		{
			
			
			if (FillImageBuffer())
			{
				info.x = IMAGE_WIDTH;
				info.y = IMAGE_HEIGHT;
				info.res = 0;
				info.res = FP_IMG_COLORS_INVERTED;
				info.real_x = IMAGE_REAL_WIDTH;
				info.real_y = IMAGE_REAL_HEIGHT;


				dwRet = RunScript(stop_capture_script,sizeof(stop_capture_script)/sizeof(stop_capture_script[0]));	
				
				DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("Stop capture script dwRet %08x"),dwRet));
			}	
		
		}

		
	}

	
	if (dwRet == ERROR_SUCCESS)
	{
		memcpy(pOut,&info,sizeof(imageinfo_t));
		*pdwBytesWritten =  sizeof(imageinfo_t);
	
		return ERROR_SUCCESS;
	}

	return ERROR_GEN_FAILURE;
  
}

DWORD  Driver::GetImage(PBYTE pIn, DWORD dwIn,
						PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten)
{
	
	
	DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("Driver::GetImage")));
/*
	if (m_img && m_img->length <= dwOut)
	{
		
		DEBUGMSG (ZONE_DRVCALLS, (DTAG TEXT("copy image size %d "),m_img->length));

		memcpy(pOut,m_img->data,m_img->length);
		*pdwBytesWritten = m_img->length;

		ResetImage();

		return ERROR_SUCCESS;
	}
	
*/	
	return ERROR_INVALID_PARAMETER;
}

DWORD  Driver::GetBufferName(	PBYTE pIn, DWORD dwIn,
										PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten)
{
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("Driver::GetBufferName")));
	
	size_t namesize = wcslen(m_ImageBufferName) * sizeof(wchar_t);
	
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("name size %d dwOut %d"),namesize,dwOut));

	if(m_hImageBuffer && dwOut >= sizeof(widestring_t)+namesize)
	{
		widestring_t* pStr = (widestring_t*)pOut; 
		wcscpy(pStr->buffer,m_ImageBufferName);
		pStr->size = namesize;
		*pdwBytesWritten = sizeof(widestring_t)+namesize;
	
		return ERROR_SUCCESS;
	}

	return ERROR_INVALID_PARAMETER;
}



int  Driver::DevInit()
{
	
	int ret = 0;
		

	do
	{
			if (!OpenPipe())	
				break;
			
			const WCHAR wsSubClassRegKey[] = CLIENT_REGKEY_SZ;
			m_hStreamDevice = ActivateDevice (wsSubClassRegKey, (DWORD)this);
			

			DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Stream dev %08x"),m_hStreamDevice));

			if (!m_hStreamDevice)
				break;

			ret = m_lpUsbFuncs->lpRegisterNotificationRoutine (m_hDevice, USBDeviceNotificationCallback, this);
	
			DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "lpRegisterNotificationRoutine %d"),ret));

	}while(0);
	

	if (!ret)
	{
		if (m_hStreamDevice)
			DeactivateDevice (m_hStreamDevice);
		
		//USB_PIPE is freed by the destuctor
	}
	

	return ret;
}


void* Driver::AllocImageBuffer(DWORD size)
{
	
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Driver::AllocImageBuffer")));

	if (m_hImageBuffer)
		return 0;


	HANDLE hMapping = 0;
	wchar_t name[IMAGEBUFFER_NAME_SIZE];
	DWORD i;

	for (i = 0; i < 1000; ++i)
	{
		swprintf(name,L"SDM_IMAGE_BUFFER_%08x",i);

		hMapping = CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE,0,size,name);
		
		if (hMapping != 0)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				CloseHandle(hMapping);
			}
			else
				break;
		}

	}

	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "hMapping %08x i %d"),hMapping,i));

	if (i != 1000 && hMapping)
	{
		
		void* ptr = MapViewOfFile(hMapping,FILE_MAP_ALL_ACCESS,0,0,0);

		if (ptr)
		{
			m_hImageBuffer = hMapping;
			m_ImageBufferSize = size;
			wcscpy(m_ImageBufferName,name);
			
			DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "ImageName %s size %d"),m_ImageBufferName,m_ImageBufferSize));

			return ptr;
		}	
		
		CloseHandle(hMapping);

	}


	return 0;
}

void Driver::FreeImageBuffer(void* ptr)
{
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Driver::FreeImageBuffer")));

	if (ptr && m_hImageBuffer)
	{
			UnmapViewOfFile(ptr);
			CloseHandle(m_hImageBuffer);
			m_hImageBuffer = 0;
			m_ImageBufferSize = 0;;
			memset(m_ImageBufferName,0,sizeof(m_ImageBufferName));
	}
}


void Driver::DetachDevice()
{

	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT( "Driver::DetachDevice" )));
	
	m_bDetached = true;

	DeactivateDevice (m_hStreamDevice);
	m_hStreamDevice = 0;
	
	m_lpUsbFuncs->lpUnRegisterNotificationRoutine(m_hDevice, USBDeviceNotificationCallback, this);
	
	EnterCriticalSection(&m_Lock);

	//Free pipes here since they might not be valid outside of the detach function
	//m_adapter.Uninit();
	
	LeaveCriticalSection(&m_Lock);
	
	/*		
			We do not need to block here since the sole reason for blocking is to
			prevent the system from unloading the dll before we release the driver object.
			
			The behaviour of the system is to actually prevent such behavior by checking
			internal reference count on the driver object. Since we do not have access to this refcount
			the driver has its own refcount variable.
			
			The actual behaviour is as follows. The system refcount acts pretty much like our
			private refcount implementation. That is each time the driver is accessed its refcount is
			addrefed/released. The dll is loaded(addrefed) by at least twice once by the "USBManager" and once by
			ActivateDevice (DEVICE.EXE). When USBDetachCallback returns the USBManager probably unloads the dll by it mght still be
			reffed by DEVICE.EXE since there might be pending requests.
			In such case the dll is placed in a wait queue that is scanned by DEVICE.EXE to determine when it is safe to unload the object.
			Since our ref comes after the primary driver reffing we can be sure that when DEVICE.EXE finally decides to
			unload the module we no longer have a live object
	*/


	//Release();

	//WaitForSingleObject(m_hReleaseEvent,INFINITE);

}



//======================================================================
// USBDeviceNotificationCallback - This routine is called by the USB
// host controller when a USB event occurs that the driver needs to
// know about.
// 
BOOL Driver::USBDeviceNotificationCallback (	LPVOID lpvNotifyParameter, DWORD dwCode,
													LPDWORD* dwInfo1, LPDWORD* dwInfo2,
													LPDWORD* dwInfo3, LPDWORD* dwInfo4)
{
	if (dwCode == USB_CLOSE_DEVICE)
	{
		
		DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT( "Device detach" )));
		
		// Get the pointer to our device context.
		Driver* pDrv = (Driver*)lpvNotifyParameter;
		
		if (pDrv)			
		{

			pDrv->DetachDevice();
				
			pDrv->Release();
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT( "Exiting Device detach" )));

			return TRUE;
		}

		
	}

	return FALSE;
}



bool	Driver::OpenPipe()
{	
	
	DEBUGMSG(ZONE_USBLOAD,(DTAG TEXT("Driver::OpenPipe")));
	DEBUGMSG(ZONE_USBLOAD,(DTAG TEXT("hDevice %08X,lpUsbFuncs %08X,lpInterface %08X"),m_hDevice,m_lpUsbFuncs,0));
	

	bool ret = false;

	do
	{	


			LPCUSB_DEVICE devInfo = m_lpUsbFuncs->lpGetDeviceInfo(m_hDevice);
	
			DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("Getting DevInfo")));
	
			if (!devInfo)
				break;

			
			m_lpInterface = &devInfo->lpActiveConfig->lpInterfaces[0];	
		
			DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("Interface count %d"),devInfo->lpActiveConfig->dwNumInterfaces));
			DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("bConfigurationValue %d"),devInfo->lpActiveConfig->Descriptor.bConfigurationValue));
			DEBUGMSG(ZONE_USBLOAD,(DTAG TEXT("Endpoint count %d"),m_lpInterface->Descriptor.bNumEndpoints));
	

			const USB_ENDPOINT* lpEndpoint = m_lpInterface->lpEndpoints; 	

#ifdef DEBUG		
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("Endpoint size %d"),lpEndpoint->dwCount));
			DumpEndpointDescriptor(&lpEndpoint->Descriptor);
#endif
			
			m_inpipe = m_lpUsbFuncs->lpOpenPipe(m_hDevice,&m_lpInterface->lpEndpoints[0].Descriptor);	
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("Open inpipe %08x Err 0x%08x"),m_inpipe,GetLastError()));
			
			if (!m_inpipe)
				break;
						

			m_hCompletion = CreateEvent(NULL,true,false,NULL);
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("CreateEvent %08x Err 0x%08x"),m_hCompletion,GetLastError()));

			if (!m_hCompletion)
				break;

			ret = true;

	}while (0);


	if (!ret)
	{
		ClosePipe();
	}


	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("OpenPipes: inpipe %08x"),m_inpipe));

	
	return ret;
	
}

void	Driver::ClosePipe()
{
		
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Driver::ClosePipe" )));
		
	if (m_inpipe)
	{
			m_lpUsbFuncs->lpClosePipe(m_inpipe);
			m_inpipe = NULL;
	}
		
	if (m_hCompletion)
	{
			CloseHandle(m_hCompletion);
			m_hCompletion = NULL;
	}

	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Exiting Driver::ClosePipe" )));
}




int		Driver::BulkRead(unsigned char* bytes, int size,int timeout)
{
		
		DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Driver::BulkRead")));
	
		if (m_inpipe != NULL)
		{
			DWORD bytestransfered = 0;
			USB_ERROR err =0;
			
			
			__try 
			{
				

				USB_PIPE pipe = m_inpipe;
				DWORD flags = USB_IN_TRANSFER;
				
				
				DWORD ret = IssueBulkTransfer(	m_lpUsbFuncs,pipe,DummyNotifyRoutine,
												m_hCompletion,flags,
												bytes,0,size,&bytestransfered,timeout,&err );

				DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "ret %08x err %08x transfered" ),ret,err,bytestransfered));

				if (ret == ERROR_SUCCESS)
				{
					return bytestransfered;
				}
			
			}
			__except (EXCEPTION_EXECUTE_HANDLER) 
			{
				
			}

		}
		
		return -1;

}

DWORD WINAPI Driver::DummyNotifyRoutine(LPVOID lpvNotifyParameter)
{
	SetEvent((HANDLE)lpvNotifyParameter);
	return 1;
}



int Driver::WriteRegister(int Request,int Value,int Index,char* buffer,int len,int timeout)
{

	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "WriteRegister\n Request 0x%04x Value 0x%04x Index 0x%04x len 0x%04x\n" ),Request,Value,Index,len));

	USB_ERROR error = 0;
	DWORD dwTransferred = 0;
	
	USB_DEVICE_REQUEST request = {	(USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_VENDOR | USB_REQUEST_FOR_DEVICE), Request,Value,Index,len };

	DWORD dwRet = IssueVendorTransfer(m_lpUsbFuncs,m_hDevice,DummyNotifyRoutine,m_hCompletion,
									  (USB_OUT_TRANSFER | USB_SHORT_TRANSFER_OK),&request,buffer,0,&dwTransferred,timeout,&error);



	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "ret %08x err %08x transfered %08x" ),dwRet,error,dwTransferred));


	return (int)dwRet;
}

int Driver::ReadRegister(int Request,int Value,int Index,char* buffer,int len,int timeout)
{
	
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "ReadRegister\n Request 0x%04x Value 0x%04x Index 0x%04x len 0x%04x\n" ),Request,Value,Index,len));

	USB_ERROR error = 0;
	DWORD dwTransferred = 0;
	
	USB_DEVICE_REQUEST request = {	(USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_VENDOR | USB_REQUEST_FOR_DEVICE), Request,Value,Index,len };

	DWORD dwRet = IssueVendorTransfer(m_lpUsbFuncs,m_hDevice,DummyNotifyRoutine,m_hCompletion,
									  (USB_IN_TRANSFER |  USB_SHORT_TRANSFER_OK),&request,buffer,0,&dwTransferred,timeout,&error);


	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "ret %08x err %08x transfered %08x" ),dwRet,error,dwTransferred));


	return (int)dwRet;
}

int Driver::RunScript(const control_req_t* pScript, int nElements)
{
	int i;

	for (i = 0; i < nElements; ++i)
	{	
		char* pBuffer = 0;
		int   cbBuffer = 0;
		
		if ((pScript[i].Flags & FLAG_INPUT_BUFFER))
		{
			pBuffer = (char*)&(pScript[i].Data);
			cbBuffer = 1;
		}		

		if (0 > WriteRegister(pScript[i].Request,pScript[i].Value,pScript[i].Index,pBuffer,cbBuffer,5000))
			return -1;
	}
	
	return 0;
}


bool    Driver::FillImageBuffer()
{


	unsigned char* pBuffer = m_ImageBuffer;
	int read = 0;

	m_bHaveImage = false;

	for (int i = 0; i < 75; ++i)
	{
		read = BulkRead(pBuffer,0x1000,5000);
				
		if (read != 0x1000)
		{
			return false;	
		}

		pBuffer += 0x1000;

	}
	
	m_bHaveImage = true;

	return true;
}