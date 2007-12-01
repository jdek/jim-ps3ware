/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */


typedef struct
{
	uint32_t        gpu_dst_base;
	uint32_t        gpu_src_base;
	uint32_t       *dma_pool;
	uint32_t	dma_pool_size;
	uint32_t 	dma_ptr;
}
dma_pool_t;

typedef enum 
{
	XDR_TO_DDR,
	DDR_TO_XDR
}
transfer_t;

int put_dma_dword_async
(
    dma_pool_t *pool,
    uint32_t data,
    transfer_t type,
    uint32_t dst_offset,
    uint32_t *fifo_buffer
);


