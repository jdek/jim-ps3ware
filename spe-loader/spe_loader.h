/**
 * SPE Loader
 * --------------------------------
 * Licensed under the BSDv2 
 * spe_header.h  - header for shared data for main function in the loader example
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

#ifndef __SPE_LOADER_H
#define __SPE_LOADER_H

#include <stdint.h>
#include <stdio.h>

typedef struct arg_s {
	uint64_t fileaddr;
	uint64_t fsize;
	int	argument;
} arg_t ;

#ifdef __SPU__

#include <spu_mfcio.h>
#include <spu_intrinsics.h>

typedef struct functions_s {
	void (*dmaGetnWait)(void *localstore, unsigned long long extern_adr, uint32_t size, int tag);	
	void (*printint)(int to_print);
}functions_t;


 void dmaGetnWait(void *localstore, unsigned long long extern_adr, uint32_t size, int tag)
{
	int uiMask=1<<tag;
	mfc_get(localstore, extern_adr, size, tag, 0, 0);
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();	
}

void printint(int to_print){
	printf("Printing int %d\n",to_print);	
}

 void dmaWaitAny(unsigned int uiMask) {
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();	
}


#endif

#endif

