#ifndef FP_INTERNAL_H
#define FP_INTERNAL_H

#include <stdlib.h>
#include "usb.h"
#include "dbg.h"

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned int gboolean;

#ifdef __cplusplus
extern "C"
{
#endif


enum fpi_log_level {
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
};



#define fp_dbg //dbg_printA
#define fp_info //dbg_printA
#define fp_warn //dbg_printA
#define fp_err //dbg_printA


struct fp_dev {
	struct fp_driver *drv;
	usb_dev_handle *udev;
	uint32_t devtype;
	void *priv;

	int nr_enroll_stages;

	/* drivers should not mess with these */
	int __enroll_stage;
};

struct fp_img_dev {
	struct fp_dev *dev;
	usb_dev_handle *udev;
	void *priv;
};


struct fp_img_driver {

	int (*init)(struct fp_img_dev *dev, unsigned long driver_data);
	void (*exit)(struct fp_img_dev *dev);
	int (*await_finger_on)(struct fp_img_dev *dev);
	int (*await_finger_off)(struct fp_img_dev *dev);
	int (*capture)(struct fp_img_dev *dev, gboolean unconditional,
		struct fp_img **image);
};


/* bit values for fp_img.flags */
#define FP_IMG_V_FLIPPED 		(1<<0)
#define FP_IMG_H_FLIPPED 		(1<<1)
#define FP_IMG_COLORS_INVERTED	(1<<2)
#define FP_IMG_BINARIZED_FORM	(1<<3)

#define FP_IMG_STANDARDIZATION_FLAGS (FP_IMG_V_FLIPPED | FP_IMG_H_FLIPPED \
	| FP_IMG_COLORS_INVERTED)

#pragma warning(push)
#pragma warning(disable : 4200)

struct fp_img {
	int width;
	int height;
	size_t length;
	uint16_t flags;
	struct fp_minutiae *minutiae;
	unsigned char *binarized;
	unsigned char data[0];
};

#pragma warning(pop)


struct fp_img *fpi_img_new(struct fp_img_dev* pDev,size_t length);
void   fp_img_free(struct fp_img_dev* pDev,struct fp_img* pImg);



#ifdef __cplusplus
}
#endif


#endif