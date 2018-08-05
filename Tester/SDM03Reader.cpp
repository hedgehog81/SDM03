// SDM03Reader.cpp: implementation of the SDM03Reader class.
//
//////////////////////////////////////////////////////////////////////

#include "SDM03Reader.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include "drvproto.h"






int SDM03Reader::GetDevices(unsigned long* pList,unsigned long max_devs)
{
	
	assert(pList);
	assert(max_devs);

	int devcount = 0;
	const WCHAR* KEY_PATH = TEXT("Drivers\\Active");
	HKEY  hKey = 0;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE,KEY_PATH,0,0,&hKey);

	if (hKey)
	{
		
		int i = 0;
		const int MAX_NAME = 128;
		WCHAR  name[MAX_NAME+1];
		unsigned long namelen;
		HKEY hSubkey;
		
		while (devcount < max_devs)
		{

			namelen = MAX_NAME;
			if (ERROR_SUCCESS  != RegEnumKeyEx(hKey,i++,name,&namelen,0,0,0,0))
				break;
			
			name[namelen] = 0;

			//printf("key name = %S\r\n",name);
			
			hSubkey = 0;
			
			RegOpenKeyEx(hKey,name,0,0,&hSubkey);

			if (hSubkey)
			{
				namelen = (MAX_NAME * sizeof(WCHAR));
				
				if (ERROR_SUCCESS == RegQueryValueEx(hSubkey,TEXT("Name"),0,NULL,(LPBYTE)name,&namelen))
				{
					//printf("Driver %S\r\n",name);
					
					if (name[0] == TEXT('S') && name[1] == TEXT('D') && name[2] == TEXT('M'))
					{
						//printf("Found fingerprint device..\r\n");
						pList[devcount++] = _wtoi(name+3);
					}
				}
			
				
				RegCloseKey(hSubkey);
			}

		}

		RegCloseKey(hKey);
	}

	return devcount;


}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SDM03Reader::SDM03Reader()
{
	m_hDevice = INVALID_HANDLE_VALUE;
}

SDM03Reader::~SDM03Reader()
{
	Close();
}


DWORD SDM03Reader::Open(int index)
{
		
	Close();
	
	wchar_t name[20];
	memset(name,0,sizeof(name));
	
	swprintf(name,L"SDM%d:",index);

	HANDLE hDrv = CreateFile(name,(GENERIC_READ | GENERIC_WRITE),
											0,0,OPEN_EXISTING,0,0);


	if (hDrv == INVALID_HANDLE_VALUE)
		return GetLastError();
	
	
	if (!m_image.Open(hDrv))
	{
		CloseHandle(hDrv);
		return ERROR_GEN_FAILURE;
	}


	m_hDevice = hDrv;
	return ERROR_SUCCESS;
}

void SDM03Reader::Close()
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
		m_image.Close();
	}
}

DWORD SDM03Reader::Capture()
{
	DWORD dwRet = ERROR_GEN_FAILURE;
	DWORD written = 0;
	
	memset(&m_image.m_imageinfo,0,sizeof(m_image.m_imageinfo));
	
	assert(m_hDevice != INVALID_HANDLE_VALUE);
	
	if (m_hDevice == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_PARAMETER;


	if( DeviceIoControl(m_hDevice,IOCTRL_CAPTURE,NULL,0,&m_image.m_imageinfo,sizeof(m_image.m_imageinfo),
								&written,NULL) )
	{
		
		dwRet = ERROR_SUCCESS;

	}
	else
		dwRet = GetLastError();

	
	return dwRet;
	
}



SDM03Image::SDM03Image()
{
	m_hMapping = 0;
	m_pImage = 0;
}

SDM03Image::~SDM03Image()
{

}

bool SDM03Image::Open(HANDLE hDevice)
{
	
	//printf("SDM03Image::Open\r\n");

	bool ret = false;

	assert(hDevice != INVALID_HANDLE_VALUE);

	if (hDevice != INVALID_HANDLE_VALUE)
	{
		size_t mysize = sizeof(widestring_t) + 129 * sizeof(wchar_t);
		widestring_t* pStr = (widestring_t*)calloc(1,mysize);
		DWORD dwWritten = 0;
		
		if (pStr)
		{
			//printf("DeviceIoControl\r\n");

			if (DeviceIoControl(hDevice,IOCTRL_GET_BUFFER_NAME,0,0,pStr,mysize,&dwWritten,0))
			{
				//printf("got %d name %S\r\n",dwWritten,pStr->buffer);
				
				m_hMapping = CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READONLY,0,0,pStr->buffer);
				
				if (m_hMapping)
				{
					//printf("m_hMapping %08x\r\n",m_hMapping);

					m_pImage = (unsigned char*)MapViewOfFile(m_hMapping,FILE_MAP_READ,0,0,0);

					//printf("m_pImage %08x\r\n",m_pImage);

					if (!m_pImage)
					{
						CloseHandle(m_hMapping);
						m_hMapping = 0;
					}
					else
						ret = true;
				}
			
			}

			free(pStr);
		}
	}
	
	return ret;
}	

void SDM03Image::Close()
{
	if (IsOk())
	{
		UnmapViewOfFile(m_pImage);
		CloseHandle(m_hMapping);
		m_hMapping = 0;
		m_pImage = 0;
	}
}


DWORD SDM03Image::GetRealWidth() const
{
	if (IsOk())
	{
		return m_imageinfo.real_x;
	}

	return 0;
}

DWORD SDM03Image::GetRealHeight() const
{
	if (IsOk())
	{
		return m_imageinfo.real_y;
	}

	return 0;
}


DWORD SDM03Image::GetWidth() const
{
	if (IsOk())
	{
		return m_imageinfo.x;
	}

	return 0;
}

DWORD SDM03Image::GetHeight() const
{
	if (IsOk())
	{
		return m_imageinfo.y;
	}

	return 0;
}

DWORD SDM03Image::GetSize() const
{

	if (IsOk())
	{
		return GetHeight() * GetWidth();
	}

	return 0;
}

DWORD SDM03Image::GetFlags() const
{
	if (IsOk())
	{
		return m_imageinfo.flags;
	}

	return 0;
}
		

const unsigned char* SDM03Image::GetImageBits() const
{
	if (IsOk())
	{
		return m_pImage;
	}

	return 0;
}

		
