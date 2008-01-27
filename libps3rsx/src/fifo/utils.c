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
#include <errno.h>

#include <linux/types.h>
#include <linux/fb.h>
#include <asm/ps3fb.h>
#include "utils.h"
#include "ioctl.h"

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
	OUT_RING  (1);
	
	BEGIN_RING(Nv3D, NV40TCL_ALPHA_TEST_FUNC, 1);
	OUT_RING  (NV40TCL_ALPHA_TEST_FUNC_GREATER);
	
	BEGIN_RING(Nv3D, NV40TCL_ALPHA_TEST_REF, 1);
	OUT_RING  (0x128);
	
	
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

static int map_resource(int fd, off_t offset, struct resource *res)
{
	void *virt;

	virt = mmap(0, res->len, PROT_READ | PROT_WRITE,
		    MAP_SHARED, fd, offset);

	if (virt == MAP_FAILED)
		return -1;

	res->virt = virt;

	return 0;
}

int unmap_resource(struct resource *res)
{
	munmap(res->virt, res->len);
	res->virt = NULL;

	return 0;
}

int unmap_gpu(struct gpu *gpu)
{
	unmap_resource(&gpu->ctrl);
	unmap_resource(&gpu->fifo);
	unmap_resource(&gpu->vram);

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

#define RAMHT_BITS 11

uint32_t hash_handle(int channel, uint32_t handle)
{
	uint32_t hash = 0;
	int i;

//	printf("ch%d handle=0x%08x\n", channel, handle);
	
	for (i = 32; i > 0; i -= RAMHT_BITS) {
		hash ^= (handle & ((1 << RAMHT_BITS) - 1));
		handle >>= RAMHT_BITS;
	}
	
	hash ^= channel << (RAMHT_BITS - 4);
	hash <<= 3;

//	printf("hash = %08x\n", hash);

	return hash;
}

int sync_gpu(struct gpu *gpu)
{
    int val;
    ioctl(gpu->fb_fd, FBIO_WAITFORVSYNC, &val);
    return val;
}


int gpu_init(struct gpu *gpu)
{
	struct ps3rsx_ioctl_context_info info;
	int ret = 0;
	int fb_fd, rsx_fd;
	int val = 0;

	memset(gpu, 0, sizeof(gpu));

	if ((fb_fd = open(DEV_VFB, O_RDWR)) < 0)
	{
		perror("open");
		return -1;
	}

	/* get the screen res info */
	if ((ret = ioctl(fb_fd, PS3FB_IOCTL_SCREENINFO, &gpu->res)) < 0)
	{
		perror("ioctl");
		goto err_close_fb;
	}

	/* stop that incessant blitting! */
	if ((ret = ioctl(fb_fd, PS3FB_IOCTL_ON, 0)) < 0)
	{
		perror("ioctl");
		goto err_close_fb;
	}

	/* wait for vsync */
	if ((ret = ioctl(fb_fd, FBIO_WAITFORVSYNC, &val)) < 0)
	{
		perror("ioctl");
		goto err_leave_direct;
	}

	/* open gpu */
	if ((ret = open(DEV_RSX, O_RDWR)) < 0)
	{
		fprintf(stderr, "failed to open RSX device: %s\n", strerror(errno));
		goto err_leave_direct;
	}
  
	rsx_fd = ret;

	/* get context info */
	if ((ret = ioctl(rsx_fd, PS3RSX_IOCTL_CONTEXT_INFO, &info)) < 0)
	{
		perror("ioctl");
		goto err_close_rsx;
	}

/*
	printf("id %d, ioif %d at 0x%08x, vram %d, fifo %d, ctrl %d, reports %d\n",
	       info.hw_id, info.ioif_size, info.ioif_addr,
	       info.vram_size, info.fifo_size, info.ctrl_size, info.reports_size);
*/

	gpu->hw_id = info.hw_id;
	gpu->ioif  = info.ioif_addr;
	gpu->xram.len = info.ioif_size;
	gpu->vram.len = info.vram_size;
	gpu->fifo.len = info.fifo_size;
	gpu->ctrl.len = info.ctrl_size;
	gpu->reports.len = info.reports_size;

	/* map XDR apperture */
	if ((ret = map_resource(rsx_fd, OFFSET_IOIF, &gpu->xram)) < 0)
	{
		fprintf(stderr, "failed to map vram\n");
		goto err_close_rsx;
	}

	/* map video RAM */
	if ((ret = map_resource(rsx_fd, OFFSET_VRAM, &gpu->vram)) < 0)
	{
		fprintf(stderr, "failed to map vram\n");
		goto err_unmap_ioif;
	}

	/* map FIFO */
	if ((ret = map_resource(rsx_fd, OFFSET_FIFO, &gpu->fifo)) < 0)
	{
		fprintf(stderr, "failed to map fifo\n");
		goto err_unmap_vram;
	}

	/* map FIFO control registers */
	if ((ret = map_resource(rsx_fd, OFFSET_CTRL, &gpu->ctrl)) < 0)
	{
		fprintf(stderr, "failed to map ctrl\n");
		goto err_unmap_fifo;
	}

	/* map DMA notify region */
	if ((ret = map_resource(rsx_fd, OFFSET_REPORTS, &gpu->reports)) < 0)
	{
		fprintf(stderr, "failed to map ctrl\n");
		goto err_unmap_ctrl;
	}

	/* enter exclusive mode */
	if ((ret = ioctl(rsx_fd, PS3RSX_IOCTL_EXCLUSIVE, NULL)) < 0)
	{
		perror("ioctl");
		goto err_unmap_reports;
	}

	gpu->fb_fd = fb_fd;
	gpu->rsx_fd = rsx_fd;
	gpu->init = 1;

	return 0;

err_unmap_reports:
	unmap_resource(&gpu->reports);
err_unmap_ctrl:
	unmap_resource(&gpu->ctrl);
err_unmap_fifo:
	unmap_resource(&gpu->fifo);
err_unmap_vram:
	unmap_resource(&gpu->vram);
err_unmap_ioif:
	unmap_resource(&gpu->xram);
err_close_rsx:
	close(rsx_fd);
err_leave_direct:
	ioctl(fb_fd, PS3FB_IOCTL_OFF, 0);
err_close_fb:
	close(fb_fd);

	return ret;
}

void gpu_cleanup(struct gpu *gpu)
{
	int rsx_fd, fb_fd;

	fb_fd = gpu->fb_fd;
	rsx_fd = gpu->rsx_fd;

	unmap_resource(&gpu->reports);
	unmap_resource(&gpu->ctrl);
	unmap_resource(&gpu->fifo);
	unmap_resource(&gpu->vram);
	unmap_resource(&gpu->xram);
	close(rsx_fd);
	ioctl(fb_fd, PS3FB_IOCTL_OFF, 0);
	close(fb_fd);

	gpu->init = 0;
}
