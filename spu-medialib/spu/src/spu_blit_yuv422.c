/**
 * SPU YUV420(YV12) to yuv2(yuyv) conversion kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_blit_yuv420_yuv2.c - Main loop for the yuv420 to yuv2 blitter
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

#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <spu_dmalib.h>
#include <yuv_datastructs.h>
#include <spu_control.h>
#include <spu_yuv_conv.h>

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgi0[2];

	int tgo0[2];

	int tgio0[2];

	tgi0[0]=1;
	tgi0[1]=2;

	
	tgio0[0]=11;
	tgio0[1]=12;

	tgo0[0]=13;
	tgo0[1]=14;
/*	tgo1[0]=15;
	tgo1[1]=16;*/	

	
	int selOut = 0;
	int selIn = 0;
	int msg=RUN;
	int waiting=0;
	int tag = 31;
	struct img_args *iargs;
	iargs =(struct img_args*)memalign(128,sizeof(*iargs));
	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag);
	
	printf("spu_blit_yuv422_to_argb: SRC width %d,DST width %d\n",iargs->src_w,iargs->drw_w);
	printf("spu_blit_yuv422_to_argb: SRC height %d,DST height %d\n",iargs->src_h,iargs->drw_h);
	
	while (spu_stat_in_mbox() == 0);
		msg=spu_read_in_mbox();
//	first=0;

	vector unsigned char *InOutbuffer[2];

	vector unsigned char *Inbuffer[2];

	vector unsigned char *Outbuffer[2];

	int Outwidth=(4*iargs->drw_w+3)&~3;
	int Inwidth=(2*iargs->src_w+7)&~7;

	Inbuffer[0]=(vector unsigned char*)memalign(128,Inwidth);
	Inbuffer[1]=(vector unsigned char*)memalign(128,Inwidth);

	if (iargs->BLEND)
	{
		InOutbuffer[0]=(vector unsigned char*)memalign(128,Outwidth);
		InOutbuffer[1]=(vector unsigned char*)memalign(128,Outwidth);
	}

	Outbuffer[0]=(vector unsigned char*)memalign(128,Outwidth);
	Outbuffer[1]=(vector unsigned char*)memalign(128,Outwidth);

	unsigned long long Inp,Outp,InOutp;
	
	int i=0;
//	int update=1;


	while (msg!=STOP)
	{
		selOut = 0;
		selIn = 0;

		Inp=iargs->Inp0[0];
		InOutp=iargs->Outp0[0];
		Outp=iargs->Outp0[0];

		dmaGet(Inbuffer[0],Inp,Inwidth,tgi0[0]);
		Inp=Inp+iargs->Istride[0]*2;

		dmaGet(Inbuffer[1],Inp,Inwidth,tgi0[1]);
		Inp=Inp+iargs->Istride[0]*2;

// 		if (iargs->BLEND)
// 		{
// 			dmaGet(InOutbuffer[0],InOutp,Outwidth,tgio0[0]);
// 			InOutp=InOutp+iargs->Ostride[0]*4;
// 			dmaGet(InOutbuffer[1],InOutp,Outwidth,tgio0[1]);
// 			InOutp=InOutp+iargs->Ostride[0]*4;
// 		}



		selIn=0;
		selOut=0;

		for (i=0;i < iargs->drw_h ;i++) {
			dmaWaitTag(tgi0[selIn]);
			
// 			if (iargs->BLEND)
// 				dmaWaitTag(tgio0[selIn]); 
			dmaWaitTag(tgo0[selOut]);
			if (iargs->SourceFormat==YUY2||iargs->SourceFormat==YUYV422)
			{
				yuv422_to_argb(Inbuffer[selIn],Outbuffer[selOut],iargs->drw_w);
			//	printf("spe_blitter: YUV422->ARGB\n");
			}
			//yuv420_to_yuv2(Yinbuffer[selIn],Uinbuffer[selIn],Vinbuffer[selIn],Outbuffer[selOut],iargs->Istride[0]);
			
		//	if (iargs->BLEND)
			//	blend(InOutbuffer[selIn],OutBuffer[selOut],iargs->ALPHA,iargs->SourceFormat);
			
			dmaPut(Outbuffer[selOut],Outp,Outwidth,tgo0[selOut]);
		
// 			if (iargs->BLEND){
// 				dmaGet(InOutbuffer[selIn],InOutp,Outwidth,tgio0[selIn]);
// 				InOutp=InOutp+iargs->Ostride[0];
// 					
// 			}

			dmaGet(Inbuffer[selIn],Inp,Inwidth,tgi0[selIn]);
			
			Inp=Inp+iargs->Istride[0]*2;
			Outp=Outp+iargs->Ostride[0]*4;
			selIn=selIn^1;
			selOut=selOut^1;
		}
	

		while (spu_stat_out_intr_mbox() == 0);
		msg=RDY;
		spu_writech(SPU_WrOutIntrMbox, msg);
		waiting=1;
		
		while (waiting){
			
			while (spu_stat_in_mbox() == 0);
			msg=spu_read_in_mbox();
			
			if (msg == RUN){
				waiting=0;
			}
			else if (msg == STOP)
			{
				waiting=0;
			}
			else if (msg == UPDATE)
			{
				tag=30;
 				dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process the new image	
//  			//	update=1; // update filters to reflect the new image!
// 				Outwidth=(iargs->drw_w+3)&~3;
// 				Inwidth=(iargs->src_w+7)&~7;
// 				free(Inbuffer[0]);
// 				free(Inbuffer[1]);
// 	
// 				free(Outbuffer[0]);
// 				free(Outbuffer[1]);
// 				
// 				Inbuffer[0]=(vector unsigned char*)memalign(128,Inwidth);
// 				Inbuffer[1]=(vector unsigned char*)memalign(128,Inwidth);
// 			
// 				if (iargs->BLEND)
// 				{
// 					free(InOutbuffer[0]);
// 					free(InOutbuffer[1]);	
// 					InOutbuffer[0]=(vector unsigned char*)memalign(128,Outwidth);
// 					InOutbuffer[1]=(vector unsigned char*)memalign(128,Outwidth);
// 				}
// 	
// 				Outbuffer[0]=(vector unsigned char*)memalign(128,Outwidth);
// 				Outbuffer[1]=(vector unsigned char*)memalign(128,Outwidth);
			}
		}
	}
		 

	return 0;
}
