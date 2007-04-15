#ifndef __SPU_PRINT_H
#define __SPI_PRINT_H

#include <stdlib>

static inline void printfvec(char * name,vector float vf) {
{
	printf("Vector %s=( %f, %f, %f, %f )\n",name, spu_extract(vf,0), spu_extract(vf,1), spu_extract(vf,2), spu_extract(vf,3));
}


static inline void printintvec(char * name,vector int vi) {
	
	printf("Vector %s=( %d, %d, %d, %d )\n",name, spu_extract(vi,0), spu_extract(vi,1), spu_extract(vi,2), spu_extract(vi,3));
}

static inline void printshortvec(char * name,vector short vs) {
	
	printf("Vector %s=( %d, %d, %d, %d, %d, %d, %d, %d )\n",name, spu_extract(vs,0), spu_extract(vs,1), spu_extract(vs,2), spu_extract(vs,3), spu_extract(vs,4), spu_extract(vs,5), spu_extract(vs,6), spu_extract(vs,7));
}

void printcharvec(char*name,vector unsigned char vc) {

printf("Vector %s=( %d, %d, %d, %d, %d, %d, %d, %d,%d, %d, %d, %d, %d, %d, %d, %d )\n",name, spu_extract(vc,0), spu_extract(vc,1), spu_extract(vc,2), spu_extract(vc,3), spu_extract(vc,4), spu_extract(vc,5), spu_extract(vc,6), spu_extract(vc,7), spu_extract(vc,8), spu_extract(vc,9), spu_extract(vc,10), spu_extract(vc,11), spu_extract(vc,12), spu_extract(vc,13), spu_extract(vc,14), spu_extract(vc,15));
	
}
#endif
