/*
  PS3 RSX kernel module

  Copyright(c) 2007 Vivien Chappelier <vivien.chappelier@free.fr>

  This file is based on :

    linux/drivers/video/ps3fb.c -- PS3 RSX frame buffer device
  
  	Copyright (C) 2006 Sony Computer Entertainment Inc.
 	Copyright 2006, 2007 Sony Corporation

  This program is free software; you can redistribute it and/or modify it 
  under the terms of version 2 of the GNU General Public License as 
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but WITHOUT 
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
  The full GNU General Public License is included in this distribution in the
  file called COPYING.
  
*/

#include <linux/kernel.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>

#include <asm/abs_addr.h>
#include <asm/lv1call.h>

#include "ps3rsx.h"

#define DEVICE_NAME "ps3rsx"
#define DESCRIPTION "PS3 RSX access module"
#define VERSION "1.0.0"
#define AUTHOR "Copyright(c) 2007 Vivien Chappelier <vivien.chappelier@free.fr>"

MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION(VERSION);
MODULE_AUTHOR(AUTHOR);
MODULE_LICENSE("GPL");

#define dbg(fmt, ...) do {                                                    \
        if (debug)                                                            \
                printk(KERN_DEBUG DEVICE_NAME ": " fmt "\n", ## __VA_ARGS__); \
} while(0)
#define info(fmt, ...) \
	printk(KERN_INFO DEVICE_NAME ": " fmt "\n", ## __VA_ARGS__)
#define warn(fmt, ...) \
	printk(KERN_WARNING DEVICE_NAME ": " fmt "\n", ## __VA_ARGS__)
#define err(fmt, ...) \
	printk(KERN_ERR DEVICE_NAME ": " fmt "\n", ## __VA_ARGS__)

#define PS3FB_DEV                         0 /* /dev/fb0 */
#define GPU_DRV_INFO_SIZE	(128 * 1024)
#define GPU_CTRL_SIZE	          (4 * 1024)
#define GPU_FIFO_SIZE		 (64 * 1024)
#define GPU_XDR_PAGE_SIZE      (1024 * 1024) /* iomap must be aligned on 1MB */
#define GPU_IOIF	      (0x0d000000UL)

#define L1GPU_CONTEXT_ATTRIBUTE_FIFO_SETUP 0x001
#define L1GPU_CONTEXT_ATTRIBUTE_FB_BLIT 0x601
#define L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_FLIP 0x102


int debug;
module_param(debug, bool, 0);
MODULE_PARM_DESC(debug, "debug mode");

int major;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "device major");

#define DDR_SIZE (0x0fe00000)

struct display_head {
	u64 be_time_stamp;
	u32 status;
	u32 offset;
	u32 res1;
	u32 res2;
	u32 field;
	u32 reserved1;

	u64 res3;
	u32 raster;

	u64 vblank_count;
	u32 field_vsync;
	u32 reserved2;
};

struct rsx_irq {
	u32 irq_outlet;
	u32 status;
	u32 mask;
	u32 video_cause;
	u32 graph_cause;
	u32 user_cause;

	u32 res1;
	u64 res2;

	u32 reserved[4];
};

struct rsx_driver_info {
	u32 version_driver;
	u32 version_rsx;
	u32 memory_size;
	u32 hardware_channel;

	u32 nvcore_frequency;
	u32 memory_frequency;

	u32 reserved1[16];

	u32 dma_obj[256];
	u32 gfx_obj[256];
	u32 notify_obj[256];
	u32 unk_obj[256];

	u32 reserved2[23];
	struct display_head display_head[8];
	struct rsx_irq irq;
};

struct rsx_fifo_regs {
        u32 reserved1[16];
        u32 wptr;
        u32 rptr;
        u32 reserved2[3];
        u32 eptr;
};

struct rsx_notifier {
	u64 timestamp;
	u32 error;
	u32 status;
};

struct ps3rsx_context {
	u64 context_handle;	/* handle to the video context */

        u64 fifo_lpar;
        u64 fifo_size;
        u64 fifo_ioif;
        u64 ctrl_lpar;
        u64 ctrl_size;
        u64 info_lpar;
        u64 info_size;
        u64 reports_lpar;
        u64 reports_size;

	struct rsx_driver_info *drv_info;
        struct rsx_fifo_regs *ctrl;
	void *reports;
	void *fifo;

	int hw_id;          /* hardware context number */
};

struct ps3rsx {
	void *xdr;		/* XDR apperture to the RSX */
	u64 xdr_lpar;           /* physical address of XDR apperture */
	int xdr_size;		/* amount of ram visible by the RSX */
	u64 ioif_lpar;
	u32 ioif_addr;
	int ioif_size;
	atomic_t available;
	struct ps3rsx_context ucontext;
	struct ps3rsx_context kcontext;
	struct fb_info *fb_info;

	u64 vram_handle;	/* handle to the video memory */
        u64 vram_lpar;
        u64 vram_size;
	void *vram;		/* memory mapped video memory (slow access) */

	struct fb_info *ps3fb_info;
	struct rsx_fifo_regs *ps3fb_ctrl;
};

static struct ps3rsx ps3rsx;

struct ps3rsx_par {
        u32 pseudo_palette[16];
};

static struct fb_fix_screeninfo ps3rsx_fix __initdata = {
        .id =           DEVICE_NAME,
        .type =         FB_TYPE_PACKED_PIXELS,
        .visual =       FB_VISUAL_TRUECOLOR,
        .accel =        FB_ACCEL_NONE,
};

#define OP(subch, tag, size) (((size) << 18) | ((subch) << 13) | (tag))

/* program to bind HV objects to subchannels */
u32 fifo_setup_program[] = {
        OP(1, 0x000, 1), // bind to subchannel 1
        0x31337303,      // Memory to Memory instance
        OP(1, 0x180, 3), // 
        0x66604200,      // DMA notifier to reports + 0x1000
        0xfeed0001,      // DMA source from DMA system RAM instance
        0xfeed0000,      // DMA dest to DMA video RAM instance

        OP(2, 0x000, 1), // bind to subchannel 2
        0x3137c0de,      // Memory to Memory instance
        OP(2, 0x180, 3), // 
        0x66604200,      // DMA notifier to reports + 0x1000
        0xfeed0000,      // DMA source from DMA video RAM instance
        0xfeed0001,      // DMA dest to DMA system RAM instance

        OP(3, 0x000, 1), // bind to subchannel 3
        0x313371c3,      // 2D ContextSurface instance
        OP(3, 0x180, 3), // 
        0x66604200,      // DMA notifier to reports + 0x1000
        0xfeed0000,      // DMA source from DMA video RAM instance
        0xfeed0000,      // DMA dest to DMA video RAM instance

        OP(4, 0x000, 1), // bind to subchannel 4
        0x31337a73,      // Swizzled Surface instance
        OP(4, 0x180, 2), // 
        0x66604200,      // DMA notifier to reports + 0x1000
        0xfeed0000,      // DMA source from DMA video RAM instance

        OP(5, 0x000, 1), // bind to subchannel 5
        0x31337808,      // Image from CPU instance
        OP(5, 0x180, 8), //  
        0x66604200,      // DMA notifier to reports + 0x1000
        0x00000000,      // colorkey
        0x00000000,      // clip rectangle
        0x00000000,      // pattern
        0x00000000,      // ROP
        0x00000000,      // beta1
        0x00000000,      // beta4
        0x313371c3,      // surface
        OP(5, 0x2fc, 2), //
        0x00000003,      // operation srccopy
        0x00000004,      // color format A8R8G8B8

        OP(6, 0x000, 1), // bind to subchannel 6
        0x3137af00,      // Scaled Image instance
        OP(6, 0x180, 1), // 
        0x66604200,      // DMA notifier to reports + 0x1000

};

/* definitions for conversion from physical to logical partition address */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,16)
unsigned long ps3_mm_phys_to_lpar(unsigned long phys_addr);
#define p_to_lp ps3_mm_phys_to_lpar
#else
extern unsigned long p_to_lp(long pa);
#endif

static int ps3rsx_context_allocate(struct ps3rsx_context *context, int num);
static void ps3rsx_context_free(struct ps3rsx_context *context);

static int ps3rsx_fifo_wait(struct rsx_fifo_regs *regs, int timeout)
{
        /* wait until setup commands are processed */
	while (--timeout) {
		if (regs->wptr == regs->rptr && regs->wptr == regs->eptr)
			break;
		msleep(1);
	}
	if (timeout == 0) {
		err("FIFO timeout (%08x/%08x/%08x)",
		    regs->wptr, regs->rptr, regs->eptr);
		return -ETIMEDOUT;
	}

	return 0;
}

static int ps3rsx_exclusive(void)
{
	int status;

        /*
	 * Blit a large region to the screen.
	 * This will put commands in the ps3fb FIFO, and kick it.
	 * We should have enough time to tweak the end pointer while the RSX
	 * is processing the first commands, so that the last command is not
	 * executed.
	 */
        status = lv1_gpu_context_attribute(ps3rsx.ucontext.context_handle,
                                           L1GPU_CONTEXT_ATTRIBUTE_FB_BLIT,
                                           0, GPU_IOIF,
                                           (1ULL << 31) | (1280 << 16) | 1280,
                                           1280*4);

	if (status) {
                err("lv1_gpu_context_attribute FIFO_SETUP failed: %d", status);
		return -EBUSY;
	}

        /* go back two operations to skip the last command */
	ps3rsx.ps3fb_ctrl->wptr -= 8;

        /* wait for end of blit */
	if (ps3rsx_fifo_wait(ps3rsx.ps3fb_ctrl, 500) < 0) {
		err("timeout waiting for ps3fb FIFO");
		return -EBUSY;
	}

	return 0;
}

static int ps3rsx_kick_fifo(struct ps3rsx_context *context)
{
	int status;

	/* asking the HV for a blit will kick the fifo */
	status = lv1_gpu_context_attribute(context->context_handle,
                                           L1GPU_CONTEXT_ATTRIBUTE_FB_BLIT,
					   0, 0, 0, 0);
	if (status) {
                err("lv1_gpu_context_attribute FIFO_SETUP failed: %d", status);
		return -EBUSY;
	}

	return 0;
}

static int ps3rsx_context_allocate(struct ps3rsx_context *context, int num)
{
	int status;

	context->info_size = GPU_DRV_INFO_SIZE;
	context->ctrl_size = GPU_CTRL_SIZE;
	status = lv1_gpu_context_allocate(ps3rsx.vram_handle,
					  0,
					  &context->context_handle,
                                          &context->ctrl_lpar,
                                          &context->info_lpar,
                                          &context->reports_lpar,
                                          &context->reports_size);
	if (status) {
		err("lv1_gpu_context_allocate failed: %d\n", status);
		goto err;
	}

	info("context 0x%lx dma=%lx driver=%lx reports=%lx reports_size=%lx",
	     context->context_handle,
	     context->ctrl_lpar, context->info_lpar,
	     context->reports_lpar, context->reports_size);

	context->ctrl = ioremap(context->ctrl_lpar, context->ctrl_size);
	context->drv_info = (struct rsx_driver_info *)
		ioremap(context->info_lpar, context->info_size);
	context->reports  = ioremap(context->reports_lpar,
				    context->reports_size);

	info("ctrl=%p drv=%p reports=%p",
	     context->ctrl, context->drv_info, context->reports);

	info("version %x.%x RSX rev%d %dMB RAM channel %d core %dMHz mem %dMHz",
	     context->drv_info->version_driver >> 8,
	     context->drv_info->version_driver & 0xff,
	     context->drv_info->version_rsx,
	     context->drv_info->memory_size >> 20,
	     context->drv_info->hardware_channel,
	     context->drv_info->nvcore_frequency / 1000000,
	     context->drv_info->memory_frequency / 1000000);

	context->hw_id = context->drv_info->hardware_channel;

	/* map XDR apperture to GPU_IOIF */
	status = lv1_gpu_context_iomap(context->context_handle, GPU_IOIF,
                                       ps3rsx.xdr_lpar,
				       ps3rsx.xdr_size, 0);
        if (status) {
                err("lv1_gpu_context_iomap failed: %d", status);
                goto err;
        }

        info("remapped XDR apperture at %p size %dkB to RSX",
	     ps3rsx.xdr, ps3rsx.xdr_size / 1024);

	/* allocate room for the FIFO at end of apperture */
	context->fifo_lpar = ps3rsx.xdr_lpar +
		ps3rsx.xdr_size - (num + 1) * GPU_FIFO_SIZE;
	context->fifo_size = GPU_FIFO_SIZE;
	context->fifo_ioif = GPU_IOIF +
		ps3rsx.xdr_size - (num + 1) * GPU_FIFO_SIZE;
        context->fifo = ps3rsx.xdr +
		ps3rsx.xdr_size - (num + 1) * GPU_FIFO_SIZE;

	/* setup the FIFO */
        memcpy(context->fifo, fifo_setup_program, sizeof(fifo_setup_program));

	status = lv1_gpu_context_attribute(context->context_handle,
                                           L1GPU_CONTEXT_ATTRIBUTE_FIFO_SETUP,
					   context->fifo_ioif +  /* wptr */
					   sizeof(fifo_setup_program),
                                           context->fifo_ioif,   /* rptr */
					   0, 0);
	if (status) {
                err("lv1_gpu_context_attribute FIFO_SETUP failed: %d", status);
		goto err;
	}

	/* run the FIFO */
	if (ps3rsx_kick_fifo(context) < 0) {
		err("failed to run fifo");
		goto err;
	}

	/* wait for end of setup */
	if (ps3rsx_fifo_wait(context->ctrl, 100) < 0) {
		err("context setup timeout");
		goto err;
	}

	return 0;

err:
	ps3rsx_context_free(context);
	return -EBUSY;
}

static void ps3rsx_context_free(struct ps3rsx_context *context)
{
	if (context->ctrl) {
		iounmap(context->ctrl);
		context->ctrl = 0;
	}

	if (context->drv_info) {
		iounmap(context->drv_info);
		context->drv_info = 0;
	}

	if (context->reports) {
		iounmap(context->reports);
		context->reports = 0;
	}

	if (context->context_handle) {
		printk("free context\n");
		lv1_gpu_context_free(context->context_handle);
		context->context_handle = 0;
	}
}

static int ps3rsx_ioctl(struct fb_info *info,
			unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
        int retval = -EFAULT;
	struct ps3rsx_context *context;

	dbg("ps3rsx_ioctl (%08x)", cmd);

	context = &ps3rsx.ucontext;

        switch (cmd) {
	
	
	case PS3RSX_IOCTL_FLIP_SCRN:
		
	
		lv1_gpu_context_attribute( context->context_handle, L1GPU_CONTEXT_ATTRIBUTE_DISPLAY_FLIP, 0, arg, 0, 0 );
		break;
        

	case PS3RSX_IOCTL_CONTEXT_INFO:
               {
                       struct ps3rsx_ioctl_context_info res;

                       dbg("PS3RSX_IOCTL_CONTEXT_INFO");

		       res.hw_id     = context->hw_id;
                       res.vram_size = ps3rsx.vram_size;
		       res.ioif_size = ps3rsx.ioif_size;
                       res.fifo_size = context->fifo_size;
                       res.ctrl_size = context->ctrl_size;
                       res.info_size = context->info_size;
                       res.reports_size = context->reports_size;
		       res.ioif_addr = ps3rsx.ioif_addr;

                       if (!copy_to_user(argp, &res, sizeof(res)))
                               retval = 0;
                       break;
               }

	case PS3RSX_IOCTL_KICK_FIFO:
		dbg("PS3RSX_IOCTL_KICK_FIFO");
		retval = ps3rsx_kick_fifo(context);
		break;

	case PS3RSX_IOCTL_EXCLUSIVE:
		dbg("PS3RSX_IOCTL_EXCLUSIVE");
		retval = ps3rsx_exclusive();
		break;

        default:
                retval = -ENOIOCTLCMD;
                break;
	}

        return retval;
}

static int ps3rsx_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long vsize = vma->vm_end - vma->vm_start;
	unsigned long physical;
	unsigned long psize;
	struct ps3rsx_context *context;
	int segment;

	dbg("ps3rsx_mmap @ %08lx", off);

	context = &ps3rsx.ucontext;

	/* memory map:
	   0x00000000 - 0x0fdfffff : video RAM
	   0x0fe00000 - 0x0fe0ffff : reports area
	   0x10000000 - 0x1fffffff : system RAM apperture
	   0x20000000 - 0x2000ffff : FIFO
	   0x30000000 - 0x30000fff : FIFO registers
	   0x40000000 - 0x40007fff : driver info
	*/

	segment = (off >> 28) & 0xf;
	off &= (1UL << 28) - 1;

	switch (segment) {
	case 0:
		if (off < 0x0fe00000) {
			/* VRAM */
			physical = ps3rsx.vram_lpar + off;
			psize    = ps3rsx.vram_size - off;
		} else {
			off -= 0x0fe00000;
			physical = context->reports_lpar + off;
			psize    = context->reports_size - off;
		}
		break;
	case 1:  /* DDR */
		physical = ps3rsx.ioif_lpar + off;
		psize    = ps3rsx.ioif_size - off;
		break;
	case 2: /* FIFO */
		physical = context->fifo_lpar + off;
		psize    = context->fifo_size - off;
		break;
	case 3: /* FIFO registers */
		physical = context->ctrl_lpar + off;
		psize    = context->ctrl_size - off;
		break;
	case 4: /* driver info */
		physical = context->info_lpar + off;
		psize    = context->info_size - off;
		break;
	default:
		return -EINVAL;
	}

	if (vsize > psize)
		return -EINVAL; /* spans too high */

	if (remap_pfn_range(vma, vma->vm_start,
			    physical >> PAGE_SHIFT,
			    vsize, vma->vm_page_prot))
		return -EAGAIN;
 
	dbg("ps3rsx: mmap framebuffer P(%d/%lx)->V(%lx)", segment, off,
	    vma->vm_start);
	
	return 0;
}

static int ps3rsx_open(struct fb_info *info, int user)
{
	if (!atomic_dec_and_test(&ps3rsx.available)) {
		atomic_inc(&ps3rsx.available);
		return -EBUSY; /* already open */
	}

	/* create a context */
	if (ps3rsx_context_allocate(&ps3rsx.ucontext, 2) < 0) {
		err("failed to allocate kernel RSX context");
		return -ENODEV;
	}

        return 0;
}

static int ps3rsx_release(struct fb_info *info, int user)
{
	/* free context */
	ps3rsx_context_free(&ps3rsx.ucontext);

	atomic_inc(&ps3rsx.available);

        return 0;
}

static int ps3rsx_setcolreg(unsigned int regno, unsigned int red,
			    unsigned int green, unsigned int blue,
			    unsigned int transp, struct fb_info *info)
{
        if (regno >= 16)
                return 1;

        red >>= 8;
        green >>= 8;
        blue >>= 8;
        transp >>= 8;

        ((u32 *)info->pseudo_palette)[regno] = transp << 24 | red << 16 |
                                               green << 8 | blue;
        return 0;
}

static struct fb_ops ps3rsx_ops = {
        .fb_open        = ps3rsx_open,
        .fb_release     = ps3rsx_release,
        .fb_setcolreg   = ps3rsx_setcolreg,
        .fb_mmap        = ps3rsx_mmap,
        .fb_ioctl        = ps3rsx_ioctl,
        .fb_compat_ioctl = ps3rsx_ioctl
};

static void cleanup_ps3rsx(void)
{
	/* free context */
	ps3rsx_context_free(&ps3rsx.kcontext);

	/* unmap video ram */
	if (ps3rsx.vram) {
		iounmap(ps3rsx.vram);
		ps3rsx.vram = 0;
	}

	/* free memory handle */
	if (ps3rsx.vram_handle) {
		printk("free memory\n");
		lv1_gpu_memory_free(ps3rsx.vram_handle);
		ps3rsx.vram_handle = 0;
	}

	if (ps3rsx.ps3fb_ctrl)
		iounmap(ps3rsx.ps3fb_ctrl);

        unregister_framebuffer(ps3rsx.fb_info);
        fb_dealloc_cmap(&ps3rsx.fb_info->cmap);
        framebuffer_release(ps3rsx.fb_info);
}

static int init_ps3rsx(void)
{
        struct ps3rsx_par *par;
	int status;
	struct fb_info *info;

	/*
	 * Retrieve info from ps3fb. Here we assume ps3fb is present and is
	 * the first registered framebuffer.
	 */
	if (num_registered_fb < 1) {
		err("could not find ps3fb");
		return -ENODEV;
	}

	info = ps3rsx.ps3fb_info = registered_fb[PS3FB_DEV];

	/*
	 * The apperture is 1MB aligned. Since ps3fb offset for non-fullscreen
	 * mode does not exceed 512kB in any video mode, this should always
	 * give the base pointer to XDR memory.
	 */
        ps3rsx.xdr = (void *) _ALIGN_DOWN((unsigned long) info->screen_base,
					  (1 << 20));
	ps3rsx.xdr_lpar = p_to_lp(__pa(ps3rsx.xdr));
	ps3rsx.xdr_size = _ALIGN_UP(info->fix.smem_len, (1 << 20));

	/*
	 * Reserve room for the FIFOs. Depending on ps3fb implementation,
	 * the FIFO for ps3fb context is either at the beginning or the end
	 * of the XDR apperture. We play safe by reserving both areas, and
	 * reserving additional space for our own FIFOs.
	 */
	ps3rsx.ioif_lpar = ps3rsx.xdr_lpar + GPU_FIFO_SIZE;
	ps3rsx.ioif_size = ps3rsx.xdr_size - 9 * GPU_FIFO_SIZE;
	ps3rsx.ioif_addr = GPU_IOIF + GPU_FIFO_SIZE;

	info("reserved XDR memory is @%p, len %d",
	     ps3rsx.xdr, ps3rsx.xdr_size);

	atomic_set(&ps3rsx.available, 1);

	/*
	 * Set memory limits. Setting the first parameter to zero provides
	 * access to VRAM above 254MB though DMA. This region contains the
	 * instance RAM (RAMIN) which holds graphics and DMA objects.
	 */
	status = lv1_gpu_memory_allocate(0, 0, 0, 0, 0,
					 &ps3rsx.vram_handle,
					 &ps3rsx.vram_lpar);
        if (status) {
                err("lv1_gpu_memory_allocate failed: %d", status);
		goto err;
        }

	ps3rsx.vram_size = DDR_SIZE;
	ps3rsx.vram = ioremap(ps3rsx.vram_lpar, ps3rsx.vram_size);

	info("%ldMB of DDR video ram at 0x%lx mapped at %p handle %lx",
	     ps3rsx.vram_size >> 20, ps3rsx.vram_lpar, ps3rsx.vram,
	     ps3rsx.vram_handle);

	/* Create a context */
	if (ps3rsx_context_allocate(&ps3rsx.kcontext, 1) < 0) {
		err("failed to allocate kernel RSX context");
		goto err;
	}

	/*
	 * Get the ps3fb context fifo control registers.
	 * We need them to perform the 'FIFO workaround', i.e. getting control
	 * of ps3fb FIFOs while it is used for blitting a large portion of XDR.
	 *
	 * This assumes the control registers lpar are given sequentially,
	 * which is the case with current HV version.
	 */
	ps3rsx.ps3fb_ctrl = ioremap(ps3rsx.kcontext.ctrl_lpar - GPU_CTRL_SIZE,
				    ps3rsx.kcontext.ctrl_size);

	info = framebuffer_alloc(sizeof(struct ps3rsx_par), NULL);
        if (!info)
                goto err;

	par = info->par;

        info->screen_base = (char __iomem *) ps3rsx.vram;
        info->fbops = &ps3rsx_ops;

	info->fix = ps3rsx_fix;
        info->fix.smem_start = virt_to_abs(ps3rsx.vram);
        info->fix.smem_len = ps3rsx.vram_size;
	info->pseudo_palette = par->pseudo_palette;
	info->flags = FBINFO_DEFAULT;

	status = fb_alloc_cmap(&info->cmap, 256, 0);
        if (status < 0)
                goto err_framebuffer_release;

	status = register_framebuffer(info);
        if (status < 0)
                goto err_fb_dealloc;

	ps3rsx.fb_info = info;

	return 0;

err_fb_dealloc:
        fb_dealloc_cmap(&info->cmap);
err_framebuffer_release:
        framebuffer_release(info);
err:
	cleanup_ps3rsx();
	return -EBUSY;
}

static int __init _ps3rsx_init(void)
{
	info(DESCRIPTION ", " VERSION);
	return init_ps3rsx();
}

static void __exit _ps3rsx_exit(void)
{
	dbg("unloading ps3rsx");
	cleanup_ps3rsx();
}

module_init(_ps3rsx_init);
module_exit(_ps3rsx_exit);
