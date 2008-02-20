/**
 * SPE Loader
 * --------------------------------
 * Licensed under the BSDv2 
 * loader.c  - source code for main function in the loader example
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <libspe2.h>
#include <pthread.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include "shared_functions.h"


typedef struct run_struct_s {
		struct spe_context * ctx;
		struct arg_s *args;
		pthread_t pts;
		int thread_id;
		uint64_t *argp;
		uint64_t *envp;
		spe_program_handle_t *image;
		unsigned int createflags;
   		unsigned int runflags;
   		unsigned int entry;
}run_struct_t;

static void * spe_thread(void * arg); 

int main () {
	
	char *fname="spe_loadme";
	
	
	run_struct_t *rs;
	rs = (struct run_struct_s *)memalign(64,sizeof(struct run_struct_s ));
	
	//arg_t *args;
	
	rs->args = (struct arg_s *)memalign(128,sizeof(struct arg_s));
	
	printf("Reading file\n");
	
	FILE *LOADME;
	
	LOADME=fopen(fname,"rb");
	if (LOADME==NULL) {fprintf(stderr,"Error opening file"); return -1; 
	}else {
		printf("File opened\n");
	}
	fseek(LOADME,0,SEEK_END);
	
	rs->args->fsize=ftell(LOADME);
	
	rs->args->argument=1337;
	
	char *buffer;

	printf("ppu: buffer pointer %p\n",&buffer);
	buffer= (char  *)memalign(128, (( rs->args->fsize + 15 ) &~15) );

	rewind(LOADME);

	fread(buffer,1,rs->args->fsize,LOADME);

	printf("ppu: first 4 bytes of executable 0:%1x 1:%1x 2:%1x 3:%1x\n",buffer[0],buffer[1],buffer[2],buffer[3]);

	rs->args->fileaddr=buffer;

	rs->argp = rs->args;

	printf("ppu: filesize is %d\n",rs->args->fsize);
	printf("ppu: file buffer is allocated at %p\n",rs->args->fileaddr);

	rs->envp =(void*)sizeof(struct arg_s);
	
	rs->createflags = 0;
	
	rs->entry = SPE_DEFAULT_ENTRY;
	
	rs->runflags = 0;
	
	rs->image=spe_image_open("spe_loader");
	
	rs->ctx = spe_context_create(rs->createflags, NULL);
	printf("ppu: starting spu thread\n");
	rs->thread_id = pthread_create(&rs->pts, NULL,&spe_thread,rs);
	
	uint32_t msg=0;
	
	while (spe_out_mbox_status(rs->ctx) == 0);
	spe_out_mbox_read(rs->ctx,&msg,1);
	
	printf("ppu: spe sendt pre load handshake %d\n",msg);

	while (spe_out_mbox_status(rs->ctx) == 0);
	spe_out_mbox_read(rs->ctx,&msg,1);

	printf("ppu: spe sendt back %d\n",msg);	

	fclose(LOADME);
	
	free(buffer);

	spe_context_destroy(rs->ctx);
	printf("ppu_thread: spu context destroyed\n");	

	return 0;
}
static void * spe_thread(void * arg)
{
	run_struct_t * arg_ptr;
	arg_ptr=(run_struct_t *) arg;	

	if (spe_program_load(arg_ptr->ctx, arg_ptr->image) < 0) 
	{
		fprintf(stderr,"ppu: error loading spu-elf");
		pthread_exit(NULL);
	}

	printf("ppu_thread: starting spu\n");
	spe_context_run(arg_ptr->ctx, &arg_ptr->entry, arg_ptr->runflags, arg_ptr->argp, arg_ptr->envp, NULL);
	printf("ppu_thread: exit from SPU\n");

	pthread_exit(NULL);
}