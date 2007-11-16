/**
 * SPU Rectangle Draw kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * draw_rect.c  - source code for draw_rect.h
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <spu-medialib/data_2d.h>
#include <spu-medialib/spu_control.h>

#include "draw_rect.h"

extern spe_program_handle_t spu_draw_rect_handle;
// extern spe_program_handle_t spu_blit_yuv420_scale_handle;
// extern spe_program_handle_t spu_blit_yuv422_handle;
// extern spe_program_handle_t spu_yuv2argb_scaler_handle;
// 

struct draw_rect_s {
		struct data_2 *data;
		struct spe_context * ctx;
		struct spe_event_unit event;
		spe_event_handler_ptr_t spe_event;
		pthread_t pts;
		int thread_id;
		void *argp;
		void *envp;
		unsigned int createflags;
   		unsigned int runflags;
   		unsigned int entry;
		spe_program_handle_t run_handle;
//		char *filename;
};

static void * local_spe_thread(void * arg) 
{
	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;	

	if (spe_program_load(arg_ptr->ctx, &arg_ptr->run_handle) < 0) 
	{
		perror("error loading spu-elf");
		pthread_exit(NULL);
	}

	printf("starting Spu\n");
	spe_context_run(arg_ptr->ctx, &arg_ptr->entry, arg_ptr->runflags,arg_ptr->argp,arg_ptr->envp, NULL);

	spe_context_destroy(arg_ptr->ctx);
	pthread_exit(NULL);
}

draw_rect_t* init_draw_rect()
{
	
// 	int status;
	struct draw_rect_s* local;
	local=(struct draw_rect_s *)memalign(128,sizeof(struct draw_rect_s));
	local->data=(struct data_2*)memalign(128,sizeof(struct data_2));

	local->argp=local->data;
	local->envp= (void*)sizeof(struct data_2);
	
	local->createflags=0;
//	bs->createflags=SPE_EVENTS_ENABLE;
	local->entry=SPE_DEFAULT_ENTRY;
	local->runflags=0;
	local->run_handle=spu_draw_rect_handle;
	local->ctx=spe_context_create(local->createflags, NULL);
	printf("context created\n");
	local->thread_id=pthread_create(&local->pts,NULL,&local_spe_thread,local);
	printf("thread started\n");

// 	bs->spe_event = spe_event_handler_create();
// 	bs->event.spe = bs->ctx;
// 	bs->event.events = SPE_EVENT_OUT_INTR_MBOX | SPE_EVENT_SPE_STOPPED;
// 	spe_event_handler_register(bs->spe_event, &bs->event);
	
	return local;
}

void prepare_solid(draw_rect_t *arg, ea_t outpoiter,int pitchOut,int bppO,int alu, Spu_Pixel planemask, Spu_Pixel Fg) { //initiate a solid fill operation

	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;
	arg_ptr->data->Outp=(unsigned long long)outpoiter;
	arg_ptr->data->pitchOut=pitchOut;
	arg_ptr->data->bppO=bppO;
	arg_ptr->data->alu=alu;
	arg_ptr->data->planemask=planemask;
	arg_ptr->data->Fg=Fg;
	arg_ptr->data->operation=FILL;

}

void solid(draw_rect_t *arg, ea_t outpointer,int x1, int x2 , int y1 , int y2) { //requires initialisation

	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;
	arg_ptr->data->Outp=(unsigned long long)outpointer;
	arg_ptr->data->width=x2-x1;
	arg_ptr->data->height=y2-y1;
	arg_ptr->data->dst_x=x1;
	arg_ptr->data->dst_y=y1;
	draw_rect_send_message(arg_ptr,RUN);
}

void prepare_copy(draw_rect_t *arg, ea_t outpoiter,ea_t inpointer,int pitchIn,int pitchOut,int bppO,int alu, Spu_Pixel planemask) { //initiate a copy operation

	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;
	arg_ptr->data->Outp=(unsigned long long)outpoiter;
	arg_ptr->data->Inp=(unsigned long long)inpointer;
	arg_ptr->data->pitchIn=pitchIn;
	arg_ptr->data->pitchOut=pitchOut;
	arg_ptr->data->bppO=bppO;
	arg_ptr->data->alu=alu;
	arg_ptr->data->planemask=planemask;
// 	arg_ptr->data->Fg=Fg;
	arg_ptr->data->operation=COPY;

// 00262     Bool        (*PrepareCopy) (PixmapPtr       pSrcPixmap,
// 00263                                 PixmapPtr       pDstPixmap,
// 00264                                 int             dx,
// 00265                                 int             dy,
// 00266                                 int             alu,
// 00267                                 Pixel           planemask);

}

void copy(draw_rect_t *arg, ea_t outpointer,ea_t inpointer,int srcX, int srcY , int dstX , int dstY,int width,int height) { //requires initialisation

	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;
	arg_ptr->data->Outp=(unsigned long long)outpointer;
	arg_ptr->data->Inp=(unsigned long long)inpointer;
	arg_ptr->data->src_x=srcX;
	arg_ptr->data->src_y=srcY;
	arg_ptr->data->width=width;
	arg_ptr->data->height=height;
	arg_ptr->data->dst_x=dstX;
	arg_ptr->data->dst_y=dstY;
	draw_rect_send_message(arg_ptr,RUN);

// 00293     void        (*Copy) (PixmapPtr       pDstPixmap,
// 00294                          int    srcX,
// 00295                          int    srcY,
// 00296                          int    dstX,
// 00297                          int    dstY,
// 00298                          int    width,
// 00299                          int    height);
}

unsigned int draw_rect_receive_message_hard(draw_rect_t *arg)
{
	unsigned int message;
	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;

	while (spe_out_mbox_status(arg_ptr->ctx) == 0);
	spe_out_mbox_read(arg_ptr->ctx,&message,1);
		
	return message;

}


void draw_rect_destroy(draw_rect_t* arg)
{
	unsigned int message=STOP;
	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;

	spe_in_mbox_write(arg_ptr->ctx,&message,1,SPE_MBOX_ALL_BLOCKING);
	
	pthread_join(arg_ptr->pts,NULL);
	spe_context_destroy(arg_ptr->ctx);
	
}

void draw_rect_send_message(draw_rect_t *arg,unsigned int message)
{
	struct draw_rect_s * arg_ptr;
	arg_ptr=(struct draw_rect_s *) arg;
	while (spe_in_mbox_status(arg_ptr->ctx) == 0); // switch this to a nice little interupt based one
	spe_in_mbox_write(arg_ptr->ctx,&message,1,SPE_MBOX_ALL_BLOCKING);	
}