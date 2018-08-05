// LIBUSBAdapter.h: interface for the LIBUSBAdapter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIBUSBADAPTER_H__5DAA5DBC_41C7_456F_9185_89F0171C67E5__INCLUDED_)
#define AFX_LIBUSBADAPTER_H__5DAA5DBC_41C7_456F_9185_89F0171C67E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <usbclient.h>


class LIBUSBAdapter  
{
public:
	LIBUSBAdapter();							
	~LIBUSBAdapter();

	DWORD Init(HANDLE hDevice,LPCUSB_FUNCS lpUsbFuncs,
								LPCUSB_INTERFACE lpInterface);
	
	void Uninit();

	int ClaimInterface(int id);
	int ReleaseInterface(int id);

	int BulkReadWrite(int ep, char *bytes, int size,int timeout,bool read);

private:
	LIBUSBAdapter(const LIBUSBAdapter& other);
	LIBUSBAdapter& operator=(const LIBUSBAdapter& other);
	
	bool OpenPipes();
	void ClosePipes();
	
	static DWORD WINAPI DummyNotifyRoutine(LPVOID lpvNotifyParameter);

	bool IsOk() const {return (m_outpipe && m_inpipe);};
private:
	HANDLE m_hDevice;
	LPCUSB_FUNCS m_lpFunc;
	LPCUSB_INTERFACE m_lpInterface;
	
	USB_PIPE m_outpipe;
	USB_PIPE m_inpipe;
	
	HANDLE m_hCompletion;

};

#endif // !defined(AFX_LIBUSBADAPTER_H__5DAA5DBC_41C7_456F_9185_89F0171C67E5__INCLUDED_)
