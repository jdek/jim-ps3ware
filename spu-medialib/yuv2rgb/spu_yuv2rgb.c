/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * spu_yuv2rgb.cpp - SPU YUV to RGB conversion kernel
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * Copyright (c) 2007, John Kelley <ps2dev@kelley.ca>
 *
 * $Id$
 */

#include <spu_mfcio.h>
#include <malloc.h>
#include <stdio.h>
#include "libspedma.h"
#include "spu_colorspace.h"
#include "yuv_datastructs.h"
#include "spu_control.h"

#define PPU_ADDR_MASK	0xFF00000000000000ULL

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgiA=1, tgiB=2, tgoA=3, tgoB=4;
	int selOut = 0;
	int tag = 1;
	struct img_args *iargs;
	
	iargs =(struct img_args*)memalign(128,sizeof(*iargs));
	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process image
	
	//32-bit ppu program fix
	if ((iargs->Ystart & PPU_ADDR_MASK) == PPU_ADDR_MASK) {
		printf("Detected 32-bit PPU program, fixing pointers for sign extension\n");
		iargs->Ystart    &= 0xFFFFFFFF;
		iargs->Ustart    &= 0xFFFFFFFF;
		iargs->Vstart    &= 0xFFFFFFFF;
		iargs->Output[0] &= 0xFFFFFFFF;
		iargs->Output[1] &= 0xFFFFFFFF;
	}

	vector unsigned char* IybufferA=(vector unsigned char*)memalign(128,iargs->width*4);	// 4 lines of Y 
	vector unsigned char* IubufferA=(vector unsigned char*)memalign(128,iargs->width);	// 1 line of U
	vector unsigned char* IvbufferA=(vector unsigned char*)memalign(128,iargs->width);	// 1 line of V
	vector unsigned char* IybufferB=(vector unsigned char*)memalign(128,iargs->width*4);	// 4 lines of Y
	vector unsigned char* IubufferB=(vector unsigned char*)memalign(128,iargs->width);	// 1 line of U
	vector unsigned char* IvbufferB=(vector unsigned char*)memalign(128,iargs->width);	// 1 line of V

	vector unsigned char* ObufferA=(vector unsigned char*)memalign(128,iargs->maxwidth*4*2); //2 lines of ARGB
	vector unsigned char* ObufferB=(vector unsigned char*)memalign(128,iargs->maxwidth*4*2); //2 lines of ARGB

	vector unsigned char* IyAp;
	vector unsigned char* IyBp;
	vector unsigned char* IuAp;
	vector unsigned char* IuBp;
	vector unsigned char* IvAp;
	vector unsigned char* IvBp;

	unsigned long long Op, Yp, Up, Vp;
	unsigned int msg;
	
	while (spu_stat_in_mbox() == 0);
	msg=spu_read_in_mbox();
	if (msg==RUN){	
		printf("RUN\n");
	}
	
	while (msg!=STOP) 
	{
		Op=iargs->Output[selOut];
		Yp=iargs->Ystart;
		Up=iargs->Ustart;
		Vp=iargs->Vstart;
		
		dmaGet(IybufferA,Yp,iargs->width*4,tgiA);
		dmaGet(IubufferA,Up,iargs->width,tgiA);
		dmaGet(IvbufferA,Vp,iargs->width,tgiA);

		
		Yp=Yp+iargs->width*4;
		Up=Up+iargs->width;
		Vp=Vp+iargs->width;
		
		dmaGet(IybufferB,Yp,iargs->width*4,tgiB);

		dmaGet(IubufferB,Up,iargs->width,tgiB);

		dmaGet(IvbufferB,Vp,iargs->width,tgiB);

		
		for (int i=0;i < iargs->height/8;i++) { // loop asumes height is a factor of eight
	
			dmaWaitTag(tgiA);
			
			dmaWaitTag(tgoA);
			yuv420toARGB(IybufferA,IubufferA,IvbufferA,ObufferA,iargs->width,iargs->maxwidth);
			dmaPut(ObufferA,Op,iargs->maxwidth*4*2,tgoA);
			
			Op += iargs->maxwidth*4*2;
			IyAp=IybufferA+iargs->width*2/16;
			IuAp=IubufferA+iargs->width/32;
			IvAp=IvbufferA+iargs->width/32;
			
			dmaWaitTag(tgoB);
			yuv420toARGB(IyAp,IuAp,IvAp,ObufferB,iargs->width,iargs->maxwidth);
			dmaPut(ObufferB,Op,iargs->maxwidth*4*2,tgoB);

			Yp=Yp+iargs->width*4;
			Up=Up+iargs->width;
			Vp=Vp+iargs->width;
			Op += iargs->maxwidth*4*2;

			if (iargs->height/8-1 != i){ // do not get on the last loop ..
		
				dmaGet(IybufferA,Yp,iargs->width*4,tgiA);
				dmaGet(IubufferA,Up,iargs->width,tgiA);
				dmaGet(IvbufferA,Vp,iargs->width,tgiA);
			}
		
			dmaWaitTag(tgiB);
			
			dmaWaitTag(tgoA);
			yuv420toARGB(IybufferB,IubufferB,IvbufferB,ObufferA,iargs->width,iargs->maxwidth);
			dmaPut(ObufferA,Op,iargs->maxwidth*4*2,tgoA);
			
			Op += iargs->maxwidth*4*2;	
			IyBp=IybufferB+iargs->width*2/16;
			IuBp=IubufferB+iargs->width/32;
			IvBp=IvbufferB+iargs->width/32;		

			dmaWaitTag(tgoB);
			yuv420toARGB(IyBp,IuBp,IvBp,ObufferB,iargs->width,iargs->maxwidth);
			dmaPut(ObufferB,Op,iargs->maxwidth*4*2,tgoB);

			Yp=Yp+iargs->width*4;
			Up=Up+iargs->width;
			Vp=Vp+iargs->width;
			Op += iargs->maxwidth*4*2;
			
			if (iargs->height/8-1 != i) { // do not get on the last loop ..

				dmaGet(IybufferB,Yp,iargs->width*4,tgiB);
				dmaGet(IubufferB,Up,iargs->width,tgiB);
				dmaGet(IvbufferB,Vp,iargs->width,tgiB);
			}
		}
		dmaWaitTag(tgoA);
		dmaWaitTag(tgoB);
		
		while (spu_stat_out_mbox() == 0);
		msg=RDY;
		// MANUALLY FLIPPING THE BUFFERS
		selOut = selOut ^ 1;
		spu_write_out_mbox(msg);	
		
		while (spu_stat_in_mbox() == 0);
		msg=spu_read_in_mbox();
		
		if (msg == RUN){

		}
		else if (msg == STOP)
		{
			printf("Stopping\n");
		}
	}
	
	return 0;
}

