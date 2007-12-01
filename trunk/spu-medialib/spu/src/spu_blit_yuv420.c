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
#include <spu-medialib/yuv_datastructs.h>
#include <spu-medialib/spu_control.h>
#include <spu_yuv_conv.h>

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgiy0[2];
 	int tgiy1[2];
//  	int tgiy2[2];
//  	int tgiy3[2];
	int tgiu0[2];
// 	int tgiu1[2];
	int tgiv0[2];
// 	int tgiv1[2];
	int tgo0[2];
 	int tgo1[2];
// 
	tgiy0[0]=1;
	tgiy0[1]=2;
 	tgiy1[0]=3;
 	tgiy1[1]=4;
	tgiu0[0]=5;
	tgiu0[1]=6;
// 	tgiu1[0]=7;
// 	tgiu1[1]=8;
	tgiv0[0]=9;
	tgiv0[1]=10;
// 	tgiv1[0]=11;
// 	tgiv1[1]=12;

	tgo0[0]=13;
	tgo0[1]=14;
	tgo1[0]=15;
	tgo1[1]=16;

	
	int selOut = 0;
	int selIn = 0;
	int msg=RUN;
	int waiting=0;
	int first=1;
	int tag = 31;
	struct img_args *iargs;
	iargs =(struct img_args*)memalign(128,sizeof(*iargs));
	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag);

	while (spu_stat_in_mbox() == 0);
		msg=spu_read_in_mbox();
 	if (msg==RUN){	
 		printf("yuv420 blitter: Starting Up\n");
 	}

	int LuminanceDmaStride;
	int CrominanceDmaStride;

	int OutputDmaStride;
	int bppO;
	int bppI;
	if ((iargs->TargetFormat == YUY2) || (iargs->TargetFormat == YUYV422))
		bppO=16;

	if ((iargs->TargetFormat == ARGB))
		bppO=32;

	LuminanceDmaStride=iargs->Istride[0];
	CrominanceDmaStride=iargs->Istride[1];
		
	OutputDmaStride=iargs->Ostride[0]*bppO/8;

	vector unsigned char *Yinbuffer[2];
 	Yinbuffer[0]=(vector unsigned char*)memalign(128,LuminanceDmaStride);
 	Yinbuffer[1]=(vector unsigned char*)memalign(128,LuminanceDmaStride);

	vector unsigned char *Y1inbuffer[2];
 	Y1inbuffer[0]=(vector unsigned char*)memalign(128,LuminanceDmaStride);
 	Y1inbuffer[1]=(vector unsigned char*)memalign(128,LuminanceDmaStride);

	vector unsigned char *Uinbuffer[2];
 	Uinbuffer[0]=(vector unsigned char*)memalign(128,CrominanceDmaStride);
 	Uinbuffer[1]=(vector unsigned char*)memalign(128,CrominanceDmaStride);

	vector unsigned char *Vinbuffer[2];
 	Vinbuffer[0]=(vector unsigned char*)memalign(128,CrominanceDmaStride);
 	Vinbuffer[1]=(vector unsigned char*)memalign(128,CrominanceDmaStride);

	vector unsigned char *Outbuffer[2];
 	Outbuffer[0]=(vector unsigned char*)memalign(128,OutputDmaStride+16*4);
 	Outbuffer[1]=(vector unsigned char*)memalign(128,OutputDmaStride+16*4);

	vector unsigned char *Outbuffer1[2];
 	Outbuffer1[0]=(vector unsigned char*)memalign(128,OutputDmaStride+16*4);
 	Outbuffer1[1]=(vector unsigned char*)memalign(128,OutputDmaStride+16*4);

	unsigned long long Yip,Uip,Vip,Op;
	
	int i=0;
	int update=0;
//	msg=RUN;



	while (msg!=STOP)
	{
		selOut = 0;
		selIn = 0;
// 		if (update)
// 		{
// 			update=0;
// 			dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag);
// 		//	printf("iargs->lines=%d\n" ,iargs->lines);
// 			free(Yinbuffer[0]);
// 			free(Uinbuffer[0]);
// 			free(Vinbuffer[0]);
// 			free(Yinbuffer[1]);
// 			free(Uinbuffer[1]);
// 			free(Vinbuffer[1]);
// 			
// 			free(Outbuffer[0]);
// 			free(Outbuffer[1]);
// 			
// 			Yinbuffer[0]=(vector unsigned char*)memalign(128,iargs->Istride[0]*2);
// 			Yinbuffer[1]=(vector unsigned char*)memalign(128,iargs->Istride[0]*2);
// 		
// 	
// 			Uinbuffer[0]=(vector unsigned char*)memalign(128,iargs->Istride[0]*2);
// 			Uinbuffer[1]=(vector unsigned char*)memalign(128,iargs->Istride[0]*2);
// 		
// 	
// 			Vinbuffer[0]=(vector unsigned char*)memalign(128,iargs->Istride[0]*2);
// 			Vinbuffer[1]=(vector unsigned char*)memalign(128,iargs->Istride[0]*2);
// 		
// 
// 			Outbuffer[0]=(vector unsigned char*)memalign(128,iargs->Ostride[0]*4);
// 			Outbuffer[1]=(vector unsigned char*)memalign(128,iargs->Ostride[0]*4);
// 		
// 			LuminanceDmaStride=iargs->Istride[0]*2;
// 			CrominanceDmaStride=iargs->Istride[1];
// 		
// 			OutputDmaStride=iargs->Ostride[0]*4;
// 		}
	
		Yip=iargs->Inp0[0];
		Uip=iargs->Inp1[0];
		Vip=iargs->Inp2[0];
		Op=iargs->Outp0[0];
		
		dmaGet(Yinbuffer[0],Yip,LuminanceDmaStride,tgiy0[0]);
		Yip=Yip+LuminanceDmaStride;
		dmaGet(Y1inbuffer[0],Yip,LuminanceDmaStride,tgiy1[0]);
		Yip=Yip+LuminanceDmaStride;
		
		dmaGet(Uinbuffer[0],Uip,CrominanceDmaStride,tgiu0[0]);
		dmaGet(Vinbuffer[0],Vip,CrominanceDmaStride,tgiv0[0]);
		
		Uip=Uip+CrominanceDmaStride;
		Vip=Vip+CrominanceDmaStride;
		
		dmaGet(Yinbuffer[1],Yip,LuminanceDmaStride,tgiy0[1]);
		Yip=Yip+LuminanceDmaStride;
		dmaGet(Y1inbuffer[1],Yip,LuminanceDmaStride,tgiy1[1]);
		Yip=Yip+LuminanceDmaStride;

		dmaGet(Yinbuffer[1],Yip,LuminanceDmaStride,tgiy0[1]);
		dmaGet(Uinbuffer[1],Uip,CrominanceDmaStride,tgiu0[1]);
		dmaGet(Vinbuffer[1],Vip,CrominanceDmaStride,tgiv0[1]);
	
		Uip=Uip+CrominanceDmaStride;
		Vip=Vip+CrominanceDmaStride;
		selIn=0;
		selOut=0;
		//printf("loop starting\n");
		for (i=0;i < (iargs->drw_h/2) ;i++) {
			dmaWaitTag(tgiy0[selIn]);
			dmaWaitTag(tgiu0[selIn]); //checking the Y dma may suffice but is risky..
			dmaWaitTag(tgiv0[selIn]);
			dmaWaitTag(tgo0[selOut]);

			if ((iargs->TargetFormat==YUY2)||(iargs->TargetFormat==YUYV422)) 
			{
				yuv420_to_yuyv422(Yinbuffer[selIn],Y1inbuffer[selIn],Uinbuffer[selIn],Vinbuffer[selIn],Outbuffer[selOut],Outbuffer1[selOut],iargs->drw_w,0);
			//	printf("yuy420 to yuv422\n");
			}

			if ((iargs->TargetFormat==ARGB))
			{
				yuv420_to_argb(Yinbuffer[selIn],Y1inbuffer[selIn],Uinbuffer[selIn],Vinbuffer[selIn],Outbuffer[selOut],Outbuffer1[selOut],iargs->drw_w,0);		
			}
			dmaPut(Outbuffer[selOut],Op,iargs->drw_w*bppO/8,tgo0[selOut]);
			Op=Op+OutputDmaStride;
			dmaPut(Outbuffer1[selOut],Op,iargs->drw_w*bppO/8,tgo1[selOut]);
			Op=Op+OutputDmaStride;
		//	printf("loop %d\n",i);
 			dmaGet(Yinbuffer[selIn],Yip,LuminanceDmaStride,tgiy0[selIn]);
			Yip=Yip+LuminanceDmaStride;
			dmaGet(Y1inbuffer[selIn],Yip,LuminanceDmaStride,tgiy1[selIn]);
			Yip=Yip+LuminanceDmaStride;

 			dmaGet(Uinbuffer[selIn],Uip,CrominanceDmaStride,tgiu0[selIn]);
 			dmaGet(Vinbuffer[selIn],Vip,CrominanceDmaStride,tgiv0[selIn]);

			
			Uip=Uip+CrominanceDmaStride;
			Vip=Vip+CrominanceDmaStride;
			
			selIn=selIn^1;
			selOut=selOut^1;
		}
		//printf("loop finished \n");

		while (spu_stat_out_intr_mbox() == 0);
		msg=RDY;
		spu_writech(SPU_WrOutIntrMbox, msg);
		waiting=1;
		
		while (waiting){
			
			while (spu_stat_in_mbox() == 0);
			msg=spu_read_in_mbox();
			
			if (msg == RUN){
// 				selOut = selOut ^ 1; // flips the output buffer pointers
// 				selIn = selIn ^ 1; // flips the input buffer pointers	
				waiting=0;
			}
			else if (msg == STOP)
			{
			//	printf("blitter: Stopping\n");
				waiting=0;
			}
			else if (msg == UPDATE)
			{
				tag=28;
				dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process the new image	
 			//	update=1; // update filters to reflect the new image!
// 				selOut=0;
// 				selIn=0;
			}
		}
	}
		 

	return 0;
}
