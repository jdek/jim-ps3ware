/**
 * SPU Rectangle Draw kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * draw_rect.h - ppu header for the spe accellerated rectangle draw 
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

#ifndef __DRAW_RECT_H
#define __DRAW_RECT_H

#include <stdint.h>
#include <libspe2.h>
#include <spu-medialib/data_2d.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct data_2 data_2_t;

struct draw_rect_s;
typedef struct draw_rect_s draw_rect_t;

typedef uint64_t ea_t;


draw_rect_t* init_draw_rect();


void prepare_solid(draw_rect_t *, ea_t outpoiter,int pitchOut,int bppO,int alu, Spu_Pixel planemask, Spu_Pixel Fg); 

void solid(draw_rect_t *, ea_t outpointer,int x1, int x2 , int y1 , int y2); 

void prepare_copy(draw_rect_t *arg, ea_t outpoiter,ea_t inpointer,int pitchIn,int pitchOut,int bppO,int alu, Spu_Pixel planemask);

void copy(draw_rect_t *arg, ea_t outpointer,ea_t inpointer,int srcX, int srcY , int dstX , int dstY,int width,int height);

unsigned int draw_rect_receive_message_hard(draw_rect_t *);

void draw_rect_send_message(draw_rect_t*,unsigned int message);

void draw_rect_destroy(draw_rect_t*);


#ifdef __cplusplus
}
#endif

#endif

