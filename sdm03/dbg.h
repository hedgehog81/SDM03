#ifndef DBG_H
#define DBG_H

#ifdef __cplusplus
extern "C"
{
#endif

void dbg_print(const wchar_t* str, ...);
void dbg_printA(const char* str, ...);
void dbg_init(HINSTANCE hInst);
void dbg_fini(void);


#ifdef DEBUG
	#undef DEBUGMSG
	#define DEBUGMSG(x,y) dbg_print y
	#define DEBUGINIT dbg_init
	#define DEBUGFINI dbg_fini

#else
	
	#define DEBUGINIT
	#define DEBUGFINI void
	
#endif


#ifdef __cplusplus
}
#endif


#endif