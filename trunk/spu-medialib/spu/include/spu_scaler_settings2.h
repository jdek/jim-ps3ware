/**
 * SPU YUV420/YV12 scaler to ARGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * scaler_settings.h - local variables used in conversion process
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

#ifndef __SPU_SCALER_SETTINGS2_H
#define __SPU_SCALER_SETTINGS2_H

//struct scaler_settings_s;
typedef struct scaler_settings_s scaler_settings_t;
struct scaler_settings_s {
	int *wfilterpos; //position of the filter in the buffer
	int *crfilterpos;
	int width;
	int smallcroma;
	int smallcromaline0;
	int smallcromaline1;
	vector unsigned char *sWfilter0;// shufflefilter0
	vector unsigned char *sWfilter1;//shufflefilter1 0+1
	vector unsigned char *crsWfilter0;
	vector unsigned char *crsWfilter1;
	vector short *wWfilter0;//weghtfilter0
	vector short *wWfilter1;//weightfilter1 (1-wWfilter0)
	vector float *wWfilterf0;//weghtfilter0
	vector float *wWfilterf1;//weightfilter1 (1-wWfilter0)
	short wHfilter;
	vector float wHfilter0;
	vector float wHfilter1;
	vector float wHfilterf0;
	vector float wHfilterf1;
	vector unsigned char *source00; //sourcepointer0
	vector unsigned char *source01; //sourcepointer1 0+1
//	vector float *fbuff00;
//	vector float *fbuff01;
//	vector float *fbuff10;
//	vector float *fbuff11;
	vector short *Output;
	vector float *Outputf;
} __attribute__((aligned(128)));



#endif
