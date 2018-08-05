#ifndef DEFINES_H_
#define DEFINES_H_




#if 0
#ifdef DEBUG
#undef DEBUGMSG
#define DEBUGMSG(cond,printf_exp)   \
   ((void)((cond)?(wprintf printf_exp),1:0))
#endif
#endif


#define dim(a)  (sizeof(a)/sizeof(a[0]))
//
// Debug zone support
//
// Used as a prefix string for all debug zone messages.
#define DTAG        TEXT ("SDM03: ")

// Debug zone constants
#define ZONE_ERROR      DEBUGZONE(0)
#define ZONE_WARNING    DEBUGZONE(1)
#define ZONE_FUNC       DEBUGZONE(2)
#define ZONE_INIT       DEBUGZONE(3)
#define ZONE_DRVCALLS   DEBUGZONE(4)
#define ZONE_EXENTRY  (ZONE_FUNC | ZONE_DRVCALLS)
#define ZONE_USBLOAD    DEBUGZONE(5)
#define ZONE_THREAD     DEBUGZONE(6)
#define ZONE_THSTATE	DEBUGZONE(7)



//
// Our current version
//
#define DRV_MAJORVER   1
#define DRV_MINORVER   0
//
// USB constants
//

#define DRIVER_NAME   TEXT("SDM03.dll")

#define DEVICE_PREFIX TEXT("SDM")

#define CLASS_NAME_SZ    TEXT("SDM03")
#define CLIENT_REGKEY_SZ TEXT("Drivers\\USB\\ClientDrivers\\SDM03")




#endif