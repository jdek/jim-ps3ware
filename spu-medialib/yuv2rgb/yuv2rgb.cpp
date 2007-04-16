/*Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *         * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *             * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 *             THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *             "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *             LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *             A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *             CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *             EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *             PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *             PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *             LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *             NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *             SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

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
//#include "fb_lib.h" will come soon..

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
   	spe_stop_info_t       stop_info;
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
	int frame=10;
	char *filename="default";
	
	if (nArg > 0) filename=cArg[1];
	if (nArg > 1) width=(int)atoi(cArg[2]);
	if (nArg > 2) height=(int)atoi(cArg[3]);
	if (nArg > 3) frame=atoi(cArg[4]);
	


//	control ctrl;
//	PS3FB  FB;
//	FB=new PS3FB();
//	FB.open();
//	cout<<"BPP:"<<FB.getBPP()<<endl;
//	FB->clear();
	
	
	ifstream Source;
	Source.open(filename,ios::binary);
	char* IbufferA;
	char* IbufferB;
	IbufferA=(char*)memalign(128,width*height+((width*height)/2));
	IbufferB=(char*)memalign(128,width*height+((width*height)/2));
	char * RAMbufferA=(char*)memalign(128,width*height*4);
	char * RAMbufferB=(char*)memalign(128,width*height*4);
//	char* ObufferA=(char*)memalign(128,width*height*4);
	
	int ftot =10000; //number of frames to run trough
	int fcount=1;	

	Source.seekg((width*height+width*height/2)*frame);
	Source.read(IbufferA,(width*height+(width*height)/2));
	cout<<"reading file"<<endl;
		 // Read in prior to spe startup..
	Source.seekg((width*height+width*height/2)*frame);
	Source.read(IbufferB,(width*height+(width*height)/2));
	Source.close();
	cout<<"file closed"<<endl;

	
	

	char * Ypointer;
	char * Upointer;
	char * Vpointer;
	char * OpA;
	char * OpB;
	Ypointer=IbufferA;
	Upointer=Ypointer+width*height;
	Vpointer=Upointer+width*height/4;
	
//	FB = new PS3FB();
	
	struct img_args *iargs;

	iargs=(struct img_args*)memalign(128,sizeof(*iargs));
	cout<<"iargs created"<<endl;
	iargs->width=width;
	iargs->height=height;
	iargs->Ystart=(unsigned long long)IbufferA;
	iargs->Ustart=iargs->Ystart+width*height;
	iargs->Vstart=iargs->Ystart + width*height + width*height/4;
//	iargs->maxwidth=FB.getWidth();
//	iargs->maxheight=FB.getHeight();
	iargs->maxwidth=1280;
	iargs->maxheight=720;
	iargs->Ostart=(unsigned long long)RAMbufferA;
	iargs->Ostartb=iargs->Ostart;


	int thread_id;
	pthread_t pts;
	struct thread_args t_args;
	spe_context_ptr_t ctx;
   	unsigned int   createflags = 0;

	int envp;	
	void* argp;


	ctx=spe_context_create(createflags, NULL);
	cout<<"context created"<<endl;

	argp=iargs;
	envp=sizeof(*iargs);	
	t_args.ctx=ctx;
	char* image;
	image="spu_yuv2rgb";
	cout<<image<<"\n";
	t_args.image=image;
	t_args.argp=argp;
	t_args.envp=(void*)envp;

	thread_id=pthread_create(&pts,NULL,&spe_thread,&t_args);
	printf("spe run initiatied\n");

	unsigned int msg=RUN;
	char * C;
	while (spe_in_mbox_status(ctx) == 0); //start processing
	spe_in_mbox_write(ctx,&msg,1,SPE_MBOX_ALL_BLOCKING);
	
	unsigned int msg2=RDY;
	
	int counter=0;
	int lastcount=0;

	double time_elapsed;
	double start=mysecond();
	double stop;
	double old_time_elapsed=-11.0;

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
			printf("clock%d\n",stop);
			time_elapsed=(double)stop-start;
			printf("Time of execution :%f\n",time_elapsed);
			printf("Number of frames displayed %d\n",fcount);
			printf("width : %d, height : %d , FPS : %f\n",iargs->width,iargs->height,fcount/time_elapsed);
		}
		if (msg2 ==RDY ) {
			
		//	FB.swapBuffers(true); //framebuffer

			if (time_elapsed > (old_time_elapsed + 10))
			{
				printf("Frames per second %f:\n",counter/time_elapsed);
				counter=0;
				old_time_elapsed=time_elapsed;
			}

	//		C=RAMbufferA;		//RAMbuffers swapping
	//		RAMbufferA=RAMbufferB;
	//		RAMbufferB=C;

			fcount++;
			msg2=RDY;
			while (spe_in_mbox_status(ctx) == 0);//start on next image
			spe_in_mbox_write(ctx,&msg2,1,SPE_MBOX_ALL_BLOCKING);
		}
	

	}
	
	pthread_join(pts,NULL);
	spe_context_destroy(ctx);
	return(0) ;
}

