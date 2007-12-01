#ifndef __DATA_2D_H
#define __DATA_2D_H

#include <spu-medialib/spu_medialib_types.h>

struct data_2 {
	int alu;
	int operation;
	int pitchIn;
	int pitchOut;
	int width;
	int height;

	int dx;
	int dy;

	int x1;
	int y1;
	int x2;
	int y2;

	int bppI;
	int bppO;
	
	int src_x;
	int src_y;
	int dst_x;
	int dst_y;

	Spu_Pixel Fg;
	Spu_Pixel planemask;
	
	addr_t Inp;
	addr_t Outp;

} __attribute__((aligned(128)));

#endif
