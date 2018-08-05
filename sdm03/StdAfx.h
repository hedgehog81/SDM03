// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__ACE436DC_BF31_443F_9AB9_E26C9748F778__INCLUDED_)
#define AFX_STDAFX_H__ACE436DC_BF31_443F_9AB9_E26C9748F778__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include "LIBUSBAdapter.h"
#include <USBdi.h>					// USB includes
#include <usb100.h>					// USB includes
#include <usbclient.h>				// USB client driver helper code


#include "dbg.h"
#include "defines.h"


extern HINSTANCE g_hInst;

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__ACE436DC_BF31_443F_9AB9_E26C9748F778__INCLUDED_)
