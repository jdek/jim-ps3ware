/**
 * SPU YUV scaler kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * example.cpp - ppu example implementation 
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


#include <libspe2.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <spu_control.h>
#include "yuvscaler.h"

using namespace std;

double mysecond() { 
	struct timeval tp; 
	int i; 
	i = gettimeofday(&tp,NULL); 
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 ); 
	}


int main (int nArg, char* cArg[]) {
	int srcW=352;
	int srcH=288;
	int dstW=1920;
	int dstH=1080;
	int curBuf = 0;
	int ftot = 1000; //< Number of frames to run through for FPS accuracy
	int fcount = 1;
	char *filename="default", *inBuf[2];
	char *RAMBufferA[2], *Ypointer, *Upointer, *Vpointer;
	unsigned int msg;

	ifstream Source;
	ofstream Destination;

        int counter=0;
        double time_elapsed=0.0;
        double start=mysecond();
        double stop;
        double old_time_elapsed=-11.0;
	int frame=0;
	//Parse commandline arguments
	if (nArg > 0) filename=cArg[1];
	if (nArg > 1) srcW=(int)atoi(cArg[2]);
	if (nArg > 2) srcH=(int)atoi(cArg[3]);
	if (nArg > 3) frame=(int)atoi(cArg[4]);

	//Open input file and read 
	Source.open(filename,ios::binary);
	Destination.open("dest.yuv,ios::binary");
	inBuf[0]=(char*)memalign(128,srcW*srcH+((srcW*srcH)/2));
	inBuf[1]=(char*)memalign(128,srcW*srcH+((srcW*srcH)/2));
	Source.seekg((srcW*srcH+srcW*srcH/2)*frame);

	Source.read(inBuf[0],(srcW*srcH+(srcW*srcH)/2));
	Source.read(inBuf[1],(srcW*srcH+(srcW*srcH)/2));

	Ypointer=inBuf[curBuf];
	Upointer=Ypointer+srcW*srcH;
	Vpointer=Upointer+srcW*srcH/4;

	RAMBufferA[0]=(char*)memalign(128,dstW*dstH+((dstW*dstH)/2));
	RAMBufferA[1]=(char*)memalign(128,dstW*dstH+((dstW*dstH)/2));	

	
	yuvscaler_t *yuvs = sws_init_yuvscaler(srcW, srcH, dstW, dstH, (ea_t)inBuf[0], (ea_t)inBuf[1], (ea_t)RAMBufferA[0], (ea_t)RAMBufferA[1]);

	printf("yuvscaler_initialised\n");

	spe_context_ptr_t ctx=sws_getCTX(yuvs); //not used !

	sws_send_message(yuvs,RDY);

	while (msg != STOP)
	{
		counter++;
		
		msg=sws_receive_message(yuvs);	

		if (fcount == ftot)  {
			sws_send_message(yuvs,STOP);
			msg=STOP;
			stop=mysecond();
			printf("clock %f\n", stop);
			time_elapsed=(double)stop-start;
			printf("Time of execution :%f\n",time_elapsed);
			printf("Number of frames displayed %d\n",fcount);
			printf("width : %d, height : %d , FPS : %f\n",srcW,srcH,fcount/time_elapsed);
		}
		if (msg ==RDY ) {

			if (time_elapsed > (old_time_elapsed + 10))
			{
				printf("Frames per second %f:\n",counter/time_elapsed);
				counter=0;
				old_time_elapsed=time_elapsed;
			}

			fcount++;
	
			sws_send_message(yuvs,RDY);
		}
	}
	Destination.write(RAMBufferA[0],dstW*dstH + ((dstW*dstH)/2)); // writes your new scaled YUV to a file!
	Destination.close();
	Source.close();
	
	sws_yuvscaler_destroy(yuvs);

	return(0) ;
}

