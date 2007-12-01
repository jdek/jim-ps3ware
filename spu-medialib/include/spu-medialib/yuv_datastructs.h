/**
 * SPU-MEDIALIB utility
 * --------------------------------
 * Licensed under the BSD license, see LICENSE for details
 *
 * yuv_datastructs.h - YUV argument structure
 *
 * Copyright (c) 2007, Kristian Jerpetjøn <kristian.jerpetjoen@gmail.com>
 *
 * $Id: yuv_datastructs.h 26 2007-04-20 01:42:55Z warren $
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

#ifndef __YUV_DATASTRUCTS_H
#define __YUV_DATASTRUCTS_H

enum format { YUY2, YUYV422, YUV420 ,YV12 , YUV444, ARGB };
enum msg_form {INTR,HARD,SOFT};

struct img_args {
	int srcW;
	int srcH;
	int maxwidth;
	int offset;
	int dstW;
	int dstH;
	
 	
	unsigned long long  Ystart[2];
	unsigned long long  Ustart[2];
	unsigned long long  Vstart[2];
	unsigned long long  Output[2];

	//New improved datastructs above kept for backwards compatibility union perhaps.
	int src_x;
	int src_y;
	int src_w;
	int src_h;
	int drw_x;
	int drw_y;
	int drw_w;
	int drw_h;

	int Istride[2];
	int Ostride[2];

	int SCALE;
	float scalefactor;
	
	int BLEND;
	char ALPHA;

	int SourceFormat;
	int TargetFormat;
	int MessageForm;
	
// 	#ifdef __spe__
		unsigned long long  Inp0[2];
		unsigned long long  Inp1[2];
		unsigned long long  Inp2[2];
		unsigned long long  Outp0[2];
		unsigned long long  Outp1[2];
		unsigned long long  Outp2[2];
	//NEW stuff end

} __attribute__((aligned(128)));

#endif

