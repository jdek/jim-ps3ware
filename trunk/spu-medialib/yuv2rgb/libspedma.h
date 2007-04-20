/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * libspedma.h - SPU DMA Macros 
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 *
 * $Id$
 */

#ifndef __LIBSPEDMA_H
#define __LIBSPEDMA_H

#include <spu_mfcio.h>
typedef unsigned int uint32_t;

static inline void dmaGetnWait(void *localstore, unsigned long long extern_adr, uint32_t size, int tag)
{
	int uiMask=1<<tag;
	mfc_get(localstore, extern_adr, size, tag, 0, 0);
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();	
}

static inline void dmaPutnWait(void *localstore, unsigned long long extern_adr, uint32_t size, int tag)
{	
	int uiMask=1<<tag;
	mfc_put(localstore, extern_adr, size, tag, 0, 0);
	mfc_write_tag_mask(uiMask);
	mfc_read_tag_status_any();
}

static inline void dmaWaitTag(int tag)
{
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

