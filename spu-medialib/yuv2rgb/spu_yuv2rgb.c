
/* 
Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include <spu_mfcio.h>
#include <malloc.h>
#include <stdio.h>
#include "libspedma.h"
#include "spu_colorspace.h"
#include "yuv_datastructs.h"
#include "spu_control.h"

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgiA=1;
	int tgiB=2;
	int tgoA=3;
	int tgoB=4;
	
	

	int tag = 1,tag_mask=1<<tag;		
	int buffsize;
	struct img_args *iargs;	
	iargs =(struct img_args*)memalign(128,sizeof(*iargs));

	dmaGetnWait(iargs,(unsigned int)argp,(int)envp,tag); //getting neccesary data to process image

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




	unsigned long long Op;
	unsigned long long Yp;
	unsigned long long Up;
	unsigned long long Vp;
	unsigned long long tmp;
	unsigned int msg;
	
	while (spu_stat_in_mbox() == 0);
	msg=spu_read_in_mbox();
	if (msg==RUN){	
		printf("RUN\n");
	}
	
	while (msg!=STOP) 
	{
		Op=iargs->Ostart;
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
			
			Op=Op+iargs->maxwidth*4*2;
			IyAp=IybufferA+iargs->width*2/16;
			IuAp=IubufferA+iargs->width/32;
			IvAp=IvbufferA+iargs->width/32;
			
			dmaWaitTag(tgoB);
			yuv420toARGB(IyAp,IuAp,IvAp,ObufferB,iargs->width,iargs->maxwidth);
			dmaPut(ObufferB,Op,iargs->maxwidth*4*2,tgoB);

			Yp=Yp+iargs->width*4;
			Up=Up+iargs->width;
			Vp=Vp+iargs->width;
			Op=Op+iargs->maxwidth*4*2;

			if (iargs->height/8-1 != i){ // do not get on the last loop ..
		
				dmaGet(IybufferA,Yp,iargs->width*4,tgiA);
				dmaGet(IubufferA,Up,iargs->width,tgiA);
				dmaGet(IvbufferA,Vp,iargs->width,tgiA);
			}
		
			dmaWaitTag(tgiB);
			
			dmaWaitTag(tgoA);
			yuv420toARGB(IybufferB,IubufferB,IvbufferB,ObufferA,iargs->width,iargs->maxwidth);
			dmaPut(ObufferA,Op,iargs->maxwidth*4*2,tgoA);
			
			Op=Op+iargs->maxwidth*4*2;	
			IyBp=IybufferB+iargs->width*2/16;
			IuBp=IubufferB+iargs->width/32;
			IvBp=IvbufferB+iargs->width/32;		

			dmaWaitTag(tgoB);
			yuv420toARGB(IyBp,IuBp,IvBp,ObufferB,iargs->width,iargs->maxwidth);
			dmaPut(ObufferB,Op,iargs->maxwidth*4*2,tgoB);

			Yp=Yp+iargs->width*4;
			Up=Up+iargs->width;
			Vp=Vp+iargs->width;
			Op=Op+iargs->maxwidth*4*2;
			
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
			tmp=iargs->Ostart;
			iargs->Ostart=iargs->Ostartb;
			iargs->Ostartb=tmp;
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





