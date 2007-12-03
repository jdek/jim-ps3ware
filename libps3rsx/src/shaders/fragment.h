/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */

typedef struct
{
	uint32_t 	num_regs;
	uint32_t	not_used;
	uint32_t 	dword_length;
	uint32_t	aux[1];
}
fragment_shader_desc_t;


int set_fragment_shader
(
    const fragment_shader_desc_t *desc,
    const uint32_t data_storage,
    uint32_t *fifo_buffer,
    uint32_t hw_sub 
);



