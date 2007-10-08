/**
 * PS3 FrameBuffer library
 * -----------------------
 * Licensed under the BSD license, see LICENSE for details
 * 
 * libfb.h - Interface to the FrameBuffer Library
 *
 * Copyright (c) 2007 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2007 Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * $Id$
 */

#ifndef __LIBPS3FB_H
#define __LIBPS3FB_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Initialize the FrameBuffer library and disable the console
  *
  * @return address of first frame on success, NULL on failure
  */
void *ps3fb_init();

/**
  * Get the horizontal resolution of the FrameBuffer
  *
  * @return int horizontal resolution
  */
int ps3fb_getXres();

/**
  * Get the vertical resolution of the FrameBuffer
  *
  * @return int vertical resolution
  */
int ps3fb_getYres();

/**
  * Get the horizontal offset of the FrameBuffer
  *
  * @return int horizontal offset
  */
int ps3fb_getXoff();

/**
  * Get the vertical offset of the FrameBuffer
  *
  * @return int vertical offset
  */
int ps3fb_getYoff();

/**
  * Get the number of frames in the framebuffer
  *
  * @return int number of frame buffers
  */

int ps3fb_getnum_frames();

/**
  * Swap the FrameBuffer immediately
  *
  * @return address of the BackBuffer
  */
void *ps3fb_swap();

/**
  * Swap the FrameBuffer after waiting for Vsync
  *
  * @return address of the BackBuffer
  */
void *ps3fb_swapVsync();

/**
  * Cleanup FrameBuffer library variables and enable the console
  *
  * @return none
  */
void ps3fb_cleanup();

/**
  * Get a textual description of the error when an error condition occurs
  *
  * @return description of the error
  */
char *ps3fb_getError();

#ifdef __cplusplus
} // extern "C"
#endif

#endif

