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
#include "rop.h"

int set_blend_mode
(
    const blend_desc_t *desc,
    uint32_t *fifo,
    uint32_t hw_sub 
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	size_t dst = desc->src_color_blend | ( (size_t)desc->src_alpha_blend << 16 );
	size_t src = desc->dst_color_blend | ( (size_t)desc->dst_alpha_blend << 16 );
	
	
	BEGIN_RING( Nv3D, NV40TCL_BLEND_FUNC_SRC, 2 );
	OUT_RING( src );
	OUT_RING( dst );
	
	BEGIN_RING( Nv3D, NV40TCL_BLEND_EQUATION, 1 );
	OUT_RING( 0x80068006 );
	
	
	return ptr - fifo;
}

int blend_enable
(
    uint32_t blend_enable, 
    uint32_t *fifo,
    uint32_t hw_sub 
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	BEGIN_RING( Nv3D, NV40TCL_BLEND_ENABLE, 1 );
	OUT_RING( blend_enable );
	
	return ptr - fifo;

}

int set_depth_mode
(
    depth_modes_t mode,
    uint32_t *fifo,
    uint32_t hw_sub 	
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	BEGIN_RING( Nv3D, NV40TCL_DEPTH_FUNC, 1 );
	OUT_RING( mode );
	
	return ptr - fifo;

}


int depth_enable
(
    uint32_t depth_write_enable,
    uint32_t depth_test_enable, 
    uint32_t *fifo,
    uint32_t hw_sub 
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	BEGIN_RING( Nv3D,  NV40TCL_DEPTH_WRITE_ENABLE, 2 );
	OUT_RING( depth_write_enable );
	OUT_RING( depth_test_enable );
	
	
	return ptr - fifo;

}
