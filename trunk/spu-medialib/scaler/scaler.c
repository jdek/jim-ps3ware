/* 
 * Copyright (c) 2007, Rasmus Rohde <rohde@duff.dk>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 *             THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *             "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *             LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *             A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *             CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *             EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *             PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *             PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *             LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *             NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *             SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include <spu_mfcio.h>
#include <spu_intrinsics.h>

#define MAX_WIDTH 1280

typedef struct {
	unsigned long long in;
	unsigned long long out;

	unsigned int in_width; // Must be a multiple of 4	
	float scale;
	unsigned int out_height;
	unsigned int out_width_total; // Must be a multiple of 4
	unsigned int out_width_overscan; // Must be a multiple of 4
	unsigned int page_size; // Must be a multiple of 16
} __attribute__((aligned(16))) spe_data;

/*
  Bilinear scaler. Input must be 24-bit RGB where unsigned char in[0] = 0, in[1] = red, 
  in[2] = green, in[3] = blue.

  Output will be in the same format.

  To quit the SPE thread write a number <= 0 in its mailbox.
  To scale to out + (x - 1) * page_size write x >= 1 in its mailbox

  A zero will be written to the interrupt mailbox when a new frame is ready
  to be received.
 */
int main(unsigned long long spe_id, unsigned long long program_data_ea, unsigned 
	 long long env) {
	vector unsigned char temp_output_vec[MAX_WIDTH/4];
	vector float fframe[MAX_WIDTH*2];
	vector unsigned char frame[MAX_WIDTH/2];

	spe_data pd __attribute__((aligned(16)));

	unsigned int yc __attribute__((aligned(16)));
	unsigned int xc __attribute__((aligned(16)));
	unsigned int out_height __attribute__((aligned(16)));
	unsigned int out_width_total __attribute__((aligned(16)));
	unsigned int out_width __attribute__((aligned(16)));
	float scale __attribute__((aligned(16)));
	
	unsigned long long in __attribute__((aligned(16)));
	unsigned int width2x __attribute__((aligned(16)));

	vector unsigned char new_pixel_perm[4] = {{19, 23, 27, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						  {0, 1, 2, 3, 19, 23, 27, 31, 0, 0, 0, 0, 0, 0, 0, 0},
						  {0, 1, 2, 3, 4, 5, 6, 7, 19, 23, 27, 31, 0, 0, 0, 0},
						  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 19, 23, 27, 31}};
	
	vector unsigned int width_vec = spu_splats(pd.in_width);
	vector unsigned char extract_pix1 = {16, 16, 16, 16, 16, 16, 16, 1, 16, 16, 16, 2, 16, 16, 16, 3};
	vector unsigned char extract_pix2 = {16, 16, 16, 16, 16, 16, 16, 5, 16, 16, 16, 6, 16, 16, 16, 7};
	vector unsigned char extract_pix3 = {16, 16, 16, 16, 16, 16, 16, 9, 16, 16, 16, 10, 16, 16, 16, 11};
	vector unsigned char extract_pix4 = {16, 16, 16, 16, 16, 16, 16, 13, 16, 16, 16, 14, 16, 16, 16, 15};
	vector unsigned char zero_vec = spu_splats((unsigned char)0);
	vector float onef_vector = spu_splats(1.0f);

	int tag_id_input = 0;
	int tag_id_output = 0;
	
	//Initiate copy
	mfc_get(&pd, program_data_ea, sizeof(pd), tag_id_input, 0, 0);
	//Wait for completion
	mfc_write_tag_mask(1<<tag_id_input);
	mfc_read_tag_status_any();

	width2x = pd.in_width * 2;
	in = pd.in;	
	out_height = pd.out_height;
	scale = pd.scale;
	out_width = (pd.out_width_total - pd.out_width_overscan) >> 2;
	out_width_total = pd.out_width_total << 2;
	
	do {
		int mbox_val;
		int last_pixel_y = -57;
		unsigned long long out __attribute__((aligned(16)));

		//Wait for completion from possible previous tranfser
		mfc_write_tag_mask(1<<tag_id_output);
		mfc_read_tag_status_any();

		// Tell PPU we are ready for the next frame
		spu_writech(SPU_WrOutIntrMbox, 0);

		mbox_val = spu_read_in_mbox();

		if(mbox_val <= 0)
			break;

		out = pd.out + (mbox_val - 1) * pd.page_size;

		for(yc=0; yc<out_height; yc++) {
			int pixel_y = (int)(yc * scale);
			vector float weight_y = spu_splats(yc*scale - pixel_y);
			
			if(pixel_y != last_pixel_y) {
				if(pixel_y+1 != last_pixel_y) {
					//Initiate copy
					mfc_get(frame, in+pixel_y*width2x*2, width2x*4, tag_id_input, 0, 0);
					//Wait for completion
					mfc_write_tag_mask(1<<tag_id_input);
					mfc_read_tag_status_any();
					for(xc=0; xc<width2x / 4; xc++) {
						vector unsigned char pix1 = spu_shuffle(frame[xc], zero_vec, extract_pix1);
						vector unsigned char pix2 = spu_shuffle(frame[xc], zero_vec, extract_pix2);
						vector unsigned char pix3 = spu_shuffle(frame[xc], zero_vec, extract_pix3);
						vector unsigned char pix4 = spu_shuffle(frame[xc], zero_vec, extract_pix4);
						fframe[xc*4] = spu_convtf((vector unsigned int)pix1, 0);
						fframe[xc*4+1] = spu_convtf((vector unsigned int)pix2, 0);
						fframe[xc*4+2] = spu_convtf((vector unsigned int)pix3, 0);
						fframe[xc*4+3] = spu_convtf((vector unsigned int)pix4, 0);
					}
				} else {
					//Initiate copy
					mfc_get(frame, in+(pixel_y+1)*width2x*2, width2x*2, tag_id_input, 0, 0);
					for(xc=0; xc<width2x / 8; xc++) {
						fframe[xc] = fframe[xc+width2x/2];
					}

					//Wait for completion
					mfc_write_tag_mask(1<<tag_id_input);
					mfc_read_tag_status_any();
					
					for(xc=0; xc<width2x / 8; xc++) {
						vector unsigned char pix1 = spu_shuffle(frame[xc], zero_vec, extract_pix1);
						vector unsigned char pix2 = spu_shuffle(frame[xc], zero_vec, extract_pix2);
						vector unsigned char pix3 = spu_shuffle(frame[xc], zero_vec, extract_pix3);
						vector unsigned char pix4 = spu_shuffle(frame[xc], zero_vec, extract_pix4);
						fframe[xc*4+width2x/2] = spu_convtf((vector unsigned int)pix1, 0);
						fframe[xc*4+width2x/2+1] = spu_convtf((vector unsigned int)pix2, 0);
						fframe[xc*4+width2x/2+2] = spu_convtf((vector unsigned int)pix3, 0);
						fframe[xc*4+width2x/2+3] = spu_convtf((vector unsigned int)pix4, 0);
					}
				}
				last_pixel_y = pixel_y;
			}

			vector float pixel_x = {0.0f, scale, scale*2.0f, scale*3.0f};
			vector float scale_adder = spu_splats(scale*4.0f);
			for(xc=0; xc<out_width; xc++) {
				vector unsigned int floor_pixel_x = spu_convtu(pixel_x, 0);
				vector float weight_x = spu_sub(pixel_x, spu_convtf(floor_pixel_x, 0));
				vector float weight_4 = spu_mul(weight_y, weight_x);
				vector float weight_3 = spu_mul(spu_sub(onef_vector, weight_y), weight_x);
				vector float weight_2 = spu_mul(weight_y, spu_sub(onef_vector, weight_x));
				vector float weight_1 = spu_mul(spu_sub(onef_vector, weight_y), spu_sub(onef_vector, weight_x));

				vector unsigned int offset1 = floor_pixel_x;
				vector unsigned int offset2 = spu_add(offset1, width_vec);
				vector unsigned int offset3 = spu_add(offset1, spu_splats(1u));
				vector unsigned int offset4 = spu_add(offset2, spu_splats(1u));

				int i;
				vector unsigned char new_pixel_x4 = spu_splats((unsigned char)0);
				for(i=0; i<4; i++) {
					unsigned int i_offset1 = spu_extract(offset1, i);
					unsigned int i_offset2 = spu_extract(offset2, i);
					unsigned int i_offset3 = spu_extract(offset3, i);
					unsigned int i_offset4 = spu_extract(offset4, i);
					float f_w_1 = spu_extract(weight_1, i);
					float f_w_2 = spu_extract(weight_2, i);
					float f_w_3 = spu_extract(weight_3, i);
					float f_w_4 = spu_extract(weight_4, i);
					
					vector float vtemp = spu_mul(fframe[i_offset1], spu_splats(f_w_1));
					vtemp = spu_madd(fframe[i_offset2], spu_splats(f_w_2), vtemp);
					vtemp = spu_madd(fframe[i_offset3], spu_splats(f_w_3), vtemp);
					vtemp = spu_madd(fframe[i_offset4], spu_splats(f_w_4), vtemp);

					vector unsigned int new_pixel = spu_convtu(vtemp, 0);
					
					new_pixel_x4 = spu_shuffle(new_pixel_x4, (vector unsigned char)new_pixel, new_pixel_perm[i]);				
				}
				temp_output_vec[xc] = new_pixel_x4;

				pixel_x = spu_add(pixel_x, scale_adder);
			}

			mfc_put(temp_output_vec, out, out_width << 4, tag_id_output, 0, 0);

			out += out_width_total;
		}
	} while(1);
	
	return 0;
}
