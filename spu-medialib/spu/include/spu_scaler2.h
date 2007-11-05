/**
 * SPU YUV420/YV12 scaler to ARGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_scaler_argb.h - spu scaling functions colorspace conversions and filter generators.
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

#ifndef __SPU_SCALER_ARGB_H
#define __SPU_SCALER_ARGB_H
#include <spu_pack.h>
#include <math.h>
#include "spu_scaler_settings2.h"

static inline void initHFilter(int srcW,int srcH, int dstH,int * hfilterpos0,int * hfilterpos1,float *weightH,unsigned long long *dmapos,unsigned long long *dmacromapos/* vector float *weightHfilter0, vector float *weightHfilter1*/)
{
	int smallcroma;
	if (srcW%32 != 0) 
	{
		smallcroma=1;
	} else {
		smallcroma=0;

	}
	int i;
	int tmp;
	float w0,w1;
	float scale=(1.0*((srcH/2)-1))/(1.0*((dstH/2)-1));
	dmacromapos[0]=0;
	dmapos[0]=0;
	
		
	for (i=1;i < srcH + 4;i++)
	{
		if (smallcroma) {
			if ((i&1) == 1) //are we fetching a source that is aligned
				{
					dmacromapos[i]=dmacromapos[i-1] + (((srcW)>>1)&~15);
				} else {
					dmacromapos[i]=(srcW*i)/2;
				}
		} else {
				dmacromapos[i]=((srcW>>1)&~15)*i;// + ((srcW>>1+15)&~15)*i; //rounded down
		}
		dmapos[i]=i*srcW;
 	//	dmapos[2*i+1]=(2*i+1)*srcW;

	}

	for (i=0;i < dstH + 4; i++) 
	{
		//int s=
		hfilterpos0[i]=i*scale;
		hfilterpos1[i]=hfilterpos0[i]+1; //this is not the same as hfilterpos0[h+1]
		w1=i*scale-(hfilterpos0[i]);
		w0=1.0-w1;
		weightH[i]=w0;
	//	weightHfilter0[i]=(vector float){w0,w0,w0,w0};
	//	weightHfilter1[i]=(vector float){w1,w1,w1,w1};
	}

}

static inline void initHF(int srcW,int srcH, int dstH,int * hfilterpos0,int * hfilterpos1,int *weightH,unsigned long long *dmapos,unsigned long long *dmacromapos/* vector float *weightHfilter0, vector float *weightHfilter1*/)
{
	int smallcroma;
	if (srcW%32 != 0) 
	{
		smallcroma=1;
	} else {
		smallcroma=0;

	}
	int i;
	int tmp;
	float w0,w1;
	float scale=(1.0*((srcH/2)-1))/(1.0*((dstH/2)-1));
	dmacromapos[0]=0;
	dmapos[0]=0;
	
		
	for (i=1;i < srcH + 4;i++)
	{
		if (smallcroma) {
			if ((i&1) == 1) //are we fetching a source that is aligned
				{
					dmacromapos[i]=dmacromapos[i-1] + (((srcW)>>1)&~15);
				} else {
					dmacromapos[i]=(srcW*i)/2;
				}
		} else {
				dmacromapos[i]=((srcW>>1)&~15)*i;// + ((srcW>>1+15)&~15)*i; //rounded down
		}
		dmapos[i]=i*srcW;
 	//	dmapos[2*i+1]=(2*i+1)*srcW;

	}

	for (i=0;i < dstH + 4; i++) 
	{
		//int s=
		hfilterpos0[i]=i*scale;
		hfilterpos1[i]=hfilterpos0[i]+1; //this is not the same as hfilterpos0[h+1]
		w1=i*scale-(hfilterpos0[i]);
		w0=1.0-w1;
// 		weightH[i]=(w0*2048);
		weightH[i]=(w0*2048);
	//	weightHfilter0[i]=(vector float){w0,w0,w0,w0};
	//	weightHfilter1[i]=(vector float){w1,w1,w1,w1};
	}

}

static inline void initWF(int srcW,int dstW,int type,int *filterpos,vector unsigned char *shufflefilter0 ,vector unsigned char* shufflefilter1,vector short *weightfilter0,vector short *weightfilter1)
{
	int i;
	int j;
	int tmp;
	float scale;
	scale=(1.0*((srcW/2)-1))/(1.0*(dstW/2)-1);

	uint8_t src0[4],src1[4];
	int wf0[4],wf1[4];
	int current;
	for (i=0; i<dstW/4;i++)
	{
		current=i*4*scale;
		filterpos[i]=current/16;
		for (j=0; j<4; j++)
		{
			tmp=(i*4+j)*(scale*2048);
			wf0[j]=2048-((i*4+j)*(scale*2048)-tmp);// - (int)(i*4+j)*scale;
			wf1[j]=2048-wf0[j];
/*			tmp=(i*4+j)*(scale*65535);
			wf0[j]=65535-((i*4+j)*(scale*65535)-tmp);// - (int)(i*4+j)*scale;
			wf1[j]=65535-wf0[j];	*/	
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
		weightfilter0[i]=(vector short){0,wf0[0],0,wf0[1],0,wf0[2],0,wf0[3]};
		weightfilter1[i]=(vector short){0,wf1[0],0,wf1[1],0,wf1[2],0,wf1[3]};
	}

}

static inline void initWcrF(int srcW,int dstW,int type,int *crfilterpos,vector unsigned char *crshufflefilter0 ,vector unsigned char* crshufflefilter1)
{
	int i;
	int j;
	int tmp;
	float scale;
	scale=(1.0*((srcW/2)-1))/(1.0*(dstW/2-1));
	
	uint8_t src0[4],src1[4];
	float wf0[4],wf1[4];
	int current;

	for (i=0; i< dstW/4;i++)
	{
		current=8+i*4*scale;
		crfilterpos[i]=current/16;

		src0[0]=8+(i*4*scale)-crfilterpos[i]*16;
		src0[1]=8+((i*4+1)*scale)-crfilterpos[i]*16;
		src0[2]=8+((i*4+2)*scale)-crfilterpos[i]*16;
		src0[3]=8+((i*4+3)*scale)-crfilterpos[i]*16;
		src1[0]=src0[0]+1;
		src1[1]=src0[1]+1;
		src1[2]=src0[2]+1;
		src1[3]=src0[3]+1;

		
		crshufflefilter0[i]=(vector unsigned char){ 0x80,0x80,0x80,src0[0],0x80,0x80,0x80,src0[1],0x80,0x80,0x80,src0[2],0x80,0x80,0x80,src0[3] };
		crshufflefilter1[i]=(vector unsigned char){ 0x80,0x80,0x80,src1[0],0x80,0x80,0x80,src1[1],0x80,0x80,0x80,src1[2],0x80,0x80,0x80,src1[3] };
	}
}


static inline void initWFilter(int srcW,int dstW,int type,int *filterpos,vector unsigned char *shufflefilter0 ,vector unsigned char* shufflefilter1,vector float *weightfilter0,vector float *weightfilter1)
{
	int i;
	int j;
	int tmp;
	float scale;
	scale=(1.0*((srcW/2)-1))/(1.0*(dstW/2)-1);

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


static inline void initWcrFilter(int srcW,int dstW,int type,int *crfilterpos,vector unsigned char *crshufflefilter0 ,vector unsigned char* crshufflefilter1)
{
	int i;
	int j;
	int tmp;
	float scale;
//	srcW=srcW/2;
//	dstW=dstW/2;
	scale=(1.0*((srcW/2)-1))/(1.0*(dstW/2-1));
	
	uint8_t src0[4],src1[4];
	float wf0[4],wf1[4];
	int current;
// 	for (i=0; i< dstW/4 + 1;i++)
// 	{
// 		current=8+i*4*scale;
// 		crfilterpos[i]=current/16;
// 
// 		src0[0]=8+(i*4*scale)-crfilterpos[i]*16;
// 		src0[1]=8+((i*4+1)*scale)-crfilterpos[i]*16;
// 		src0[2]=8+((i*4+2)*scale)-crfilterpos[i]*16;
// 		src0[3]=8+((i*4+3)*scale)-crfilterpos[i]*16;
// 		src1[0]=src0[0]+1;
// 		src1[1]=src0[1]+1;
// 		src1[2]=src0[2]+1;
// 		src1[3]=src0[3]+1;
// 		crshufflefilter0[i]=(vector unsigned char){ 0x80,0x80,0x80,src0[0],0x80,0x80,0x80,src0[1],0x80,0x80,0x80,src0[2],0x80,0x80,0x80,src0[3] };
// 		crshufflefilter1[i]=(vector unsigned char){ 0x80,0x80,0x80,src1[0],0x80,0x80,0x80,src1[1],0x80,0x80,0x80,src1[2],0x80,0x80,0x80,src1[3] };
// 	}

	for (i=0; i< dstW/4;i++)
	{
		current=8+i*4*scale;
		crfilterpos[i]=current/16;

		src0[0]=8+(i*4*scale)-crfilterpos[i]*16;
		src0[1]=8+((i*4+1)*scale)-crfilterpos[i]*16;
		src0[2]=8+((i*4+2)*scale)-crfilterpos[i]*16;
		src0[3]=8+((i*4+3)*scale)-crfilterpos[i]*16;
		src1[0]=src0[0]+1;
		src1[1]=src0[1]+1;
		src1[2]=src0[2]+1;
		src1[3]=src0[3]+1;

		
		crshufflefilter0[i]=(vector unsigned char){ 0x80,0x80,0x80,src0[0],0x80,0x80,0x80,src0[1],0x80,0x80,0x80,src0[2],0x80,0x80,0x80,src0[3] };
		crshufflefilter1[i]=(vector unsigned char){ 0x80,0x80,0x80,src1[0],0x80,0x80,0x80,src1[1],0x80,0x80,0x80,src1[2],0x80,0x80,0x80,src1[3] };
	}
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

static inline void scale(scaler_settings_t *sc)
{
	int i;
	int width=sc->width;
	vector float wHfilter0={sc->wHfilter,sc->wHfilter,sc->wHfilter,sc->wHfilter};
	vector float wHfilter1={1-sc->wHfilter,1-sc->wHfilter,1-sc->wHfilter,1-sc->wHfilter};
	
	int inc=0;
	int dest=0;
	vector float Out0;
	vector float Out1;

		vector unsigned char shuf00,shuf01,shuf02,shuf03,shuf10,shuf11,shuf12,shuf13;
		vector unsigned char in00,in01,in02,in03,in10,in11,in12,in13;
	
		vector unsigned char *line0sw0=sc->sWfilter0; 
		vector unsigned char *line0sw1=sc->sWfilter1;
		vector unsigned char *line1sw0=sc->sWfilter0;
		vector unsigned char *line1sw1=sc->sWfilter1;
		int *wfilterpos0=sc->wfilterpos;
		int *wfilterpos1=sc->wfilterpos;

		if (sc->smallcromaline0==1)
		{
			line0sw0=sc->crsWfilter0;
			line0sw1=sc->crsWfilter1;
			wfilterpos0=sc->crfilterpos;
		} 
	
		if (sc->smallcromaline1==1)
		{
			line1sw0=sc->crsWfilter0;
			line1sw1=sc->crsWfilter1;
			wfilterpos1=sc->crfilterpos;
		} 

		in00=sc->source00[wfilterpos0[0]];
		in01=sc->source00[wfilterpos0[0]+1];
		in02=sc->source00[wfilterpos0[1]];
		in03=sc->source00[wfilterpos0[1]+1];
		
		in10=sc->source01[wfilterpos1[0]];
		in11=sc->source01[wfilterpos1[0]+1];
		in12=sc->source01[wfilterpos1[1]];
		in13=sc->source01[wfilterpos1[1]+1];

		shuf00=line0sw0[0];
		shuf01=line0sw1[0];
		shuf02=line0sw0[1];
		shuf03=line0sw1[1];

		shuf10=line1sw0[1];
		shuf11=line1sw1[1];
		shuf12=line1sw0[1];
		shuf13=line1sw1[1];
		
		dest=0;
		inc=0;

		for (i=0;i < (width>>3);i++)
		{
			sc->Outputf[2*dest]=Out0;
			sc->Outputf[2*dest+1]=Out1;
			
			int nextpos00=wfilterpos0[2*i+2];
			int nextpos01=wfilterpos0[2*i+2]+1;
			int nextpos02=wfilterpos0[2*i+3];
			int nextpos03=wfilterpos0[2*i+3]+1;

			int nextpos10=wfilterpos1[2*i+2];
			int nextpos11=wfilterpos1[2*i+2]+1;
			int nextpos12=wfilterpos1[2*i+3];
			int nextpos13=wfilterpos1[2*i+3]+1;
		
			vector unsigned char nextin00=sc->source00[nextpos00];
			vector unsigned char nextin01=sc->source00[nextpos01];
			vector unsigned char nextin02=sc->source00[nextpos02];
			vector unsigned char nextin03=sc->source00[nextpos03];
			
			vector unsigned char nextin10=sc->source01[nextpos10];
			vector unsigned char nextin11=sc->source01[nextpos11];
			vector unsigned char nextin12=sc->source01[nextpos12];
			vector unsigned char nextin13=sc->source01[nextpos13];

		

			vector unsigned char nextshuf00=line0sw0[2*i+2];
			vector unsigned char nextshuf01=line0sw1[2*i+2];
			vector unsigned char nextshuf02=line0sw0[2*i+3];
			vector unsigned char nextshuf03=line0sw1[2*i+3];

			vector unsigned char nextshuf10=line1sw0[2*i+2];
			vector unsigned char nextshuf11=line1sw1[2*i+2];
			vector unsigned char nextshuf12=line1sw0[2*i+3];
			vector unsigned char nextshuf13=line1sw1[2*i+3];	


			vector float input00 = spu_convtf(((vector unsigned int)spu_shuffle(in00, in01,shuf00)),0);
			vector float input01 = spu_convtf(((vector unsigned int)spu_shuffle(in00, in01,shuf01)),0);	
			vector float input10 = spu_convtf(((vector unsigned int)spu_shuffle(in10, in11,shuf10)),0);
			vector float input11 = spu_convtf(((vector unsigned int)spu_shuffle(in10, in11,shuf11)),0);
			
			vector float w00=spu_mul(sc->wWfilterf0[2*i],wHfilter0); //px0,0 //first 4 pixel weight values
			vector float w01=spu_mul(sc->wWfilterf1[2*i],wHfilter0); // px 0,1 aka w(+1)
			vector float w10=spu_mul(sc->wWfilterf0[2*i],wHfilter1); // px1,0
			vector float w11=spu_mul(sc->wWfilterf1[2*i],wHfilter1);// px1,1
			
			vector float newpixel0=spu_mul(w00,input00);
			newpixel0=            spu_madd(w01,input01,newpixel0);
			newpixel0=            spu_madd(w10,input10,newpixel0);
			Out0=                 spu_madd(w11,input11,newpixel0);	

			vector float input02 = spu_convtf(((vector unsigned int)spu_shuffle(in02, in03,shuf02)),0);
			vector float input03 = spu_convtf(((vector unsigned int)spu_shuffle(in02, in03,shuf03)),0);
			vector float input12 = spu_convtf(((vector unsigned int)spu_shuffle(in12, in13,shuf12)),0);
			vector float input13 = spu_convtf(((vector unsigned int)spu_shuffle(in12, in13,shuf13)),0);

			vector float w02=spu_mul(sc->wWfilterf0[2*i+1],wHfilter0); //px0,0 //first 4 pixel weight values
			vector float w03=spu_mul(sc->wWfilterf1[2*i+1],wHfilter0); // px 0,1 aka w(+1)
			vector float w12=spu_mul(sc->wWfilterf0[2*i+1],wHfilter1); // px1,0
			vector float w13=spu_mul(sc->wWfilterf1[2*i+1],wHfilter1);// px1,1
			
			vector float newpixel1=spu_mul(w02,input02);
			newpixel1=            spu_madd(w03,input03,newpixel1);
			newpixel1=            spu_madd(w12,input12,newpixel1);
			Out1=                 spu_madd(w13,input13,newpixel1);	

			in00=nextin00;
			in01=nextin01;
			in02=nextin02;
			in03=nextin03;

			in10=nextin10;
			in11=nextin11;
			in12=nextin12;
			in13=nextin13;

			shuf00=nextshuf00;
			shuf01=nextshuf01;
			shuf02=nextshuf02;
			shuf03=nextshuf03;
			
			shuf10=nextshuf10;
			shuf11=nextshuf11;
			shuf12=nextshuf12;
			shuf13=nextshuf13;
			
			dest=dest + inc;
			inc=1;
		
		}
		sc->Outputf[2*dest]=Out0;
		sc->Outputf[2*dest+1]=Out1;
//	}
}

static inline void scale2(scaler_settings_t *sc)
{
	int i;
	int width=sc->width;
	vector short wHfilter0={0,sc->wHfilter,0,sc->wHfilter,0,sc->wHfilter,0,sc->wHfilter};
	vector short wHfilter1={0,2048-sc->wHfilter,0,2048-sc->wHfilter,0,2048-sc->wHfilter,0,2048-sc->wHfilter};
// 	vector short wHfilter0={0,sc->wHfilter,0,sc->wHfilter,0,sc->wHfilter,0,sc->wHfilter};
// 	vector short wHfilter1={0,65535-sc->wHfilter,0,65535-sc->wHfilter,0,65535-sc->wHfilter,0,65535-sc->wHfilter};
	int inc=0;
	int dest=0;
	vector int Out0;
	vector int Out1;

		vector unsigned char shuf00,shuf01,shuf02,shuf03,shuf10,shuf11,shuf12,shuf13;
		vector unsigned char in00,in01,in02,in03,in10,in11,in12,in13;
	
		vector unsigned char *line0sw0=sc->sWfilter0; 
		vector unsigned char *line0sw1=sc->sWfilter1;
		vector unsigned char *line1sw0=sc->sWfilter0;
		vector unsigned char *line1sw1=sc->sWfilter1;
		int *wfilterpos0=sc->wfilterpos;
		int *wfilterpos1=sc->wfilterpos;

		if (sc->smallcromaline0==1)
		{
			line0sw0=sc->crsWfilter0;
			line0sw1=sc->crsWfilter1;
			wfilterpos0=sc->crfilterpos;
		} 
	
		if (sc->smallcromaline1==1)
		{
			line1sw0=sc->crsWfilter0;
			line1sw1=sc->crsWfilter1;
			wfilterpos1=sc->crfilterpos;
		} 

		in00=sc->source00[wfilterpos0[0]];
		in01=sc->source00[wfilterpos0[0]+1];
		in02=sc->source00[wfilterpos0[1]];
		in03=sc->source00[wfilterpos0[1]+1];
		
		in10=sc->source01[wfilterpos1[0]];
		in11=sc->source01[wfilterpos1[0]+1];
		in12=sc->source01[wfilterpos1[1]];
		in13=sc->source01[wfilterpos1[1]+1];

		shuf00=line0sw0[0];
		shuf01=line0sw1[0];
		shuf02=line0sw0[1];
		shuf03=line0sw1[1];

		shuf10=line1sw0[1];
		shuf11=line1sw1[1];
		shuf12=line1sw0[1];
		shuf13=line1sw1[1];
/*		
		dest=0;
		inc=0;*/

		for (i=0;i < (width>>3);i++)
		{
			
		//	sc->Output[2*dest+1]=Out1;
			
// 			int nextpos00=wfilterpos0[2*i+2];
// 			int nextpos01=wfilterpos0[2*i+2]+1;
// 			int nextpos02=wfilterpos0[2*i+3];
// 			int nextpos03=wfilterpos0[2*i+3]+1;
// 
// 			int nextpos10=wfilterpos1[2*i+2];
// 			int nextpos11=wfilterpos1[2*i+2]+1;
// 			int nextpos12=wfilterpos1[2*i+3];
// 			int nextpos13=wfilterpos1[2*i+3]+1;
		
			int nextpos00=wfilterpos0[2*i];
			int nextpos01=wfilterpos0[2*i]+1;
			int nextpos02=wfilterpos0[2*i+1];
			int nextpos03=wfilterpos0[2*i+1]+1;

			int nextpos10=wfilterpos1[2*i];
			int nextpos11=wfilterpos1[2*i]+1;
			int nextpos12=wfilterpos1[2*i+1];
			int nextpos13=wfilterpos1[2*i+1]+1;			

// 			vector unsigned char nextin00=sc->source00[nextpos00];
// 			vector unsigned char nextin01=sc->source00[nextpos01];
// 			vector unsigned char nextin02=sc->source00[nextpos02];
// 			vector unsigned char nextin03=sc->source00[nextpos03];
// 			
// 			vector unsigned char nextin10=sc->source01[nextpos10];
// 			vector unsigned char nextin11=sc->source01[nextpos11];
// 			vector unsigned char nextin12=sc->source01[nextpos12];
// 			vector unsigned char nextin13=sc->source01[nextpos13];

			in00=sc->source00[nextpos00];
			in01=sc->source00[nextpos01];
			in02=sc->source00[nextpos02];
			in03=sc->source00[nextpos03];

			in10=sc->source01[nextpos10];
			in11=sc->source01[nextpos11];
			in12=sc->source01[nextpos12];
			in13=sc->source01[nextpos13];

			shuf00=line0sw0[2*i];
			shuf01=line0sw1[2*i];
			shuf02=line0sw0[2*i+1];
			shuf03=line0sw1[2*i+1];
			
			shuf10=line1sw0[2*i];
			shuf11=line1sw1[2*i];
			shuf12=line1sw0[2*i+1];
			shuf13=line1sw1[2*i+1];	
// 			shuf10=nextshuf10;
// 			shuf11=nextshuf11;
// 			shuf12=nextshuf12;
// 			shuf13=nextshuf13;
/*			vector unsigned char nextshuf00=line0sw0[2*i+2];
			vector unsigned char nextshuf01=line0sw1[2*i+2];
			vector unsigned char nextshuf02=line0sw0[2*i+3];
			vector unsigned char nextshuf03=line0sw1[2*i+3];

			vector unsigned char nextshuf10=line1sw0[2*i+2];
			vector unsigned char nextshuf11=line1sw1[2*i+2];
			vector unsigned char nextshuf12=line1sw0[2*i+3];
			vector unsigned char nextshuf13=line1sw1[2*i+3];*/	


			vector short input00 = (vector short)spu_shuffle(in00, in01,shuf00); //TODO make this smaller as were not using much space here
			vector short input01 = (vector short)spu_shuffle(in00, in01,shuf01);	
			vector short input10 = (vector short)spu_shuffle(in10, in11,shuf10);
			vector short input11 = (vector short)spu_shuffle(in10, in11,shuf11);
			
			vector int w00=spu_mulo(sc->wWfilter0[2*i],wHfilter0); //px0,0 //first 4 pixel weight values
			vector int w01=spu_mulo(sc->wWfilter1[2*i],wHfilter0); // px 0,1 aka w(+1)
			vector int w10=spu_mulo(sc->wWfilter0[2*i],wHfilter1); // px1,0
			vector int w11=spu_mulo(sc->wWfilter1[2*i],wHfilter1);// px1,1

			w00=spu_rlmaska(w00,-11);			
			w01=spu_rlmaska(w01,-11);
			w10=spu_rlmaska(w10,-11);
			w11=spu_rlmaska(w11,-11);
// 			vector int w00=spu_mulsr(sc->wWfilter0[2*i],wHfilter0); //px0,0 //first 4 pixel weight values
// 			vector int w01=spu_mulsr(sc->wWfilter1[2*i],wHfilter0); // px 0,1 aka w(+1)
// 			vector int w10=spu_mulsr(sc->wWfilter0[2*i],wHfilter1); // px1,0
// 			vector int w11=spu_mulsr(sc->wWfilter1[2*i],wHfilter1);// px1,1
			vector int newpixel0=spu_mulo((vector short)w00,input00);
			newpixel0=            spu_madd((vector short)w01,input01,newpixel0);
			newpixel0=            spu_madd((vector short)w10,input10,newpixel0);
			Out0=                 spu_madd((vector short)w11,input11,newpixel0);	
			Out0=spu_rlmaska(Out0,-11);

	
			vector short input02 = (vector short)spu_shuffle(in02, in03,shuf02);
			vector short input03 = (vector short)spu_shuffle(in02, in03,shuf03);
			vector short input12 = (vector short)spu_shuffle(in12, in13,shuf12);
			vector short input13 = (vector short)spu_shuffle(in12, in13,shuf13);

//  			vector int w02=spu_mulsr(sc->wWfilter0[2*i+1],wHfilter0); //px0,0 //first 4 pixel weight values
// 			vector int w03=spu_mulsr(sc->wWfilter1[2*i+1],wHfilter0); // px 0,1 aka w(+1)
// 			vector int w12=spu_mulsr(sc->wWfilter0[2*i+1],wHfilter1); // px1,0
// 			vector int w13=spu_mulsr(sc->wWfilter1[2*i+1],wHfilter1);// px1,1
			vector int w02=spu_mulo(sc->wWfilter0[2*i+1],wHfilter0); //px0,0 //first 4 pixel weight values
			vector int w03=spu_mulo(sc->wWfilter1[2*i+1],wHfilter0); // px 0,1 aka w(+1)
			vector int w12=spu_mulo(sc->wWfilter0[2*i+1],wHfilter1); // px1,0
			vector int w13=spu_mulo(sc->wWfilter1[2*i+1],wHfilter1);// px1,1

			w02=spu_rlmaska(w02,-11);			
			w03=spu_rlmaska(w03,-11);
			w12=spu_rlmaska(w12,-11);
			w13=spu_rlmaska(w13,-11);
			vector int newpixel1=spu_mulo((vector short)w02,input02);
			newpixel1=            spu_madd((vector short)w03,input03,newpixel1);
			newpixel1=            spu_madd((vector short)w12,input12,newpixel1);
			Out1=                 spu_madd((vector short)w13,input13,newpixel1);	
			Out1=spu_rlmaska(Out1,-11);
	
// 			in00=nextin00;
// 			in01=nextin01;
// 			in02=nextin02;
// 			in03=nextin03;
// 
// 			in10=nextin10;
// 			in11=nextin11;
// 			in12=nextin12;
// 			in13=nextin13;

// 			shuf00=nextshuf00;
// 			shuf01=nextshuf01;
// 			shuf02=nextshuf02;
// 			shuf03=nextshuf03;
// 			
// 			shuf10=nextshuf10;
// 			shuf11=nextshuf11;
// 			shuf12=nextshuf12;
// 			shuf13=nextshuf13;
			sc->Output[i]=(vector signed short)spu_shuffle(Out0,Out1,((vector unsigned char){2,3,6,7,10,11,14,15,18,19,22,23,26,27,30,31}));
// 			dest=dest + inc;
// 			inc=1;
		
		}
		sc->Output[dest]=(vector signed short)spu_shuffle(Out0,Out1,((vector unsigned char){2,3,6,7,10,11,14,15,18,19,22,23,26,27,30,31}));
	//	sc->Output[2*dest+1]=Out1;
//	}
}

// static inline vector unsigned char packfARGB(vector float R, vector float G, vector float B)
// {
// 		vector unsigned int max = spu_splats((unsigned int)0x00FF);
// 		static vector unsigned char Rff={0,19,2,3,4,23,6,7,8,27,10,11,12,31,14,15};
// 		static vector unsigned char Gff={0,1,19,3,4,5,23,7,8,9 ,27,11,12,13,31,15};
// 		vector unsigned int Ri=spu_convtu(R,0);
// 		Ri= spu_sel(Ri, max, spu_cmpgt(Ri,255));
// 		vector unsigned int Gi=spu_convtu(G,0);
// 		Gi= spu_sel(Gi, max, spu_cmpgt(Gi,255));
// 		vector unsigned int Bi=spu_convtu(B,0);
// 		Bi= spu_sel(Bi, max, spu_cmpgt(Bi,255));
// 		vector unsigned char ARGB;
// 		ARGB=spu_shuffle((vector unsigned char)Bi,(vector unsigned char)Ri,Rff);
// 		ARGB=spu_shuffle(ARGB,(vector unsigned char)Gi,Gff);
// 		return ARGB;
// }
// static inline void yuv420toARGBint(vector int *Y0,vector int *Y1,vector int *U, vector int *V,vector unsigned char *ARGB0,vector unsigned char *ARGB1, int width,int maxwidth)
// {
// 
// 		vector int Yf0,Yf1,Yf2,Yf3,Yf4,Yf5,Yf6,Yf7;
// 		vector int Uf0,Uf1;
// 		vector int Vf0,Vf1;
// 		vector int R00,R01,R02,R03,R10,R11,R12,R13,G00,G01,G02,G03,G10,G11,G12,G13,B00,B01,B02,B03,B10,B11,B12,B13;
// 		vector int yfv,ufuv,vfu;
// 		vector int E,D;
// 
// 		int i;
// 
// 		int dest=0;
// 		int inc=0;
// 			
// 		Yf0=Y0[0];
// 		Yf1=Y0[1];
// 		Yf2=Y0[2];
// 		Yf3=Y0[3];
// 		Yf4=Y1[0];
// 		Yf5=Y1[1];
// 		Yf6=Y1[2];
// 		Yf7=Y1[3];
// 		
// 		Uf0=U[0];
// 		Uf1=U[1];
// 			
// 		Vf0=V[0];
// 		Vf1=V[1];
// 
// 
// 		for (i =0;i < width>>4;i++) {
// 			vector int nextYf0=Y0[4*i+4];
// 			vector int nextYf1=Y0[4*i+5];
// 			vector int nextYf2=Y0[4*i+6];
// 			vector int nextYf3=Y0[4*i+7];
// 			
// 			vector int nextYf4=Y1[4*i+4];
// 			vector int nextYf5=Y1[4*i+5];
// 			vector int nextYf6=Y1[4*i+6];
// 			vector int nextYf7=Y1[4*i+7];
// 
// 			
// 			vector int nextUf0=U[2*i+2];
// 			vector int nextUf1=U[2*i+3];
// 			
// 			vector int nextVf0=V[2*i+2];
// 			vector int nextVf1=V[2*i+3];
// 
// 			E=spu_sub(Vf0,((vector float){128.0,128.0,128.0,128.0}));
// 			D=spu_sub(Uf0,((vector float){128.0,128.0,128.0,128.0}));
// 
// 			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
// 			
// 			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
// 			
// 			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));
// 
// 			R00=spu_add(Yf0,unpackfaabb(yfv));
// 			G00=spu_sub(Yf0,unpackfaabb(ufuv));
// 			B00=spu_add(Yf0,unpackfaabb(vfu));
// 
// 
// 			R10=spu_add(Yf4,unpackfaabb(yfv));
// 			G10=spu_sub(Yf4,unpackfaabb(ufuv));
// 			B10=spu_add(Yf4,unpackfaabb(vfu));
// 		
// 		
// 			R01=spu_add(Yf1,unpackfccdd(yfv));
// 			G01=spu_sub(Yf1,unpackfccdd(ufuv));
// 			B01=spu_add(Yf1,unpackfccdd(vfu));
// 
// 
// 			R11=spu_add(Yf5,unpackfccdd(yfv));
// 			G11=spu_sub(Yf5,unpackfccdd(ufuv));
// 			B11=spu_add(Yf5,unpackfccdd(vfu));
// 
// 
// 			E=spu_sub(Vf1,((vector float){128.0,128.0,128.0,128.0}));
// 			D=spu_sub(Uf1,((vector float){128.0,128.0,128.0,128.0}));
// 
// 			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
// 
// 			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
// 	
// 			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));
// 
// 			R02=spu_add(Yf2,unpackfaabb(yfv));
// 			G02=spu_sub(Yf2,unpackfaabb(ufuv));
// 			B02=spu_add(Yf2,unpackfaabb(vfu));
// 
// 			R12=spu_add(Yf6,unpackfaabb(yfv));
// 			G12=spu_sub(Yf6,unpackfaabb(ufuv));
// 			B12=spu_add(Yf6,unpackfaabb(vfu));
// 
// 
// 		
// 			R03=spu_add(Yf3,unpackfccdd(yfv));
// 			G03=spu_sub(Yf3,unpackfccdd(ufuv));
// 			B03=spu_add(Yf3,unpackfccdd(vfu));
// 	
// 
// 
// 			R13=spu_add(Yf7,unpackfccdd(yfv));
// 			G13=spu_sub(Yf7,unpackfccdd(ufuv));
// 			B13=spu_add(Yf7,unpackfccdd(vfu));
// 	
// 
// 			ARGB0[4*i]  = packfARGB(R00,G00,B00);
// 			ARGB0[4*i+1]= packfARGB(R01,G01,B01);
// 			ARGB0[4*i+2]= packfARGB(R02,G02,B02);
// 			ARGB0[4*i+3]= packfARGB(R03,G03,B03);
// 			
// 			ARGB1[4*i]  = packfARGB(R10,G10,B10);
// 			ARGB1[4*i+1]= packfARGB(R11,G11,B11);
// 			ARGB1[4*i+2]= packfARGB(R12,G12,B12);
// 			ARGB1[4*i+3]= packfARGB(R13,G13,B13);
// 
// 			Yf0=nextYf0;
// 			Yf1=nextYf1;
// 			Yf2=nextYf2;
// 			Yf3=nextYf3;
// 			Yf4=nextYf4;
// 			Yf5=nextYf5;
// 			Yf6=nextYf6;
// 			Yf7=nextYf7;
// 			
// 			Uf0=nextUf0;
// 			Uf1=nextUf1;
// 			
// 			Vf0=nextVf0;
// 			Vf1=nextVf1;
// 
// 			dest=dest + inc;
// 			inc=1;
// 		}
// 
// }
// static inline void yuv420toARGBfloat(vector float *Y0,vector float *Y1,vector float *U, vector float *V,vector unsigned char *ARGB0,vector unsigned char *ARGB1, int width,int maxwidth)
// {
// 
// 		vector float Yf0,Yf1,Yf2,Yf3,Yf4,Yf5,Yf6,Yf7;
// 		vector float Uf0,Uf1;
// 		vector float Vf0,Vf1;
// 		vector float R00,R01,R02,R03,R10,R11,R12,R13,G00,G01,G02,G03,G10,G11,G12,G13,B00,B01,B02,B03,B10,B11,B12,B13;
// 		vector float yfv,ufuv,vfu;
// 		vector float E,D;
// 
// 		int i;
// 
// 		int dest=0;
// 		int inc=0;
// 			
// 		Yf0=Y0[0];
// 		Yf1=Y0[1];
// 		Yf2=Y0[2];
// 		Yf3=Y0[3];
// 		Yf4=Y1[0];
// 		Yf5=Y1[1];
// 		Yf6=Y1[2];
// 		Yf7=Y1[3];
// 		
// 		Uf0=U[0];
// 		Uf1=U[1];
// 			
// 		Vf0=V[0];
// 		Vf1=V[1];
// 
// 
// 		for (i =0;i < width>>4;i++) {
// 			vector float nextYf0=Y0[4*i+4];
// 			vector float nextYf1=Y0[4*i+5];
// 			vector float nextYf2=Y0[4*i+6];
// 			vector float nextYf3=Y0[4*i+7];
// 			
// 			vector float nextYf4=Y1[4*i+4];
// 			vector float nextYf5=Y1[4*i+5];
// 			vector float nextYf6=Y1[4*i+6];
// 			vector float nextYf7=Y1[4*i+7];
// 
// 			
// 			vector float nextUf0=U[2*i+2];
// 			vector float nextUf1=U[2*i+3];
// 			
// 			vector float nextVf0=V[2*i+2];
// 			vector float nextVf1=V[2*i+3];
// 
// 			E=spu_sub(Vf0,((vector float){128.0,128.0,128.0,128.0}));
// 			D=spu_sub(Uf0,((vector float){128.0,128.0,128.0,128.0}));
// 
// 			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
// 			
// 			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
// 			
// 			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));
// 
// 			R00=spu_add(Yf0,unpackfaabb(yfv));
// 			G00=spu_sub(Yf0,unpackfaabb(ufuv));
// 			B00=spu_add(Yf0,unpackfaabb(vfu));
// 
// 
// 			R10=spu_add(Yf4,unpackfaabb(yfv));
// 			G10=spu_sub(Yf4,unpackfaabb(ufuv));
// 			B10=spu_add(Yf4,unpackfaabb(vfu));
// 		
// 		
// 			R01=spu_add(Yf1,unpackfccdd(yfv));
// 			G01=spu_sub(Yf1,unpackfccdd(ufuv));
// 			B01=spu_add(Yf1,unpackfccdd(vfu));
// 
// 
// 			R11=spu_add(Yf5,unpackfccdd(yfv));
// 			G11=spu_sub(Yf5,unpackfccdd(ufuv));
// 			B11=spu_add(Yf5,unpackfccdd(vfu));
// 
// 
// 			E=spu_sub(Vf1,((vector float){128.0,128.0,128.0,128.0}));
// 			D=spu_sub(Uf1,((vector float){128.0,128.0,128.0,128.0}));
// 
// 			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
// 
// 			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
// 	
// 			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));
// 
// 			R02=spu_add(Yf2,unpackfaabb(yfv));
// 			G02=spu_sub(Yf2,unpackfaabb(ufuv));
// 			B02=spu_add(Yf2,unpackfaabb(vfu));
// 
// 			R12=spu_add(Yf6,unpackfaabb(yfv));
// 			G12=spu_sub(Yf6,unpackfaabb(ufuv));
// 			B12=spu_add(Yf6,unpackfaabb(vfu));
// 
// 
// 		
// 			R03=spu_add(Yf3,unpackfccdd(yfv));
// 			G03=spu_sub(Yf3,unpackfccdd(ufuv));
// 			B03=spu_add(Yf3,unpackfccdd(vfu));
// 	
// 
// 
// 			R13=spu_add(Yf7,unpackfccdd(yfv));
// 			G13=spu_sub(Yf7,unpackfccdd(ufuv));
// 			B13=spu_add(Yf7,unpackfccdd(vfu));
// 	
// 
// 			ARGB0[4*i]  = packfARGB(R00,G00,B00);
// 			ARGB0[4*i+1]= packfARGB(R01,G01,B01);
// 			ARGB0[4*i+2]= packfARGB(R02,G02,B02);
// 			ARGB0[4*i+3]= packfARGB(R03,G03,B03);
// 			
// 			ARGB1[4*i]  = packfARGB(R10,G10,B10);
// 			ARGB1[4*i+1]= packfARGB(R11,G11,B11);
// 			ARGB1[4*i+2]= packfARGB(R12,G12,B12);
// 			ARGB1[4*i+3]= packfARGB(R13,G13,B13);
// 
// 			Yf0=nextYf0;
// 			Yf1=nextYf1;
// 			Yf2=nextYf2;
// 			Yf3=nextYf3;
// 			Yf4=nextYf4;
// 			Yf5=nextYf5;
// 			Yf6=nextYf6;
// 			Yf7=nextYf7;
// 			
// 			Uf0=nextUf0;
// 			Uf1=nextUf1;
// 			
// 			Vf0=nextVf0;
// 			Vf1=nextVf1;
// 
// 			dest=dest + inc;
// 			inc=1;
// 		}
// 
// }

static inline void unpack(scaler_settings_t *sc)
{
	
	int i;
	if (!sc->smallcromaline0) {
		for (i=0;i < sc->width>>4 ;i++) 
		{
			sc->Outputf[4*i]  = unpackhh(sc->source00[i]);
			sc->Outputf[4*i+1]= unpacklh(sc->source00[i]);
			sc->Outputf[4*i+2]= unpackhl(sc->source00[i]);
			sc->Outputf[4*i+3]= unpackll(sc->source00[i]);
		}
	} else {
		sc->Outputf[0]=unpackhl(sc->source00[0]);
		sc->Outputf[1]=unpackll(sc->source00[0]);
		for (i=1;i<sc->width/16 + 1;i++)
		{
			sc->Outputf[4*i-2]=unpackhh(sc->source00[i]);
			sc->Outputf[4*i-1]=unpacklh(sc->source00[i]);
			sc->Outputf[4*i]  =unpackhl(sc->source00[i]);
			sc->Outputf[4*i+1]=unpackll(sc->source00[i]);
		}
	}
	
}

static inline void unpack2(scaler_settings_t *sc)
{
	
	int i;
	if (!sc->smallcromaline0) {
		for (i=0;i < sc->width>>5 ;i++) 
		{
			sc->Output[4*i]  = (vector short)spu_shuffle(sc->source00[2*i],sc->source00[2*i],((vector unsigned char){0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7}));
			sc->Output[4*i+1]= (vector short)spu_shuffle(sc->source00[2*i],sc->source00[2*i],((vector unsigned char){0x80,8,0x80,9,0x80,10,0x80,11,0x80,12,0x80,13,0x80,14,0x80,15}));
			sc->Output[4*i+2]  = (vector short)spu_shuffle(sc->source00[2*i+1],sc->source00[2*i+1],((vector unsigned char){0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7}));
			sc->Output[4*i+3]= (vector short)spu_shuffle(sc->source00[2*i+1],sc->source00[2*i+1],((vector unsigned char){0x80,8,0x80,9,0x80,10,0x80,11,0x80,12,0x80,13,0x80,14,0x80,15}));
// 			sc->Output[4*i+2]= unpackhl(sc->source00[i]);
// 			sc->Output[4*i+3]= unpackll(sc->source00[i]);
		}
	} else {
		sc->Output[2*i]  =(vector short) spu_shuffle(sc->source00[i],sc->source00[i],((vector unsigned char){0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7}));
		for (i=1;i<sc->width/16 + 1;i++)
		{
			sc->Output[2*i-1]=(vector short)spu_shuffle(sc->source00[i],sc->source00[i],((vector unsigned char){0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7}));
			sc->Output[2*i]=(vector short)spu_shuffle(sc->source00[i],sc->source00[i],((vector unsigned char){0x80,8,0x80,9,0x80,10,0x80,11,0x80,12,0x80,13,0x80,14,0x80,15}));
// 			sc->Output[4*i]	 =unpackhl(sc->source00[i]);
// 			sc->Output[4*i+1]=unpackll(sc->source00[i]);
		}
	}
	
}

#endif
