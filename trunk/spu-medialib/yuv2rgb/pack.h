/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * pack.h - YUV vector unpacking helper functions
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 *
 * $Id$
 */

#ifndef __PACK_H
#define __PACK_H

static inline vector float unpackhh(vector unsigned char a) {
	return (spu_convtf((vector unsigned int)spu_shuffle(a,((vector unsigned char){0}),((vector unsigned char){16,16,16,0,16,16,16,1,16,16,16,2,16,16,16,3})),0));
}

static inline vector float unpacklh(vector unsigned char a) {
	return (spu_convtf((vector signed int)spu_shuffle(a,((vector unsigned char){0}),((vector unsigned char){16,16,16,4,16,16,16,5,16,16,16,6,16,16,16,7})),0));
}

static inline vector float unpackhl(vector unsigned char a) {
	return (spu_convtf((vector signed int)spu_shuffle(a,((vector unsigned char){0}),((vector unsigned char){16,16,16,8,16,16,16,9,16,16,16,10,16,16,16,11})),0));
}

static inline vector float unpackll(vector unsigned char a) {
	return (spu_convtf((vector signed int)spu_shuffle(a,((vector unsigned char){0}),((vector unsigned char){16,16,16,12,16,16,16,13,16,16,16,14,16,16,16,15})),0));
}


static inline vector float unpackfaabb(vector float a) {
	return (spu_shuffle(a,a,((vector unsigned char){0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7})));
}

static inline vector float unpackfccdd(vector float a) {
	return (spu_shuffle(a,a,((vector unsigned char){8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15})));
}

#endif

