/*Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#ifndef __LIBSPEDMA_H
#define __LIBSPEDMA_H

#include <spu_mfcio.h>
typedef unsigned int uint32_t;
//typedef unsigned int uintptr_tls -l;

static inline void dmaGetnWait(void *localstore, unsigned long long extern_adr, uint32_t size, int tag)
{
	static int i=1;
	int uiMask=1<<tag;
	mfc_get(localstore, extern_adr, size, tag, 0, 0);
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();	
}

static inline void dmaPutnWait(void *localstore, unsigned long long extern_adr, uint32_t size, int tag)
{	
	static int i=1;
	int uiMask=1<<tag;
	mfc_put(localstore, extern_adr, size, tag, 0, 0);
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();
}

static inline void dmaWaitTag(int tag)
{
	static int i=1;
	int uiMask=1<<tag;
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_all();
}

static inline void dmaGet(void *localstore, unsigned long long extern_adr, uint32_t size, int tag) {
//	printf("Get to %08x from %08x size %i\n",localstore,extern_adr,size); 
	mfc_get(localstore, extern_adr,size,tag, 0, 0);
}

static inline void dmaGetb(void *localstore, unsigned long long extern_adr, uint32_t size, int tag) {
	//  printf("Get to %08x from %08x size %i\n", ls, ea, size);
	mfc_getb(localstore, extern_adr, size, tag, 0, 0);
}
static inline void dmaGetf(void *localstore, unsigned long long extern_adr, uint32_t size, int tag) {
	//  printf("Get to %08x from %08x size %i\n", ls, ea, size);
	mfc_getf(localstore, extern_adr, size, tag, 0, 0);
}



static inline void dmaPut(void *localstore, unsigned long long extern_adr, uint32_t size, int tag) {
	//  printf("Get to %08x from %08x size %i\n", ls, ea, size);
	mfc_put(localstore, extern_adr, size, tag, 0, 0);
}
static inline void dmaPutb(void *localstore, unsigned long long extern_adr, uint32_t size, int tag) {
	//  printf("Get to %08x from %08x size %i\n", ls, ea, size);
	mfc_putb(localstore, extern_adr, size, tag, 0, 0);
}
static inline void dmaPutf(void *localstore, unsigned long long extern_adr, uint32_t size, int tag) {
	//  printf("Get to %08x from %08x size %i\n", ls, ea, size);
	mfc_putf(localstore, extern_adr, size, tag, 0, 0);
}


static inline void dmaWaitAll(unsigned int uiMask) {
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_all();
}

static inline void dmaWaitAny(unsigned int uiMask) {
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();	
}


#endif
