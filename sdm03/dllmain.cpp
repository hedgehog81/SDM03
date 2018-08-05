#include "stdafx.h"
#include "register.h"


#ifdef DEBUG

DBGPARAM dpCurSettings = 
{
	TEXT("SDM03"), 
	{
		TEXT("Errors"),TEXT("Warnings"),TEXT("Functions"), TEXT("Init"),
		TEXT("Driver Calls"),TEXT("USBLoad"),TEXT("Thread"),TEXT("Frame Info"), 
	},
	0x000F
};
#endif //DEBUG


HINSTANCE g_hInst = NULL;



BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_hInst = (HINSTANCE)hModule;
		DEBUGREGISTER(g_hInst);
		DEBUGINIT(g_hInst);
		DisableThreadLibraryCalls(g_hInst);
		DEBUGMSG(ZONE_INIT,(DTAG TEXT( "DLL_PROCESS_ATTACH" )));
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		DEBUGMSG(ZONE_INIT,(DTAG TEXT( "DLL_PROCESS_DETACH" )));
		DEBUGFINI();
	}
	
	return TRUE;
}

