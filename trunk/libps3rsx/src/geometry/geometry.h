/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */

typedef enum
{
    TRIANGLES = 0x5,
}
primitives_t;

typedef enum
{
    EMPTY=0x0,//not used?
    FLOAT=0x2,
    UBYTE=0x4,
    SHORT=0x5
}
geom_format_t;

typedef enum
{
    POS0 = 0x0,
    WGHT = 0x1,
    NRML = 0x2,
    COL0 = 0x3,
    COL1 = 0x4,
    FOGC = 0x5,
    TEX0 = 0x8,
    TEX1 = 0x9,
    TEX2 = 0xA,
    TEX3 = 0xB,
    TEX4 = 0xC,
    TEX5 = 0xD,
    TEX6 = 0xE,
    TEX7 = 0xF,
}
geom_type_t;

typedef enum
{
    DDR = 0x0,
    XDR = 0x1,
}
geom_channels_t;


typedef struct
{
	geom_format_t format;
	geom_type_t type;
	uint16_t stride;
	uint16_t components;
	uint16_t offset;
}
geom_desc_t;

void set_xdr_base_address
(
    uint32_t gpu_ioif
);


int set_geometry_source
(
    const geom_desc_t *desc,
    geom_channels_t channel,
    uint32_t base_offset,
    uint32_t *fifo,
    uint32_t hw_sub
);



int set_index_source
(
    geom_channels_t channel,
    uint32_t offset,
    uint32_t *fifo,
    uint32_t hw_sub
);

int set_geometry_pipe
(
    uint32_t ddr_id,
    uint32_t xdr_id,
    uint32_t *fifo,
    uint32_t hw_sub
);


int draw_indexed_primitives
(
    primitives_t type,
    uint32_t first_index,
    uint32_t num_primitives,
    uint32_t *fifo,
    uint32_t hw_sub
);

