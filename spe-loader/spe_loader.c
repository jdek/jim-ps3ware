/**
 * SPE Loader
 * --------------------------------
 * Licensed under the BSDv2 
 * spe_loader.c  - source code for spu function in the loader example
 * Copyright (c) 2007, Kristian Jerpetj�n <kristian.jerpetjoen@gmail.com>
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
#include "shared_functions.h"
#include <malloc.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	arg_t *args =(arg_t *) memalign(128, (envp +15)&~15);
	
	dmaGetnWait(args,argp,(envp +15)&~15,1);

	printf("spu: Dma of args complete\n");

	printf("spu: filesize is %d\n",(int)args->fsize);
	printf("spu: file is located at h:%x\tl:%x\n",(unsigned int)(args->fileaddr>>32),(unsigned int)args->fileaddr);

	//args->argument=1337;
	printf("spu: args argument is %d\n",args->argument);
	///fetch the executable code as data

	
	char *data=(char*)memalign(128,(args->fsize +15)&~15);

	dmaGetnWait(data,args->fileaddr,(args->fsize +15)&~15,1);

	printf("spu: Dma of Data(program) complete\n");
	
	///setting up some functions to share
	functions_t functions;
	
	functions.dmaGetnWait=dmaGetnWait;
	
	functions.printint=printint;

	while (spu_stat_out_mbox() == 0);
	int result=1337;
	spu_write_out_mbox(result);
	
	int (*runme)(functions_t *func,arg_t *arg);

	printf("spu: byte 0:%1x 1:%1x 2:%1x 3:%1x\n",data[0],data[1],data[2],data[3]);
	runme=(void*)data;

	printf("spu: attemting to run spe as data from spe\n");
	result=runme(&functions,args);
	printf("spu: spe from spe completed with results %d \n" , result );
	
	while (spu_stat_out_mbox() == 0);
	
	spu_write_out_mbox(result);
	return result;
}