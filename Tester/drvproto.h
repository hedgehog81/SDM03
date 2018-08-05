#ifndef DRVPROTO_H
#define DRVPROTO_H


enum IOCODES
{
	IOCTRL_GET_BUFFER_NAME = 1,
	IOCTRL_CHK_FINGER,
	IOCTRL_CAPTURE,
	IOCTRL_GET_IMAGE,
};


#pragma warning(push)
#pragma warning(disable : 4200)

struct widestring_t
{
	unsigned int size;
	wchar_t buffer[0];
};



#pragma warning(pop)


struct imageinfo_t
{
	unsigned int res;
	unsigned int flags;
	unsigned int x;
	unsigned int y;
	unsigned real_x;
	unsigned real_y;
} ;




#endif