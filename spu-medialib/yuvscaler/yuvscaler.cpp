/**
 * SPU YUV scaler kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * yuvscaler.cpp - spu_example implementation 
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
#include "spu_control.h"
#include "yuv_datastructs.h"
//#include "fb_lib.h"

using namespace std;

double mysecond() { 
	struct timeval tp; 
	int i; 
	i = gettimeofday(&tp,NULL); 
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 ); 
	}

struct thread_args {
	struct spe_context * ctx;
	char * image;
	void * argp;
	void * envp;
};


void * spe_thread(void * arg) 
{
	
   	unsigned int          runflags    = 0;
   	unsigned int          entry       = SPE_DEFAULT_ENTRY;
	struct thread_args * arg_ptr;
	
	arg_ptr=(struct thread_args *) arg;
	printf("SPE file%s \n",arg_ptr->image);	
	printf("test");	
   	spe_program_handle_t * program;

	program = spe_image_open("spu_yuvscaler");
	
  	printf("image opened\n"); 
   	if (spe_program_load(arg_ptr->ctx, program) < 0) 
	{
		perror("program load");
		pthread_exit(NULL);
	}
	printf("spe loaded \n");

	spe_context_run(arg_ptr->ctx, &entry, runflags,arg_ptr->argp,arg_ptr->envp, NULL);
	pthread_exit(NULL);
}

int main (int nArg, char* cArg[]) {
	int width=352;
	int height=288;
	int curBuf = 0;
	int ftot = 1000; //< Number of frames to run through for FPS accuracy
	int fcount = 1;
	char *filename="default", *inBuf[2];
	char *RAMBufferA[2], *Ypointer, *Upointer, *Vpointer;
	
	struct img_args *iargs;
	ifstream Source;
	ofstream Destination;
	int thread_id;
	pthread_t pts;

	struct thread_args t_args;
	spe_context_ptr_t ctx;
        unsigned int createflags = 0;
        int envp;
        void* argp;
	char *image;
	unsigned int msg=RUN, msg2=RDY;
        int counter=0;
        double time_elapsed=0.0;
        double start=mysecond();
        double stop;
        double old_time_elapsed=-11.0;
	int frame=0;
	//Parse commandline arguments
	if (nArg > 0) filename=cArg[1];
	if (nArg > 1) width=(int)atoi(cArg[2]);
	if (nArg > 2) height=(int)atoi(cArg[3]);
	if (nArg > 3) frame=(int)atoi(cArg[4]);
	printf("getting image nr %d\n",frame);

	//Open input file and read the first two frames in
	Source.open(filename,ios::binary);
	Destination.open("dest.yuv,ios::binary");
	inBuf[0]=(char*)memalign(128,width*height+((width*height)/2));
	inBuf[1]=(char*)memalign(128,width*height+((width*height)/2));
	Source.seekg((width*height+width*height/2)*frame);

	Source.read(inBuf[0],(width*height+(width*height)/2));
	Source.read(inBuf[1],(width*height+(width*height)/2));


	Ypointer=inBuf[curBuf];
	Upointer=Ypointer+width*height;
	Vpointer=Upointer+width*height/4;
	
	iargs=(struct img_args*)memalign(128,sizeof(*iargs));
	cout<<"iargs created"<<endl;
	iargs->srcW=width;
	iargs->srcH=height;
	iargs->Ystart[0]=(unsigned long long)inBuf[curBuf];
	iargs->Ustart[0]=iargs->Ystart[0]+width*height;
	iargs->Vstart[0]=iargs->Ystart[0]+width*height + width*height/4;
	iargs->Ystart[1]=iargs->Ystart[0];
	iargs->Ustart[1]=iargs->Ustart[0];
	iargs->Vstart[1]=iargs->Vstart[0];


	iargs->dstW=1920; // set this to desired resolution
	iargs->dstH=1080;

	RAMBufferA[0]=(char*)memalign(128,iargs->dstW*iargs->dstH+((iargs->dstW*iargs->dstH)/2));
	RAMBufferA[1]=(char*)memalign(128,iargs->dstW*iargs->dstH+((iargs->dstW*iargs->dstH)/2));	

	iargs->Output[0]=(unsigned long long)RAMBufferA[0];
	iargs->Output[1]=(unsigned long long)RAMBufferA[1]; //< swap the fb to get the address of the other buffer

	ctx=spe_context_create(createflags, NULL);
	cout<<"context created"<<endl;

	argp=iargs;
	envp=sizeof(*iargs);	
	t_args.ctx=ctx;
	image="spu_yuv2rgb";
	cout<<image<<"\n";
	t_args.image=image;
	t_args.argp=argp;
	t_args.envp=(void*)envp;

	thread_id=pthread_create(&pts,NULL,&spe_thread,&t_args);
	printf("spe run initiatied\n");
	
	while (spe_in_mbox_status(ctx) == 0); //start processing
	spe_in_mbox_write(ctx,&msg,1,SPE_MBOX_ALL_BLOCKING);
	
	while (msg2 != STOP)
	{
		counter++;

		while (spe_out_mbox_status(ctx) == 0);
			spe_out_mbox_read(ctx,&msg2,1);
		
		if (fcount == ftot)  {
			msg2=STOP;
			while (spe_in_mbox_status(ctx) == 0);
			spe_in_mbox_write(ctx,&msg2,1,SPE_MBOX_ALL_BLOCKING);
			stop=mysecond();
			printf("clock %f\n", stop);
			time_elapsed=(double)stop-start;
			printf("Time of execution :%f\n",time_elapsed);
			printf("Number of frames displayed %d\n",fcount);
			printf("width : %d, height : %d , FPS : %f\n",iargs->srcW,iargs->srcH,fcount/time_elapsed);
		}
		if (msg2 ==RDY ) {

			if (time_elapsed > (old_time_elapsed + 10))
			{
				printf("Frames per second %f:\n",counter/time_elapsed);
				counter=0;
				old_time_elapsed=time_elapsed;
			}

			fcount++;
			msg2=RDY;
		//	msg2=UPDATE //this will update your resultion but keep running!
		//	msg2=STOP; // this will stop the loop!
			while (spe_in_mbox_status(ctx) == 0);//start on next image
			spe_in_mbox_write(ctx,&msg2,1,SPE_MBOX_ALL_BLOCKING);
		}
	

	}
	Destination.write(RAMBufferA[0],iargs->dstW*iargs->dstH + ((iargs->dstW*iargs->dstH)/2)); // writes your new scaled YUV to a file!
	Destination.close();
	
	// cleanup	
	pthread_join(pts,NULL);
	spe_context_destroy(ctx);

	Source.close();
	return(0) ;
}

