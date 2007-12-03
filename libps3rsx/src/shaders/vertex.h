/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */

typedef struct
{
	uint32_t 	vp_in;
	uint32_t	vp_out;
	uint32_t 	dword_length;
	uint32_t	aux[1];
}
vertex_shader_desc_t;


int set_vertex_shader
(
    const vertex_shader_desc_t *desc,
    const uint32_t *data_storage,
    uint32_t *fifo_buffer,
    uint32_t hw_sub 
);

int set_vertex_shader_constants
(
    const float *data,
    uint32_t start_index,
    uint32_t number_of_floats,
    uint32_t *fifo_buffer,
    uint32_t hw_sub 
);


