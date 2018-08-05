// SDM03Reader.h: interface for the SDM03Reader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SDM03READER_H__88854BA0_F1B7_4562_96D5_3F2B1D90121E__INCLUDED_)
#define AFX_SDM03READER_H__88854BA0_F1B7_4562_96D5_3F2B1D90121E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windef.h>
#include "drvproto.h"


class SDM03Reader;

class SDM03Image
{

friend class SDM03Reader;

public:
		
		DWORD GetWidth() const;
		DWORD GetHeight() const;
		DWORD GetSize() const;
		DWORD GetFlags() const;
		
		DWORD GetRealWidth() const;
		DWORD GetRealHeight() const;

		bool HasImage() const {return GetWidth() > 0 && GetHeight() > 0;};

		const unsigned char* GetImageBits() const;
		
private:
		SDM03Image(const SDM03Image& other);
		SDM03Image& operator=(const SDM03Image& other);
				
		SDM03Image();
		~SDM03Image();
		
		bool Open(HANDLE hDevice);
		void Close();
		
		bool IsOk() const {return m_pImage != 0;};

private:
	
	imageinfo_t    m_imageinfo;
	unsigned char* m_pImage;
	HANDLE  m_hMapping;
};


class SDM03Reader  
{
public:
	SDM03Reader();
	~SDM03Reader();

	DWORD Open(int index);
	void Close();

	DWORD Capture();
	const SDM03Image& GetImage() {return m_image;};

	static int GetDevices(unsigned long* pList,unsigned long max_devs);

private:
	SDM03Reader(const SDM03Reader& other);
	SDM03Reader& operator=(const SDM03Reader& other);

private:
	HANDLE m_hDevice;
	SDM03Image m_image; 

};

#endif // !defined(AFX_SDM03READER_H__88854BA0_F1B7_4562_96D5_3F2B1D90121E__INCLUDED_)
