#ifndef __MPEG2_H
#define __MPEG2_H
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <malloc.h>
using namespace std;

extern "C"


struct mpeg2_dec_s;
typedef struct mpeg2_dec_s mpeg2_dec_t;

int is_open(mpeg2_dec_t*);
int is_end(mpeg2_dec_t*);
char nextbyte(mpeg2_dec_t*);
mpeg2_dec_t* open_file(char* filename);
void close_file(mpeg2_dec_t*);
void sequence_start(mpeg2_dec_t*);

static char * frame_rate_str[16] = {
	"Invalid frame_rate_code",
	"23.976", "24", "25" , "29.97",
	"30" , "50", "59.94", "60" ,
	"Invalid frame_rate_code", "Invalid frame_rate_code",
	"Invalid frame_rate_code", "Invalid frame_rate_code",
	"Invalid frame_rate_code", "Invalid frame_rate_code",
	"Invalid frame_rate_code"
};

static char * aspect_ratio_str[16] = {
	"Forbidden", "1.0", "4:3", "16:9" , "1:2.21",
	"Reserved" , "Reserved", "Reserved", "Reserved" ,
	"Reserved" , "Reserved", "Reserved", "Reserved" ,
	"Reserved" , "Reserved", "Reserved" 
};

#endif
