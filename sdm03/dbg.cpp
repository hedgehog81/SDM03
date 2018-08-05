#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <tchar.h>
#include "dbg.h"

static CRITICAL_SECTION s_lock;
static HANDLE s_hFile = INVALID_HANDLE_VALUE;

void dbg_init(HINSTANCE hInst)
{
	InitializeCriticalSection(&s_lock);
	
	
	HMODULE hMod = (HMODULE)hInst;
	TCHAR path[MAX_PATH+1];
	DWORD ret = GetModuleFileName(hMod,path,MAX_PATH);
	
	if (ret != 0)
	{
		DWORD i = 0;

		path[ret] = 0x00;

		for (i = ret; i >= 0 ; --i)
		{
			if (path[i] == TEXT('\\'))
			{	
				path[i+1] = 0x00;
				_tcscat(path,TEXT("log.txt"));
				break;
			}
		}
		
		if (i != ret && i != 0)
		{
	
			s_hFile = CreateFile(path,GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_ALWAYS,0,0);
	
			if (s_hFile != INVALID_HANDLE_VALUE)
				SetFilePointer(s_hFile,0,0,FILE_END);
		}
	}
}		

void dbg_fini(void)
{
	if (s_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(s_hFile);
	
	DeleteCriticalSection(&s_lock);
}


static void Lock(void)
{
	EnterCriticalSection(&s_lock);
}

static void Unlock(void)
{
	LeaveCriticalSection(&s_lock);
}

/*
void dbg_print(const wchar_t* str, ...)
{
	HMODULE hMod = (HMODULE)g_hInst;
	TCHAR path[MAX_PATH+1];
	DWORD ret = GetModuleFileName(hMod,path,MAX_PATH);
	
	if (ret != 0)
	{
		DWORD i = 0;

		path[ret] = 0x00;

		for (i = ret; i >= 0 ; --i)
		{
			if (path[i] == TEXT('\\'))
			{	
				path[i+1] = 0x00;
				_tcscat(path,TEXT("log.txt"));
				break;
			}
		}
		
		if (i != ret && i != 0)
		{

			HANDLE hFile = CreateFile(path,GENERIC_WRITE,0,0,OPEN_ALWAYS,0,0);

			if (hFile != INVALID_HANDLE_VALUE)
			{

				TCHAR w_string[1024];
				char  s_string[1024];
				va_list params;
				DWORD dwWritten = 0;

				SYSTEMTIME tm = {0};
				
				SetFilePointer(hFile,0,0,FILE_END);
				
				GetLocalTime(&tm);

				ret = sprintf(s_string,"%02d/%02d/%04d %02d:%02d:%02d ",tm.wDay,tm.wMonth,tm.wYear,
																		tm.wHour,tm.wMinute,tm.wSecond);
				
				
				WriteFile(hFile,(void*)s_string,ret,&dwWritten,NULL);

				va_start(params,str);
				_vstprintf(w_string,str,params);
				va_end(params);
				
				ret = wcstombs(s_string,w_string,sizeof(s_string));
				
				WriteFile(hFile,(void*)s_string,ret,&dwWritten,NULL);
				
				s_string[0] = '\r';
				s_string[1] = '\n';
					
				WriteFile(hFile,(void*)s_string,2,&dwWritten,NULL);

				CloseHandle(hFile);
			}


		}



	}
	
	

}

*/




static void outputdbgstringA(const char* str)
{
#if 0	
	HMODULE hMod = (HMODULE)g_hInst;
	TCHAR path[MAX_PATH+1];
	DWORD ret = GetModuleFileName(hMod,path,MAX_PATH);
	
	if (ret != 0)
	{
		DWORD i = 0;

		path[ret] = 0x00;

		for (i = ret; i >= 0 ; --i)
		{
			if (path[i] == TEXT('\\'))
			{	
				path[i+1] = 0x00;
				_tcscat(path,TEXT("log.txt"));
				break;
			}
		}
		
		if (i != ret && i != 0)
		{
#endif
			//HANDLE hFile = CreateFile(path,GENERIC_WRITE,0,0,OPEN_ALWAYS,0,0);

			if (s_hFile != INVALID_HANDLE_VALUE)
			{
						char  s_string[128];
						
						DWORD dwWritten = 0;

						SYSTEMTIME tm = {0};
						
						GetLocalTime(&tm);

						DWORD ret = sprintf(s_string,"%02d/%02d/%04d %02d:%02d:%02d (%d) ",tm.wDay,tm.wMonth,tm.wYear,
																				tm.wHour,tm.wMinute,tm.wSecond,GetTickCount());	
						Lock();

						//SetFilePointer(hFile,0,0,FILE_END);

						WriteFile(s_hFile,(void*)s_string,ret,&dwWritten,NULL);

						WriteFile(s_hFile,(void*)str,strlen(str),&dwWritten,NULL);
						
						s_string[0] = '\r';
						s_string[1] = '\n';
							
						WriteFile(s_hFile,(void*)s_string,2,&dwWritten,NULL);
						
						Unlock();

						//CloseHandle(hFile);


			}
#if 0		
		}
	
	}
#endif

}


static void outputdbgstring(const wchar_t* str)
{	
	char s_string[1024];
	int ret = wcstombs(s_string,str,sizeof(s_string));
	outputdbgstringA(s_string);
}


void dbg_print(const wchar_t* str, ...)
{

	wchar_t w_string[1024];
	va_list params;

	va_start(params,str);
	_vstprintf(w_string,str,params);
	va_end(params);	

	outputdbgstring(w_string);
	

}

void dbg_printA(const char* str, ...)
{

	char string[1024];
	va_list params;

	va_start(params,str);
	sprintf(string,str,params);
	va_end(params);	

	outputdbgstringA(string);
}





