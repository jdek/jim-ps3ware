/**
 * SPU 2D rectangular draw kernel
 * --------------------------------
 * Licensed under the BSDv2 
 *
 * spu_draw_rect.h - Main loop for the rectangular draw kernel
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

#include <spu_mfcio.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <spu_dmalib.h>
#include <spu_alu.h>
#include <data_2d.h>

#include <spu_control.h>
#include <spu_print.h>

#define MAXWIDTH 1920
#define MAXHEIGHT 1080



int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) 
{
	int tgi[3][2];
	int tgo[3][2];
	int tgt[3][2];

	tgi[0][0] = 1;
	tgi[0][1] = 2;
	tgi[1][0] = 3;
	tgi[1][1] = 4;
	tgi[2][0] = 5;
	tgi[2][1] = 6;

	tgo[0][0] = 13;
	tgo[0][1] = 14;
	tgo[1][0] = 15;
	tgo[1][1] = 16;
	tgo[2][0] = 17;
	tgo[2][1] = 18;

	tgt[0][0] = 19;
	tgt[0][1] = 20;
	tgt[1][0] = 21;
	tgt[1][1] = 22;
	tgt[2][0] = 23;
	tgt[2][1] = 24;

	int tag = 31; //used for all todo list dma's

	int SelIn = 0;
	int SelOut = 0;
	
	int msg;
	int waiting;	

// 	while ( spu_stat_in_mbox() == 0 );
// 	
// 	msg = spu_read_in_mbox();
	
// 	if (msg == RUN) {
// 	
// 		fprintf( stderr, "spu_2d: Starting Up\n" );
// 	}

	struct data_2 *data;

	data = (struct data_2*) memalign( 128, sizeof( *data ) );

	//dmaGetnWait(data, (unsigned int) argp, (int) envp, tag); //getting neccesary data to process the data.. hmm cant this be dma'ed before startup..

	vector unsigned char *INPUT[3][2];

	INPUT[0][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 );
	INPUT[0][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 );
	INPUT[1][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 );
	INPUT[1][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 );
	INPUT[2][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 );
	INPUT[2][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 );


	vector unsigned char *OUTPUT[3][2];
	OUTPUT[0][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	OUTPUT[0][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	OUTPUT[1][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	OUTPUT[1][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	OUTPUT[2][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	OUTPUT[2][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );

	vector unsigned char *TEMP[3][2];
	TEMP[0][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	TEMP[0][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	TEMP[1][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	TEMP[1][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	TEMP[2][0] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	TEMP[2][1] = (vector unsigned char *) memalign( 128, MAXWIDTH * 4 + 64 );
	
	unsigned long long Inp;
	unsigned long long Tmp;
	unsigned long long Outp;
	
	int Inpitch;
	int Outpitch;

	int dmaInW;
	int dmaOutW;

	int Inoff1 = 0;
	int Inoff2 = 0;
	int Outoff1 = 0;
	int Outoff2 = 0;
	int lines = 0;

	vector unsigned char color;

	initselfilters();

	initshufflefilters();
	msg=RDY;
	
	while (msg != STOP) 
	{
		while (spu_stat_out_mbox() == 0);

			msg=RDY;

		spu_write_out_mbox(msg);

			waiting=1;
// 		}
	
		while ( waiting ) {
			
			while ( spu_stat_in_mbox() == 0 );

			msg = spu_read_in_mbox();
			
			if (msg == RUN) {
				int tag=28;
				dmaGetnWait( data, (unsigned int)argp, (int)envp, tag ); 
				waiting=0;
			}
			else if ( msg == STOP ) {
// 				fprintf(stderr,"spu_yuvscaler: Stopping\n");
				waiting=0;
			}
			else if ( msg == UPDATE ) {
				int tag=28;
				dmaGetnWait( data, (unsigned int)argp, (int)envp, tag ); //getting neccesary data to process the data.. hmm cant this be dma'ed before startup..
// 				first=1; // update filters to reflect the new image!

			}
		}
		
// 		if (first)
// 		{
// 			
// 			
// 		}
		
		Inp = data->Inp; //yes you just have to be 128 bit (preferably also 128 byte.) aligned..

// 		Tmp = data->Outp;

		Outp = data->Outp;

		
		Outp = Outp + ( data->dst_y * data->pitchOut * data->bppO ) / 8;

		Outp = Outp + ( ( data->dst_x * data->bppO ) / 8 ) & ~ 15;

		Tmp=Outp;
		
		Outoff1 = ( ( data->dst_x * data->bppO ) / 8)& 0xF;// - ( ( ( data->dst_x * data->bppO ) / 8) & ~15); //- ( ( ( data->dst_x * data->bppI ) / 8) & ~15);

		Outoff2 = ( ( ( data->dst_x + data->width ) * data->bppO ) / 8 )&0xF;// - ( ( ( ( data->dst_x + data->width ) * data->bppO ) / 8 )& ~15);

		dmaOutW = ( ( ( data->width * data->bppO ) / 8 ) + Outoff1 + 15 ) & ~ 15;
// 		printf("off1 %d, off2 %d, dmaOutW %d\n",Outoff1,Outoff2,dmaOutW);

		Outpitch = (data->pitchOut * data->bppO ) / 8;
		
		dmaGet( TEMP[0][0], Tmp, dmaOutW, tgt[0][0] );

		Tmp = Tmp + Outpitch;

		dmaGet( TEMP[1][0], Tmp, dmaOutW, tgt[1][0] );

		Tmp = Tmp + Outpitch;

		dmaGet( TEMP[2][0], Tmp, dmaOutW, tgt[2][0] );

		Tmp = Tmp + Outpitch;


		dmaGet( TEMP[0][1], Tmp, dmaOutW, tgt[0][1] );

		Tmp = Tmp + Outpitch;

		dmaGet( TEMP[1][1], Tmp, dmaOutW, tgt[1][1] );

		Tmp = Tmp + Outpitch;

		dmaGet( TEMP[2][1], Tmp, dmaOutW, tgt[2][1] );

		Tmp = Tmp + Outpitch;
		

		if ( data->operation == COPY ) {

			Inp = Inp + ( data->src_y * data->pitchIn * data->bppI ) / 8; 

			Inp = Inp + ( ( data->src_x * data->bppI ) / 8 ) & ~ 15;

			Inoff1 = ( ( data->src_x * data->bppI ) / 8) & 0x7; 

			Inoff2 = ( ( ( data->src_x + data->width ) * data->bppI ) / 8 ) & 0x7;
	
			dmaInW = ( ( ( data->width * data->bppI ) / 8 ) + Inoff1 + Inoff2 + 15 ) & ~ 15;

			Inpitch = ( data->pitchIn * data->bppI ) / 8;	
		

			dmaGet( INPUT[0][0], Inp, dmaInW, tgi[0][0] );

			Inp = Inp + Inpitch;

			dmaGet( INPUT[1][0], Inp, dmaInW, tgi[1][0] ); 

			Inp = Inp + Inpitch;

			dmaGet( INPUT[2][0], Inp, dmaInW, tgi[2][0] ); 

			Inp = Inp + Inpitch;


			dmaGet( INPUT[0][1], Inp,dmaInW, tgi[0][1] );

			Inp = Inp+Inpitch;

			dmaGet( INPUT[1][1], Inp, dmaInW, tgi[1][1] ); 

			Inp = Inp + Inpitch;

			dmaGet( INPUT[2][1], Inp, dmaInW, tgi[2][1] ); 

			Inp = Inp + Inpitch;
		}
	
		if ( data->operation == FILL ) {
			color = (vector unsigned char){data->Fg.A, data->Fg.R, data->Fg.G, data->Fg.B, 
				 data->Fg.A, data->Fg.R ,data->Fg.G, data->Fg.B,
				 data->Fg.A, data->Fg.R ,data->Fg.G, data->Fg.B,
				 data->Fg.A, data->Fg.R ,data->Fg.G, data->Fg.B};
// 			printcharvec("color",color);
		}
		SelIn = 0;
		SelOut = 0;
		
	//	lines = data->height;	
		int i;
		for (i=0 ; i< data->height/3 ; i++ ) {
	
			dmaWaitTag( tgi[0][SelIn] );

			dmaWaitTag( tgi[1][SelIn] );

			dmaWaitTag( tgi[2][SelIn] );


			dmaWaitTag( tgt[0][SelIn] );

			dmaWaitTag( tgt[1][SelIn] );

			dmaWaitTag( tgt[2][SelIn] );

			
			dmaWaitTag( tgo[0][SelOut] );

			dmaWaitTag( tgo[1][SelOut] );

			dmaWaitTag( tgo[2][SelOut] );

			switch (data->operation) {

				case FILL:

// 				if ( data->alu == SPUset ) {
				
					fill_line( TEMP[0][SelIn], OUTPUT[0][SelOut], Outoff1, Outoff2, data->width,data->alu, data->bppO, color);
	
					fill_line( TEMP[1][SelIn], OUTPUT[1][SelOut], Outoff1, Outoff2, data->width,data->alu, data->bppO, color);
	
					fill_line( TEMP[2][SelIn], OUTPUT[2][SelOut], Outoff1, Outoff2, data->width,data->alu, data->bppO, color);
	// 				}
					break;
				
			//}
				case COPY:
// 			if ( data->operation == COPY ) {
					copy_line( TEMP[0][SelIn], OUTPUT[0][SelOut],INPUT[0][SelIn], Outoff1, Outoff2,Inoff1, Inoff2, data->width,data->alu, data->bppO );	
					copy_line( TEMP[1][SelIn], OUTPUT[1][SelOut],INPUT[1][SelIn], Outoff1, Outoff2,Inoff1, Inoff2, data->width,data->alu, data->bppO );	
					copy_line( TEMP[2][SelIn], OUTPUT[2][SelOut],INPUT[2][SelIn], Outoff1, Outoff2,Inoff1, Inoff2, data->width,data->alu, data->bppO );	
					break;

				default :
					break;
			}
// 			}

			dmaGet( TEMP[0][SelIn], Tmp, dmaOutW, tgt[0][SelIn] );

			Tmp = Tmp + Outpitch;
	
			dmaGet( TEMP[1][SelIn], Tmp, dmaOutW, tgt[1][SelIn] );
	
			Tmp = Tmp + Outpitch;
	
			dmaGet( TEMP[2][SelIn], Tmp, dmaOutW, tgt[2][SelIn] );
	
			Tmp = Tmp + Outpitch;


			dmaPut( OUTPUT[0][SelOut], Outp, dmaOutW, tgo[0][SelOut] );

			Outp = Outp + Outpitch;

			dmaPut( OUTPUT[1][SelOut], Outp, dmaOutW, tgo[1][SelOut] );

			Outp = Outp + Outpitch;

			dmaPut( OUTPUT[2][SelOut], Outp, dmaOutW, tgo[2][SelOut] );

			Outp = Outp + Outpitch;


			if ( data->operation == COPY ) {
			
				dmaGet( INPUT[0][SelIn], Inp,dmaInW, tgi[0][SelIn] );

				Inp = Inp+Inpitch;
	
				dmaGet( INPUT[1][SelIn], Inp, dmaInW, tgi[1][SelIn] ); 

				Inp = Inp + Inpitch;
	
				dmaGet( INPUT[2][SelIn], Inp, dmaInW, tgi[2][SelIn] ); 

				Inp = Inp + Inpitch;
		
			}
			
			SelOut=SelOut^1;
			SelIn=SelIn^1;
				
		} 
		
		for (i=0 ; i < data->height - 3*(data->height / 3 ) ; i++ ) {
	
			dmaWaitTag( tgi[i][SelIn] );

			dmaWaitTag( tgt[i][SelIn] );

			
			dmaWaitTag( tgo[i][SelOut] );



			
			if ( data->operation == FILL ) {
				
					fill_line( TEMP[i][SelIn], OUTPUT[i][SelOut], Outoff1, Outoff2, data->width,data->alu, data->bppO, color);	
			} 
	
			if ( data->operation == COPY ) {
// 				static inline void copy_line( vector unsigned char *Tmp, vector unsigned char *Out, vector unsigned char *In,int Off1, int Off2,int Ioff1, int Ioff2, int width, int alu, int bpp,  ) {
					copy_line( TEMP[i][SelIn], OUTPUT[i][SelOut],INPUT[i][SelIn], Outoff1, Outoff2,Inoff1, Inoff2, data->width,data->alu, data->bppO );	
			} 


			dmaPut( OUTPUT[i][SelOut], Outp, dmaOutW, tgo[i][SelOut] );

			Outp = Outp + Outpitch;
				
		} 
			
		dmaWaitTag( tgo[0][SelOut] );

		dmaWaitTag( tgo[1][SelOut] );

		dmaWaitTag( tgo[2][SelOut] );

		SelOut=SelOut^1;

		dmaWaitTag( tgo[0][SelOut] );

		dmaWaitTag( tgo[1][SelOut] );

		dmaWaitTag( tgo[2][SelOut] );
		
//		dmaWaitTag( tgo[o][SelOut] ); //FIXME do this perhaps later
// 		if (iargs->MessageForm == INTR)
// 		{
// 			while (spu_stat_out_intr_mbox() == 0);
// 
// 			msg = RDY;
// 			spu_writech(SPU_WrOutIntrMbox, msg);
// 
// 			waiting=1;
// 		}

// 		if (iargs->MessageForm == HARD)
// 		{

		
		

	}
	
	return 0;
}
