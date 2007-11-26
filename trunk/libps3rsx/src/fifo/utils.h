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
