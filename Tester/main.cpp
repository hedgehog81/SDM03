#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "SDM03Reader.h"




typedef BOOL (*USBUnInstallDriver)(void);
typedef BOOL (*USBInstallDriver)(LPCWSTR szDriverLibFile);


static void DriverCntrl(const wchar_t* drv,bool install)
{
	
	HMODULE hMod = LoadLibrary(drv);
	
	if (hMod)
	{
		
		USBInstallDriver pInst = (USBInstallDriver)GetProcAddress(hMod,TEXT("USBInstallDriver"));
		USBUnInstallDriver pUnInst = (USBUnInstallDriver)GetProcAddress(hMod,TEXT("USBUnInstallDriver"));
		
		if (pInst && pUnInst)
		{
			BOOL bRet = false;

			if (install)
			{
				bRet = pInst(drv);
			}
			else
			{
				bRet = pUnInst();
			}

			printf("Ret = %d\r\n",bRet);

		}
		else
			printf("Unable to get entrypoints\r\n");

		FreeLibrary(hMod);
	}
	else
		printf("Unable to load %S\r\n",drv);

	
}


#define PIXEL(buffer,y,x,width) (buffer)[((y) * (width))+(x)] 



#if 1
static void WriteImage(const SDM03Image& img)
{
	printf("WriteImage\r\n");

	
	if (img.HasImage())
	{
		
		HANDLE hFile = CreateFile(L"img.bin",GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);	
		

		if (hFile != INVALID_HANDLE_VALUE)
		{
			
			DWORD written = 0;

			WriteFile(hFile,img.GetImageBits(),img.GetSize(),&written,NULL);
				
			
			CloseHandle(hFile);
		}

	
	}
	


}
#endif


static void ListDevices(void)
{
	const int MAX_DEVS = 10;
	unsigned long devlist[MAX_DEVS];

	int devcnt = SDM03Reader::GetDevices(devlist,MAX_DEVS);
	
	for (int i =0 ; i < devcnt; ++i)
	{
		printf("devid %d\r\n",devlist[i]);
	}
	
	
	printf("found %d device(s)\r\n",devcnt);
}



static void TestDriver(const wchar_t* name)
{
	int index = _wtoi(name);

	SDM03Reader reader;
	
	if (reader.Open(index) != ERROR_SUCCESS)
	{
		printf("unable to open the device!\r\n");
	}
	else
	{
		printf("opened %d\r\n",index);

	
		DWORD ret;

		for (int i =0 ; i < 10; ++i)
		{
			printf("Please scan a finger\r\n");
			
			DWORD ticks = GetTickCount();

			ret = reader.Capture();
				
			const SDM03Image& img = reader.GetImage();

			printf("ticks %d\r\n",GetTickCount()-ticks);

			printf("ret = %d\r\n" ,ret);

			if (!ret)
			{	
				
				printf("img info x %d y %d l %d\r\n",img.GetWidth(),img.GetHeight(),img.GetSize());
			
				if (img.GetSize() > 1)
				{
					WriteImage(img);
					break;
				}
			
			}

			
		}	
		
	
	}


}



int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	
	
	if (argc < 2)
	{
		printf("drvtest [-ls|-i|-u|-t] [dllname|devname]\r\n");
		return 1;
	}
	
	

	wchar_t cmd = *(argv[1]+1); 
	
	switch (cmd)
	{
	case TEXT('i'):
	case TEXT('u'):
			DriverCntrl(argv[2],cmd == TEXT('i'));
		break;
	
	case TEXT('t'):
			TestDriver(argv[2]);
		break;
	case TEXT('l'):
			ListDevices();
		break;
	
	}

	return 0;
}

