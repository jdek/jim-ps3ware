/**
 * SPU YUV420/YV12 scaler to ARGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_yuv2argb_scaler.h - Main loop for the spu yuv2argb_scaler.
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

#include <spu_mfcio.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <spu_dmalib.h>
#include <yuv_datastructs.h>
#include <spu_control.h>
#include <spu_print.h>
#include "scaler_settings.h"
#include "spu_scaler_argb.h"

#define MAXWIDTH 1920
#define MAXHEIGHT 1080



int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgiy0[2];
	int tgiy1[2];
	int tgiu0[2];
	int tgiu1[2];
	int tgiv0[2];
	int tgiv1[2];
	int tgo[2];
	
	tgiu1[0]=1;
	tgiu1[1]=2;
	tgo[0]=3;
	tgo[1]=4;
	tgiy0[0]=5;
	tgiy0[1]=6;
	tgiy1[0]=7;
	tgiy1[1]=8;
	tgiu0[0]=9;
	tgiu0[1]=10;
	tgiv0[0]=11;
	tgiv0[1]=12;
	tgiv1[1]=13;
	tgiv1[1]=14;
	
	int selOut = 0;
	int selIn = 0;
	int tag = 16;
	int LineSelIn=0;
	int LineSelOut=0;
		
	int selY0In = 0;
	int selY1In = 0;
	int selCrIn = 0;
	struct img_args *iargs;
	
	iargs =(struct img_args*)memalign(128,sizeof(*iargs));

	unsigned long long Cp;

	int first=1;

	unsigned long long Op;
	unsigned int msg;
	unsigned long long YIp,UIp,VIp,YOp;

	int crblock0;
	int crblock1;
	int srcsmallcroma=0;
;
	int noscale=1;

 	static	int crblockdst1;
	static	int crblockdst0;
	scaler_settings_t sc;

	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process image
	printf("spu_yuv2argb_scaler: SRC width %i,DST width %i\n",iargs->srcW,iargs->dstW);
	
	vector unsigned char *widthfilter0=(vector unsigned char*)memalign(128,MAXWIDTH*4+16);
	vector unsigned char *widthfilter1=(vector unsigned char*)memalign(128,MAXWIDTH*4+16);

	vector unsigned char *crwidthfilter0=(vector unsigned char*)memalign(128,MAXWIDTH*2+16);
	vector unsigned char *crwidthfilter1=(vector unsigned char*)memalign(128,MAXWIDTH*2+16);	

	vector float * weightWfilter0=(vector float*)memalign(128,MAXWIDTH*4+16);
	vector float * weightWfilter1=(vector float*)memalign(128,MAXWIDTH*4+16);

	float weightHfilter[MAXHEIGHT+1];

	int dmacromapos[MAXHEIGHT/2+2];
	int dmapos[MAXHEIGHT+2];
	
	vector float * Ytemp0=(vector float *)memalign(128,MAXWIDTH*4+16);
	vector float * Ytemp1=(vector float *)memalign(128,MAXWIDTH*4+16);
	vector float * Utemp=(vector float *)memalign(128,MAXWIDTH*2+16);
	vector float * Vtemp=(vector float *)memalign(128,MAXWIDTH*2+16);

	int wfilterpos[MAXWIDTH+1];
	int hfilterpos0[MAXHEIGHT+2];
	int hfilterpos1[MAXHEIGHT+2];
	int crwfilterpos[MAXWIDTH/2+1];

	vector unsigned char *InputY0[2];
	InputY0[0]=(vector unsigned char*)memalign(128,MAXWIDTH); 
	InputY0[1]=(vector unsigned char*)memalign(128,MAXWIDTH); 

	vector unsigned char *InputU0[2];
	InputU0[0]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16); 
	InputU0[1]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16); 
	
	vector unsigned char *InputV0[2];
	InputV0[0]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16); 
	InputV0[1]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16);

	vector unsigned char *InputY1[2];
	InputY1[0]=(vector unsigned char*)memalign(128,MAXWIDTH); 
	InputY1[1]=(vector unsigned char*)memalign(128,MAXWIDTH); 

	vector unsigned char *InputU1[2];
	InputU1[0]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16); 
	InputU1[1]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16);

	vector unsigned char *InputV1[2];
	InputV1[0]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16); 
	InputV1[1]=(vector unsigned char*)memalign(128,MAXWIDTH/2+16); 	

	vector unsigned char* Output[2];
	Output[0]=(vector unsigned char*)memalign(128,MAXWIDTH*8);	// 1line output
	Output[1]=(vector unsigned char*)memalign(128,MAXWIDTH*8);	// 1line output
	
	while (spu_stat_in_mbox() == 0);
		msg=spu_read_in_mbox();
	if (msg==RUN){	
		printf("spu_yuv2argb_scaler: Starting Up\n");
	}
	
	while (msg!=STOP) 
	{
		int h=0;
		int i;
		Op=iargs->Output[selOut];
		
		if (first)
		{
			crblock0=(iargs->srcW>>1)&~15; // rounded down
			crblock1=((iargs->srcW>>1) + 15)&~15; //rounded up
			crblockdst1=((iargs->dstW>>1) + 15)&~15;//destination size rounded up.
			crblockdst0=((iargs->dstW>>1) + 7)&~7;//destination size rounded up.

			initHFilter(iargs->srcW,iargs->srcH,iargs->dstH,hfilterpos0,hfilterpos1,weightHfilter,dmapos,dmacromapos);
			
			for (i=0;i < iargs->dstH;i++)
			{
				printf("Hfilterpos0 dst: %d, src:%d, weight:%f\n",i,hfilterpos0[i],weightHfilter[i]);
				printf("Hfilterpos1 dst: %d, src:%d, weight:%f\n",i,hfilterpos1[i],1.0-weightHfilter[i]);
			}
			
			if ((iargs->srcW==iargs->dstW)&&(iargs->srcH==iargs->dstH))
			{
				printf("spu_yuv2argb_scaler: No scaling proceeding with direct csc\n");
				noscale=1;
				
			} else {

				noscale=0;
				printf("spu_yuv2argb_scaler: Scaling, computing shuffle filters\n");

				initWFilter(iargs->srcW,iargs->dstW,1,wfilterpos,widthfilter0,widthfilter1,weightWfilter0,weightWfilter1);

				for (i=0;i < iargs->dstW/4;i++)
				{
					printf("filterpos dst: %d, src:%d\n",i,wfilterpos[i]);
					printcharvec("widthfilter0",widthfilter0[i]);
					printcharvec("widthfilter1",widthfilter1[i]);
					printfvec("weightWfilter0",weightWfilter0[i]);
					printfvec("weightWfilter1",weightWfilter1[i]);
				}				

				srcsmallcroma=0;
				sc.smallcroma=0;
				if ((iargs->srcW%32) != 0)
				{
					sc.smallcroma=1;
					srcsmallcroma=1;	
					initWcrFilter(iargs->srcW/2,iargs->dstW/2,1,crwfilterpos,crwidthfilter0,crwidthfilter1);	
					printf("spu_yuv2argb_scaler: Computing Crshuffle filter\n");
	
					for (i=0;i < (iargs->dstW>>1)/4;i++)
					{
						printf("crwfilterpos dst: %d, src:%d, weight:%f\n",i,crwfilterpos[i]);
						printcharvec("crwidthfilter0",crwidthfilter0[i]);
						printcharvec("crwidthfilter1",crwidthfilter1[i]);
						printfvec("weightWfilter0",weightWfilter0[i]);
						printfvec("weightWfilter1",weightWfilter1[i]);
					
					}
							
				}
				
				sc.wWfilter0=weightWfilter0;
				sc.wWfilter1=weightWfilter1;
				sc.wfilterpos=wfilterpos;
				sc.sWfilter0=widthfilter0; 
				sc.sWfilter1=widthfilter1;
				sc.crsWfilter0=crwidthfilter0;
				sc.crsWfilter1=crwidthfilter1;
				sc.crfilterpos=crwfilterpos;

				sc.smallcromaline=0;
				
			}
			first=0;
			printf("spu_yuv2argb_scaler: Initiation completed\n");
		}

	
		YIp = iargs->Ystart[selIn];
		UIp = iargs->Ustart[selIn];
		VIp = iargs->Vstart[selIn];
		Op = iargs->Output[selOut];

		
		LineSelOut=0;
		selY0In=0; 
		selY1In=0;
		selCrIn=0;


		dmaGet(InputY0[0],YIp+dmapos[hfilterpos0[0]],iargs->srcW,tgiy0[0]); 
		dmaGet(InputY1[0],YIp+dmapos[hfilterpos1[0]],iargs->srcW,tgiy1[0]); 
		dmaGet(InputY0[1],YIp+dmapos[hfilterpos0[1]],iargs->srcW,tgiy0[1]); 
		dmaGet(InputY1[1],YIp+dmapos[hfilterpos1[1]],iargs->srcW,tgiy1[1]); 
		

		dmaGet(InputU0[0],UIp+dmacromapos[hfilterpos0[0]],crblock1,tgiu0[0]);
		dmaGet(InputU0[1],UIp+dmacromapos[hfilterpos0[1]],crblock1,tgiu0[1]);
		dmaGet(InputU1[0],UIp+dmacromapos[hfilterpos1[0]],crblock1,tgiu1[0]);	
		dmaGet(InputU1[1],UIp+dmacromapos[hfilterpos1[1]],crblock1,tgiu1[1]); 

		dmaGet(InputV0[0],VIp+dmacromapos[hfilterpos0[0]],crblock1,tgiv0[0]);
		dmaGet(InputV0[1],VIp+dmacromapos[hfilterpos0[1]],crblock1,tgiv0[1]);
		dmaGet(InputV1[0],VIp+dmacromapos[hfilterpos1[0]],crblock1,tgiv1[0]);	
		dmaGet(InputV1[1],VIp+dmacromapos[hfilterpos1[1]],crblock1,tgiv1[1]);



		for (h=0; h < iargs->dstH>>1; h++) //asume that output is allways h/2
		{

			sc.width=iargs->dstW;
			sc.smallcroma=0;

			sc.wHfilter=weightHfilter[2*h];
			dmaWaitTag(tgiy0[selY0In]);
			dmaWaitTag(tgiy1[selY1In]);
			sc.source00=InputY0[selY0In];
			sc.source01=InputY1[selY1In];
			sc.Output=Ytemp0;
			
			if (noscale) {
				unpack(&sc);
			} else {
				scale(&sc);	
			}	
								//first Y line scaled
			dmaGet(InputY0[selY0In],YIp+dmapos[hfilterpos0[2*h+2]],iargs->srcW,tgiy0[selY0In]); 
			if (!noscale) { //if we are scaling we also need the second line
				dmaGet(InputY1[selY1In],YIp+dmapos[hfilterpos1[2*h+2]],iargs->srcW,tgiy1[selY1In]); 
			}
			selY0In=selY0In^1;
			selY1In=selY1In^1;
			

			sc.wHfilter=weightHfilter[2*h+1];
			dmaWaitTag(tgiy0[selY0In]);
			dmaWaitTag(tgiy1[selY0In]);
			sc.source00=InputY0[selY0In];
			sc.source01=InputY1[selY0In];
			sc.Output=Ytemp1;
			if (noscale) {
				unpack(&sc);
			} else {
				scale(&sc);	
			}								//second Y line scaled
			dmaGet(InputY0[selY0In],YIp+dmapos[hfilterpos0[2*h+3]],iargs->srcW,tgiy0[selY0In]); 
			if(!noscale) { //if we are scaling we also need the second line
				dmaGet(InputY1[selY1In],YIp+dmapos[hfilterpos1[2*h+3]],iargs->srcW,tgiy1[selY1In]); 
		
			}
			selY0In=selY0In^1;
			selY1In=selY1In^1;
		
			if (srcsmallcroma) //these settings applly for both U and V
			{	
				sc.smallcroma=1;
				if ((hfilterpos0[h]&1)==1) {
					sc.smallcromaline=0;	
				} else if ((hfilterpos1[h]&1)==1){
					sc.smallcromaline=1;
				} else {
					sc.smallcroma=0; //both lines are 128 bit alligned only when doing extreme downscaling can this happen
				}
			}
			if (noscale) {
				sc.width=crblockdst0;//crblockdst1;
			} else {
				sc.width=crblockdst0;
			}
		//	sc.width=iargs->dstW/2;
			sc.wHfilter=weightHfilter[h];
			
	
			dmaWaitTag(tgiu0[selCrIn]);
			dmaWaitTag(tgiu1[selCrIn]);
			sc.Output=Utemp;
			sc.source00=InputU0[selCrIn];
			sc.source01=InputU1[selCrIn];
			if (noscale) {
				unpack(&sc);
			} else {
				scale(&sc);	
			}

			dmaWaitTag(tgiv0[selCrIn]);
			dmaWaitTag(tgiv1[selCrIn]);
			sc.Output=Vtemp;
			sc.source00=InputV0[selCrIn];
			sc.source01=InputV1[selCrIn];
			if (noscale) {
				unpack(&sc);
			} else {
				scale(&sc);	
			}

			dmaGet(InputV0[selCrIn],VIp+dmacromapos[hfilterpos0[h+2]],crblock1,tgiu0[selCrIn]); 		//this is allways pos 0 
			dmaGet(InputU0[selCrIn],UIp+dmacromapos[hfilterpos0[h+2]],crblock1,tgiv0[selCrIn]); 
			if(!noscale) {	//if we are scaling we also need the second line
				dmaGet(InputV1[selCrIn],VIp+dmacromapos[hfilterpos1[h+2]],crblock1,tgiu1[selCrIn]);
				dmaGet(InputU1[selCrIn],UIp+dmacromapos[hfilterpos1[h+2]],crblock1,tgiv1[selCrIn]); 
			} 

			selCrIn=selCrIn^1;
							
			yuv420toARGBfloat(Ytemp0,Ytemp1,Utemp,Vtemp,Output[LineSelOut],iargs->dstW,iargs->maxwidth);
			dmaPut(Output[LineSelOut],Op,iargs->maxwidth*8,tgo[LineSelOut]);
			Op=Op+iargs->maxwidth*8;
			
			LineSelOut=LineSelOut^1;
		} 
	
		
		while (spu_stat_out_intr_mbox() == 0);
		msg=RDY;
		spu_writech(SPU_WrOutIntrMbox, msg);
		
		while (spu_stat_in_mbox() == 0);
		msg=spu_read_in_mbox();
		
		if (msg == RUN){
			
		}
		else if (msg == STOP)
		{
			printf("spu_yuvscaler: Stopping\n");
		}
		else if (msg == UPDATE)
		{
			dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process the new image	
			first=0; // update filters to reflect the new image!
		}
		selOut = selOut ^ 1; // flips the output buffer pointers
		selIn = selIn ^ 1; // flips the input buffer pointers
	}
	
	return 0;
}
