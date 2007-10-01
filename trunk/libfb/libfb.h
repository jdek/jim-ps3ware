/**
 * PS3 FrameBuffer library
 * -----------------------
 * Licensed under the BSD license, see LICENSE for details
 * 
 * libfb.h - Interface to the FrameBuffer Library
 *
 * Copyright (c) 2007 John Kelley <ps2dev@kelley.ca>
 *
 * $Id$
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
  * Initialize the FrameBuffer library and disable the console
  *
  * @return address of first frame on success, NULL on failure
  */
void *fb_init();

/**
  * Get the horizontal resolution of the FrameBuffer
  *
  * @return int horizontal resolution
  */
int fb_getXres();

/**
  * Get the vertical resolution of the FrameBuffer
  *
  * @return int vertical resolution
  */
int fb_getYres();

/**
  * Swap the FrameBuffer immediately
  *
  * @return address of the BackBuffer
  */
void *fb_swap();

/**
  * Swap the FrameBuffer after waiting for Vsync
  *
  * @return address of the BackBuffer
  */
void *fb_swapVsync();

/**
  * Cleanup FrameBuffer library variables and enable the console
  *
  * @return none
  */
void fb_cleanup();

/**
  * Get a textual description of the error when an error condition occurs
  *
  * @return description of the error
  */
char *fb_getError();
#ifdef __cplusplus
} // extern "C"
#endif


