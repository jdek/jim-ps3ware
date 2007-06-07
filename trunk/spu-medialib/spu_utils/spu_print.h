/**
 * SPU-MEDIALIB utility
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_print.h - SPU Print Macros 
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


#ifndef __SIMDPRINT_H
#define __SIMDPRINT_H
static inline void printfvec(char * name,vector float vf)
{
	printf("Vector %s=( %f, %f, %f, %f )\n",name, spu_extract(vf,0), spu_extract(vf,1), spu_extract(vf,2), spu_extract(vf,3));
}


static inline void printintvec(char * name,vector int vi)
{
	printf("Vector %s=( %d, %d, %d, %d )\n",name, spu_extract(vi,0), spu_extract(vi,1), spu_extract(vi,2), spu_extract(vi,3));
}


static inline void printshortvec(char * name,vector short vs)
{
	printf("Vector %s=( %d, %d, %d, %d, %d, %d, %d, %d )\n",name, spu_extract(vs,0), spu_extract(vs,1), spu_extract(vs,2), spu_extract(vs,3), spu_extract(vs,4), spu_extract(vs,5), spu_extract(vs,6), spu_extract(vs,7));
}

void printcharvec(char* name,vector unsigned char vc)
{
printf("Vector %s=( %d, %d, %d, %d, %d, %d, %d, %d,%d, %d, %d, %d, %d, %d, %d, %d )\n",name, spu_extract(vc,0), spu_extract(vc,1), spu_extract(vc,2), spu_extract(vc,3), spu_extract(vc,4), spu_extract(vc,5), spu_extract(vc,6), spu_extract(vc,7), spu_extract(vc,8), spu_extract(vc,9), spu_extract(vc,10), spu_extract(vc,11), spu_extract(vc,12), spu_extract(vc,13), spu_extract(vc,14), spu_extract(vc,15));
	
}

#endif

