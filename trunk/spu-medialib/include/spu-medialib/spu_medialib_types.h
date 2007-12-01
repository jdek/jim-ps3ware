/**
 * SPU Medialib 
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_medialib_types.h - header for the spu medialib containing data types 
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
#ifndef __SPU_MEDIALIB_TYPES_H
#define __SPU_MEDIALIB_TYPES_H

#include <stdint.h>

#ifdef __powerpc__
	#ifndef __powerpc64__
		
		#define spu_mask(val) ((uint64_t) ((uint32_t) val))
	#else
		#define spu_mask(val) ((uint64_t) val)
	#endif
#endif


typedef uint64_t addr_t;


enum ALU { SPUcopy, SPUxor, SPUinvert, SPUor, SPUclear, SPUand, SPUandReverse, SPUnoop, SPUnor, SPUequiv, SPUorReverse, SPUcopyInverted, SPUorInverted, 		   SPUnand, SPUset, SPUblend};

enum OPER {FILL,COPY,DOWN};

typedef struct {
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
 }Spu_Pixel;

#endif
