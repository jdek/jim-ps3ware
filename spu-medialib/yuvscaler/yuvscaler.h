/**
 * SPU YUV scaler kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * yuvscaler.h - ppu header for the spe accellerated yuvscaler 
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 * $Id:
 */

// Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>

// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The names of the authors may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef __YUVSCALER_H
#define __YUVSCALER_H

#ifdef __cplusplus
extern "C" {
#endif



struct yuvscaler_s;
typedef struct yuvscaler_s yuvscaler_t;

typedef void * ea_t;

/**
* init_yuvscaler 
* initiates the spu yuvscaler.
*/ 
yuvscaler_t * sws_init_yuvscaler(int srcW, int srcH, int dstW, int dstH,
                                ea_t front_inBuffer, ea_t back_inBuffer,
                                ea_t front_outBuffer, ea_t back_outBuffer);

/**
*gets the spe context pointer from the initiated yuvscaler
*/
spe_context_ptr_t sws_getCTX(yuvscaler_t*);

/**
* recive_message waits for a interrupt message from the spu notifying a loop completion
*/
unsigned int sws_receive_message(yuvscaler_t*);


/**
* sends a message to the spu telling it what to do options are RDY UPDATE and STOP defined in spu_control.h
*/
void sws_send_message(yuvscaler_t*,unsigned int message);

/**
* destroys the spu scaler object and this object
*/
void sws_yuvscaler_destroy(yuvscaler_t*);

/**
* get functions for src and dst sizes
*/
unsigned int sws_get_dstW(const yuvscaler_t*);
unsigned int sws_get_srcW(const yuvscaler_t*);
unsigned int sws_get_dstH(const yuvscaler_t*);
unsigned int sws_get_srcH(const yuvscaler_t*);

/**
* set functions for src and dst sizes (only usable if followed by a update!)
*/
void sws_set_dstW(yuvscaler_t*,int dstw);
void sws_set_srcW(yuvscaler_t*,int srcw);
void sws_set_dstH(yuvscaler_t*,int dsth);
void sws_set_srcH(yuvscaler_t*,int srch);

#ifdef __cplusplus
}
#endif

#endif
