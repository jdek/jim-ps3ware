#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>
#include "mpeg2.h"

using namespace std;



int main (int nArg, char* cArg[]) {

	if (nArg <2) 
	{
		printf("Missing Filename\n");
		return -1;
	}
	if (nArg ==2)
		printf("Opening file %s\n",cArg[1]);

	char buffer[2][4096];
	int currentbuff=0;

//	ifstream source;
//	mp2dec.source.open(cArg[1],ios::binary|ios::in);
	mpeg2_dec_t *mp2dec=open_file(cArg[1]);

	if (!(is_open(mp2dec)))
	{
		printf("error opening file %s\n",cArg[1]);
		return -1;
	}
	
	int count=0;
	int B;
	do
	{
		if (is_end(mp2dec))
			break;
		do {
			if (is_end(mp2dec))
				break;

			B=nextbyte(mp2dec);
			if ((B==0) && (count <2))
				count++;
			if ((B==1) && (count ==2))
				break;
			if ((B != 0 ) && (B != 1))
				count=0;

			
		}while(1);
		B=nextbyte(mp2dec);
		switch (B)
		{
			case 0xb3: 
		//		printf("sequence start\n");
				sequence_start(mp2dec);
				break;
			case 0xb2:
		//		printf("User data start\n");
				break;
			case 0x00:
		//		printf("Picture start\n");
				break;
			case 0xb7:
		//		printf("Sequence end\n");
				break;
			case 0xb8:
		//		printf("group start code\n");
				break;
			default : 
		//		printf("Slice_start_code %#x\n",B);
				break;
		}

	} while (1);
	
	close_file(mp2dec);
}


