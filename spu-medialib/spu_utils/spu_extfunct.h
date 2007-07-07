/**
 * SPU-MEDIALIB utility
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_extfunct.h - SPU Extended functions
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

#ifndef __SPU_EXTFUNCT_H
#define __SPU_EXTFUNCT_H

static inline vector signed short spu_smul(vector signed short A, vector signed short B)
{
	vector signed int ABe=spu_mule(A,B);
	vector signed int ABo=spu_mulo(A,B);
	
	return (vector signed short)spu_shuffle(spu_rlmaska(ABe,-11),spu_rlmaska(ABo,-11),((vector unsigned char){2,3,18,19,6,7,22,23,10,11,26,27,14,15,30,31}));

};

static inline vector signed short spu_smadd(vector signed short A, vector signed short B,vector signed short C)
{	
	// this should be moved inline to avoid stalls....
	vector signed int ABe=spu_mule(A,B);
	vector signed int ABo=spu_mulo(A,B);
	return spu_add((vector signed short)spu_shuffle(spu_rlmaska(ABe,-11),spu_rlmaska(ABo,-11),((vector unsigned char){2,3,18,19,6,7,22,23,10,11,26,27,14,15,30,31})),C);

};

#endif