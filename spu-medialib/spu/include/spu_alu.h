/**
 * SPU ALU kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * draw_alu.h - spu header for manipulation 
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
#ifndef __SPU_ALU_H
#define __SPU_ALU_H

#include <data_2d.h>

static const vector unsigned char one={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
vector unsigned char selfilter4[16];
vector unsigned char shufflefilter[16];


static inline void fill_line( vector unsigned char *Tmp, vector unsigned char *Out, int Off1, int Off2, int width, int alu ,int bpp, vector unsigned char color ) {
	//SPUclear,SPUnoop,SPUequiv,SPUcopyInverted,SPUset
	int i = 0;
	switch ( alu ) {
		case SPUor: 

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_or( color, Tmp[ i ] );
				Out[ i + 1 ] = spu_or( color, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_or( color, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_or( color, Tmp[ i + 3 ] );
			}
			
			break;
	
		case SPUinvert:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_xor( one, Tmp[ i ] );
				Out[ i + 1 ] = spu_xor( one, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_xor( one, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_xor( one, Tmp[ i + 3 ] );
			}

			break;

		case SPUxor:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_xor( color, Tmp[ i ] );
				Out[ i + 1 ] = spu_xor( color, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_xor( color, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_xor( color, Tmp[ i + 3 ] );
			}

			break;
	
		case SPUnor:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_nor( color, Tmp[ i ] );
				Out[ i + 1 ] = spu_nor( color, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_nor( color, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_nor( color, Tmp[ i + 3 ] );
			}

			break;

		case SPUorReverse:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_or( Tmp[ i ], color  );
				Out[ i + 1 ] = spu_or( Tmp[ i + 1 ], color );
				Out[ i + 2 ] = spu_or( Tmp[ i + 2 ], color);
				Out[ i + 3 ] = spu_or( Tmp[ i + 3 ], color );
			}

			break;

		case SPUand:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_and( color, Tmp[ i ] );
				Out[ i + 1 ] = spu_and( color, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_and( color, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_and( color, Tmp[ i + 3 ] );
			}
			break;

		case SPUandReverse:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_and( Tmp[ i ], color  );
				Out[ i + 1 ] = spu_and( Tmp[ i + 1 ], color );
				Out[ i + 2 ] = spu_and( Tmp[ i + 2 ], color);
				Out[ i + 3 ] = spu_and( Tmp[ i + 3 ], color );
			}
			
			break;

		case SPUnand:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_nand( color, Tmp[ i ] );
				Out[ i + 1 ] = spu_nand( color, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_nand( color, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_nand( color, Tmp[ i + 3 ] );
			}

			break;

// 		case SPUblend:
			
	
		default:
			break;
	}

	// fix alignement

	if ( Off1 > 0 ) {

		Out[0] = spu_sel( Out[0], Tmp[0], selfilter4[ Off1 ] );
	}

	

	if ( Off2 > 0 ) {

		i = ( Off1 + ( ( width * bpp ) / 8 ) ) / 16;

		Out[i] = spu_sel( Tmp[i], Out[i], selfilter4[ Off2 ] );
	}

}

static inline void copy_line( vector unsigned char *Tmp, vector unsigned char *Out, vector unsigned char *In ,int Off1, int Off2,int Ioff1, int Ioff2, int width, int alu, int bpp ) {
	//SPUclear,SPUnoop,SPUequiv,SPUcopyInverted,SPUset
	int i = 0;
	int neg=0;
	
	int shuf=Ioff2-Off1;
	
	if (shuf < 0 ) {
		int shuf=16+shuf;
		neg=0;
	}

	vector unsigned char shufle0,shufle1,shufle2,shufle3;

	switch ( alu ) {
		case SPUor: 
			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0=spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1=spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2=spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3=spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
			
				
		
				Out[ i ]     = spu_or( shufle0, Tmp[ i ] );
				Out[ i + 1 ] = spu_or( shufle1, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_or( shufle2, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_or( shufle3, Tmp[ i + 3 ] );
			}
			
			break;
	
		case SPUinvert:

			for ( i = 0 ; i < width; i = i + 4 ) { //do this at least once.
	
				Out[ i ]     = spu_xor( one, Tmp[ i ] );
				Out[ i + 1 ] = spu_xor( one, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_xor( one, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_xor( one, Tmp[ i + 3 ] );
			}

			break;

		case SPUxor:

			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0 = spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1 = spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2 = spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3 = spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
	
				Out[ i ]     = spu_xor( shufle0, Tmp[ i ] );
				Out[ i + 1 ] = spu_xor( shufle1, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_xor( shufle2, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_xor( shufle3, Tmp[ i + 3 ] );
			}

			break;
	
		case SPUnor:

			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0 = spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1 = spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2 = spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3 = spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
	
				Out[ i ]     = spu_nor( shufle0, Tmp[ i ] );
				Out[ i + 1 ] = spu_nor( shufle1, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_nor( shufle2, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_nor( shufle3, Tmp[ i + 3 ] );
			}

			break;

		case SPUorReverse:

			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0 = spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1 = spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2 = spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3 = spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
	
				Out[ i ]     = spu_or( Tmp[ i ], shufle0  );
				Out[ i + 1 ] = spu_or( Tmp[ i + 1 ], shufle1 );
				Out[ i + 2 ] = spu_or( Tmp[ i + 2 ], shufle2 );
				Out[ i + 3 ] = spu_or( Tmp[ i + 3 ], shufle3 );
			}

			break;

		case SPUand:

			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0 = spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1 = spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2 = spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3 = spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
	
				Out[ i ]     = spu_and( shufle0, Tmp[ i ] );
				Out[ i + 1 ] = spu_and( shufle1, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_and( shufle2, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_and( shufle3, Tmp[ i + 3 ] );
			}
			break;

		case SPUandReverse:

			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0 = spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1 = spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2 = spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3 = spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
	
				Out[ i ]     = spu_and( Tmp[ i ], shufle0  );
				Out[ i + 1 ] = spu_and( Tmp[ i + 1 ], shufle1 );
				Out[ i + 2 ] = spu_and( Tmp[ i + 2 ], shufle2 );
				Out[ i + 3 ] = spu_and( Tmp[ i + 3 ], shufle3 );
			}
			
			break;

		case SPUnand:

			i=0;
			if (neg ) { 
			
				i=-1;
			}

			for ( i; i < width; i = i + 4 ) { //do this at least once.

				shufle0 = spu_shuffle( In[ i ], In[ i + 1], shufflefilter[ shuf ] );
				shufle1 = spu_shuffle( In[ i + 1], In[ i + 2 ], shufflefilter[ shuf ] );
				shufle2 = spu_shuffle( In[ i + 2 ], In[ i + 3 ], shufflefilter[ shuf ] );
				shufle3 = spu_shuffle( In[ i + 3 ], In[ i + 4 ], shufflefilter[ shuf ] );
	
				Out[ i ]     = spu_nand( shufle0, Tmp[ i ] );
				Out[ i + 1 ] = spu_nand( shufle1, Tmp[ i + 1 ] );
				Out[ i + 2 ] = spu_nand( shufle2, Tmp[ i + 2 ] );
				Out[ i + 3 ] = spu_nand( shufle3, Tmp[ i + 3 ] );
			}

			break;

// 		case SPUblend:
			
	
		default:
			break;
	}

	// fix alignement

	if ( Off1 > 0 ) {

		Out[0] = spu_sel( Out[0], Tmp[0], selfilter4[ Off1 ] );
	}

	

	if ( Off2 > 0 ) {

		i = ( Off1 + ( ( width * bpp ) / 8 ) ) / 16;

		Out[i] = spu_sel( Tmp[i], Out[i], selfilter4[ Off2 ] );
	}

}


static inline void initselfilters() {

	selfilter4[0]  = (vector unsigned char) { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[1]  = (vector unsigned char) { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[2]  = (vector unsigned char) { 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[3]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[4]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[5]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[6]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[7]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[8]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[9]  = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[10] = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[11] = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[12] = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
	selfilter4[13] = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00 };
	selfilter4[14] = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00 };
	selfilter4[15] = (vector unsigned char) { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
}

static inline void initshufflefilters() {

	shufflefilter[0]  = (vector unsigned char ) { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 };
	shufflefilter[1]  = (vector unsigned char ) { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16 };
	shufflefilter[2]  = (vector unsigned char ) { 2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17 };
	shufflefilter[3]  = (vector unsigned char ) { 3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
	shufflefilter[4]  = (vector unsigned char ) { 4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
	shufflefilter[5]  = (vector unsigned char ) { 5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
	shufflefilter[6]  = (vector unsigned char ) { 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
	shufflefilter[7]  = (vector unsigned char ) { 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 };
	shufflefilter[8]  = (vector unsigned char ) { 8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
	shufflefilter[9]  = (vector unsigned char ) { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
	shufflefilter[10] = (vector unsigned char ) {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
	shufflefilter[11] = (vector unsigned char ) {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 };
	shufflefilter[12] = (vector unsigned char ) {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
	shufflefilter[13] = (vector unsigned char ) {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
	shufflefilter[14] = (vector unsigned char ) {14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
	shufflefilter[15] = (vector unsigned char ) {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };
}

#endif
