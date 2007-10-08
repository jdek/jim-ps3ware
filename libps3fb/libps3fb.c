/**
 * PS3 FrameBuffer library
 * -----------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * fb.c - FrameBuffer Library
 *
 * Copyright (c) 2007 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2007 Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * $Id$
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ps3fb.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

struct fb_vblank vblank;
struct ps3fb_ioctl_res res;
int fb, length, frame;
void *fbAddr;
uint32_t crt;
char errorStr[2048];

#define FALSE	0
#define TRUE	1
static inline void *fberror(char *msg) {
	sprintf(errorStr, msg); 
	close(fb);
	return NULL;
}

char *fb_getError() {
	return errorStr;
}

static inline void enableCursor(int enable) {
	int fd = open("/dev/console", O_NONBLOCK);
	if (fd >= 0) {
		ioctl(fd, KDSETMODE, enable ? KD_TEXT : KD_GRAPHICS);
		close(fd);
	}
}

void *ps3fb_init() {
	fbAddr=(void *)0xDEADBEEF;
	sprintf(errorStr, "No Error");
	fb = open("/dev/fb0", O_RDWR);
	if (fb < 0)
		return fberror("Could not open /dev/fb0");
	if (ioctl(fb, FBIOGET_VBLANK, (unsigned long)&vblank) < 0)
		fberror("FBIOGET_VBLANK call failed");
	if (!(vblank.flags & FB_VBLANK_HAVE_VSYNC))
		return fberror("NO VSYNC");
	if (ioctl(fb, PS3FB_IOCTL_SCREENINFO, (unsigned long)&res) < 0)
		return fberror("Could not get ScreenInfo");
	// length in bytes of all frameBuffer memory
	length = res.xres * res.yres * 4 * res.num_frames;
	fbAddr = mmap(NULL, length, PROT_WRITE, MAP_SHARED, fb, 0);
	if (!fbAddr)
		return fberror("Failed to mmap framebuffer");
	// clear both FBs
        memset(fbAddr, 0x00, length);
        // disable kernel vsync
        ioctl(fb, PS3FB_IOCTL_ON, 0);
	enableCursor(0);
	frame = 0;
	sprintf(errorStr, "we made it!!! %p\n", fbAddr);
	return fbAddr;
}

void *ps3fb_swap() {
	ioctl(fb, PS3FB_IOCTL_FSEL, (unsigned long)&frame);
	frame = !frame;
	return fbAddr + ((res.xres * res.yres * 4) * (frame)); // should this be !frame?
}

void *ps3fb_swapVsync() {
	ioctl(fb, FBIO_WAITFORVSYNC, (unsigned long)&crt);
	ioctl(fb, PS3FB_IOCTL_FSEL, (unsigned long)&frame);
	frame = !frame;
	return fbAddr + ((res.xres * res.yres * 4) * (frame)); // should this be !frame?	
}

int ps3fb_getXres() {
	return res.xres;
}

int ps3fb_getYres() {
	return res.yres;
}

int ps3fb_getXoff() {
	return res.xoff;
}

int ps3fb_getYoff() {
	return res.yoff;
}

int ps3fb_getnum_frames() {
	return res.num_frames;
}

void ps3fb_cleanup() {
	//enable kernel vsync
	ioctl(fb, PS3FB_IOCTL_OFF, 0);
	munmap(NULL, length);
	close(fb);
	enableCursor(1);
}
