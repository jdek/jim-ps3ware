/*
 * Copyright (C) 2006  Sony Computer Entertainment Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _PS3FB_H_
#define _PS3FB_H_

/* ioctl */
#define PS3FB_IOCTL_SETMODE       _IOW('r',  1, int) /* set video mode */
#define PS3FB_IOCTL_GETMODE       _IOR('r',  2, int) /* get video mode */
#define PS3FB_IOCTL_SCREENINFO    _IOR('r',  3, int) /* get screen info */
#define PS3FB_IOCTL_ON            _IO('r', 4)        /* use IOCTL_FSEL */
#define PS3FB_IOCTL_OFF           _IO('r', 5)        /* return to normal-flip */
#define PS3FB_IOCTL_FSEL          _IOW('r', 6, int)  /* blit and flip request */ 

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC         _IOW('F', 0x20, uint32_t) /* wait for vsync */
#endif

struct ps3fb_ioctl_res {
	uint32_t xres; /* frame buffer x_size */
	uint32_t yres; /* frame buffer y_size */
	uint32_t xoff; /* margine x  */
	uint32_t yoff; /* margine y */
	uint32_t num_frames; /* num of frame buffers */
};

#endif /* _PS3FB_H_ */
