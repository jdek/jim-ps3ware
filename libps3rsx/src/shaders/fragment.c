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
#include "fragment.h"

int set_fragment_shader
(
    const fragment_shader_desc_t *desc,
    const uint32_t data_storage,
    uint32_t *fifo,
    uint32_t hw_subch 
)
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_subch;

	printf( "frag prog 0x%x num_regs 0x%x \n", data_storage, desc->num_regs );
	
	BEGIN_RING(Nv3D, NV40TCL_FP_ADDRESS, 1);
	OUT_RING  ( data_storage | NV40TCL_FP_ADDRESS_DMA0);
	BEGIN_RING(Nv3D, NV40TCL_FP_CONTROL, 1);
	OUT_RING  ( ( desc->num_regs << NV40TCL_FP_CONTROL_TEMP_COUNT_SHIFT ) );

	return ptr - fifo;
}
