/**
 * SPU-MEDIALIB utility
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_matrix.h - SPU Matrix functions
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

#ifndef __SPU_MATRIX_H
#define __SPU_MATRIX_H

static inline void rotateshortmatrix8(vector signed short *M)
{
	//where M[0]=[0,,8]
	//	M[N]=[,,,,]
	//	M[7]=[0,,8]
	vector unsigned char merge2h=(vector unsigned char){0,1,16,17,2,3,18,19,4,5,20,21,6,7,22,23};
	vector unsigned char merge2l=(vector unsigned char){8,9,24,25,10,11,26,27,12,13,28,29,14,15,30,31};
	vector unsigned char merge4h=(vector unsigned char){0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
	vector unsigned char merge4l=(vector unsigned char){8,9,10,11,24,25,26,27,12,13,14,15,28,29,30,31};
	vector unsigned char merge8h=(vector unsigned char){0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
	vector unsigned char merge8l=(vector unsigned char){8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31};
	
	vector signed short tmp20=spu_shuffle(M[0],M[1],merge2h);
	vector signed short tmp21=spu_shuffle(M[0],M[1],merge2l);
	vector signed short tmp22=spu_shuffle(M[2],M[3],merge2h);
	vector signed short tmp23=spu_shuffle(M[2],M[3],merge2l);
	vector signed short tmp24=spu_shuffle(M[4],M[5],merge2h);
	vector signed short tmp25=spu_shuffle(M[4],M[5],merge2l);
	vector signed short tmp26=spu_shuffle(M[6],M[7],merge2h);
	vector signed short tmp27=spu_shuffle(M[6],M[7],merge2l);

	vector signed short tmp40=spu_shuffle(tmp20,tmp22,merge4h);
	vector signed short tmp41=spu_shuffle(tmp20,tmp22,merge4l);
	vector signed short tmp42=spu_shuffle(tmp21,tmp23,merge4h);
	vector signed short tmp43=spu_shuffle(tmp21,tmp23,merge4l);
	vector signed short tmp44=spu_shuffle(tmp24,tmp26,merge4h);
	vector signed short tmp45=spu_shuffle(tmp24,tmp26,merge4l);
	vector signed short tmp46=spu_shuffle(tmp25,tmp27,merge4h);
	vector signed short tmp47=spu_shuffle(tmp25,tmp27,merge4l);

	M[0]=spu_shuffle(tmp40,tmp44,merge8h);
	M[1]=spu_shuffle(tmp40,tmp44,merge8l);
	M[2]=spu_shuffle(tmp41,tmp45,merge8h);
	M[3]=spu_shuffle(tmp41,tmp45,merge8l);
	M[4]=spu_shuffle(tmp42,tmp46,merge8h);
	M[5]=spu_shuffle(tmp42,tmp46,merge8l);
	M[6]=spu_shuffle(tmp43,tmp47,merge8h);
	M[7]=spu_shuffle(tmp43,tmp47,merge8l);
}

static inline void rotatefloatmatrix8(vector float *M)
{

	//Matrix form
	//M[a],M[a+8]
	//...,.....
	//M[7],M[15]
	//Rotation... 
	static vector unsigned char upper2=(vector unsigned char){0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
	static vector unsigned char lower2=(vector unsigned char){8,9,10,11,24,25,26,27,12,13,14,15,28,29,30,31};
	static vector unsigned char upper4=(vector unsigned char){0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
	static vector unsigned char lower4=(vector unsigned char){8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31};

	vector float tmp00=spu_shuffle(M[0],M[1],upper2);
	vector float tmp01=spu_shuffle(M[2],M[3],upper2);
	vector float tmp02=spu_shuffle(M[0],M[1],lower2);
	vector float tmp03=spu_shuffle(M[2],M[3],lower2);

	vector float tmp04=spu_shuffle(M[4],M[5],upper2);
	vector float tmp05=spu_shuffle(M[6],M[7],upper2);
	vector float tmp06=spu_shuffle(M[4],M[5],lower2);
	vector float tmp07=spu_shuffle(M[6],M[7],lower2);

	vector float tmp08=spu_shuffle(M[8],M[9],upper2);
	vector float tmp09=spu_shuffle(M[10],M[11],upper2);
	vector float tmp10=spu_shuffle(M[8],M[9],lower2);
	vector float tmp11=spu_shuffle(M[10],M[11],lower2);

	vector float tmp12=spu_shuffle(M[12],M[13],upper2);
	vector float tmp13=spu_shuffle(M[14],M[15],upper2);
	vector float tmp14=spu_shuffle(M[12],M[13],lower2);
	vector float tmp15=spu_shuffle(M[14],M[15],lower2);

	M[0]=spu_shuffle(tmp00,tmp01,upper4);//needs pipeline
	M[1]=spu_shuffle(tmp00,tmp01,lower4);
	M[2]=spu_shuffle(tmp02,tmp03,upper4);//needs pipeline
	M[3]=spu_shuffle(tmp02,tmp03,lower4);


	M[8]=spu_shuffle(tmp04,tmp05,upper4);//needs pipeline
	M[9]=spu_shuffle(tmp04,tmp05,lower4);
	M[10]=spu_shuffle(tmp06,tmp07,upper4);//needs pipeline
	M[11]=spu_shuffle(tmp06,tmp07,lower4);


	M[4]=spu_shuffle(tmp08,tmp09,upper4);//needs pipeline
	M[5]=spu_shuffle(tmp08,tmp09,lower4);
	M[6]=spu_shuffle(tmp10,tmp11,upper4);//needs pipeline
	M[7]=spu_shuffle(tmp10,tmp11,lower4);


	M[12]=spu_shuffle(tmp12,tmp13,upper4);//needs pipeline
	M[13]=spu_shuffle(tmp12,tmp13,lower4);
	M[14]=spu_shuffle(tmp14,tmp15,upper4);//needs pipeline
	M[15]=spu_shuffle(tmp14,tmp15,lower4);


}

static inline void matrix_mul_scalarf(vector float *M,float Mul,int size)
{
	int i=0;
	vector float Multiply=(vector float)spu_splats(Mul);
	for (i=0;i<size;i++)
	{
		M[i]=spu_mul(M[i],Multiply);
	}
}

static inline void matrix_short_shift_right(vector signed short *M,int shift,int size)

{
	int i;
	for (i=0;i<size;i++)
		M[i]=spu_rlmaska(M[i],-shift);

}



#endif
