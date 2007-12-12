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

#include "../../src/types.h"
#include "../../include/nouveau_class.h"


int setup_and_voodoo
(
    uint32_t dma_notifier,
    uint32_t dma_fb,
    uint32_t obj_3d,
    uint32_t *fifo,
    uint32_t hw_sub
)
{
	uint32_t i;
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	BEGIN_RING(Nv3D, 0, 1);
	OUT_RING  (obj_3d);

	BEGIN_RING(Nv3D, NV40TCL_DMA_NOTIFY, 1);
	OUT_RING  (dma_notifier);
	BEGIN_RING(Nv3D, NV40TCL_DMA_TEXTURE0, 1);
	OUT_RING  (dma_fb);

	BEGIN_RING(Nv3D, NV40TCL_DMA_COLOR0, 2);
	OUT_RING  (dma_fb);
	OUT_RING  (dma_fb);

	BEGIN_RING(Nv3D, NV40TCL_DMA_ZETA, 1 );
	OUT_RING  (dma_fb);


	/* voodoo */
	BEGIN_RING(Nv3D, 0x1ea4, 3);
	OUT_RING  (0x00000010);
	OUT_RING  (0x01000100);
	OUT_RING  (0xff800006);
	BEGIN_RING(Nv3D, 0x1fc4, 1);
	OUT_RING  (0x06144321);
	BEGIN_RING(Nv3D, 0x1fc8, 2);
	OUT_RING  (0xedcba987);
	OUT_RING  (0x00000021);
	BEGIN_RING(Nv3D, 0x1fd0, 1);
	OUT_RING  (0x00171615);
	BEGIN_RING(Nv3D, 0x1fd4, 1);
	OUT_RING  (0x001b1a19);
	BEGIN_RING(Nv3D, 0x1ef8, 1);
	OUT_RING  (0x0020ffff);
	BEGIN_RING(Nv3D, 0x1d64, 1);
	OUT_RING  (0x00d30000);
	BEGIN_RING(Nv3D, 0x1e94, 1);
	OUT_RING  (0x00000001);


	/* default 3D state */
	BEGIN_RING(Nv3D, NV40TCL_STENCIL_FRONT_ENABLE, 1);
	OUT_RING  (0);
	BEGIN_RING(Nv3D, NV40TCL_STENCIL_BACK_ENABLE, 1);
	OUT_RING  (0);
	BEGIN_RING(Nv3D, NV40TCL_ALPHA_TEST_ENABLE, 1);
	OUT_RING  (0);
	BEGIN_RING(Nv3D, NV40TCL_DEPTH_WRITE_ENABLE, 1);
	OUT_RING  (1);
	BEGIN_RING(Nv3D, NV40TCL_DEPTH_TEST_ENABLE, 1);
	OUT_RING  (1);
	BEGIN_RING(Nv3D, NV40TCL_DEPTH_FUNC, 1);
	OUT_RING  (NV40TCL_DEPTH_FUNC_LESS);
	BEGIN_RING(Nv3D, NV40TCL_COLOR_MASK, 1);
	OUT_RING  (0x01010101); /* TR,TR,TR,TR */
	BEGIN_RING(Nv3D, NV40TCL_CULL_FACE_ENABLE, 1);
	OUT_RING  (0);
	BEGIN_RING(Nv3D, NV40TCL_BLEND_ENABLE, 1);
	OUT_RING  (0);
	BEGIN_RING(Nv3D, NV40TCL_COLOR_LOGIC_OP_ENABLE, 2);
	OUT_RING  (0);
	OUT_RING  (NV40TCL_COLOR_LOGIC_OP_COPY);
	BEGIN_RING(Nv3D, NV40TCL_DITHER_ENABLE, 1);
	OUT_RING  (0);
	BEGIN_RING(Nv3D, NV40TCL_SHADE_MODEL, 1);
	OUT_RING  (NV40TCL_SHADE_MODEL_SMOOTH);
	BEGIN_RING(Nv3D, NV40TCL_POLYGON_OFFSET_FACTOR,2);
	OUT_RINGf (0.0);
	OUT_RINGf (0.0);
	BEGIN_RING(Nv3D, NV40TCL_POLYGON_MODE_FRONT, 2);
	OUT_RING  (NV40TCL_POLYGON_MODE_FRONT_FILL);
	OUT_RING  (NV40TCL_POLYGON_MODE_BACK_FILL);
	BEGIN_RING(Nv3D, NV40TCL_POLYGON_STIPPLE_PATTERN(0), 0x20);
	for (i=0;i<0x20;i++)
		OUT_RING  (0xFFFFFFFF);
	for (i=0;i<16;i++)
	{
		BEGIN_RING(Nv3D, NV40TCL_TEX_ENABLE(i), 1);
		OUT_RING  (0);
	}

	BEGIN_RING(Nv3D, 0x1d78, 1);
	OUT_RING  (0x110);
	return ptr - fifo;
}

int setup_buffers(
    const setup_buffer_t *buffers,
    uint32_t *fifo,
    uint32_t hw_sub )
{

	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;
	uint16_t width = buffers->width;
	uint16_t height = buffers->height;

	printf( "width = %d height = %d \n", (uint32_t)width, (uint32_t)height );

	BEGIN_RING(Nv3D, NV40TCL_VIEWPORT_TRANSLATE_X, 8);
	OUT_RINGf (0.5f * width );
	OUT_RINGf (0.5f * height );
	OUT_RINGf (0.0);
	OUT_RINGf (0.0);
	OUT_RINGf (0.5f * width );
	OUT_RINGf (0.5f * height );
	OUT_RINGf (1.0);
	OUT_RINGf (1.0);

	BEGIN_RING(Nv3D, NV40TCL_RT_ENABLE, 1);
	OUT_RING  (NV40TCL_RT_ENABLE_COLOR0);

	BEGIN_RING(Nv3D, NV40TCL_RT_HORIZ, 2);
	OUT_RING  ((width << 16));
	OUT_RING  ((height << 16));
	BEGIN_RING(Nv3D, NV40TCL_SCISSOR_HORIZ, 2);
	OUT_RING  ((width << 16));
	OUT_RING  ((height << 16));
	BEGIN_RING(Nv3D, NV40TCL_VIEWPORT_HORIZ, 2);
	OUT_RING  ((width << 16));
	OUT_RING  ((height << 16));
	BEGIN_RING(Nv3D, NV40TCL_VIEWPORT_CLIP_HORIZ(0), 2);
	OUT_RING  ((width << 16));
	OUT_RING  ((height << 16));


	BEGIN_RING(Nv3D, NV40TCL_ZETA_OFFSET, 1 );
	OUT_RING( buffers->offsetDepth );
	BEGIN_RING(Nv3D, NV40TCL_ZETA_PITCH, 1 );
	OUT_RING( buffers->pitchDepth );



	BEGIN_RING(Nv3D, NV40TCL_RT_FORMAT, 3);
	OUT_RING  (NV40TCL_RT_FORMAT_TYPE_LINEAR |
	           buffers->typeDepth |
	           NV40TCL_RT_FORMAT_COLOR_A8R8G8B8);
	OUT_RING  ( buffers->pitchColor );
	OUT_RING  ( buffers->offsetColor );

	return ptr - fifo;
}

int clear_buffers(
    const clear_buffer_t *buffers,
    uint32_t *fifo,
    uint32_t hw_sub )
{
	uint32_t *ptr = fifo;
	uint32_t Nv3D = hw_sub;

	BEGIN_RING(Nv3D, NV40TCL_CLEAR_VALUE_COLOR, 1 );
	OUT_RING( buffers->rgba );

	BEGIN_RING(Nv3D, NV40TCL_CLEAR_VALUE_DEPTH, 1 );
	OUT_RING( buffers->depth );


	BEGIN_RING(Nv3D,NV40TCL_CLEAR_BUFFERS,1 );
	OUT_RING
	(
	    ( buffers->clearB ? NV40TCL_CLEAR_BUFFERS_COLOR_B : 0 ) |
	    ( buffers->clearG ? NV40TCL_CLEAR_BUFFERS_COLOR_G : 0 ) |
	    ( buffers->clearR ? NV40TCL_CLEAR_BUFFERS_COLOR_R : 0 ) |
	    ( buffers->clearA ? NV40TCL_CLEAR_BUFFERS_COLOR_A : 0 ) |
	    ( buffers->clearD ? NV40TCL_CLEAR_BUFFERS_STENCIL : 0 ) |
	    ( buffers->clearD ? NV40TCL_CLEAR_BUFFERS_DEPTH : 0 )
	);

	return ptr - fifo;
}


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
	printf("mmap: %s len %d\n", name, (uint32_t)res->len);

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




int enter_direct(struct ps3fb_ioctl_res * res)
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

	/* get the screen res info */
	if( res )
	{
		if ((ret = ioctl(fd, PS3FB_IOCTL_SCREENINFO, res)) < 0)
		{
			perror("ioctl");
			goto out;
		}
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

