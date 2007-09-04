
#include "mpeg2.h"

using namespace std;

struct mpeg2_dec_s{
	FILE *fp;
	int width;
	int height;
	int aspect;
	int framerate;
	int bitrate;
	int marker_bit;
	int vbv_buffer_size_value;
	int constrained_parameters_flag;
	int load_intra_quantiser_matrix;
	int load_non_intra_quantiser_matrix;
	char intra_quantiser[64];
	char non_intra_quantiser[64]; 
	int open;
	//	ifstream source;
};


mpeg2_dec_s *open_file(char* filename)
{
	
	struct mpeg2_dec_s *mp2dec;
//	mp2dec=(struct mpeg2_dec_s*)memalign(64,sizeof(struct mpeg2_dec_s));
//	mp2dec->source.open(filename,ios::in|ios::binary);
	if((mp2dec->fp=fopen(filename,"r"))==NULL) 
	{
		printf("error opening file %s\n",filename); 
		exit(1);
	}
	else
		mp2dec->open=1;

	return mp2dec;
}

void close_file(mpeg2_dec_s *mp2dec)
{
	fclose(mp2dec->fp);
}

int is_end(mpeg2_dec_s* mp2dec)
{
	return feof(mp2dec->fp);

}

int is_open(mpeg2_dec_s* mp2dec)
{
//	struct mpeg2_dec_s* mp2dec;
//	mp2dec=(struct mpeg2_dec_s *) arg;
	return mp2dec->open;
}

char nextbyte(mpeg2_dec_s* mp2dec)
{
	return getc(mp2dec->fp);
}

void sequence_start(mpeg2_dec_s* mp2dec)
{
	char buffer[8];
//	fgets(buffer,8,mp2dec->fp);
	fread(buffer,1,8,mp2dec->fp);
	mp2dec->width=(buffer[0]<<4)|(buffer[1]>>4);
	printf("Width \t:%d\n",mp2dec->width);

	mp2dec->height=((buffer[1]&0x0F)<<8)|buffer[2];
	printf("Height \t:%d\n",mp2dec->height);

	mp2dec->aspect=buffer[3]>>4;
	printf("Aspect \t:%s\n",aspect_ratio_str[mp2dec->aspect]);

	mp2dec->framerate=buffer[3]&0x0F;
	printf("FPS\t:%s\n",frame_rate_str[mp2dec->framerate]);
	
	mp2dec->bitrate=buffer[4]<<10|buffer[5]<<2|buffer[6]>>6;
	printf("bitrate\t:%d\n",mp2dec->bitrate);

	mp2dec->marker_bit=(buffer[6]>>5)&0x01;
	printf("marker_bit\t:%d\n",mp2dec->marker_bit);

	mp2dec->vbv_buffer_size_value=((buffer[6]&0x1F)<<5) | buffer[7]>>3;
	printf("vbv_buffer_size_value:\t %d \n",mp2dec->vbv_buffer_size_value);

	mp2dec->constrained_parameters_flag=(buffer[7]>>2)&0x01;

	mp2dec->load_intra_quantiser_matrix=(buffer[7]>>1)&0x01;

	if (mp2dec->load_intra_quantiser_matrix)
	{
		char buffer2[64];
		mp2dec->intra_quantiser[0]=(buffer[7]<<7)|(buffer2[0]>>1);
		fread(buffer2,1,64,mp2dec->fp);
		int i;
		for (i=1;i<64;i++)
		{
			mp2dec->intra_quantiser[i]=(buffer2[i-1]<<7) | (buffer2[i]>>1);
		}
		mp2dec->load_non_intra_quantiser_matrix=buffer2[63]&0x01;
	}
	else
	{
		mp2dec->load_non_intra_quantiser_matrix=buffer[7]&0x01;
	}

	if (mp2dec->load_non_intra_quantiser_matrix)
	{
		char buffer3[64];
		fread(buffer3,1,64,mp2dec->fp);
		int i;
		for (i=0;i<64;i++)
		mp2dec->non_intra_quantiser[i]=buffer3[i];
	}
	
	printf("sequence start end\n");
		

	
//	if (mp2dec->marker_bit==0x01)
		
	
 
}


