#include "stdafx.h"
#include "fp_internal.h"
#include "driver.h"

struct fp_img *fpi_img_new(struct fp_img_dev* pDev, size_t length)
{
	//fp_img* pImg = (fp_img *)VirtualAlloc(0,length+sizeof(fp_img),MEM_COMMIT,PAGE_READWRITE);
	
	AES2501Driver* pDrv = static_cast<AES2501Driver*>(pDev);

	fp_img* pImg = (fp_img*)pDrv->AllocImageBuffer(length+sizeof(fp_img));
		
	if (pImg != 0)
	{
		pImg->length = length;
	}

	return pImg; 
}

void   fp_img_free(struct fp_img_dev* pDev,struct fp_img* pImg)
{
	AES2501Driver* pDrv = static_cast<AES2501Driver*>(pDev);
	pDrv->FreeImageBuffer(pImg);
	//VirtualFree(pImg,0,MEM_RELEASE);
}