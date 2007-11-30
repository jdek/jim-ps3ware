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
#include "vertex.h"

int set_vertex_shader
(
    const vertex_shader_desc_t *desc,
    const uint32_t *data_storage,
    uint32_t *fifo,
    uint32_t hw_subch
)
{
	uint32_t i;
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_subch;

	BEGIN_RING(Nv3D, NV40TCL_VP_UPLOAD_FROM_ID, 1);
	OUT_RING  (0);

	for ( i=0; i<desc->dword_length; i+=4 )
	{
		BEGIN_RING(Nv3D, NV40TCL_VP_UPLOAD_INST(0), 4);
		OUT_RING  (data_storage[i + 0]);
		OUT_RING  (data_storage[i + 1]);
		OUT_RING  (data_storage[i + 2]);
		OUT_RING  (data_storage[i + 3]);

		//printf( "%x %x %x %x \n", data_storage[i + 0], data_storage[i + 1],  data_storage[i + 2],  data_storage[i + 3] );

	}


	BEGIN_RING(Nv3D, NV40TCL_VP_START_FROM_ID, 1);
	OUT_RING  (0);

	BEGIN_RING(Nv3D, NV40TCL_VP_ATTRIB_EN, 2);
	OUT_RING  ( desc->vp_in );
	OUT_RING  ( desc->vp_out );

	BEGIN_RING( Nv3D, 0x1478, 1 );
	OUT_RING  (0);

	//printf( "%x \n", desc->vp_in );
	//printf( "%x \n", desc->vp_out );


	return ptr - fifo;
}

int set_vertex_shader_constants
(
    const float *data,
    uint32_t start_index,
    uint32_t number_of_floats,
    uint32_t *fifo,
    uint32_t hw_subch
)
{
	uint32_t i;
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_subch;

	return ptr - fifo;

	BEGIN_RING( Nv3D, NV40TCL_VP_UPLOAD_CONST_ID, number_of_floats + 1 );
	OUT_RING( start_index );

	for( i = 0; i < number_of_floats; ++i )
	{
		OUT_RINGf( data[i] );
	}

	return ptr - fifo;
}
