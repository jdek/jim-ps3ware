
#include <libspe2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <spu-medialib/spu_control.h>
#include <spu-medialib/draw_rect.h>
#include <ps3fb/libps3fb.h>
#include <spu-medialib/data_2d.h>
#include <spu-medialib/spu_medialib_types.h>

using namespace std;

typedef struct {
    uint8_t  identsize;          // size of ID field that follows 18 byte header (0 usually)
    uint8_t  colourmaptype;      // type of colour map 0=none, 1=has palette
    uint8_t  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colourmapstart;     // first colour map entry in palette
    short colourmaplength;    // number of colours in palette
    uint8_t  colourmapbits;      // number of bits per palette entry 15,16,24,32

    short xstart;             // image x origin
    short ystart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
    uint8_t  bits;               // image bits per pixel 8,16,24,32
    uint8_t  descriptor;         // image descriptor bits (vh flip bits)
    
    // pixel data follows header
    
} TGA_HEADER;

double mysecond() { 
	struct timeval tp; 
	int i; 
	i= gettimeofday(&tp,NULL); 
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 ); 
	}


int main (int nArg, char* cArg[]) {

	TGA_HEADER tga;
	tga.identsize=0;
	tga.colourmaptype=0;
	tga.colourmapbits=0;
	tga.xstart=0;
	tga.ystart=0;
	tga.width=1920;
	tga.height=1080;
	tga.bits=32;
	

	int srcW=1920;
	int srcH=1080;
	int dstW=1920;
	int dstH=1080;
	int maxwidth=1920;
	int offset=0;
	int curBuf = 0;
	int ftot = 5000; //< Num1er of frames to run through for FPS accuracy
	int fcount = 1;
	char *filename="default.tga", *inBuf[2];
	char *RAMBuffer[2], *Ypointer, *Upointer, *Vpointer;
	unsigned int msg;

	ifstream Source;
	ofstream Destination;

        int counter=0;
        double time_elapsed=0.0;
        double start=mysecond();
        double stop;
        double old_time_elapsed=-11.0;
	int frame=0;
	//Parse commandline arguments
	if (nArg > 0) filename=cArg[1];
	if (nArg > 1) srcW=(int)atoi(cArg[2]);
	if (nArg > 2) srcH=(int)atoi(cArg[3]);
	if (nArg > 3) frame=(int)atoi(cArg[4]);
	if (nArg > 4) maxwidth=(int)atoi(cArg[5]);
	int inbuffersize=srcW*srcH*3/2;
	int tmp_buffersize=srcW*srcH*4;
	//Open input file and read 
	Source.open(filename,ios::binary);
	Destination.open("dest3.tga",ios::binary);
	
	char *temp;//=0x000002000000002000000000800738042000;
	temp=(char *)memalign(128,18);
	temp[2]=0x02;
	temp[7]=0x20;
	temp[12]=0x80;
	temp[13]=0x07;
	temp[14]=0x38;
	temp[15]=0x04;
	temp[16]=0x20;
	temp[17]=0x20;
//	memcpy(temp,&tga,sizeof(tga));
//	strcpy(temp,&tga);

//	Destination<<tga;
	Destination.write(temp,18);
	
	inBuf[0]=(char*)memalign(128,inbuffersize);
//	inBuf[1]=(char*)memalign(128,srcW*srcH+((srcW*srcH)/2));
	Source.seekg(inbuffersize*frame);

	Source.read(inBuf[0],inbuffersize);
//	Source.read(inBuf[1],(srcW*srcH+(srcW*srcH)/2));
	Source.close();
	

	RAMBuffer[0]=(char*)memalign(128,tmp_buffersize);
	RAMBuffer[1]=(char*)memalign(128,tmp_buffersize);	
	Ypointer=RAMBuffer[0];
	Upointer=Ypointer+srcW*srcH;
	Vpointer=Upointer+srcW*srcH/4;
	void* fbuf[2];
// 	fbuf[0]=ps3fb_init();
// 	fbuf[1]=ps3fb_swap();
// 	ps3fb_swap();//swap back!
	fbuf[0]=RAMBuffer[0];
	fbuf[1]=RAMBuffer[0];
//	yuv2rgb_t *yuvcsc = csc_init_yuv2rgb(srcW, srcH, offset, maxwidth, (ea_t)inBuf[0], (ea_t)inBuf[0], fbuf0,fbuf1);
	
//	blitter_t *bt;
// 	blitter_t *bt[6];
	msg=RUN;
	
	draw_rect_t *draw = init_draw_rect();
	printf("initiating spu\n");
// void prepare_solid(draw_rect_t *, ea_t outpoiter,int alu, Pixel planemask, Pixel Fg); 
// 
// void solid(draw_rect_t *, ea_t outpointer,int x1, int x2 , int y1 , int y2); 
	

	
	Spu_Pixel planemask;
	Spu_Pixel color;
 	color.A=255;
 	color.R=50;
 	color.G=100;
 	color.B=0;
	fprintf(stderr,"prepare copy to %p\n",fbuf[0]);
	prepare_solid(draw,fbuf[0],1920,32,SPUcopy,planemask,color);
	printf("draw_solid\n");
	int output=0;
	int update=0;
	int count=0;
	int bppi=12;
	int bppo=32;
	int i;
	int spes=1;
// 	for (i=0;i<spes;i++){
// 		 bt[i]=blitt(myarg);
// 		 send_message(bt[i],RUN);
// 	}
	int X;
	int Y;
	msg=draw_rect_receive_message_hard(draw);
	solid(draw,fbuf[0],0,1920,0,1080);
	int 	selOut=0;
//	solid(draw,(unsigned long long)fbuf[selOut],0,1920,0,0);
	selOut=0;

	int y=1;
	int x=1;
//	Pixel color;
 	color.A=255;
 	color.R=135;
 	color.G=128;
 	color.B=80;
	msg=draw_rect_receive_message_hard(draw);
	prepare_solid(draw,fbuf[0],1920,32,SPUcopy,planemask,color);
// 	msg=draw_rect_receive_message_hard(draw);
	solid(draw,fbuf[selOut],0,1,0,1);
// 	msg=draw_rect_receive_message_hard(draw);
// 	ps3fb_swapVsync();
// // 	solid(draw,(unsigned long long)fbuf[selOut],210,215,0,520);
// 	msg=draw_rect_receive_message_hard(draw);
// // 	solid(draw,(unsigned long long)fbuf[selOut],210,215,0,520);
// // 	msg=draw_rect_receive_message_hard(draw);
// 	msg=STOP;
		
	while (msg != STOP)
	{
		msg=draw_rect_receive_message_hard(draw);
			
		
// 		 send_message(bt[i],STOP);
// 
// 		}
	//	
	//	counter++;
	//	send_message(bt[count],STOP);
	//	for (i=0;i<spes;i++){
			
	//	}
	//	msg=receive_message(bt[count]);
		
		if (fcount == ftot)  {
	//		destroy(bt[count]);
			
			msg=STOP;
			stop=mysecond();
		//	printf("clock %f\n", stop);
			time_elapsed=(double)stop-start;
			float fps=(fcount*spes)/time_elapsed;
			float MpixelsI=srcW*srcH;
			float MpixelsO=dstW*dstH;
			float totframes=fps;
			float  In=((totframes/100)*MpixelsI*bppi/8)/10000;
			float  Out=((totframes/100)*MpixelsO*bppo/8)/10000;
			float Tot=In+Out;
			
			printf("Time of execution :%f\n",time_elapsed);
			printf("Number of frames displayed %d\n",fcount*spes);
			printf("width : %d, height : %d , FPS : %f\n",srcW,srcH,fps);
			printf("RAM->SPU datarate %fMB/s \n",In);
			printf("SPU->RAM datarate %fMB/s \n",Out);
			printf("total datarate %fMB/s \n",Tot);
		}
		if (msg == RDY ) {
			if (time_elapsed > (old_time_elapsed + 10))
			{
				//printf("Frames per second %f:\n",counter/time_elapsed);
				counter=0;
				old_time_elapsed=time_elapsed;
			}

			fcount++;
			

			
		//	for (i=0;i<spes;i++){
// 				if (update) {
// 					send_message(bt[i],UPDATE);
// 				}
// 				send_message(bt[i],RUN)
//  			x=0;
// 			int x2=1;
// 			for (i = 0; i < 32; i++ ) {
// 				solid(draw,(unsigned long long)fbuf[selOut],x,x+x2,0,130);
// 				
// 				msg=draw_rect_receive_message_hard(draw);
// 				x=x+60;
// 				x2=x2+1;
// 			}
 //			solid(draw,(unsigned long long)fbuf[selOut],1919,1920,0,1080);
// 			msg=draw_rect_receive_message_hard(draw);
			x=0;
			int x2=1;

			for (i = 0; i < 32; i++ ) {
				solid(draw,fbuf[selOut],x,x+x2,0,120);
				x=x+60;
				x2=x2+1;
	
				msg=draw_rect_receive_message_hard(draw);
 			}
		//	draw->data->Fg->R=128;
			x=1;
			x2=1;
			for (i = 0; i < 32; i++ ) {
				solid(draw,fbuf[selOut],x,x+x2,130,250);
				x=x+60;
				x2=x2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}
			x=2;
			x2=1;
			for (i = 0; i < 32; i++ ) {
				solid(draw,fbuf[selOut],x,x+x2,260,380);
				x=x+60;
				x2=x2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}
// 
			x=3;
			x2=1;
			for (i = 0; i < 32; i++ ) {
				solid(draw,fbuf[selOut],x,x+x2,390,510);
				x=x+60;
				x2=x2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}

			x=1920;
			y=540;
			int y2=1;




			for (i = 0; i < 16; i++ ) {
				solid(draw,fbuf[selOut],0,460,y,y+y2);
				y=y+20;
				y2=y2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}
// 			x=1920;
			y=541;
			y2=1;




			for (i = 0; i < 16; i++ ) {
				solid(draw,fbuf[selOut],480,940,y,y+y2);
				y=y+20;
				y2=y2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}
// 			x=1920;
			y=542;
			y2=1;




			for (i = 0; i < 16; i++ ) {
				solid(draw,fbuf[selOut],960,1420,y,y+y2);
				y=y+20;
				y2=y2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}
// 			x=1920;
			y=543;
			y2=1;




			for (i = 0; i < 16; i++ ) {
				solid(draw,fbuf[selOut],1440,1900,y,y+y2);
				y=y+20;
				y2=y2+1;
	
				msg=draw_rect_receive_message_hard(draw);
			}
// 
			solid(draw,fbuf[selOut],0,1920,1079,1080);
//	}
			if (output) {
				ps3fb_swapVsync();
			}
			if (update) {
// 				myarg->Outp0[0]=(ea_t)fbuf[selOut];
				selOut=selOut^1;
			}
// 			if (y < 1080) {
// 				y++;
// 			} else { 
// 				y=1;
// 			}
// 			if (x < 1920) {
// 				x++;
// 			} else { 
// 				x=1;
// 			}
			
		//	ps3fb_swap();
			
		//	destroy(bt[count]);
	//		send_message(bt[count],STOP);
			msg=STOP;
		}
	
	}
	Destination.write(RAMBuffer[0],srcW*srcH*4); // writes your new scaled YUV to a file!
	Destination.close();
	
// 	for (i=0;i<spes;i++){
// 		destroy(bt[i]);
// 	}
	ps3fb_cleanup();
	draw_rect_destroy(draw);

	return(0) ;
}
