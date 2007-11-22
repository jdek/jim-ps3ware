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
#include "textures.h"

int set_texture_2D( 
const texture_desc_t *desc, 
uint32_t offset, 
uint32_t unit, 
tex_wrap_t wrap, 
tex_filter_t mag_filter,
tex_filter_t min_filter,
uint32_t *fifo,
uint32_t hw_subch )
{
  uint32_t *ptr = fifo;
  uint32_t Nv3D = hw_subch;

  uint32_t swz =
    NV40TCL_TEX_SWIZZLE_S0_X_S1 | NV40TCL_TEX_SWIZZLE_S0_Y_S1 |
    NV40TCL_TEX_SWIZZLE_S0_Z_S1 | NV40TCL_TEX_SWIZZLE_S0_W_S1 |
    NV40TCL_TEX_SWIZZLE_S1_X_X | NV40TCL_TEX_SWIZZLE_S1_Y_Y |
    NV40TCL_TEX_SWIZZLE_S1_Z_Z | NV40TCL_TEX_SWIZZLE_S1_W_W;


  BEGIN_RING(Nv3D, NV40TCL_TEX_OFFSET(unit), 8);
  OUT_RING  ( offset );

  uint32_t tex_fmt = ( ( desc->format ) << NV40TCL_TEX_FORMAT_FORMAT_SHIFT ) |
    NV40TCL_TEX_FORMAT_DIMS_2D |
    NV40TCL_TEX_FORMAT_DMA0 |
    NV40TCL_TEX_FORMAT_NO_BORDER | 
    (0x8000) |
    ( desc->mips << NV40TCL_TEX_FORMAT_MIPMAP_COUNT_SHIFT);

  OUT_RING  ( tex_fmt );
  
  OUT_RING  (
    ( wrap << NV40TCL_TEX_WRAP_S_SHIFT ) |
    ( wrap << NV40TCL_TEX_WRAP_T_SHIFT ) |
    ( wrap << NV40TCL_TEX_WRAP_R_SHIFT ) );

  OUT_RING  (NV40TCL_TEX_ENABLE_ENABLE | 0x78000 );
  OUT_RING  (swz);

  OUT_RING  (
    ( min_filter << NV40TCL_TEX_FILTER_MIN_SHIFT ) |
    ( mag_filter << NV40TCL_TEX_FILTER_MAG_SHIFT ) | 
	0x3fd6 );

  OUT_RING  ( ( desc->width << 16 ) | desc->height );
  OUT_RING  (0); /* border ARGB */
  BEGIN_RING(Nv3D, NV40TCL_TEX_SIZE1(unit), 1);

  OUT_RING  ( (1 << NV40TCL_TEX_SIZE1_DEPTH_SHIFT) | 0 );

  return ptr - fifo;

	
}
