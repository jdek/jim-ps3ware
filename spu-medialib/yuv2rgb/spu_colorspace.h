/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2
 *
 * spu_colorspace.h - SPU YUV to RGB conversion kernel
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * $Id:
 */

// Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>

// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The names of the authors may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef __SPU_COLORSPACE_H
#define __SPU_COLORSPACE_H

#include <spu_pack.h>

static inline void YUV2RGB(vector float Y,vector float U, vector float V ,vector float *R,vector float *G, vector float *B )
{
		vector float E=spu_sub(V,((vector float){128.0,128.0,128.0,128.0}));
		vector float D=spu_sub(U,((vector float){128.0,128.0,128.0,128.0}));

		R[0]=spu_add(Y,spu_mul(E,((vector float){1.402,1.402,1.402,1.402})));
		G[0]=spu_sub(Y,spu_add((spu_mul(D,((vector float){0.34414,0.34414,0.34414,0.34414}))),(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414})))));
		B[0]=spu_add(Y,spu_mul(D,((vector float) {1.772,1.772,1.772,1.772})));
}


static inline vector unsigned char packfARGB(vector float R, vector float G, vector float B)
{
		vector unsigned int max = spu_splats((unsigned int)0x00FF);
		static vector unsigned char Rff={0,19,2,3,4,23,6,7,8,27,10,11,12,31,14,15};
		static vector unsigned char Gff={0,1,19,3,4,5,23,7,8,9 ,27,11,12,13,31,15};
		vector unsigned int Ri=spu_convtu(R,0);
		Ri= spu_sel(Ri, max, spu_cmpgt(Ri,255));
		vector unsigned int Gi=spu_convtu(G,0);
		Gi= spu_sel(Gi, max, spu_cmpgt(Gi,255));
		vector unsigned int Bi=spu_convtu(B,0);
		Bi= spu_sel(Bi, max, spu_cmpgt(Bi,255));
		vector unsigned char ARGB;
		ARGB=spu_shuffle((vector unsigned char)Bi,(vector unsigned char)Ri,Rff);
		ARGB=spu_shuffle(ARGB,(vector unsigned char)Gi,Gff);
		return ARGB;
}

static inline void yuv420toARGB(vector unsigned char *Y,vector unsigned char *U, vector unsigned char *V,vector unsigned char *ARGB, int width,int maxwidth,int LSB)
{

		vector float Yf0,Yf1,Yf2,Yf3,Yf4,Yf5,Yf6,Yf7;
		vector float Uf0,Uf1;
		vector float Vf0,Vf1;
		vector float R00,R01,R02,R03,R10,R11,R12,R13,G00,G01,G02,G03,G10,G11,G12,G13,B00,B01,B02,B03,B10,B11,B12,B13;
		vector float yfv,ufuv,vfu;
		vector float E,D;
		vector unsigned char temp00,temp01,temp02,temp03,temp10,temp11,temp12,temp13;
		vector unsigned char Y0,Y1;
		int i;

		int dest=0;
		int inc=0;
		Y0=Y[0];
		Y1=Y[width>>4];
		for (i =0;i < (width>>4) + 1;i++) {
			ARGB[dest*4    ]=temp00;
			ARGB[dest*4 + 1]=temp01;
			ARGB[dest*4 + 2]=temp02;
			ARGB[dest*4 + 3]=temp03;
			ARGB[dest*4 +    (maxwidth>>2)]=temp10;
			ARGB[dest*4 + 1 +(maxwidth>>2)]=temp11;
			ARGB[dest*4 + 2 +(maxwidth>>2)]=temp12;
			ARGB[dest*4 + 3 +(maxwidth>>2)]=temp13;
			vector unsigned char nextY0=Y[i+1];
			vector unsigned char nextY1=Y[i+1+ (width>>4)];

			Yf0=unpackhh(Y0);
			Yf1=unpacklh(Y0);
			Yf2=unpackhl(Y0);
			Yf3=unpackll(Y0);
			Yf4=unpackhh(Y1);
			Yf5=unpacklh(Y1);
			Yf6=unpackhl(Y1);
			Yf7=unpackll(Y1);
	
			if ( LSB ) {
				Uf0=unpackhl(U[0]);
				Uf1=unpackll(U[0]);
				Vf0=unpackhl(V[0]);
				Vf1=unpackll(V[0]);
				LSB=LSB^1;
				U++;
				V++;

			} else {

				Uf0=unpackhh(U[0]);
				Uf1=unpacklh(U[0]);
				Vf0=unpackhh(V[0]);
				Vf1=unpacklh(V[0]);
				LSB=LSB^1;
			}

			E=spu_sub(Vf0,((vector float){128.0,128.0,128.0,128.0}));
			D=spu_sub(Uf0,((vector float){128.0,128.0,128.0,128.0}));

			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
			
			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
			
			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));

			R00=spu_add(Yf0,unpackfaabb(yfv));
			G00=spu_sub(Yf0,unpackfaabb(ufuv));
			B00=spu_add(Yf0,unpackfaabb(vfu));


			R10=spu_add(Yf4,unpackfaabb(yfv));
			G10=spu_sub(Yf4,unpackfaabb(ufuv));
			B10=spu_add(Yf4,unpackfaabb(vfu));
		
		
			R01=spu_add(Yf1,unpackfccdd(yfv));
			G01=spu_sub(Yf1,unpackfccdd(ufuv));
			B01=spu_add(Yf1,unpackfccdd(vfu));


			R11=spu_add(Yf5,unpackfccdd(yfv));
			G11=spu_sub(Yf5,unpackfccdd(ufuv));
			B11=spu_add(Yf5,unpackfccdd(vfu));


			E=spu_sub(Vf1,((vector float){128.0,128.0,128.0,128.0}));
			D=spu_sub(Uf1,((vector float){128.0,128.0,128.0,128.0}));

			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));

			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
	
			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));

			R02=spu_add(Yf2,unpackfaabb(yfv));
			G02=spu_sub(Yf2,unpackfaabb(ufuv));
			B02=spu_add(Yf2,unpackfaabb(vfu));

			R12=spu_add(Yf6,unpackfaabb(yfv));
			G12=spu_sub(Yf6,unpackfaabb(ufuv));
			B12=spu_add(Yf6,unpackfaabb(vfu));


		
			R03=spu_add(Yf3,unpackfccdd(yfv));
			G03=spu_sub(Yf3,unpackfccdd(ufuv));
			B03=spu_add(Yf3,unpackfccdd(vfu));
	


			R13=spu_add(Yf7,unpackfccdd(yfv));
			G13=spu_sub(Yf7,unpackfccdd(ufuv));
			B13=spu_add(Yf7,unpackfccdd(vfu));
	

			temp00=packfARGB(R00,G00,B00);
			temp01=packfARGB(R01,G01,B01);
			temp02=packfARGB(R02,G02,B02);
			temp03=packfARGB(R03,G03,B03);
			temp10=packfARGB(R10,G10,B10);
			temp11=packfARGB(R11,G11,B11);
			temp12=packfARGB(R12,G12,B12);
			temp13=packfARGB(R13,G13,B13);
			Y0=nextY0;
			Y1=nextY1;
			dest=dest + inc;
			inc=1;
		}
}

#endif
