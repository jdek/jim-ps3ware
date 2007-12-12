/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Vivien Chappelier <vivien.chappelier@free.fr>
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */

#include <linux/types.h>
#include <linux/fb.h>
#include <asm/ps3fb.h>

#define dbg(...)

#define DEV_MEM		"/dev/mem"
#define DEV_VFB		"/dev/fb0"
#define DEV_GPU_VRAM	"/dev/ps3gpu_vram"
#define DEV_GPU_FIFO	"/dev/ps3gpu_fifo"
#define DEV_GPU_CTRL	"/dev/ps3gpu_ctrl"

#define barrier() asm volatile ("" : : : "memory");
#define OUT_RING(data) *(ptr)++ = (data)
#define OUT_RINGf(data) { union f2i temp; temp.f = data;  *(ptr)++ = temp.i; }
#define BEGIN_RING(chan, tag, size)   OUT_RING(((size) << 18) | ((chan) << 13) | (tag))


union f2i
{
  float f;
  uint32_t i;
};


struct resource
{
  void *virt;
  size_t len;
};

struct gpu
{
  struct resource xram;
  struct resource vram;
  struct resource fifo;
  struct resource ctrl;
};


typedef enum
{
	Z16 = 0x20,
	Z24S8 = 0x40,
}
type_depth_t;

typedef struct
{
	uint8_t clearR;
	uint8_t clearG;
	uint8_t clearB;
	uint8_t clearA;
	uint8_t clearD;
	uint32_t rgba;
	uint32_t depth;
	
	
}
clear_buffer_t;



typedef struct
{
	uint16_t width;
	uint16_t height;
	uint32_t pitchColor;
	uint32_t pitchDepth;
	uint32_t offsetColor;
	uint32_t offsetDepth;
	type_depth_t typeDepth;
	
}
setup_buffer_t;

int setup_and_voodoo( uint32_t dma_notifier, uint32_t dma_fb, uint32_t obj_3d, uint32_t *fifo_buffer, uint32_t hw_sub );
int clear_buffers( const clear_buffer_t *buffers, uint32_t *fifo, uint32_t hw_sub );
int setup_buffers( const setup_buffer_t *buffers, uint32_t *fifo, uint32_t hw_sub );




int gpu_get_info(struct gpu *gpu);
int map_resource(char const *name, struct resource *res);
int unmap_resource(struct resource *res);
int map_gpu(struct gpu *gpu);
int unmap_gpu(struct gpu *gpu);
void fifo_push(struct gpu *gpu, int len);
void fifo_wait(struct gpu *gpu);
int enter_direct(struct ps3fb_ioctl_res *res);
int leave_direct(int fd);
uint32_t endian( uint32_t v );
uint32_t endian_fp( uint32_t v );
