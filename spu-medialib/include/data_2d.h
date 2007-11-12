#ifndef __DATA_2D_H
#define __DATA_2D_H

#include <stdint.h>

enum ALU { SPUcopy, SPUxor, SPUinvert, SPUor, SPUclear, SPUand, SPUandReverse, SPUnoop, SPUnor, SPUequiv, SPUorReverse, SPUcopyInverted, SPUorInverted, SPUnand, 	   SPUset, SPUblend};

enum OPER {FILL,COPY,DOWN};

typedef struct {
 	uint8_t R; 	
	uint8_t G;
 	uint8_t B;
 	uint8_t A;
 }Pixel;

struct data_2 {
	int alu;
	int operation;
	int pitchIn;
	int pitchOut;
	int width;
	int height;

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

	Pixel Fg;
	Pixel planemask;
	
	unsigned long long Inp;
	unsigned long long Outp;

} __attribute__((aligned(128)));

#endif
