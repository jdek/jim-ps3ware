/**
 * SPU YUV scaler kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_yuvscaler.h - Main loop for the spu scaler.
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
#include "libspedma.h"
#include "yuv_datastructs.h"
#include "spu_control.h"
#include "spu_scaler.h"
#include "spu_print.h"

#define MAXWIDTH 1920
#define MAXHEIGHT 1080

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgi[2];
	int tgo[2];
	tgi[0]=1;
	tgi[1]=2;
	tgi[0]=3;
	tgi[1]=4;
	
	int selOut = 0;
	int selIn = 0;
	int tag = 16;
	int LineSelIn=0;
	int LineSelOut=0;
	struct img_args *iargs;
	
	iargs =(struct img_args*)memalign(128,sizeof(*iargs));
	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process image
	printf("SRC width %d,DST width %d\n",iargs->srcW,iargs->dstW);
	printf("iargs->Ystart=%p\n",(int)iargs->Ystart[0]);

	vector unsigned char *widthfilter0=(vector unsigned char*)memalign(128,MAXWIDTH*4);
	vector unsigned char *widthfilter1=(vector unsigned char*)memalign(128,MAXWIDTH*4);

	vector unsigned char *crwidthfilter0=(vector unsigned char*)memalign(128,MAXWIDTH*2);
	vector unsigned char *crwidthfilter1=(vector unsigned char*)memalign(128,MAXWIDTH*2);	

	vector float * weightWfilter0=(vector float*)memalign(128,MAXWIDTH*4);
	vector float * weightWfilter1=(vector float*)memalign(128,MAXWIDTH*4);
	vector float * weightHfilter0=(vector float*)memalign(128,MAXHEIGHT*16);
	vector float * weightHfilter1=(vector float*)memalign(128,MAXHEIGHT*16);

	int wfilterpos[MAXWIDTH*4];
	int hfilterpos[MAXHEIGHT];
	int crwfilterpos[MAXWIDTH*2];

	vector unsigned char *Input[2];
	Input[0]=(vector unsigned char*)memalign(128,MAXWIDTH); 
	Input[1]=(vector unsigned char*)memalign(128,MAXWIDTH); 
	
	vector unsigned char* Output[2];
	Output[0]=(vector unsigned char*)memalign(128,MAXWIDTH);	// 1line output
	Output[1]=(vector unsigned char*)memalign(128,MAXWIDTH);	// 1line output
	
	vector float * fBuffer00=(vector float*)memalign(128,MAXWIDTH*4);// defined as max to avoid problems due to change of scalefactor..
	vector float * fBuffer01=(vector float*)memalign(128,MAXWIDTH*4);// defined as max to avoid problems due to change of scalefactor..
	vector float * fBuffer10=(vector float*)memalign(128,MAXWIDTH*4);// defined as max to avoid problems due to change of scalefactor..
	vector float * fBuffer11=(vector float*)memalign(128,MAXWIDTH*4);// defined as max to avoid problems due to change of scalefactor..

	unsigned long long Cp;

	int first=1;

	unsigned long long Op;//, Yp, Up, Vp;
	unsigned int msg;
	unsigned long long YIp,UIp,VIp,YOp;
	unsigned long long UOp,VOp;
	int crblock0;
	int crblock1;
	int crblockdst0;
	int crblockdst1;
	while (spu_stat_in_mbox() == 0);
		msg=spu_read_in_mbox();
	if (msg==RUN){	
		printf("RUN\n");
	}
	
	while (msg!=STOP) 
	{
		int h=0;
		int i;
		Op=iargs->Output[selOut];
		
		if (first==1)
		{
			initHFilter(iargs->srcH,iargs->dstH,hfilterpos,weightHfilter0,weightHfilter1);
			initWFilter(iargs->srcW,iargs->dstW,1,wfilterpos,widthfilter0,widthfilter1,weightWfilter0,weightWfilter1);
			crblock0=(iargs->srcW>>1)&~15; // rounded down
			crblock1=((iargs->srcW>>1) + 15)&~15; //rounded up
			crblockdst0=(iargs->dstW>>1)&~15; // rounded down
			crblockdst1=((iargs->dstW>>1) + 15)&~15; //rounded up
			if ((iargs->srcW%32) != 0)
			{
			// we must then compute a separate filter for every other u and v since they arent 128bit alligned.. asuming that it will allways be at least 16!!!
				initWcrFilter(iargs->srcW,iargs->dstW,1,crwfilterpos,crwidthfilter0,crwidthfilter1);	
				printf("Computing Crshufflefilter\n");
			
			}
			
			first=0;
		}

		// Luminance loop dont even think about touching this it works!
		//get source line 0 and 1 into 0 0+1 and 1 and 1+1
		YIp = iargs->Ystart[selIn];
		UIp = iargs->Ustart[selIn];
		VIp = iargs->Vstart[selIn];
		YOp = iargs->Output[selOut];
		UOp = iargs->Output[selOut] + iargs->dstW*iargs->dstH; //+ iargs->dstW; // fix this to something it should be
		VOp = UOp + iargs->dstW*iargs->dstH/4;//*2 + iargs->dstW*1.5;//fix this to something it should be

		int currentpos=0;
		int nextpos=1;
		
		dmaGetnWait(Input[0],YIp,iargs->srcW,tgi[0]);
		YIp=YIp+iargs->srcW;
		dmaGet(Input[1],YIp,iargs->srcW,tgi[1]);
		YIp=YIp+iargs->srcW;
		LineSelIn=1; // want to check the dma at beginning of the loop!
		LineSelOut=0;
		// unpacks a luminance line..
		unpacklines(Input[0],widthfilter0,widthfilter1,wfilterpos,fBuffer00,fBuffer01,iargs->dstW);
		//since we already have used input[0] lets get input[2] 
		dmaGet(Input[0],YIp,iargs->srcW,tgi[0]);
		YIp=YIp+iargs->srcW;

		int current=0;
		int lasthfilterpos=0;	

		for (h=0; h < iargs->dstH; h++) 
		{ 
			currentpos=hfilterpos[h];

			nextpos=hfilterpos[h+1];

			dmaWaitTag(tgi[LineSelIn]);// we check that the second line is awailable before we start working on it
			
			
			if (currentpos!=lasthfilterpos){ // no need to shuffle out a new line unless its actually a new line to shuffle
				scalenewline(Input[LineSelIn],widthfilter0,widthfilter1,wfilterpos,fBuffer00,fBuffer01,fBuffer10,fBuffer11,weightHfilter0[hfilterpos[h]],weightHfilter1[hfilterpos[h]],weightWfilter0,weightWfilter1,Output[LineSelOut],iargs->dstW);
			} else {
				scaleline(wfilterpos,fBuffer00,fBuffer01,fBuffer10,fBuffer11,weightHfilter0[hfilterpos[h]],weightHfilter1[hfilterpos[h]],weightWfilter0,weightWfilter1,Output[LineSelOut],iargs->dstW);
			}
	
			lasthfilterpos=currentpos;
			
			if (nextpos !=currentpos) //new line coming up so we move the old h+1 to h
			{
				vector float*temp1;
				temp1 = fBuffer00;
				fBuffer00 = fBuffer10;
				fBuffer10 = temp1;
				
				vector float* temp2;
				temp2 = fBuffer01;
				fBuffer01 = fBuffer11;
				fBuffer11 = temp2;
				//lets also get another line so that its ready for the loop after the nextone
				dmaGet( Input[LineSelIn], YIp,iargs->srcW, tgi[LineSelIn]);
				YIp = YIp + iargs->srcW;
				LineSelIn=LineSelIn ^ 1;			
			}
		

			dmaWaitTag(tgo[LineSelOut]); // wait in case the line isnt written yet.
			dmaPut(Output[LineSelOut], YOp, iargs->dstW, tgo[LineSelOut]);	
			YOp = YOp + iargs->dstW;//*4; // increment output pointer
			LineSelOut = LineSelOut ^ 1; // we flip since we want to use another buffer for the next line..
	
		}
		
		// Cr 
		dmaGetnWait(Input[0], UIp, crblock1, tgi[0]); //this is the first U line..
		UIp = UIp + crblock0; // if its not %16 we still have data in this block we need so we round down after even and up on odd!
		dmaGet(Input[1], UIp, crblock1, tgi[1]); //this is second line of U
		UIp = UIp + crblock1;
		LineSelIn=1; // want to check the dma at beginning of the loop! 
		LineSelOut=0;//lets start writing using dmaline0
		// unpacks a luminance line..
		unpacklines(Input[0],widthfilter0,widthfilter1,wfilterpos,fBuffer00,fBuffer01,((iargs->dstW>>1)+15)&~15); // this works for all even lines..
		//since we already have used input[0] lets get input[2] 
		dmaGet(Input[0],UIp,crblock1,tgi[0]);
		UIp=UIp+crblock0;

		current=0;
		lasthfilterpos=0;	
	
		for (h=0; h < iargs->dstH/2; h++) // we asume that the height of the image is a factor of 2 at least
		{ 
			vector unsigned char * tmpfilter0;
			vector unsigned char * tmpfilter1;
			int * tmpfilterpos;
			vector unsigned char * tmpOut;
			vector unsigned char shared;


			currentpos=hfilterpos[h];
			nextpos=hfilterpos[h+1];
			dmaWaitTag(tgi[LineSelIn]);// we check that the second line is awailable before we start working on it

			if ((iargs->srcW%32 != 0) && (currentpos&1 == 0)) { //current 0 position means we are unshuffeling 0+1 which is the odd..
			
				tmpfilter0=crwidthfilter0;
				tmpfilter1=crwidthfilter1;
				tmpfilterpos=crwfilterpos;
			} else {
				tmpfilter0=widthfilter0;
				tmpfilter1=widthfilter1;
				tmpfilterpos=wfilterpos;

			}
		
					
			if (currentpos!=lasthfilterpos){// no need to shuffle out a new line unless its actually a new line to shuffle
			
				scalenewline(Input[LineSelIn],tmpfilter0,tmpfilter1,tmpfilterpos,fBuffer00,fBuffer01,fBuffer10,fBuffer11,weightHfilter0[hfilterpos[h]],weightHfilter1[hfilterpos[h]],weightWfilter0,weightWfilter1,Output[LineSelOut],crblockdst1);
			} else {
				scaleline(wfilterpos,fBuffer00,fBuffer01,fBuffer10,fBuffer11,weightHfilter0[hfilterpos[h]],weightHfilter1[hfilterpos[h]],weightWfilter0,weightWfilter1,Output[LineSelOut],crblockdst1);
			}
	
				lasthfilterpos=currentpos;
			
			if (nextpos !=currentpos) //new line coming up so we move the old h+1 to h
			{
				vector float*temp1;
				temp1=fBuffer00;
				fBuffer00=fBuffer10;
				fBuffer10=temp1;
				
				vector float* temp2;
				temp2=fBuffer01;
				fBuffer01=fBuffer11;
				fBuffer11=temp2;
				//lets get another line so that its ready for the loop after the nextone
				if (LineSelIn == 0) {

						dmaGet(Input[LineSelIn], UIp, crblock1, tgi[LineSelIn]);
						UIp = UIp + crblock0; //round down if its not exactly /16
							
				} else { 

						dmaGet(Input[LineSelIn], UIp, crblock1, tgi[LineSelIn]);
						UIp = UIp + crblock1; // round up! if its exactly /16		
				}
				LineSelIn=LineSelIn ^ 1;			
			}
			
			dmaWaitTag(tgo[LineSelOut]); // wait in case the line isnt written out yet!
			if ((h&1)==0){
				if(iargs->dstW%32!=0)
				{
					shared=Output[LineSelOut][crblockdst1>>4];
				}
				dmaPut(Output[LineSelOut],UOp,crblockdst0,tgo[LineSelOut]);
				UOp=UOp+crblockdst0;
			}
			else
			{
						
				//insert the shuffle here if w isnt mod 32 BUSY					
				if(iargs->dstW%32!=0)
				{
					rightshiftnadd8(shared,Output[LineSelOut],crblockdst1);
				}
				dmaPut(Output[LineSelOut],UOp,crblockdst1,tgo[LineSelOut]);
				UOp=UOp+crblockdst1;
			}
			LineSelOut=LineSelOut ^ 1; // we flip since we want to use another buffer for the next line..
		}
		

		//Cb
		dmaGetnWait(Input[0], VIp, crblock1, tgi[0]); //this is the first U line..
		VIp = VIp + crblock0; // if its not %16 we still have data in this block we need. 
		dmaGet(Input[1], VIp, crblock1, tgi[1]); //this is second line of U
		VIp = VIp + crblock1;
		LineSelIn=1; // want to check the dma at beginning of the loop! 
		LineSelOut=0;

		unpacklines(Input[0],widthfilter0,widthfilter1,wfilterpos,fBuffer00,fBuffer01,crblock1); // this works for all even lines..
		//since we already have used input[0] lets get input[2] 
		dmaGet(Input[0],VIp,crblock1,tgi[0]);
		VIp=VIp+ crblock0;

		current=0;
		lasthfilterpos=0;	

		for (h=0; h < iargs->dstH/2; h++) // we asume that the height of the image is a factor of 2 at least
		{ 
			vector unsigned char * tmpfilter0;
			vector unsigned char * tmpfilter1;
			int * tmpfilterpos;
			vector unsigned char shared;


			currentpos=hfilterpos[h];
			nextpos=hfilterpos[h+1];

			dmaWaitTag(tgi[LineSelIn]);// we check that the second line is awailable before we start working on it

			if ((iargs->srcW%32 != 0) && (currentpos&1 == 0)) { //current 0 position means we are unshuffeling 0+1 which is the odd..
			
				tmpfilter0=crwidthfilter0;
				tmpfilter1=crwidthfilter1;
				tmpfilterpos=crwfilterpos;
			} else {
				tmpfilter0=widthfilter0;
				tmpfilter1=widthfilter1;
				tmpfilterpos=wfilterpos;

			}
		
					
			if (currentpos!=lasthfilterpos){// no need to shuffle out a new line unless its actually a new line to shuffle
			
				scalenewline(Input[LineSelIn],tmpfilter0,tmpfilter1,tmpfilterpos,fBuffer00,fBuffer01,fBuffer10,fBuffer11,weightHfilter0[hfilterpos[h]],weightHfilter1[hfilterpos[h]],weightWfilter0,weightWfilter1,Output[LineSelOut],crblockdst1);
			} else {
				scaleline(wfilterpos,fBuffer00,fBuffer01,fBuffer10,fBuffer11,weightHfilter0[hfilterpos[h]],weightHfilter1[hfilterpos[h]],weightWfilter0,weightWfilter1,Output[LineSelOut],crblockdst1);
			}
	
				lasthfilterpos=currentpos;
			
			if (nextpos !=currentpos) //new line coming up so we move the old h+1 to h
			{
				vector float*temp1;
				temp1=fBuffer00;
				fBuffer00=fBuffer10;
				fBuffer10=temp1;
				
				vector float* temp2;
				temp2=fBuffer01;
				fBuffer01=fBuffer11;
				fBuffer11=temp2;
				//lets get another line so that its ready for the loop after the nextone
				if (LineSelIn == 0) {

						dmaGet(Input[LineSelIn], VIp, crblock1, tgi[LineSelIn]);
						VIp = VIp + crblock0; //round down if its not exactly /16
							
				} else { 

						dmaGet(Input[LineSelIn], VIp, crblock1, tgi[LineSelIn]);
						VIp = VIp + crblock1; // round up! if its exactly /16		
				}
				LineSelIn=LineSelIn ^ 1;			
			}
			

			dmaWaitTag(tgo[LineSelOut]); // wait in case the line isnt written out yet!
			if ((h&1)==0){
				if(iargs->dstW%32!=0)
				{
					shared=Output[LineSelOut][crblockdst1>>4];
				}
				dmaPut(Output[LineSelOut],VOp,crblockdst0,tgo[LineSelOut]);
				VOp=VOp+crblockdst0;
			}
			else
			{
				
				//insert the shuffle here if w isnt mod 32 BUSY	
				if((iargs->dstW%32) != 0)
				{
					rightshiftnadd8(shared,Output[LineSelOut],crblockdst1);
				}
				dmaPut(Output[LineSelOut],VOp,crblockdst1,tgo[LineSelOut]);
				VOp=VOp+crblockdst1;
			}
		//	VOp = VOp + iargs->dstW*4; // increment output pointer .
			LineSelOut=LineSelOut ^ 1; // we flip since we want to use another buffer for the next line..
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
			printf("Stopping\n");
		}
		else if (msg == UPDATE)
		{
			dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process the new image	
			first=0; // update filters to reflect the new image!
		}
		selOut = selOut ^ 1; // flips the output buffers
		selIn = selIn ^ 1; // flips the input buffers
	}
	
	return 0;
}
