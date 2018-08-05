#ifndef STREAMIF_H_
#define STREAMIF_H_


#ifdef __cplusplus
extern "C"
{
#endif


DWORD SDM_Init (DWORD dwContext);
BOOL  SDM_Deinit (DWORD dwContext);
DWORD SDM_Open (DWORD dwContext, DWORD dwAccess, DWORD dwShare);
BOOL  SDM_Close (DWORD dwOpen);
DWORD SDM_Read (DWORD dwOpen, LPVOID pBuffer, DWORD dwCount);
DWORD SDM_Write (DWORD dwOpen, LPVOID pBuffer, DWORD dwCount);
DWORD SDM_Seek (DWORD dwOpen, long lDelta, WORD wType);
DWORD SDM_IOControl (DWORD dwOpen, DWORD dwCode, 
                     PBYTE pIn, DWORD dwIn,
                     PBYTE pOut, DWORD dwOut, 
                     DWORD *pdwBytesWritten);
void SDM_PowerDown (DWORD dwContext);
void SDM_PowerUp (DWORD dwContext);




#ifdef __cplusplus
}
#endif


#endif