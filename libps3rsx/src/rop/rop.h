/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */


typedef enum
{
	ZERO = 0x0,
	ONE = 0x1,
	SRC_COLOR = 0x300,
	INV_SRC_COLOR = 0x301,
	SRC_ALPHA = 0x302,
	INV_SRC_ALPHA = 0x0303,
	DST_ALPHA = 0x304,
	INV_DST_ALPHA = 0x0305,	
	DST_COLOR = 0x306,
	INV_DST_COLOR = 0x307,
	SRC_ALPHA_SAT = 0x308,
	CONST_COLOR = 0x8001,
	INV_CONST_COLOR = 0x8002,
	CONST_ALPHA = 0x8003,
	INV_CONST_ALPHA = 0x8004,
} blend_modes_t;

typedef enum
{
	DEPTH_FUNC_NEVER = 0x200,
	DEPTH_FUNC_LESS = 0x201,
	DEPTH_FUNC_EQUAL = 0x202,
	DEPTH_FUNC_LEQUAL = 0x203,
	DEPTH_FUNC_GREATER = 0x204,
	DEPTH_FUNC_NOTEQUAL = 0x205,
	DEPTH_FUNC_GEQUAL = 0x206,
	DEPTH_FUNC_ALWAYS = 0x207,
} depth_modes_t;

typedef struct
{
	blend_modes_t src_color_blend;
	blend_modes_t src_alpha_blend;
	blend_modes_t dst_color_blend;
	blend_modes_t dst_alpha_blend;
	
} blend_desc_t;

int set_blend_mode
(
    const blend_desc_t *desc,
    uint32_t *fifo_buffer,
    uint32_t hw_sub 
);


int blend_enable
(
    uint32_t blend_enable, 
    uint32_t *fifo_buffer,
    uint32_t hw_sub 
);


int set_depth_mode
(
    depth_modes_t mode,
    uint32_t *fifo_buffer,
    uint32_t hw_sub 	
);

int depth_enable
(
    uint32_t depth_write_enable,
    uint32_t depth_test_enable,
	
    uint32_t *fifo_buffer,
    uint32_t hw_sub 
);
