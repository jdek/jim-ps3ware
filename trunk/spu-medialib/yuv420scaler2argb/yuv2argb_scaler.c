/**
 * SPU YUV420/YV12 scaler to ARGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * yuv2argb_scaler.c  - source code for yuv2argb_scaler.h 
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
#include <pthread.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

      

#include <yuv_datastructs.h>
#include <spu_control.h>
#include "yuv2argb_scaler.h"

extern spe_program_handle_t spu_yuv2argb_scaler_handle;

struct yuvscaler2argb_s {
		struct img_args *iargs;
		struct spe_context * ctx;
		struct spe_event_unit event;
		spe_event_handler_ptr_t spe_event_yuv2rgb;
		pthread_t pts;
		int thread_id;
		void *argp;
		void *envp;
		unsigned int createflags;
   		unsigned int runflags;
   		unsigned int entry;
		char *filename;
};

static void * yuvscsc_spe_thread(void * arg) 
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;	

	if (spe_program_load(arg_ptr->ctx, &spu_yuv2argb_scaler_handle) < 0) 
	{
		perror("error loading spu-elf spu_yuv2argb_scaler");
		pthread_exit(NULL);
	}


	spe_context_run(arg_ptr->ctx, &arg_ptr->entry, arg_ptr->runflags,arg_ptr->argp,arg_ptr->envp, NULL);

	pthread_exit(NULL);
}

yuvscaler2argb_t * yuvscsc_init_yuv2argb_scaler(int srcW,int srcH,int dstW,int dstH,int offset, int maxwidth,ea_t front_inYBuffer, ea_t back_inYBuffer,ea_t front_inUBuffer, ea_t back_inUBuffer,ea_t front_inVBuffer, ea_t back_inVBuffer, ea_t front_outBuffer, ea_t back_outBuffer)
{
	struct yuvscaler2argb_s *yuvcsc;
	yuvcsc=(struct yuvscaler2argb_s *)memalign(64,sizeof(struct yuvscaler2argb_s));
	yuvcsc->iargs=(struct img_args *)memalign(128,sizeof(struct img_args));
	yuvcsc->iargs->srcW=srcW;
	yuvcsc->iargs->srcH=srcH;
	yuvcsc->iargs->dstH=dstH;
	yuvcsc->iargs->dstW=dstW;
	yuvcsc->iargs->offset=offset;
	yuvcsc->iargs->maxwidth=maxwidth;
	yuvcsc->iargs->Ystart[0]=(unsigned long long)front_inYBuffer;
	yuvcsc->iargs->Ystart[1]=(unsigned long long)back_inYBuffer;

	yuvcsc->iargs->Ustart[0]=(unsigned long long)front_inUBuffer;
	yuvcsc->iargs->Ustart[1]=(unsigned long long)back_inUBuffer;
	yuvcsc->iargs->Vstart[0]=(unsigned long long)front_inVBuffer;
	yuvcsc->iargs->Vstart[1]=(unsigned long long)back_inVBuffer;

	yuvcsc->iargs->Output[0]=(unsigned long long)front_outBuffer;
	yuvcsc->iargs->Output[1]=(unsigned long long)back_outBuffer;
	yuvcsc->envp=(void*)sizeof(struct img_args);
	yuvcsc->argp=yuvcsc->iargs;	
	yuvcsc->createflags=SPE_EVENTS_ENABLE;
	yuvcsc->entry=SPE_DEFAULT_ENTRY;
	yuvcsc->runflags=0;
	yuvcsc->filename="";
	yuvcsc->ctx=spe_context_create(yuvcsc->createflags, NULL);
	yuvcsc->thread_id=pthread_create(&yuvcsc->pts,NULL,&yuvscsc_spe_thread,yuvcsc);

	yuvcsc->spe_event_yuv2rgb = spe_event_handler_create();
	yuvcsc->event.spe = yuvcsc->ctx;
	yuvcsc->event.events = SPE_EVENT_OUT_INTR_MBOX | SPE_EVENT_SPE_STOPPED;
	spe_event_handler_register(yuvcsc->spe_event_yuv2rgb, &yuvcsc->event);

	return yuvcsc;
}

spe_context_ptr_t yuvscsc_getCTX(yuvscaler2argb_t * arg)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	return arg_ptr->ctx;
}

unsigned int yuvscsc_receive_message(yuvscaler2argb_t *arg)
{
	unsigned int message;
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;

	int retries = 3;

	while(retries) {
		if(spe_event_wait(arg->spe_event_yuv2rgb, &arg->event, 1, -1) <= 0 ||
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

void yuvscsc_send_message(yuvscaler2argb_t *arg,unsigned int message)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	while (spe_in_mbox_status(arg_ptr->ctx) == 0); // switch this to a nice little interupt based one
	spe_in_mbox_write(arg_ptr->ctx,&message,1,SPE_MBOX_ALL_BLOCKING);	
}


void yuvscsc_destroy(yuvscaler2argb_t* arg)
{
	unsigned int message=STOP;
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;

	spe_in_mbox_write(arg_ptr->ctx,&message,1,SPE_MBOX_ALL_BLOCKING);
	
	pthread_join(arg_ptr->pts,NULL);
	spe_context_destroy(arg_ptr->ctx);
	
}

unsigned int yuvscsc_get_dstW(const yuvscaler2argb_t* arg)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	return arg->iargs->dstW;
}
unsigned int yuvscsc_get_srcW(const yuvscaler2argb_t* arg)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	return arg->iargs->srcW;
}

unsigned int yuvscsc_get_offset(const yuvscaler2argb_t* arg)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	return arg->iargs->offset;
}

unsigned int yuvscsc_get_maxwidth(const yuvscaler2argb_t* arg)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	return arg->iargs->maxwidth;
}

void yuvscsc_set_dstW(yuvscaler2argb_t* arg,int dstw)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	arg->iargs->dstW=dstw;
}

void yuvscsc_set_srcW(yuvscaler2argb_t* arg,int srcw)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	arg->iargs->srcW=srcw;
}

void yuvscsc_set_offset(yuvscaler2argb_t* arg,int offset)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	arg->iargs->offset=offset;
}

void yuvscsc_set_maxwidth(yuvscaler2argb_t* arg,int maxwidth)
{
	struct yuvscaler2argb_s * arg_ptr;
	arg_ptr=(struct yuvscaler2argb_s *) arg;
	arg->iargs->maxwidth=maxwidth;
}
