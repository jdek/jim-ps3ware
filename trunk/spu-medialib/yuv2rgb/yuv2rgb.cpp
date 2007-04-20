/** 
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * yuv2rgb.cpp - PPU example implementation
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * Copyright (c) 2007, John Kelley <ps2dev@kelley.ca>
 *
 * $Id$
 */

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
#include "libfb.h"

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

	program = spe_image_open("spu_yuv2rgb");
	
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
	int width=1280;
	int height=720;
	int curBuf = 0;
	int ftot = 10000; //< Number of frames to run through for FPS accuracy
	int fcount = 1;
	char *filename="default", *inBuf[2];
	char *RAMBufferA, *Ypointer, *Upointer, *Vpointer;
	struct img_args *iargs;
	ifstream Source;
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

	//Parse commandline arguments
	if (nArg > 0) filename=cArg[1];
	if (nArg > 1) width=(int)atoi(cArg[2]);
	if (nArg > 2) height=(int)atoi(cArg[3]);
	
	//init framebuffer
	RAMBufferA = (char *)fb_init();	
	
	//Open input file and read the first two frames in
	Source.open(filename,ios::binary);
	inBuf[0]=(char*)memalign(128,width*height+((width*height)/2));
	inBuf[1]=(char*)memalign(128,width*height+((width*height)/2));
	
	Source.read(inBuf[0],(width*height+(width*height)/2));
	Source.read(inBuf[1],(width*height+(width*height)/2));
	
	Ypointer=inBuf[curBuf];
	Upointer=Ypointer+width*height;
	Vpointer=Upointer+width*height/4;
	
	iargs=(struct img_args*)memalign(128,sizeof(*iargs));
	cout<<"iargs created"<<endl;
	iargs->width=width;
	iargs->height=height;
	iargs->Ystart=(unsigned long long)inBuf[curBuf];
	iargs->Ustart=iargs->Ystart+width*height;
	iargs->Vstart=iargs->Ystart + width*height + width*height/4;
	iargs->maxwidth=fb_getXres();
	iargs->maxheight=fb_getYres();
	iargs->Output[0]=(unsigned long long)RAMBufferA;
	iargs->Output[1]=(unsigned long long)fb_swap(); //< swap the fb to get the address of the other buffer
	fb_swap(); //< swap back to the first buffer for consistency


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
			printf("width : %d, height : %d , FPS : %f\n",iargs->width,iargs->height,fcount/time_elapsed);
		}
		if (msg2 ==RDY ) {
			fb_swap();
			if (time_elapsed > (old_time_elapsed + 10))
			{
				printf("Frames per second %f:\n",counter/time_elapsed);
				counter=0;
				old_time_elapsed=time_elapsed;
			}

			fcount++;
			msg2=RDY;
		
			Source.read(inBuf[curBuf],(width*height+(width*height)/2));
			while (spe_in_mbox_status(ctx) == 0);//start on next image
			spe_in_mbox_write(ctx,&msg2,1,SPE_MBOX_ALL_BLOCKING);
		}
	

	}

	// cleanup	
	pthread_join(pts,NULL);
	spe_context_destroy(ctx);
	fb_cleanup();
	Source.close();
	return(0) ;
}

