/**
 * SPE Loader
 * --------------------------------
 * Licensed under the BSDv2 
 * spe_loader.c  - source code for spu function in the loader example
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
#include "spe_loader.h"


int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	arg_t *args = memalign(128, envp);
	
	dmaGetnWait(args,argp,(envp +15)&~15,1);
	
	
	///fetch the executable code as data
	char *data=(char*)memalign(128,(args->fsize +15)&~15);
	dmaGetnWait(data,argp,(args->fsize +15)&~15,1);
	
	///setting up some functions to share
	functions_t *functions;
	
	functions=memalign(128,sizeof(functions_t));
	
	functions->dmaGetnWait=dmaGetnWait;
	
	functions->printint=printint;
	
	int (*runme)(functions_t *func,arg_t *arg);
	
	runme=data;
	printf("attemting to run spe from spe\n ");
	int result=runme(functions,args);
	printf("spe from spe completed with results %d " , result );
	
	while (spu_stat_out_mbox() == 0);
	
	spu_write_out_mbox(result);
	return result;
}