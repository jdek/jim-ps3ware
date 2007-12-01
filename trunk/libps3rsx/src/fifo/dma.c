/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Vivien Chappelier <vivien.chappelier@free.fr>
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>

#include "../../include/nouveau_class.h"
#include "../fifo/utils.h"
#include "dma.h"

int put_dma_dword_async
(
    dma_pool_t *pool,
    uint32_t data,
    transfer_t type,
    uint32_t dst_offset,
    uint32_t *fifo
)
{
	uint32_t *ptr = fifo;
	pool->dma_ptr++;
	pool->dma_ptr %= pool->dma_pool_size;
	pool->dma_pool[pool->dma_ptr] = data;

	
	BEGIN_RING( type == DDR_TO_XDR ? 2 : 1, 0x30c, 8 );
	OUT_RING( pool->gpu_src_base + pool->dma_ptr * 4 );
	OUT_RING( pool->gpu_dst_base + dst_offset * 4 );
	
	OUT_RING( 0x4 );
	OUT_RING( 0x4 );
	OUT_RING( 0x4 );
	OUT_RING( 0x1 );
	OUT_RING( 0x101 );
	OUT_RING( 0x0 );

	return ptr - fifo;
}

