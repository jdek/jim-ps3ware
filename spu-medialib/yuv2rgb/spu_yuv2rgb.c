/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2
 *
 * spu_yuv2rgb.c- SPU YUV to RGB conversion kernel
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * Copyright (c) 2007, John Kelley <ps2dev@kelley.ca>
 *
 * $Id$
 */
// Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
// Copyright (c) 2007, John Kelley <ps2dev@kelley.ca>
//
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
#include <spu_dmalib.h>
#include "spu_colorspace.h"
#include <yuv_datastructs.h>
#include <spu_control.h>

#define PPU_ADDR_MASK	0xFF00000000000000ULL
#define MAXWIDTH 1920
#define MAXHEIGHT 1080

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgi[2];
	int tgo[2];
	tgi[0]=0;
	tgi[1]=1;
	tgo[0]=2;
	tgo[1]=3;
	int selOut = 0;
	int selIn = 0;
	int LineSelIn=0;
	int LineSelOut=0;
	int tag = 4;

	struct img_args *iargs;
	iargs =(struct img_args*)memalign(128,sizeof(struct img_args));
	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process image
	printf("SRC width %d,SRC Height %d\n",iargs->srcW,iargs->srcH);
	
	//32-bit ppu program fix
/*	if (((uint64_t)(iargs->Ystart) &PPU_ADDR_MASK) == PPU_ADDR_MASK) {
		printf("Detected 32-bit PPU program, fixing pointers for sign extension\n");
		iargs->Ystart[0] &= 0xFFFFFFFF;
		iargs->Ystart[1] &= 0xFFFFFFFF;
		iargs->Ustart[0] &= 0xFFFFFFFF;
		iargs->Ustart[1] &= 0xFFFFFFFF;
		iargs->Vstart[0] &= 0xFFFFFFFF;
		iargs->Vstart[1] &= 0xFFFFFFFF;
		iargs->Output[0] &= 0xFFFFFFFF;
		iargs->Output[1] &= 0xFFFFFFFF;
	}
*/

	vector unsigned char *Iybuffer[2];
	Iybuffer[0]=(vector unsigned char*)memalign(128,MAXWIDTH*2);	// 2 lines of Y 
	Iybuffer[1]=(vector unsigned char*)memalign(128,MAXWIDTH*2);	// 2 lines of Y 

	vector unsigned char *Iubuffer[2];
	Iubuffer[0]=(vector unsigned char*)memalign(128,((MAXWIDTH>>1)+15)&~15);	// 1/2 lines of U 
	Iubuffer[1]=(vector unsigned char*)memalign(128,((MAXWIDTH>>1)+15)&~15);	// 1/2 lines of U 

	vector unsigned char *Ivbuffer[2];
	Ivbuffer[0]=(vector unsigned char*)memalign(128,((MAXWIDTH>>1)+15)&~15);	// 1/2 lines of V 
	Ivbuffer[1]=(vector unsigned char*)memalign(128,((MAXWIDTH>>1)+15)&~15);	// 1/2 lines of V 

	vector unsigned char* Obuffer[2];
	Obuffer[0]=(vector unsigned char*)memalign(128,MAXWIDTH*4*2); //2 lines of ARGB
	Obuffer[1]=(vector unsigned char*)memalign(128,MAXWIDTH*4*2); //2 lines of ARGB


	unsigned long long Op, Yp, Up, Vp;
	unsigned int msg;
	int LSB=0;
	while (spu_stat_in_mbox() == 0);
	msg=spu_read_in_mbox();
	if (msg==RUN){	
		printf("RUN\n");
	}
	
	while (msg!=STOP) 
	{
		
		Op=iargs->Output[selOut];
		Op=Op+iargs->offset;
		Yp=iargs->Ystart[selIn];
		Up=iargs->Ustart[selIn];
		Vp=iargs->Vstart[selIn];
		
		dmaGet(Iybuffer[0],Yp,iargs->srcW*2,tgi[0]);
		dmaGet(Iubuffer[0],Up,((iargs->srcW>>1)+15)&~15,tgi[0]);
		dmaGet(Ivbuffer[0],Vp,((iargs->srcW>>1)+15)&~15,tgi[0]);

		
		Yp=Yp+iargs->srcW*2;
		Up=Up+((iargs->srcW>>1)&~15);
		Vp=Vp+((iargs->srcW>>1)&~15);
		
		dmaGet(Iybuffer[1],Yp,iargs->srcW*2,tgi[1]);

		dmaGet(Iubuffer[1],Up,((iargs->srcW>>1)+15)&~15,tgi[1]);

		dmaGet(Ivbuffer[1],Vp,((iargs->srcW>>1)+15)&~15,tgi[1]);
		LineSelIn=0;
		LineSelOut=0;
		int i=0;
		for (i=0;i < iargs->srcH>>1;i++) { // loop asumes srcH is at least a factor of 2 lines!
			dmaWaitTag(tgi[LineSelIn]);
			
			dmaWaitTag(tgo[LineSelOut]);

			if ((iargs->dstW%32 != 0 )&&(LineSelIn==1)) { LSB=1;} else {LSB=0;}

			yuv420toARGB(Iybuffer[LineSelIn],Iubuffer[LineSelIn],Ivbuffer[LineSelIn],Obuffer[LineSelOut],iargs->srcW,iargs->maxwidth,LSB);
			dmaPut(Obuffer[LineSelOut],Op,iargs->maxwidth*4*2,tgo[LineSelOut]);
			

			int cromblock=(((iargs->srcW>>1))&~15);
			
			if ((iargs->dstW%32 != 0 )&&(LineSelIn==1)) { cromblock=(((iargs->srcW>>1)+15)&~15);}
			Op += iargs->maxwidth*4*2;
			Yp=Yp+iargs->srcW*2;
			Up=Up+cromblock;
			Vp=Vp+cromblock;

			dmaGet(Iybuffer[LineSelIn],Yp,iargs->srcW*2,tgi[LineSelIn]);
			dmaGet(Iubuffer[LineSelIn],Up,((iargs->srcW>>1)+15)&~15,tgi[LineSelIn]);
			dmaGet(Ivbuffer[LineSelIn],Vp,((iargs->srcW>>1)+15)&~15,tgi[LineSelIn]);
			LineSelIn=LineSelIn^1;
			LineSelOut=LineSelOut^1;
		
		}
		dmaWaitTag(tgo[LineSelOut^1]);
		dmaWaitTag(tgo[LineSelOut^1]);
		
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
			dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process the new image parameters	
			//first=0; // update filters to reflect the new image!
		}
		selOut = selOut ^ 1; // flips the output buffers
		selIn = selIn ^ 1; // flips the input buffers
	}
	
	return 0;
}

