#if !defined(AFX_AES2501DRIVER_H__D8648034_D57F_44FA_A8B7_7172CF1252AF__INCLUDED_)
#define AFX_AES2501DRIVER_H__D8648034_D57F_44FA_A8B7_7172CF1252AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class Driver 
{
public:
	
	
	DWORD Open();
	DWORD Close();
	DWORD IOControl(DWORD dwCode, PBYTE pIn, DWORD dwIn,
					 PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten);
	

	static Driver* Create(	HANDLE hDevice,LPCUSB_FUNCS lpUsbFuncs,
									LPCUSB_INTERFACE lpInterface, LPCWSTR szUniqueDriverId,
									LPCUSB_DRIVER_SETTINGS lpDriverSettings);
	
	void AddRef();
	void Release();
	

private:
	
	enum {FLAG_INPUT_BUFFER = 1};
	
	enum
	{
		IMAGE_HEIGHT = 480,
		IMAGE_WIDTH  = 640,
		IMAGE_REAL_HEIGHT = 300,
		IMAGE_REAL_WIDTH  = 260
	};
	

	typedef struct control_req
	{
		int Request;
		int Value;
		int Index;
		int Data;
		int Flags;
	} control_req_t;




	~Driver();
	Driver(const Driver& other);
	Driver& operator=(const Driver& other);
	
	
	Driver(	HANDLE hDevice,LPCUSB_FUNCS lpUsbFuncs,
					LPCUSB_INTERFACE lpInterface, LPCWSTR szUniqueDriverId,
					LPCUSB_DRIVER_SETTINGS lpDriverSettings);
	
	DWORD IsFingerPresent(	PBYTE pIn, DWORD dwIn,
							PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten);
	DWORD  Capture(	PBYTE pIn, DWORD dwIn,
					PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten);
	DWORD  GetImage(	PBYTE pIn, DWORD dwIn,
					PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten);

	DWORD  GetBufferName(PBYTE pIn, DWORD dwIn,
						 PBYTE pOut, DWORD dwOut, DWORD *pdwBytesWritten);
	
	
	int  DevInit();
	
	void ResetImage();
	
	void DetachDevice();


	static BOOL USBDeviceNotificationCallback (	LPVOID lpvNotifyParameter, DWORD dwCode,
												LPDWORD* dwInfo1, LPDWORD* dwInfo2,
												LPDWORD* dwInfo3, LPDWORD* dwInfo4);
	
	static	DWORD WINAPI DummyNotifyRoutine(LPVOID lpvNotifyParameter);

	void* AllocImageBuffer(DWORD size);
	void FreeImageBuffer(void* ptr);


	bool	OpenPipe();
	void	ClosePipe();
	int		BulkRead(unsigned char* bytes, int size,int timeout);
	int		WriteRegister(int Request,int Value,int Index,char* buffer,int len,int timeout);
	int		ReadRegister(int Request,int Value,int Index,char* buffer,int len,int timeout);
	int		RunScript(const control_req_t* pScript, int nElements);


	DWORD     __IsFingerPresent(bool* pPresent);
	bool    FillImageBuffer();	


private:
	


	static const control_req_t init_script[];
	static const control_req_t uninit_script[];
	static const control_req_t capture_script[];
	static const control_req_t stop_capture_script[];



	HANDLE			 m_hStreamDevice;
	
	USB_HANDLE		 m_hDevice;
	LPCUSB_FUNCS	 m_lpUsbFuncs;
	USB_PIPE		 m_inpipe;
	LPCUSB_INTERFACE m_lpInterface;
	HANDLE			 m_hCompletion;
	
	
	CRITICAL_SECTION m_Lock;
	unsigned char*   m_ImageBuffer;
	bool			 m_bHaveImage;
		
	enum {IMAGEBUFFER_NAME_SIZE = 128};

	HANDLE			 m_hImageBuffer;
	DWORD			 m_ImageBufferSize;
	wchar_t			 m_ImageBufferName[IMAGEBUFFER_NAME_SIZE];

	LONG			 m_RefCount;

	bool			 m_bDetached;
	bool			 m_bOpen;
	

};

#endif // !defined(AFX_AES2501DRIVER_H__D8648034_D57F_44FA_A8B7_7172CF1252AF__INCLUDED_)
