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
#include "geometry.h"

uint32_t gpu_offset[2] = { 0x0, 0x0d000000 };

void set_xdr_base_address
(
    uint32_t gpu_ioif
)
{
	gpu_offset[1] = gpu_ioif;
}

int set_geometry_source
(
    const geom_desc_t *desc,
    geom_channels_t channel,
    uint32_t base_offset,
    uint32_t *fifo,
    uint32_t hw_sub
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	BEGIN_RING(Nv3D, NV40TCL_VTXFMT( desc->type ), 1 );
	OUT_RING  ( desc->format | ( desc->components << NV40TCL_VTXFMT_SIZE_SHIFT ) | ( desc->stride << NV40TCL_VTXFMT_STRIDE_SHIFT )  );

	BEGIN_RING(Nv3D, NV40TCL_VTXBUF_ADDRESS( desc->type ), 1 );
	OUT_RING  ( ( desc->offset + base_offset + gpu_offset[channel] ) | ( channel << 31 ) );

	return ptr - fifo;

}

int set_index_source
(
    geom_channels_t channel,
    uint32_t offset,
    uint32_t *fifo,
    uint32_t hw_sub
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;
#define NV40TCL_INDEX_ADDRESS 0x181c
#define NV40TCL_INDEX_CONTROL 0x1820
#define NV40TCL_INDEX_DATA    0x1824

	BEGIN_RING(Nv3D, NV40TCL_INDEX_ADDRESS, 1 );
	OUT_RING( ( offset + gpu_offset[channel] ) );

	BEGIN_RING(Nv3D, NV40TCL_INDEX_CONTROL, 1 );
	OUT_RING( 0x10 | ( channel << 31 ) );

	return ptr - fifo;
}

int set_geometry_pipe
(
    uint32_t ddr_id,
    uint32_t xdr_id,
    uint32_t *fifo,
    uint32_t hw_sub
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;
	BEGIN_RING(Nv3D, 0x1dac, 1 );
	OUT_RING( 0 );


	BEGIN_RING(Nv3D,NV40TCL_DMA_VTXBUF0, 2 );
	OUT_RING  ( ddr_id );
	OUT_RING  ( xdr_id );

	return ptr - fifo;

}




int draw_primitives
(
    uint32_t use_index_buffer,
    primitives_t type,
    uint32_t first,
    uint32_t num_primitives,
    uint32_t *fifo,
    uint32_t hw_sub
)
{
	uint32_t i;
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;
	
	if( num_primitives == 0 )
	{
	    return 0;
	}
	
	uint32_t vdata = use_index_buffer ? NV40TCL_INDEX_DATA : NV40TCL_VB_VERTEX_BATCH;
	vdata |= 0x40000000;
	
	BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
	OUT_RING  (type);

	uint32_t end = ( num_primitives & 0xff );

	if( end == 0 )
	{
	    BEGIN_RING(Nv3D, vdata, num_primitives / 0x100 );
	    for( i = 0; i < num_primitives; i += 0x100 )
	    {
		OUT_RING( ( first << NV40TCL_VB_VERTEX_BATCH_START_SHIFT ) | ( 0xff << NV40TCL_VB_VERTEX_BATCH_COUNT_SHIFT ) );
		first += 256;
	    }    
	}
	else
	{
	    BEGIN_RING(Nv3D, vdata, 1 + num_primitives / 0x100 );
	    for( i = 0; i < num_primitives - end; i += 0x100 )
	    {
		OUT_RING( ( first << NV40TCL_VB_VERTEX_BATCH_START_SHIFT ) | ( 0xff << NV40TCL_VB_VERTEX_BATCH_COUNT_SHIFT ) );
		first += 256;	
	    }
	    
	    OUT_RING( ( first << NV40TCL_VB_VERTEX_BATCH_START_SHIFT ) | ( end  << NV40TCL_VB_VERTEX_BATCH_COUNT_SHIFT ) );
	
	}
	
	

	BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
	OUT_RING  (NV40TCL_BEGIN_END_STOP);

	return ptr - fifo;
}



