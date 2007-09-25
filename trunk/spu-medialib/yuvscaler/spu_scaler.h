/**
 * SPU YUV scaler kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_scaler.h - spu scaling functions and filter generators.
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

#ifndef __SPU_SCALER_H
#define __SPU_SCALER_H

static inline void initHFilter(int srcH, int dstH,int * hfilterpos, vector float *weightHfilter0, vector float *weightHfilter1)
{
	int i;
	int tmp;
	float w0,w1;
	float scale=(1.0*srcH)/(1.0*dstH);
	for (i=0;i < dstH; i++) 
	{
		hfilterpos[i]=i*scale;
		w1=(i*scale-(hfilterpos[i]));
		w0=1-w1;

		weightHfilter0[i]=(vector float){w0,w0,w0,w0};
		weightHfilter1[i]=(vector float){w1,w1,w1,w1};
	}

}

static inline void initWFilter(int srcW,int dstW,int type,int *filterpos,vector unsigned char *shufflefilter0 ,vector unsigned char* shufflefilter1,vector float *weightfilter0,vector float *weightfilter1)
{
	int i;
	int j;
	int tmp;
	float scale;
	scale=(1.0*srcW)/(1.0*dstW);

	uint8_t src0[4],src1[4];
	float wf0[4],wf1[4];
	int current;
	for (i=0; i<dstW/4;i++)
	{
		current=i*4*scale;
		filterpos[i]=current/16;
		for (j=0; j<4; j++)
		{
			tmp=(i*4+j)*scale;
			wf0[j]=1-((i*4+j)*scale-tmp);// - (int)(i*4+j)*scale;
			wf1[j]=1-wf0[j];
		}
		src0[0]=(i*4*scale)-filterpos[i]*16;
		src0[1]=((i*4+1)*scale)-filterpos[i]*16;
		src0[2]=((i*4+2)*scale)-filterpos[i]*16;
		src0[3]=((i*4+3)*scale)-filterpos[i]*16;
		src1[0]=src0[0]+1;
		src1[1]=src0[1]+1;
		src1[2]=src0[2]+1;
		src1[3]=src0[3]+1;
		shufflefilter0[i]=(vector unsigned char){ 0x80,0x80,0x80,src0[0],0x80,0x80,0x80,src0[1],0x80,0x80,0x80,src0[2],0x80,0x80,0x80,src0[3] };
		shufflefilter1[i]=(vector unsigned char){ 0x80,0x80,0x80,src1[0],0x80,0x80,0x80,src1[1],0x80,0x80,0x80,src1[2],0x80,0x80,0x80,src1[3] };
		weightfilter0[i]=(vector float){wf0[0],wf0[1],wf0[2],wf0[3]};
		weightfilter1[i]=(vector float){wf1[0],wf1[1],wf1[2],wf1[3]};
	}

}


static inline void initWcrFilter(int srcW,int dstW,int type,int *filterpos,vector unsigned char *crshufflefilter0 ,vector unsigned char* crshufflefilter1)
{
	int i;
	int j;
	int tmp;
	float scale;
	scale=(1.0*srcW)/(1.0*dstW);

	uint8_t src0[4],src1[4];
	float wf0[4],wf1[4];
	int current;
	for (i=0; i< dstW/8;i++)
	{
		current=i*4*scale;
		filterpos[i]=current/16;

		src0[0]=8+(i*4*scale)-filterpos[i]*16;
		src0[1]=8+((i*4+1)*scale)-filterpos[i]*16;
		src0[2]=8+((i*4+2)*scale)-filterpos[i]*16;
		src0[3]=8+((i*4+3)*scale)-filterpos[i]*16;
		src1[0]=src0[0]+1;
		src1[1]=src0[1]+1;
		src1[2]=src0[2]+1;
		src1[3]=src0[3]+1;
		crshufflefilter0[i]=(vector unsigned char){ 0x80,0x80,0x80,src0[0],0x80,0x80,0x80,src0[1],0x80,0x80,0x80,src0[2],0x80,0x80,0x80,src0[3] };
		crshufflefilter1[i]=(vector unsigned char){ 0x80,0x80,0x80,src1[0],0x80,0x80,0x80,src1[1],0x80,0x80,0x80,src1[2],0x80,0x80,0x80,src1[3] };
	}

/*	for (i=0; i< dstW/8;i++)
	{
		current=8+i*4*scale;
		filterpos[i+dstW/8]=current/16;

		src0[0]=8+(i*4*scale)-filterpos[i+dstW/8]*16;
		src0[1]=8+((i*4+1)*scale)-filterpos[i+dstW/8]*16;
		src0[2]=8+((i*4+2)*scale)-filterpos[i+dstW/8]*16;
		src0[3]=8+((i*4+3)*scale)-filterpos[i+dstW/8]*16;
		src1[0]=src0[0]+1;
		src1[1]=src0[1]+1;
		src1[2]=src0[2]+1;
		src1[3]=src0[3]+1;
		crshufflefilter0[i+dstW/8]=(vector unsigned char){ 0x80,0x80,0x80,src0[0],0x80,0x80,0x80,src0[1],0x80,0x80,0x80,src0[2],0x80,0x80,0x80,src0[3] };
		crshufflefilter1[i+dstW/8]=(vector unsigned char){ 0x80,0x80,0x80,src1[0],0x80,0x80,0x80,src1[1],0x80,0x80,0x80,src1[2],0x80,0x80,0x80,src1[3] };
	}
*/

}

static inline void scalenewline(vector unsigned char * input, vector unsigned char* shuffle0,vector unsigned char* shuffle1, int *filterpos, vector float * input0, vector float * input1,vector float *output0,vector float* output1,vector float whfilter0,vector float whfilter1,vector float* wwfilter0,vector float *wwfilter1,vector unsigned char* result,int dstW)
{
		int i;
	vector unsigned char shuf00,shuf01,shuf02,shuf03,shuf10,shuf11,shuf12,shuf13,in0,in1,in2,in3,in4;
	vector unsigned char in01,in11,in21,in31;

	shuf00=shuffle0[0];
	shuf01=shuffle0[1];
	shuf02=shuffle0[2];
	shuf03=shuffle0[3];
	shuf10=shuffle1[0];
	shuf11=shuffle1[1];
	shuf12=shuffle1[2];
	shuf13=shuffle1[3];
	in0=input[filterpos[0]];
	in1=input[filterpos[1]];
	in2=input[filterpos[2]];
	in3=input[filterpos[3]];

	in01=input[filterpos[0]+1];
	in11=input[filterpos[1]+1];
	in21=input[filterpos[2]+1];
	in31=input[filterpos[3]+1];

	vector unsigned int max = spu_splats((unsigned int)0x00FF);
	static vector unsigned char upper={3,7,11,15,19,23,27,31,128,128,128,128,128,128,128,128};
	static vector unsigned char lu={16,17,18,19,20,21,22,23,3,7,11,15,128,128,128};
	static vector unsigned char ll={16,17,18,19,20,21,22,23,24,25,26,27,3,7,11,15};

	for (i=0;i<dstW/16;i++)
	{
		int nextpos0=filterpos[4*i+4];
		int nextpos1=filterpos[4*i+5];
		int nextpos2=filterpos[4*i+6];
		int nextpos3=filterpos[4*i+7];

		vector unsigned char nextin0=input[nextpos0];
		vector unsigned char nextin1=input[nextpos1];
		vector unsigned char nextin2=input[nextpos2];
		vector unsigned char nextin3=input[nextpos3];
		
		vector unsigned char nextin01=input[nextpos0+1];
		vector unsigned char nextin11=input[nextpos1+1];
		vector unsigned char nextin21=input[nextpos2+1];
		vector unsigned char nextin31=input[nextpos3+1];



		vector unsigned char nextshuf00=shuffle0[4*i+4];
		vector unsigned char nextshuf01=shuffle0[4*i+5];
		vector unsigned char nextshuf02=shuffle0[4*i+6];
		vector unsigned char nextshuf03=shuffle0[4*i+7];
		vector unsigned char nextshuf10=shuffle1[4*i+4];
		vector unsigned char nextshuf11=shuffle1[4*i+5];
		vector unsigned char nextshuf12=shuffle1[4*i+6];
		vector unsigned char nextshuf13=shuffle1[4*i+7];

		vector float w00=spu_mul(wwfilter0[4*i],whfilter0); //px0,0
		vector float w01=spu_mul(wwfilter1[4*i],whfilter0); // px 0,1 aka w(+1)
		vector float w10=spu_mul(wwfilter0[4*i],whfilter1); // px1,0
		vector float w11=spu_mul(wwfilter1[4*i],whfilter1);// px1,1

		output0[4*i]= spu_convtf(((vector unsigned int)spu_shuffle(in0, in01,shuf00)),0);
		output1[4*i]= spu_convtf(((vector unsigned int)spu_shuffle(in0, in01,shuf10)),0);
		
		vector float newpixel0=spu_mul(w00,input0[4*i]);
		newpixel0=spu_madd(w01,input1[4*i],newpixel0);
		newpixel0=spu_madd(w10,output0[4*i],newpixel0);
		newpixel0=spu_madd(w11,output1[4*i],newpixel0);

		w00=spu_mul(wwfilter0[4*i+1],whfilter0); //px0,0
		w01=spu_mul(wwfilter1[4*i+1],whfilter0); // px 0,1 aka w(+1)
		w10=spu_mul(wwfilter0[4*i+1],whfilter1); // px1,0
		w11=spu_mul(wwfilter1[4*i+1],whfilter1);// px1,1

		output0[4*i+1]= spu_convtf(((vector unsigned int)spu_shuffle(in1, in11,shuf01)),0);
		output1[4*i+1]= spu_convtf(((vector unsigned int)spu_shuffle(in1, in11,shuf11)),0);
		
		vector float newpixel1=spu_mul(w00,input0[4*i+1]);
		newpixel1=spu_madd(w01,input1[4*i+1],newpixel1);
		newpixel1=spu_madd(w10,output0[4*i+1],newpixel1);
		newpixel1=spu_madd(w11,output1[4*i+1],newpixel1);

		w00=spu_mul(wwfilter0[4*i+2],whfilter0); //px0,0
		w01=spu_mul(wwfilter1[4*i+2],whfilter0); // px 0,1 aka w(+1)
		w10=spu_mul(wwfilter0[4*i+2],whfilter1); // px1,0
		w11=spu_mul(wwfilter1[4*i+2],whfilter1);// px1,1
		
		output0[4*i+2]= spu_convtf(((vector unsigned int)spu_shuffle(in2, in21,shuf02)),0);
		output1[4*i+2]= spu_convtf(((vector unsigned int)spu_shuffle(in2, in21,shuf12)),0);

		vector float newpixel2=spu_mul(w00,input0[4*i+2]);
		newpixel2=spu_madd(w01,input1[4*i+2],newpixel2);
		newpixel2=spu_madd(w10,output0[4*i+2],newpixel2);
		newpixel2=spu_madd(w11,output1[4*i+2],newpixel2);

		w00=spu_mul(wwfilter0[4*i+3],whfilter0); //px0,0
		w01=spu_mul(wwfilter1[4*i+3],whfilter0); // px 0,1 aka w(+1)
		w10=spu_mul(wwfilter0[4*i+3],whfilter1); // px1,0
		w11=spu_mul(wwfilter1[4*i+3],whfilter1);// px1,1

		output0[4*i+3]= spu_convtf(((vector unsigned int)spu_shuffle(in3, in31,shuf03)),0);
		output1[4*i+3]= spu_convtf(((vector unsigned int)spu_shuffle(in3, in31,shuf13)),0);

		vector float newpixel3=spu_mul(w00,input0[4*i+3]);
		newpixel3=spu_madd(w01,input1[4*i+3],newpixel3);
		newpixel3=spu_madd(w10,output0[4*i+3],newpixel3);
		newpixel3=spu_madd(w11,output1[4*i+3],newpixel3);

		shuf00=nextshuf00;
		shuf01=nextshuf01;
		shuf02=nextshuf02;
		shuf03=nextshuf03;
		shuf10=nextshuf10;
		shuf11=nextshuf11;
		shuf12=nextshuf12;
		shuf13=nextshuf13;
		in0=nextin0;
		in1=nextin1;
		in2=nextin2;
		in3=nextin3;

		in01=nextin01;
		in11=nextin11;
		in21=nextin21;
		in31=nextin31;
		// pack the results int a char

		vector unsigned int tmp0=spu_convtu(newpixel0,0);
		tmp0= spu_sel(tmp0, max, spu_cmpgt(tmp0,255));
		vector unsigned int tmp1=spu_convtu(newpixel1,0);
		tmp1= spu_sel(tmp1, max, spu_cmpgt(tmp1,255));
		vector unsigned int tmp2=spu_convtu(newpixel2,0);
		tmp2= spu_sel(tmp2, max, spu_cmpgt(tmp2,255));
		vector unsigned int tmp3=spu_convtu(newpixel3,0);
		tmp3= spu_sel(tmp3, max, spu_cmpgt(tmp3,255));
		
		result[i]=(vector unsigned char)spu_shuffle((vector unsigned char)tmp0,(vector unsigned char)tmp1,upper);
		result[i]=(vector unsigned char)spu_shuffle((vector unsigned char)tmp2,result[i],lu);
		result[i]=(vector unsigned char)spu_shuffle((vector unsigned char)tmp3,result[i],ll);
		
	}
}

static inline void unpacklines(vector unsigned char * input, vector unsigned char* shuffle0, vector unsigned char* shuffle1, int *filterpos, vector float * output0, vector float * output1, int dstW)
{
	int i;
	vector unsigned char shuf00,shuf01,shuf02,shuf03,shuf10,shuf11,shuf12,shuf13,in0,in1,in2,in3,in4;

	shuf00=shuffle0[0];
	shuf01=shuffle0[1];
	shuf02=shuffle0[2];
	shuf03=shuffle0[3];
	shuf10=shuffle1[0];
	shuf11=shuffle1[1];
	shuf12=shuffle1[2];
	shuf13=shuffle1[3];
	in0=input[filterpos[0]];
	in1=input[filterpos[1]];
	in2=input[filterpos[2]];
	in3=input[filterpos[3]];
	in4=input[filterpos[4]];


	for (i=0;i<dstW/16;i++)
	{
		int nextpos0=filterpos[4*i+4];
		int nextpos1=filterpos[4*i+5];
		int nextpos2=filterpos[4*i+6];
		int nextpos3=filterpos[4*i+7];
		int nextpos4=filterpos[4*i+8];
		vector unsigned char nextin0=input[nextpos0];
		vector unsigned char nextin1=input[nextpos1];
		vector unsigned char nextin2=input[nextpos2];
		vector unsigned char nextin3=input[nextpos3];
		vector unsigned char nextin4=input[nextpos4];
		vector unsigned char nextshuf00=shuffle0[4*i+4];
		vector unsigned char nextshuf01=shuffle0[4*i+5];
		vector unsigned char nextshuf02=shuffle0[4*i+6];
		vector unsigned char nextshuf03=shuffle0[4*i+7];
		vector unsigned char nextshuf10=shuffle1[4*i+4];
		vector unsigned char nextshuf11=shuffle1[4*i+5];
		vector unsigned char nextshuf12=shuffle1[4*i+6];
		vector unsigned char nextshuf13=shuffle1[4*i+7];

		output0[4*i]= spu_convtf(((vector unsigned int)spu_shuffle(in0, in1,shuf00)),0);
		output1[4*i]= spu_convtf(((vector unsigned int)spu_shuffle(in0, in1,shuf10)),0);

		output0[4*i+1]= spu_convtf(((vector unsigned int)spu_shuffle(in1, in2,shuf01)),0);
		output1[4*i+1]= spu_convtf(((vector unsigned int)spu_shuffle(in1, in2,shuf11)),0);

		output0[4*i+2]= spu_convtf(((vector unsigned int)spu_shuffle(in2, in3,shuf02)),0);
		output1[4*i+2]= spu_convtf(((vector unsigned int)spu_shuffle(in2, in3,shuf12)),0);


		output0[4*i+3]= spu_convtf(((vector unsigned int)spu_shuffle(in3, in4,shuf03)),0);
		output1[4*i+3]= spu_convtf(((vector unsigned int)spu_shuffle(in3, in4,shuf13)),0);


		shuf00=nextshuf00;
		shuf01=nextshuf01;
		shuf02=nextshuf02;
		shuf03=nextshuf03;
		shuf10=nextshuf10;
		shuf11=nextshuf11;
		shuf12=nextshuf12;
		shuf13=nextshuf13;
		in0=nextin0;
		in1=nextin1;
		in2=nextin2;
		in3=nextin3;
		in4=nextin4;
	}
}

static inline void scaleline(int *filterpos, vector float * Input00, vector float * Input01,vector float *Input10,vector float* Input11,vector float whfilter0,vector float whfilter1,vector float* wwfilter0,vector float *wwfilter1,vector unsigned char* result,int dstW)
{
	int i;

	vector unsigned int max = spu_splats((unsigned int)0x00FF);
	static vector unsigned char upper={3,7,11,15,19,23,27,31,128,128,128,128,128,128,128,128};
	static vector unsigned char lu={16,17,18,19,20,21,22,23,3,7,11,15,128,128,128};
	static vector unsigned char ll={16,17,18,19,20,21,22,23,24,25,26,27,3,7,11,15};

	for (i=0;i<dstW/16;i++)
	{

		vector float w00=spu_mul(wwfilter0[4*i],whfilter0); //px0,0
		vector float w01=spu_mul(wwfilter1[4*i],whfilter0); // px 0,1 aka w(+1)
		vector float w10=spu_mul(wwfilter0[4*i],whfilter1); // px1,0
		vector float w11=spu_mul(wwfilter1[4*i],whfilter1);// px1,1

		
		vector float newpixel0=spu_mul(w00,Input00[4*i]);
		newpixel0=spu_madd(w01,Input01[4*i],newpixel0);
		newpixel0=spu_madd(w10,Input10[4*i],newpixel0);
		newpixel0=spu_madd(w11,Input11[4*i],newpixel0);

		w00=spu_mul(wwfilter0[4*i+1],whfilter0); //px0,0
		w01=spu_mul(wwfilter1[4*i+1],whfilter0); // px 0,1 aka w(+1)
		w10=spu_mul(wwfilter0[4*i+1],whfilter1); // px1,0
		w11=spu_mul(wwfilter1[4*i+1],whfilter1);// px1,1
		
		vector float newpixel1=spu_mul(w00,Input00[4*i+1]);
		newpixel1=spu_madd(w01,Input01[4*i+1],newpixel1);
		newpixel1=spu_madd(w10,Input10[4*i+1],newpixel1);
		newpixel1=spu_madd(w11,Input11[4*i+1],newpixel1);

		w00=spu_mul(wwfilter0[4*i+2],whfilter0); //px0,0
		w01=spu_mul(wwfilter1[4*i+2],whfilter0); // px 0,1 aka w(+1)
		w10=spu_mul(wwfilter0[4*i+2],whfilter1); // px1,0
		w11=spu_mul(wwfilter1[4*i+2],whfilter1);// px1,1

		vector float newpixel2=spu_mul(w00,Input00[4*i+2]);
		newpixel2=spu_madd(w01,Input01[4*i+2],newpixel2);
		newpixel2=spu_madd(w10,Input10[4*i+2],newpixel2);
		newpixel2=spu_madd(w11,Input11[4*i+2],newpixel2);

		w00=spu_mul(wwfilter0[4*i+3],whfilter0); //px0,0
		w01=spu_mul(wwfilter1[4*i+3],whfilter0); // px 0,1 aka w(+1)
		w10=spu_mul(wwfilter0[4*i+3],whfilter1); // px1,0
		w11=spu_mul(wwfilter1[4*i+3],whfilter1);// px1,1


		vector float newpixel3=spu_mul(w00,Input00[4*i+3]);
		newpixel3=spu_madd(w01,Input01[4*i+3],newpixel3);
		newpixel3=spu_madd(w10,Input10[4*i+3],newpixel3);
		newpixel3=spu_madd(w11,Input11[4*i+3],newpixel3);

		// pack the results int a char

		vector unsigned int tmp0=spu_convtu(newpixel0,0);
		tmp0= spu_sel(tmp0, max, spu_cmpgt(tmp0,255));
		vector unsigned int tmp1=spu_convtu(newpixel1,0);
		tmp1= spu_sel(tmp1, max, spu_cmpgt(tmp1,255));
		vector unsigned int tmp2=spu_convtu(newpixel2,0);
		tmp2= spu_sel(tmp2, max, spu_cmpgt(tmp2,255));
		vector unsigned int tmp3=spu_convtu(newpixel3,0);
		tmp3= spu_sel(tmp3, max, spu_cmpgt(tmp3,255));
		
		result[i]=(vector unsigned char)spu_shuffle((vector unsigned char)tmp0,(vector unsigned char)tmp1,upper);
		result[i]=(vector unsigned char)spu_shuffle((vector unsigned char)tmp2,result[i],lu);
		result[i]=(vector unsigned char)spu_shuffle((vector unsigned char)tmp3,result[i],ll);
		
	}
}

static inline void rightshiftnadd8(vector unsigned char start,vector unsigned char* shuffleme,int sizetoshuffle)
{
	vector unsigned char temp0,temp1;
	int i=0;
//	temp0=shuffleme[0];
	static vector unsigned char off8={8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
	temp0=shuffleme[0];
	shuffleme[0]=spu_shuffle(start,shuffleme[0],((vector unsigned char){0,1,2,3,4,5,6,7,24,25,26,27,28,29,30,31}));
	for (i=1;i<sizetoshuffle/16;i++)
	{
		temp1=shuffleme[i];
		shuffleme[i]=spu_shuffle(temp0,shuffleme[i],off8);
		temp0=temp1;
	}

}

#endif
