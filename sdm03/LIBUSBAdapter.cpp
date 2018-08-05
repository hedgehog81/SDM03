// LIBUSBAdapter.cpp: implementation of the LIBUSBAdapter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//FIXME
typedef unsigned long usb_dev_handle;


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

LIBUSBAdapter::LIBUSBAdapter()
{
	m_hDevice = NULL;
	m_lpFunc = NULL;
	m_lpInterface = NULL;
	m_outpipe = NULL;
	m_inpipe = NULL;
	m_hCompletion = NULL;
}


DWORD LIBUSBAdapter::Init(HANDLE hDevice,LPCUSB_FUNCS lpUsbFuncs,
								LPCUSB_INTERFACE lpInterface)
{

	m_hDevice = hDevice;
	m_lpFunc = lpUsbFuncs;
	
	
	DEBUGMSG(ZONE_USBLOAD,(DTAG TEXT("LIBUSBAdapter::Init")));
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("this %08x"),this));
	DEBUGMSG(ZONE_USBLOAD,(DTAG TEXT("hDevice %08X,lpUsbFuncs %08X,lpInterface %08X"),hDevice,lpUsbFuncs,lpInterface));
	
	LPCUSB_DEVICE devInfo = lpUsbFuncs->lpGetDeviceInfo(hDevice);
	
	DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("Getting DevInfo")));
	
	if (devInfo)
	{
		
		m_lpInterface = &devInfo->lpActiveConfig->lpInterfaces[0];	
		
		DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("Interface count %d"),devInfo->lpActiveConfig->dwNumInterfaces));
		DEBUGMSG (ZONE_USBLOAD, (DTAG TEXT("bConfigurationValue %d"),devInfo->lpActiveConfig->Descriptor.bConfigurationValue));
		DEBUGMSG(ZONE_USBLOAD,(DTAG TEXT("Endpoint count %d"),m_lpInterface->Descriptor.bNumEndpoints));
	
		return OpenPipes();
	}
	
	return 0;
	
}


void LIBUSBAdapter::Uninit()
{
	ClosePipes();
}

LIBUSBAdapter::~LIBUSBAdapter()
{
	ClosePipes();
}

bool LIBUSBAdapter::OpenPipes()
{
	
	bool ret = false;	

	if (!IsOk())
	{
		
		
		do
		{
			const USB_ENDPOINT* lpEndpoint = m_lpInterface->lpEndpoints; 	

#ifdef DEBUG		
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("Endpoint size %d"),lpEndpoint->dwCount));
			DumpEndpointDescriptor(&lpEndpoint->Descriptor);
#endif
			
			m_inpipe = m_lpFunc->lpOpenPipe(m_hDevice,&m_lpInterface->lpEndpoints[0].Descriptor);	
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("Open inpipe %08x Err 0x%08x"),m_inpipe,GetLastError()));
			
			if (!m_inpipe)
				break;
			
			
			lpEndpoint = (const USB_ENDPOINT*)((LPBYTE)lpEndpoint + lpEndpoint->dwCount); 	

#ifdef DEBUG
			DumpEndpointDescriptor(&lpEndpoint->Descriptor);	
#endif

			m_outpipe = m_lpFunc->lpOpenPipe(m_hDevice,&lpEndpoint->Descriptor);	
			

			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("Open outpipe %08x Err 0x%08x"),m_outpipe,GetLastError()));

			if (!m_outpipe)
				break;
			

			m_hCompletion = CreateEvent(NULL,true,false,NULL);
			
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("CreateEvent %08x Err 0x%08x"),m_hCompletion,GetLastError()));

			if (!m_hCompletion)
				break;

			ret = true;

		}while (0);


		if (!ret)
		{
			ClosePipes();
		}


		DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("OpenPipes: inpipe %08x outpipe %08x"),m_inpipe,m_outpipe));
	
	}
	
	return ret;

}

void LIBUSBAdapter::ClosePipes()
{
		DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "LIBUSBAdapter::ClosePipes" )));

		if (m_outpipe)
		{
			m_lpFunc->lpClosePipe(m_outpipe);
			m_outpipe = NULL;
		}
		
		if (m_inpipe)
		{
			m_lpFunc->lpClosePipe(m_inpipe);
			m_inpipe = NULL;
		}
		
		if (m_hCompletion)
		{
			CloseHandle(m_hCompletion);
			m_hCompletion = NULL;
		}

		DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "Exiting LIBUSBAdapter::ClosePipes" )));
}


int LIBUSBAdapter::ClaimInterface(int id)
{
	DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("ClaimInterface")));
	return IsOk();
}

int LIBUSBAdapter::ReleaseInterface(int id)
{
	return IsOk();
}



DWORD WINAPI LIBUSBAdapter::DummyNotifyRoutine(LPVOID lpvNotifyParameter)
{
	//DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "LIBUSBAdapter::DummyNotifyRoutine %08x" ),lpvNotifyParameter));
	SetEvent((HANDLE)lpvNotifyParameter);
	return 1;
}


int LIBUSBAdapter::BulkReadWrite( int ep, char *bytes, int size,int timeout,bool read)
{
	DEBUGMSG(ZONE_FUNC,(DTAG TEXT( "LIBUSBAdapter::BulkReadWrite read %d" ),read));
	
	if (IsOk())
	{
		DWORD bytestransfered = 0;
		USB_ERROR err =0;
		
		
		__try 
		{
			

			USB_PIPE pipe;
			DWORD flags;

			if (read)
			{
				pipe = m_inpipe;
				flags = USB_IN_TRANSFER;
			}	
			else
			{
				pipe = m_outpipe;
				flags = USB_OUT_TRANSFER;
			}
			
			DWORD ret = IssueBulkTransfer(	m_lpFunc,pipe,DummyNotifyRoutine,
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




extern "C"
{

	int usb_bulk_write(usb_dev_handle *dev, int ep, char *bytes, int size,
						 int timeout)
	{
			return ((LIBUSBAdapter*)dev)->BulkReadWrite(ep,bytes,size,timeout,false);
	}

	int usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size,
						int timeout)
	{
			return ((LIBUSBAdapter*)dev)->BulkReadWrite(ep,bytes,size,timeout,true);
	}


	int usb_claim_interface(usb_dev_handle *dev, int inf)
	{
			DEBUGMSG(ZONE_DRVCALLS,(DTAG TEXT("usb_claim_interface this %08x"),dev));
			return ((LIBUSBAdapter*)dev)->ClaimInterface(inf);
	}

	int usb_release_interface(usb_dev_handle *dev, int inf)
	{
			return ((LIBUSBAdapter*)dev)->ReleaseInterface(inf);
	}


	int usb_set_configuration(usb_dev_handle *dev, int configuration)
	{
		return 0;
	}

	struct usb_device *usb_device(usb_dev_handle *dev)
	{
		return 0;
	}

}