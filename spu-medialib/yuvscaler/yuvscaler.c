/**
 * SPU YUV scaler kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * yuvscaler.c - source code for yuvscaler.h 
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


#include <pthread.h>
#include <malloc.h>
#include <yuv_datastructs.h>
#include <spu_control.h>
#include "yuvscaler.h"
extern spe_program_handle_t spu_yuvscaler_handle;

struct yuvscaler_s {
		struct img_args *iargs;
		struct spe_context * ctx;
		struct spe_event_unit event;
		spe_event_handler_ptr_t spe_event_yuvscaler;
		pthread_t pts;
		int thread_id;
		void *argp;
		void *envp;
		unsigned int createflags;
   		unsigned int runflags;
   		unsigned int entry;
};

static void * sws_spe_thread(void * arg) 
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;	
//    	spe_program_handle_t * program;
// 
// 	program = spe_image_open("spu_yuvscaler");

   	if (spe_program_load(arg_ptr->ctx, &spu_yuvscaler_handle) < 0) 
	{
		perror("error loading program");
		pthread_exit(NULL);
	}
	spe_context_run(arg_ptr->ctx, &arg_ptr->entry, arg_ptr->runflags,arg_ptr->argp,arg_ptr->envp, NULL);
	pthread_exit(NULL);
}

yuvscaler_t * sws_init_yuvscaler(int srcW,int srcH,int dstW, int dstH,ea_t front_inBuffer, ea_t back_inBuffer, ea_t front_outBuffer, ea_t back_outBuffer)
{
	struct yuvscaler_s *yuvs;
	yuvs=(struct yuvscaler_s *)memalign(64,sizeof(struct yuvscaler_s));
	yuvs->iargs=(struct img_args *)memalign(128,sizeof(struct img_args));
	yuvs->iargs->srcW=srcW;
	yuvs->iargs->srcH=srcH;
	yuvs->iargs->dstW=dstW;
	yuvs->iargs->dstH=dstH;
	yuvs->iargs->Ystart[0]=(unsigned long long)front_inBuffer;
	yuvs->iargs->Ystart[1]=(unsigned long long)back_inBuffer;

	yuvs->iargs->Ustart[0]=yuvs->iargs->Ystart[0]+srcW*srcH; //maybe these should be removed...
	yuvs->iargs->Ustart[1]=yuvs->iargs->Ystart[1]+srcW*srcH;
	yuvs->iargs->Vstart[0]=yuvs->iargs->Ystart[0]+srcW*srcH + srcW*srcH/4;
	yuvs->iargs->Vstart[1]=yuvs->iargs->Ystart[1]+srcW*srcH + srcW*srcH/4;

	yuvs->iargs->Output[0]=(unsigned long long)front_outBuffer;
	yuvs->iargs->Output[1]=(unsigned long long)back_outBuffer;
	yuvs->envp=(void*)sizeof(struct img_args);
	yuvs->argp=yuvs->iargs;	
	yuvs->createflags=SPE_EVENTS_ENABLE;
	yuvs->entry=SPE_DEFAULT_ENTRY;
	yuvs->runflags=0;
	yuvs->ctx=spe_context_create(yuvs->createflags, NULL);
	yuvs->thread_id=pthread_create(&yuvs->pts,NULL,&sws_spe_thread,yuvs);

	yuvs->spe_event_yuvscaler = spe_event_handler_create();
	yuvs->event.spe = yuvs->ctx;
	yuvs->event.events = SPE_EVENT_OUT_INTR_MBOX | SPE_EVENT_SPE_STOPPED;
	spe_event_handler_register(yuvs->spe_event_yuvscaler, &yuvs->event);


	return yuvs;
}

spe_context_ptr_t sws_getCTX(yuvscaler_t * arg)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	return arg_ptr->ctx;
}

unsigned int sws_receive_message(yuvscaler_t *arg)
{
	unsigned int message;
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;

	int retries = 3;

	while(retries) {
		if(spe_event_wait(arg->spe_event_yuvscaler, &arg->event, 1, -1) <= 0 ||
			!(arg->event.events & SPE_EVENT_OUT_INTR_MBOX)) {
			retries--;
		} else {
			break;
		}
	}

	if(retries == 0) {
		perror("Failed to recive result from spe");
	}

	spe_out_intr_mbox_read(arg_ptr->ctx,&message,1,SPE_MBOX_ANY_NONBLOCKING);	
	return message;

}

void sws_send_message(yuvscaler_t *arg,unsigned int message)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	while (spe_in_mbox_status(arg_ptr->ctx) == 0); // switch this to a nice little interupt based one
	spe_in_mbox_write(arg_ptr->ctx,&message,1,SPE_MBOX_ALL_BLOCKING);	
}


void sws_yuvscaler_destroy(yuvscaler_t* arg)
{
	unsigned int message=STOP;
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;

	spe_in_mbox_write(arg_ptr->ctx,&message,1,SPE_MBOX_ALL_BLOCKING);
	
	pthread_join(arg_ptr->pts,NULL);
	spe_context_destroy(arg_ptr->ctx);
	
}

unsigned int sws_get_dstW(const yuvscaler_t* arg)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	return arg->iargs->dstW;
}
unsigned int sws_get_srcW(const yuvscaler_t* arg)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	return arg->iargs->srcW;
}

unsigned int sws_get_dstH(const yuvscaler_t* arg)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	return arg->iargs->dstH;
}

unsigned int sws_get_srcH(const yuvscaler_t* arg)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	return arg->iargs->srcH;
}

void sws_set_dstW(yuvscaler_t* arg,int dstw)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	arg->iargs->dstW=dstw;
}

void sws_set_srcW(yuvscaler_t* arg,int srcw)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	arg->iargs->srcW=srcw;
}

void sws_set_dstH(yuvscaler_t* arg,int dsth)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	arg->iargs->dstH=dsth;
}

void sws_set_srcH(yuvscaler_t* arg,int srch)
{
	struct yuvscaler_s * arg_ptr;
	arg_ptr=(struct yuvscaler_s *) arg;
	arg->iargs->srcH=srch;
}
