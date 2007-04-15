
/* 
Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#ifndef __SPU_COLORSPACE_H
#define __SPU_COLORSPACE_H

#include "pack.h"

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
		static vector unsigned char Rff=(vector unsigned char){0,19,2,3,4,23,6,7,8,27,10,11,12,31,14,15};
		static vector unsigned char Gff=(vector unsigned char){0,1,19,3,4,5,23,7,8,9 ,27,11,12,13,31,15};
		vector unsigned int Ri=spu_convtu(R,0);
		Ri= spu_sel(Ri, max, spu_cmpgt(Ri,255));
		vector unsigned int Gi=spu_convtu(B,0);
		Gi= spu_sel(Gi, max, spu_cmpgt(Gi,255));
		vector unsigned int Bi=spu_convtu(B,0);
		Bi= spu_sel(Bi, max, spu_cmpgt(Bi,255));
		vector unsigned char ARGB;
		ARGB=spu_shuffle((vector unsigned char)Bi,(vector unsigned char)Ri,Rff);
		ARGB=spu_shuffle(ARGB,(vector unsigned char)Gi,Gff);
		return ARGB;
}

static inline void yuv420toARGB(vector unsigned char *Y,vector unsigned char *U, vector unsigned char *V,vector unsigned char *ARGB, int width,int maxwidth)
{

		vector float Yf0,Yf1,Yf2,Yf3,Yf4,Yf5,Yf6,Yf7;
		vector float Uf0,Uf1;
		vector float Vf0,Vf1;
		vector float R0,G0,B0;
		vector float yfv,ufuv,vfu;
		vector float E,D;
		for (int i =0;i < width/16;i++) {
			Yf0=unpackhh(Y[i]);
			Yf1=unpacklh(Y[i]);
			Yf2=unpackhl(Y[i]);
			Yf3=unpackll(Y[i]);
			Yf4=unpackhh(Y[i+width/16]);
			Yf5=unpacklh(Y[i+width/16]);
			Yf6=unpackhl(Y[i+width/16]);
			Yf7=unpackll(Y[i+width/16]);
			
			int j=i/2;
	
			
			if ( i&1 == 1) {
				Uf0=unpackhl(U[j]);
				Uf1=unpackll(U[j]);
				Vf0=unpackhl(V[j]);
				Vf1=unpackll(V[j]);	
				
			} else {

				Uf0=unpackhh(U[j]);
				Uf1=unpacklh(U[j]);
				Vf0=unpackhh(V[j]);
				Vf1=unpacklh(V[j]);
			}
		
			E=spu_sub(Vf0,((vector float){128.0,128.0,128.0,128.0}));
			D=spu_sub(Uf0,((vector float){128.0,128.0,128.0,128.0}));

			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
			ufuv=spu_add((spu_mul(D,((vector float){0.34414,0.34414,0.34414,0.34414}))),(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));		//
			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));

			R0=spu_add(Yf0,unpackfaabb(yfv));
			G0=spu_sub(Yf0,unpackfaabb(ufuv));
			B0=spu_add(Yf0,unpackfaabb(vfu));

			ARGB[i*4]=packfARGB(R0,G0,B0);

			R0=spu_add(Yf4,unpackfaabb(yfv));
			G0=spu_sub(Yf4,unpackfaabb(ufuv));
			B0=spu_add(Yf4,unpackfaabb(vfu));

			ARGB[i*4+maxwidth/4]=packfARGB(R0,G0,B0);			

			R0=spu_add(Yf1,unpackfccdd(yfv));
			G0=spu_sub(Yf1,unpackfccdd(ufuv));
			B0=spu_add(Yf1,unpackfccdd(vfu));

			ARGB[i*4+1]=packfARGB(R0,G0,B0);

			R0=spu_add(Yf5,unpackfccdd(yfv));
			G0=spu_sub(Yf5,unpackfccdd(ufuv));
			B0=spu_add(Yf5,unpackfccdd(vfu));

			ARGB[i*4+1+maxwidth/4]=packfARGB(R0,G0,B0);

			E=spu_sub(Vf1,((vector float){128.0,128.0,128.0,128.0}));
			D=spu_sub(Uf1,((vector float){128.0,128.0,128.0,128.0}));

			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
			ufuv=spu_add((spu_mul(D,((vector float){0.34414,0.34414,0.34414,0.34414}))),(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));		//
			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));

			R0=spu_add(Yf2,unpackfaabb(yfv));
			G0=spu_sub(Yf2,unpackfaabb(ufuv));
			B0=spu_add(Yf2,unpackfaabb(vfu));

			ARGB[i*4+2]=packfARGB(R0,G0,B0);

			R0=spu_add(Yf6,unpackfaabb(yfv));
			G0=spu_sub(Yf6,unpackfaabb(ufuv));
			B0=spu_add(Yf6,unpackfaabb(vfu));

			ARGB[i*4+2+maxwidth/4]=packfARGB(R0,G0,B0);

			R0=spu_add(Yf3,unpackfccdd(yfv));
			G0=spu_sub(Yf3,unpackfccdd(ufuv));
			B0=spu_add(Yf3,unpackfccdd(vfu));
	
			ARGB[i*4+3]=packfARGB(R0,G0,B0);

			R0=spu_add(Yf7,unpackfccdd(yfv));
			G0=spu_sub(Yf7,unpackfccdd(ufuv));
			B0=spu_add(Yf7,unpackfccdd(vfu));
	
			ARGB[i*4+3+maxwidth/4]=packfARGB(R0,G0,B0);

		}
}

#endif
