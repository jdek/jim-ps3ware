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
#define DEV_RSX		"/dev/fb1"
#define OFFSET_VRAM	0x00000000
#define OFFSET_IOIF	0x10000000
#define OFFSET_FIFO	0x20000000
#define OFFSET_CTRL	0x30000000
#define OFFSET_REPORTS	0x0fe00000

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
  int init;
  int fb_fd;
  int rsx_fd;
  int hw_id;
  uint32_t ioif;
  struct ps3fb_ioctl_res res;
  struct resource xram;
  struct resource vram;
  struct resource fifo;
  struct resource ctrl;
  struct resource reports;
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




void fifo_push(struct gpu *gpu, int len);
void fifo_wait(struct gpu *gpu);
int sync_gpu(struct gpu *gpu );
uint32_t hash_handle(int channel, uint32_t handle);
uint32_t endian( uint32_t v );
uint32_t endian_fp( uint32_t v );
int gpu_init(struct gpu *gpu);
void gpu_cleanup(struct gpu *gpu);
