/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * yuv_datastructs.h - SPU argument structure
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 *
 * $Id$
 */

#ifndef __YUV_DATASTRUCTS_H
#define __YUV_DATASTRUCTS_H

struct img_args {
	int width;
	int height;
	int maxwidth;
	int maxheight;
 	unsigned long long  Ystart;
	unsigned long long  Ustart;
	unsigned long long  Vstart;
	unsigned long long  Output[2];
	unsigned long long  dummy; // to achive automatic 128 bit alligment
};

#endif

