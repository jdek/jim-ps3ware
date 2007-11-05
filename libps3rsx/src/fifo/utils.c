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

#include <linux/types.h>
#include <linux/fb.h>
#include <asm/ps3fb.h>
#include "utils.h"

uint32_t endian( uint32_t v )
{
  return ( ( ( v >> 24 ) & 0xff ) << 0 ) |
         ( ( ( v >> 16 ) & 0xff ) << 8 ) |
         ( ( ( v >> 8 ) & 0xff ) << 16 ) |
         ( ( ( v >> 0 ) & 0xff ) << 24 );
}

uint32_t endian_fp( uint32_t v )
{
  return ( ( ( v >> 16 ) & 0xffff ) << 0 ) |
         ( ( ( v >> 0 ) & 0xffff ) << 16 );

}

int gpu_get_info(struct gpu *gpu)
{
  struct ps3fb_ioctl_gpu_info info;
  int ret = -1;
  int fd;


  if ((fd = open(DEV_VFB, O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  if ((ret = ioctl(fd, PS3FB_IOCTL_GPU_INFO, &info)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  printf("vram %d fifo %d ctrl %d\n",
         info.vram_size, info.fifo_size, info.ctrl_size);

  gpu->xram.len = 16 * 1024 * 1024;
  gpu->vram.len = info.vram_size;
  gpu->fifo.len = info.fifo_size;
  gpu->ctrl.len = info.ctrl_size;

  ret = 0;
out:
  close(fd);

  return ret;
}

int map_resource(char const *name, struct resource *res)
{
  void *virt;
  int fd;

  if ((fd = open(name, O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  // TEMP
  printf("mmap: %s len %d\n", name, res->len);

  virt = mmap(0, res->len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (virt == MAP_FAILED)
    return -1;

  res->virt = virt;

  close(fd);

  return 0;
}

int unmap_resource(struct resource *res)
{
  munmap(res->virt, res->len);
  res->virt = NULL;

  return 0;
}

int map_gpu(struct gpu *gpu)
{

  
  if (map_resource(DEV_VFB, &gpu->xram) < 0)
  {
    fprintf(stderr, "failed to map vram\n");
    return -1;
  }

  if (map_resource(DEV_GPU_VRAM, &gpu->vram) < 0)
  {
    fprintf(stderr, "failed to map vram\n");
    return -1;
  }

  if (map_resource(DEV_GPU_FIFO, &gpu->fifo) < 0)
  {
    fprintf(stderr, "failed to map fifo\n");
    goto err_unmap_vram;
  }

  if (map_resource(DEV_GPU_CTRL, &gpu->ctrl) < 0)
  {
    fprintf(stderr, "failed to map ctrl\n");
    goto err_unmap_fifo;
  }

  return 0;

err_unmap_fifo:
  unmap_resource(&gpu->fifo);
err_unmap_vram:
  unmap_resource(&gpu->vram);
  return -1;
}

int unmap_gpu(struct gpu *gpu)
{
  unmap_resource(&gpu->fifo);

  return 0;
}

void fifo_push(struct gpu *gpu, int len)
{
  uint32_t *ctrl = gpu->ctrl.virt;

  ctrl[0x10] += 4 * len;
}

void fifo_wait(struct gpu *gpu)
{
  volatile uint32_t *ctrl = gpu->ctrl.virt;

  while (ctrl[0x10] != ctrl[0x11]);
}




int enter_direct(void)
{
  struct fb_fix_screeninfo fix;
  int ret = 0;
  int fd;
  int val = 0;

  if ((fd = open("/dev/fb0", O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  /* get framebuffer size */
  if ((ret = ioctl(fd, FBIOGET_FSCREENINFO, &fix)) < 0)
  {
    perror("ioctl");
    goto out;
  }


  /* stop that incessant blitting! */
  if ((ret = ioctl(fd, PS3FB_IOCTL_ON, 0)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  /* wait for vsync */
  if ((ret = ioctl(fd, FBIO_WAITFORVSYNC, &val)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  /* wait for vsync */
  if ((ret = ioctl(fd, PS3FB_IOCTL_GPU_SETUP, &val)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  /* keep open */
  return fd;

out:
  close(fd);

  return ret;
}

int leave_direct(int fd)
{
  int ret = 0;

  if ((ret = ioctl(fd, PS3FB_IOCTL_OFF, 0)) < 0)
  {
    perror("ioctl");
    goto out;
  }

out:
  close(fd);

  return ret;
}

