/**
 * SPU-MEDIALIB utility
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_dsputils.h - SPU digital signal processing utilities
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

#ifndef __SPU_UTILS_H
#define __SPU_UTILS_H

#include <spu_mfcio.h>
#include <math.h>
#include <spu_matrix.h>
#include <spu_extfunct.h>

static vector signed short C[3];
static vector signed short S[3];
static vector signed short zero;

static vector signed short SQRT_2_S;
static vector signed short SQRT_1_2_S;


static inline void IDCT_HALF_S(vector signed short DCT_S[8],vector signed short IDCT_S[8])
{
	vector signed short S30,S31,S32,S33,S34,S35,S36,S37; //stage 3 temporary values.. 
	vector signed short S20,S21,S22,S23,S24,S25,S26,S27; //stage 2 temporary values.. 
	vector signed short S10,S11,S12,S13,S14,S15,S16,S17; //stage 2 temporary values.. 

	S10=DCT_S[0];
	S11=DCT_S[4];
	S12=DCT_S[2];
	S13=DCT_S[6];
	S15=DCT_S[3];
	S16=DCT_S[5];

	//stage 0
	vector signed short T14=spu_smul(SQRT_1_2_S,DCT_S[7]);
	vector signed short T17=spu_smul(SQRT_1_2_S,DCT_S[1]);

	S14=spu_sub(T17,T14);
	S17=spu_add(T17,T14);

	//stage 2 
	S25=S15;
	S26=S16;
	
	vector signed short T22=spu_smul(S12,C[2]);
	vector signed short T23=spu_smul(S12,S[2]);
	S20=spu_add(S10,S11);
	S21=spu_sub(S10,S11);
	
	S24=spu_add(S14,S16);
	S25=spu_sub(S17,S15);
	S26=spu_sub(S14,S16);
	S27=spu_add(S17,S15);
	
	S22=spu_smadd(spu_sub(zero,S13),S[2],T22);
	S23=spu_smadd(S13,C[2],T23);

	//stage 3

	vector signed short T34=spu_smul(S24,C[1]);
	vector signed short T37=spu_smul(S24,S[1]);
	vector signed short T35=spu_smul(S25,C[0]);
	vector signed short T36=spu_smul(S25,S[0]);
	
	S30=spu_add(S20,S23);
	S31=spu_add(S21,S22);
	S32=spu_sub(S21,S22);
	S33=spu_sub(S20,S23);

	S34=spu_smadd(spu_sub(zero,S27),S[1],T34);
	S37=spu_smadd(S27,C[1],T37);

	S35=spu_smadd(spu_sub(zero,S26),S[0],T35);
	S36=spu_smadd(S26,C[0],T36);
	
	//Stage 4 

	IDCT_S[0]=spu_add(S37,S30);
	IDCT_S[1]=spu_add(S36,S31);
	IDCT_S[2]=spu_add(S35,S32);
	IDCT_S[3]=spu_add(S34,S33);
	IDCT_S[4]=spu_sub(S33,S34);
	IDCT_S[5]=spu_sub(S32,S35);
	IDCT_S[6]=spu_sub(S31,S36);
	IDCT_S[7]=spu_sub(S30,S37);

}


/*
 *IDCT does inverse discrete cosine transform of a transposed matrix and returns the results in block
 *
 *
 *
 * */
static inline void IDCT_S(vector signed short* block )
{
	IDCT_HALF_S(block,block);
	rotateshortmatrix8(block);//transpose the matrix
	IDCT_HALF_S(block,block);
	matrix_short_shift_right(block,3,8);// correcting the /256 by the 16/N^4 where N=8 (4/N^2 per dimension)
}


static inline void init_IDCT_S() // this needs to be initiated prior to execution of idct
{	
	short factor=2048;
	
	signed short SQRT=factor*sqrt(2);
	signed short SQRT_1_2=factor*sqrt(1.0/2.0);
	
	SQRT_2_S=(vector signed short){SQRT,SQRT,SQRT,SQRT,SQRT,SQRT,SQRT,SQRT};
	SQRT_1_2_S=(vector signed short){SQRT_1_2,SQRT_1_2,SQRT_1_2,SQRT_1_2,SQRT_1_2,SQRT_1_2,SQRT_1_2,SQRT_1_2};
	
	signed short C1=sqrt(2)*factor*cos(1*M_PI/16);
	signed short C3=sqrt(2)*factor*cos(3*M_PI/16);
	signed short C6=sqrt(2)*factor*cos(6*M_PI/16);

	signed short S1=sqrt(2)*factor*sin(1*M_PI/16);	
	signed short S3=sqrt(2)*factor*sin(3*M_PI/16);
	signed short S6=sqrt(2)*factor*sin(6*M_PI/16);

	C[0]=(vector signed short){C1,C1,C1,C1,C1,C1,C1,C1};
	C[1]=(vector signed short){C3,C3,C3,C3,C3,C3,C3,C3};
	C[2]=(vector signed short){C6,C6,C6,C6,C6,C6,C6,C6};
	S[0]=(vector signed short){S1,S1,S1,S1,S1,S1,S1,S1};
	S[1]=(vector signed short){S3,S3,S3,S3,S3,S3,S3,S3};
	S[2]=(vector signed short){S6,S6,S6,S6,S6,S6,S6,S6};

	zero=(vector signed short){0,0,0,0,0,0,0,0};
}

static inline void inverse_scan_progressive(vector unsigned char * Input[8],vector unsigned char * Output[4])
{
	//packing data block these can all be removed if ffmpeg supplies data as uint8_t
	vector unsigned char pack={1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31}; 
	vector unsigned char A=spu_shuffle(Input[0],Input[1],pack);
	vector unsigned char B=spu_shuffle(Input[2],Input[3],pack);
	vector unsigned char C=spu_shuffle(Input[4],Input[5],pack);
	vector unsigned char D=spu_shuffle(Input[6],Input[7],pack);
	//packing data block ends these can all be removed if ffmpeg supplies data as uint8_t

	//pipelined this is xx cycles 
	Output[0]=spu_shuffle(A,B,((vector unsigned char ){0,1,5,6,14,15,27,28,2,4,7,13,16,26,29,0x00}));
	Output[1]=spu_shuffle(A,B,((vector unsigned char ){3,8,12,17,25,30,0x00,0x00,9,11,18,24,31,0x00,0x00,0x00}));
	Output[2]=spu_shuffle(C,D,((vector unsigned char ){  0x00,0x00,0x00,0,7,13,20,22,
							       0x00,0x00,1,6,14,19,23,28}));
	Output[3]=spu_shuffle(C,D,((vector unsigned char ){	 0x00,2,5,15,18,24,27,29
							           3,4,16,17,25,26,30,31}));
	Output[0]=spu_shuffle(Output[0],C,((vector unsigned char ){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,26}));//adding nr 42
	Output[1]=spu_shuffle(Output[1],C,((vector unsigned char ){0,1,2,3,4,5,25,27,8,9,10,11,12,24,28,0x00}));//adding 41,43,,40,44
	
	Output[2]=spu_shuffle(Output[2],D,((vector unsigned char ){0x00,19,23,3,4,,5,6,7,20,22,10,11,12,13,14,15})); // only missing 1 here;
	Output[3]=spu_shuffle(Output[3],D,((vector unsigned char ){21,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15})); //move this down
	Output[1]=spu_shuffle(Output[1],D,((vector unsigned char ){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,21})); //adding 53
	Output[2]=spu_shuffle(Output[2],C,((vector unsigned char ){26,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}));//adding 10
}


static inline void dct_unquantisize_mpeg2_intra(SpuMpegEncContext *s,vector signed short *block[8], int n, int qscale)
{ 
	int i, nCoeffs;
	int level;		
    vector unsigned short *quant_matrix;
   uint16_t B0;
	
    if(s->alternate_scan) nCoeffs= 63;
    else nCoeffs= s->block_last_index[n];

    if (n < 4)
        B0 = spu_extract(block[0],0) * s->y_dc_scale;
    else
        B0 = spu_extract(block[0],0) * s->c_dc_scale;

    	quant_matrix = s->intra_matrix;

   // I guarantee that this will run faster if pipelined... unsolo
    for(i=0;i<=nCoeffs>>2;i++) {

     //   int j= s->intra_scantable.permutated[i]; //already done elsewhere

	level=spu_gather(spu_cmpabsgt(block[i],((vector short){0,0,0,0,0,0,0,0})) //if any element of block[i] isnt 0 please compute
        if !(level) {
		vector signed int odd=spu_mulo(block[i],quant_matrix[i]); 
		vector signed int even=spu_mole(block[i],quant_matrix[i]);
		
		vector signed int oddscale=spu_mulo(odd,qscale);
		vector signed int evenscale=spu_mule(even,qscale);
		

		vector signed int oddshift=spu_rlmaska(oddscale,-3);
		vector signed int evenshift= spu_rlmaska(evenscale,-3);
		block[i]=spu_shuffle(spu_rlmaska(evenshift,16),spu_rlmaska(oddshift,16),((vector unsigned char){0,1,16,17,4,5,20,21,8,9,24,25,12,13,28,29}));
        }
    }

	spu_insert(B0,block[0],0); //re insert the 0,0
}

#endif
