/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Vivien Chappelier <vivien.chappelier@free.fr>
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */

typedef enum 
{
DXT1=0x6,
DXT3=0x7,
DXT5=0x8
}
tex_format_t;

typedef struct 
{
uint32_t 	width;
uint32_t	height;
uint32_t 	mips;
tex_format_t   	format;
}
texture_desc_t;

uint32_t get_gpu_texture_size( const texture_desc_t *desc );
void convert_texture( const texture_desc_t *desc, const uint32_t *user_rgba, void *out );

typedef enum
{
REPEAT=0x1,
MIRRORED_REPEAT=0x2,
CLAMP_TO_EDGE=0x3,
CLAMP_TO_BORDER=0x4,
CLAMP=0x5,
MIRROR_CLAMP_TO_EDGE=0x6,
MIRROR_CLAMP_TO_BORDER=0x7,
MIRROR_CLAMP=0x8
}
tex_wrap_t;

typedef enum 
{
NEAREST=0x1,
LINEAR=0x2,
NEAREST_MIPMAP_NEAREST=0x3,
LINEAR_MIPMAP_NEAREST=0x4,
NEAREST_MIPMAP_LINEAR=0x5,
LINEAR_MIPMAP_LINEAR=0x6
}
tex_filter_t;

void SetTexture2D( uint32_t offset, const texture_desc_t *desc, tex_wrap_t wrap, tex_filter_t filter );
