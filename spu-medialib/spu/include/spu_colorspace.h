/**
 * SPU YUV to RGB conversion kernel
 * --------------------------------
 * Licensed under the BSDv2
 *
 * spu_colorspace.h - SPU YUV to RGB conversion kernel
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


#ifndef __SPU_COLORSPACE_H
#define __SPU_COLORSPACE_H

#include <spu_pack.h>
#include <spu_extfunct.h>


static inline void YUV2RGB(vector float Y,vector float U, vector float V ,vector float *R,vector float *G, vector float *B )
{
		vector float E=spu_sub(V,((vector float){128.0,128.0,128.0,128.0}));
		vector float D=spu_sub(U,((vector float){128.0,128.0,128.0,128.0}));

		R[0]=spu_add(Y,spu_mul(E,((vector float){1.402,1.402,1.402,1.402})));
		G[0]=spu_sub(Y,spu_add((spu_mul(D,((vector float){0.34414,0.34414,0.34414,0.34414}))),(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414})))));
		B[0]=spu_add(Y,spu_mul(D,((vector float) {1.772,1.772,1.772,1.772})));
}


static inline vector unsigned char packfARGB(vector float R, vector float G, vector float B)
{
		vector unsigned int max = spu_splats((unsigned int)0x00FF);
		static vector unsigned char Rff={0,19,2,3,4,23,6,7,8,27,10,11,12,31,14,15};
		static vector unsigned char Gff={0,1,19,3,4,5,23,7,8,9 ,27,11,12,13,31,15};
		vector unsigned int Ri=spu_convtu(R,0);
		Ri= spu_sel(Ri, max, spu_cmpgt(Ri,255));
		vector unsigned int Gi=spu_convtu(G,0);
		Gi= spu_sel(Gi, max, spu_cmpgt(Gi,255));
		vector unsigned int Bi=spu_convtu(B,0);
		Bi= spu_sel(Bi, max, spu_cmpgt(Bi,255));
		vector unsigned char ARGB;
		ARGB=spu_shuffle((vector unsigned char)Bi,(vector unsigned char)Ri,Rff);
		ARGB=spu_shuffle(ARGB,(vector unsigned char)Gi,Gff);
		return ARGB;
}

/*static inline void yuv420toARGB(vector unsigned char *Y,vector unsigned char *U, vector unsigned char *V,vector unsigned char *ARGB, int width,int maxwidth,int LSB)
{	
	static vector unsigned char splitvchar[2];
	splitvchar[0]=(vector unsigned char){0x00,0,0x00,1,0x00,2,0x00,3,0x00,4,0x00,5,0x00,6,0x00,7};
	splitvchar[1]=(vector unsigned char){0x00,8,0x00,9,0x00,10,0x00,11,0x00,12,0x00,13,0x00,14,0x00,15};
	static vector unsigned char splitvcharh={0,1,0,1,2,3,2,3,4,5,4,5,6,7,6,7};
	static vector unsigned char splitvcharl={8,9,8,9,10,11,10,11,12,13,12,13,14,15,14,15};
	static vector unsigned char GBh={0x00,0x00,1,17,0x00,0x00,3,19,0x00,0x00,5,21,0x00,0x00,7,23};
	static vector unsigned char GBl={0x00,0x00,9,25,0x00,0x00,11,27,0x00,0x00,13,29,0x00,0x00,15,31};
	static vector unsigned char Rh={0,17,2,3,4,19,6,7,8,21,10,11,12,23,14,15};
	static vector unsigned char Rl={0,25,2,3,4,27,6,7,8,29,10,11,12,31,14,15};
	vector signed short s128={128,128,128,128,128,128,128,128};//128..
	vector signed short s2871={2871,2871,2871,2871,2871,2871,2871,2871}; //1.402 <<11
	vector signed short s705={705,705,705,705,705,705,705,705};//0.3441
	static vector signed short s1462={1462,1462,1462,1462,1462,1462,1462,1462};//0.7171
	vector signed short s3629={3629,3629,3629,3629,3629,3629,3629,3629};//1.77
	vector signed short max={255,255,255,255,255,255,255,255};
	vector signed short min={0,0,0,0,0,0,0,0};

	int i;
	vector signed short E,D;
	vector signed short Yf00,Yf01,Yf10,Yf11;
	vector signed short Uf0,Vf0;

	vector signed short yfv0,ufuv0,ufuvx,vfu0;

	vector signed short yfv0h,ufuv0h,vfu0h,yfv0l,ufuv0l,vfu0l;
	vector signed short R00,B00,G00;
	vector signed short R01,B01,G01;
	vector signed short R10,B10,G10;
	vector signed short R11,B11,G11;
	
	vector unsigned char GB00,GB01,GB02,GB03,GB10,GB11,GB12,GB13;

	vector signed short RC00,RC01,RC10,RC11,BC00,BC01,BC10,BC11,GC00,GC01,GC10,GC11;
	
	for (i=0; i< (width>>4);i++)
	{
		Uf0=(vector signed short)spu_shuffle(U[i],U[i],splitvchar[LSB]);
		Vf0=(vector signed short)spu_shuffle(V[i],V[i],splitvchar[LSB]);
		E=spu_sub(Uf0,s128);
		D=spu_sub(Vf0,s128);
		Yf00=(vector signed short)spu_shuffle(Y[i],Y[i],splitvchar[0]);
		Yf01=(vector signed short)spu_shuffle(Y[i],Y[i],splitvchar[1]);
		Yf10=(vector signed short)spu_shuffle(Y[i+width>>4],Y[i+width>>4],splitvchar[0]);
		Yf11=(vector signed short)spu_shuffle(Y[i+width>>4],Y[i+width>>4],splitvchar[1]);

		//ufuvx=spu_smul(E,s1462);
		ufuvx=spu_smul(E,s1462);
		yfv0=spu_smul(E,s2871);
		vfu0=spu_smul(D,s3629);
		ufuv0=spu_smadd(D,s705,ufuvx);

		
		yfv0h=spu_shuffle(yfv0,yfv0,splitvcharh);
		ufuv0h=spu_shuffle(ufuv0,ufuv0,splitvcharh);
		vfu0h=spu_shuffle(vfu0,vfu0,splitvcharh);

		yfv0l=spu_shuffle(yfv0,yfv0,splitvcharl);
		ufuv0l=spu_shuffle(ufuv0,ufuv0,splitvcharl);
		vfu0l=spu_shuffle(vfu0,vfu0,splitvcharl);

		
		R00=spu_add(Yf00,yfv0h);
		B00=spu_add(Yf00,ufuv0h);
		G00=spu_add(Yf00,vfu0h);

		R10=spu_add(Yf10,yfv0h);
		B10=spu_add(Yf10,ufuv0h);
		G10=spu_add(Yf10,vfu0h);

		R01=spu_add(Yf01,yfv0l);
		B01=spu_add(Yf01,ufuv0l);
		G01=spu_add(Yf01,vfu0l);

		R11=spu_add(Yf11,yfv0l);
		B11=spu_add(Yf11,ufuv0l);
		G11=spu_add(Yf11,vfu0l);

		RC00=spu_sel(R00, max, spu_cmpgt(R00,255));
		RC00=spu_sel(min, RC00, spu_cmpgt(RC00,0));

		RC01=spu_sel(R01, max, spu_cmpgt(R01,255));
		RC01=spu_sel(min, RC01, spu_cmpgt(RC01,0));

		RC10=spu_sel(R10, max, spu_cmpgt(R10,255));
		RC10=spu_sel(min, RC00, spu_cmpgt(RC10,0));

		RC11=spu_sel(R11, max, spu_cmpgt(R11,255));
		RC11=spu_sel(min, RC11, spu_cmpgt(RC11,0));

		GC00=spu_sel(G00, max, spu_cmpgt(G00,255));
		GC00=spu_sel(min, GC00, spu_cmpgt(GC00,0));

		GC01=spu_sel(G01, max, spu_cmpgt(G01,255));
		GC01=spu_sel(min, GC01, spu_cmpgt(GC01,0));

		GC10=spu_sel(G10, max, spu_cmpgt(G10,255));
		GC10=spu_sel(min, GC00, spu_cmpgt(GC10,0));

		GC11=spu_sel(G11, max, spu_cmpgt(G11,255));
		GC11=spu_sel(min, GC11, spu_cmpgt(GC11,0));

		BC00=spu_sel(B00, max, spu_cmpgt(B00,255));
		BC00=spu_sel(min, BC00, spu_cmpgt(BC00,0));

		BC01=spu_sel(B01, max, spu_cmpgt(B01,255));
		BC01=spu_sel(min, BC01, spu_cmpgt(BC01,0));

		BC10=spu_sel(B10, max, spu_cmpgt(B10,255));
		BC10=spu_sel(min, BC00, spu_cmpgt(BC10,0));

		BC11=spu_sel(B11, max, spu_cmpgt(B11,255));
		BC11=spu_sel(min, BC11, spu_cmpgt(BC11,0));
			
		GB00=spu_shuffle((vector unsigned char)GC00,(vector unsigned char)BC00,GBh);
		GB01=spu_shuffle((vector unsigned char)GC00,(vector unsigned char)BC00,GBl);
		GB02=spu_shuffle((vector unsigned char)GC01,(vector unsigned char)BC01,GBh);
		GB03=spu_shuffle((vector unsigned char)GC01,(vector unsigned char)BC01,GBl);
		ARGB[i*4]=spu_shuffle((vector unsigned char)GB00,(vector unsigned char)RC00,Rh);
		ARGB[i*4 + 1]=spu_shuffle((vector unsigned char)GB01,(vector unsigned char)RC00,Rl);
		ARGB[i*4 + 2]=spu_shuffle((vector unsigned char)GB02,(vector unsigned char)RC01,Rh);
		ARGB[i*4 + 3]=spu_shuffle((vector unsigned char)GB03,(vector unsigned char)RC01,Rl);
		
		GB10=spu_shuffle((vector unsigned char)GC10,(vector unsigned char)BC10,GBh);
		GB11=spu_shuffle((vector unsigned char)GC10,(vector unsigned char)BC10,GBl);
		GB12=spu_shuffle((vector unsigned char)GC11,(vector unsigned char)BC11,GBh);
		GB13=spu_shuffle((vector unsigned char)GC11,(vector unsigned char)BC11,GBl);
		
	//	ARGB[i*4 +    (maxwidth>>2)]=spu_shuffle((vector unsigned char)GB10,(vector unsigned char)RC10,Rh);
	//	ARGB[i*4 + 1 +(maxwidth>>2)]=spu_shuffle((vector unsigned char)GB11,(vector unsigned char)RC10,Rl);
	//	ARGB[i*4 + 2 +(maxwidth>>2)]=spu_shuffle((vector unsigned char)GB12,(vector unsigned char)RC11,Rh);
	//	ARGB[i*4 + 3 +(maxwidth>>2)]=spu_shuffle((vector unsigned char)GB13,(vector unsigned char)RC11,Rl);
		
		LSB=LSB^1;
	}

}*/

/*static inline void yuv420toARGB(vector unsigned char *Y,vector unsigned char *U, vector unsigned char *V,vector unsigned char *ARGB, int width,int maxwidth,int LSB)
{

		//static vector unsigned int max= spu_splats((unsigned int)0x00FF);
		static vector unsigned int max = {0x00ff,0x00ff,0x00ff,0x00ff};
		static vector unsigned char Rff={0,19,2,3,4,23,6,7,8,27,10,11,12,31,14,15};
		static vector unsigned char Gff={0,1,19,3,4,5,23,7,8,9 ,27,11,12,13,31,15};

		vector float Yf0,Yf1,Yf2,Yf3,Yf4,Yf5,Yf6,Yf7;
		vector float Uf0,Uf1;
		vector float Vf0,Vf1;
		vector float R00,R01,R02,R03,R10,R11,R12,R13,G00,G01,G02,G03,G10,G11,G12,G13,B00,B01,B02,B03,B10,B11,B12,B13;
		vector unsigned int RI00,RI01,RI02,RI03,RI10,RI11,RI12,RI13,GI00,GI01,GI02,GI03,GI10,GI11,GI12,GI13,BI00,BI01,BI02,BI03,BI10,BI11,BI12,BI13;
		vector float yfv0,ufuv0,vfu0,yfv1,ufuv1,vfu1;
		vector float E0,D0,E1,D1;
		vector unsigned char temp00,temp01,temp02,temp03,temp10,temp11,temp12,temp13;
		vector unsigned char Y0,Y1,U0,V0;
		vector unsigned char nextU0,nextV0;
		int i;
		U0=U[0];
		V0=V[0];
		nextV0=V0;
		nextU0=U0;
		int dest=0;
		int inc=0;
		Y0=Y[0];
		Y1=Y[width>>4];
		for (i =0;i < (width>>4) + 1;i++) {
			ARGB[dest*4    ]=temp00;
			ARGB[dest*4 + 1]=temp01;
			ARGB[dest*4 + 2]=temp02;
			ARGB[dest*4 + 3]=temp03;
			ARGB[dest*4 +    (maxwidth>>2)]=temp10;
			ARGB[dest*4 + 1 +(maxwidth>>2)]=temp11;
			ARGB[dest*4 + 2 +(maxwidth>>2)]=temp12;
			ARGB[dest*4 + 3 +(maxwidth>>2)]=temp13;
			vector unsigned char nextY0=Y[i+1];
			vector unsigned char nextY1=Y[i+1+ (width>>4)];


			Yf0=unpackhh(Y0);
			Yf1=unpacklh(Y0);
			Yf2=unpackhl(Y0);
			Yf3=unpackll(Y0);
			Yf4=unpackhh(Y1);
			Yf5=unpacklh(Y1);
			Yf6=unpackhl(Y1);
			Yf7=unpackll(Y1);
			
			if (LSB)
			{ 
				LSB=LSB^1;
				Uf0=unpackhl(U0);
				Uf1=unpackll(U0);
				Vf0=unpackhl(V0);
				Vf1=unpackll(V0);				
			        nextU0=U[(i+1)>>1];
			        nextV0=V[(i+1)>>1];
			}
			else
			{
				Uf0=unpackhh(U0);
				Uf1=unpacklh(U0);
				Vf0=unpackhh(V0);
				Vf1=unpacklh(V0);
				LSB=LSB^1;				
			}
			
// 			if ( LSB ) {
// 				Uf0=unpackhl(U[0]);
// 				Uf1=unpackll(U[0]);
// 				Vf0=unpackhl(V[0]);
// 				Vf1=unpackll(V[0]);
// 				LSB=LSB^1;
// 				//U++;
// 				//V++;
// 
// 			} else {
// 
// 				Uf0=unpackhh(U[0]);
// 				Uf1=unpacklh(U[0]);
// 				Vf0=unpackhh(V[0]);
// 				Vf1=unpacklh(V[0]);
// 				LSB=LSB^1;
// 			}

			E0=spu_sub(Vf0,((vector float){128.0,128.0,128.0,128.0}));
			D0=spu_sub(Uf0,((vector float){128.0,128.0,128.0,128.0}));

			yfv0=spu_mul(E0,((vector float){1.402,1.402,1.402,1.402}));
			
			ufuv0=spu_madd(D0,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E0,((vector float){0.71414,0.71414,0.71414,0.71414}))));
			
			vfu0=spu_mul(D0,((vector float) {1.772,1.772,1.772,1.772}));

			R00=spu_add(Yf0,unpackfaabb(yfv0));
			G00=spu_sub(Yf0,unpackfaabb(ufuv0));
			B00=spu_add(Yf0,unpackfaabb(vfu0));


			R10=spu_add(Yf4,unpackfaabb(yfv0));
			G10=spu_sub(Yf4,unpackfaabb(ufuv0));
			B10=spu_add(Yf4,unpackfaabb(vfu0));
		
		
			R01=spu_add(Yf1,unpackfccdd(yfv0));
			G01=spu_sub(Yf1,unpackfccdd(ufuv0));
			B01=spu_add(Yf1,unpackfccdd(vfu0));


			R11=spu_add(Yf5,unpackfccdd(yfv0));
			G11=spu_sub(Yf5,unpackfccdd(ufuv0));
			B11=spu_add(Yf5,unpackfccdd(vfu0));


			E1=spu_sub(Vf1,((vector float){128.0,128.0,128.0,128.0}));
			D1=spu_sub(Uf1,((vector float){128.0,128.0,128.0,128.0}));

			yfv1=spu_mul(E1,((vector float){1.402,1.402,1.402,1.402}));

			ufuv1=spu_madd(D1,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E1,((vector float){0.71414,0.71414,0.71414,0.71414}))));
	
			vfu1=spu_mul(D1,((vector float) {1.772,1.772,1.772,1.772}));

			R02=spu_add(Yf2,unpackfaabb(yfv1));
			G02=spu_sub(Yf2,unpackfaabb(ufuv1));
			B02=spu_add(Yf2,unpackfaabb(vfu1));

			R12=spu_add(Yf6,unpackfaabb(yfv1));
			G12=spu_sub(Yf6,unpackfaabb(ufuv1));
			B12=spu_add(Yf6,unpackfaabb(vfu1));


		
			R03=spu_add(Yf3,unpackfccdd(yfv1));
			G03=spu_sub(Yf3,unpackfccdd(ufuv1));
			B03=spu_add(Yf3,unpackfccdd(vfu1));
	


			R13=spu_add(Yf7,unpackfccdd(yfv1));
			G13=spu_sub(Yf7,unpackfccdd(ufuv1));
			B13=spu_add(Yf7,unpackfccdd(vfu1));
	
			RI00=spu_convtu(R00,0);	
			RI01=spu_convtu(R01,0);	
			RI02=spu_convtu(R02,0);	
			RI03=spu_convtu(R03,0);	
			RI10=spu_convtu(R10,0);	
			RI11=spu_convtu(R11,0);	
			RI12=spu_convtu(R12,0);		
			RI13=spu_convtu(R13,0);	

			GI00=spu_convtu(G00,0);	
			GI01=spu_convtu(G01,0);	
			GI02=spu_convtu(G02,0);	
			GI03=spu_convtu(G03,0);	
			GI10=spu_convtu(G10,0);	
			GI11=spu_convtu(G11,0);	
			GI12=spu_convtu(G12,0);		
			GI13=spu_convtu(G13,0);		

			BI00=spu_convtu(B00,0);	
			BI01=spu_convtu(B01,0);	
			BI02=spu_convtu(B02,0);	
			BI03=spu_convtu(B03,0);	
			BI10=spu_convtu(B10,0);	
			BI11=spu_convtu(B11,0);	
			BI12=spu_convtu(B12,0);		
			BI13=spu_convtu(B13,0);		
	

			RI00=spu_sel(RI00, max, spu_cmpgt(RI00,255));	
			RI01=spu_sel(RI01, max, spu_cmpgt(RI01,255));
			RI02=spu_sel(RI02, max, spu_cmpgt(RI02,255));
			RI03=spu_sel(RI03, max, spu_cmpgt(RI03,255));
			RI10=spu_sel(RI10, max, spu_cmpgt(RI10,255));
			RI11=spu_sel(RI11, max, spu_cmpgt(RI11,255));
			RI12=spu_sel(RI12, max, spu_cmpgt(RI12,255));	
			RI13=spu_sel(RI13, max, spu_cmpgt(RI13,255));			

			GI00=spu_sel(GI00, max, spu_cmpgt(GI00,255));	
			GI01=spu_sel(GI01, max, spu_cmpgt(GI01,255));
			GI02=spu_sel(GI02, max, spu_cmpgt(GI02,255));
			GI03=spu_sel(GI03, max, spu_cmpgt(GI03,255));
			GI10=spu_sel(GI10, max, spu_cmpgt(GI10,255));
			GI11=spu_sel(GI11, max, spu_cmpgt(GI11,255));
			GI12=spu_sel(GI12, max, spu_cmpgt(GI12,255));	
			GI13=spu_sel(GI13, max, spu_cmpgt(GI13,255));	

			BI00=spu_sel(BI00, max, spu_cmpgt(BI00,255));	
			BI01=spu_sel(BI01, max, spu_cmpgt(BI01,255));
			BI02=spu_sel(BI02, max, spu_cmpgt(BI02,255));
			BI03=spu_sel(BI03, max, spu_cmpgt(BI03,255));
			BI10=spu_sel(BI10, max, spu_cmpgt(BI10,255));
			BI11=spu_sel(BI11, max, spu_cmpgt(BI11,255));
			BI12=spu_sel(BI12, max, spu_cmpgt(BI12,255));	
			BI13=spu_sel(BI13, max, spu_cmpgt(BI13,255));	

			temp00=spu_shuffle((vector unsigned char)BI00,(vector unsigned char)RI00,Rff);
			temp01=spu_shuffle((vector unsigned char)BI01,(vector unsigned char)RI01,Rff);
			temp02=spu_shuffle((vector unsigned char)BI02,(vector unsigned char)RI02,Rff);
			temp03=spu_shuffle((vector unsigned char)BI03,(vector unsigned char)RI03,Rff);
			temp10=spu_shuffle((vector unsigned char)BI10,(vector unsigned char)RI10,Rff);
			temp11=spu_shuffle((vector unsigned char)BI11,(vector unsigned char)RI11,Rff);
			temp12=spu_shuffle((vector unsigned char)BI12,(vector unsigned char)RI12,Rff);
			temp13=spu_shuffle((vector unsigned char)BI13,(vector unsigned char)RI13,Rff);

			temp00=spu_shuffle(temp00,(vector unsigned char)GI00,Gff);
			temp01=spu_shuffle(temp01,(vector unsigned char)GI01,Gff);
			temp02=spu_shuffle(temp02,(vector unsigned char)GI02,Gff);
			temp03=spu_shuffle(temp03,(vector unsigned char)GI03,Gff);
			temp10=spu_shuffle(temp10,(vector unsigned char)GI10,Gff);
			temp11=spu_shuffle(temp11,(vector unsigned char)GI11,Gff);
			temp12=spu_shuffle(temp12,(vector unsigned char)GI12,Gff);
			temp13=spu_shuffle(temp13,(vector unsigned char)GI13,Gff);

// 			temp00=packfARGB(R00,G00,B00);
// 			temp01=packfARGB(R01,G01,B01);
// 			temp02=packfARGB(R02,G02,B02);
// 			temp03=packfARGB(R03,G03,B03);
// 			temp10=packfARGB(R10,G10,B10);
// 			temp11=packfARGB(R11,G11,B11);
// 			temp12=packfARGB(R12,G12,B12);
// 			temp13=packfARGB(R13,G13,B13);
			Y0=nextY0;
			Y1=nextY1;
			U0=nextU0;
			V0=nextV0;
			dest=dest + inc;
			inc=1;
		}
}*/
static inline void yuv420toARGB(vector unsigned char *Y,vector unsigned char *U, vector unsigned char *V,vector unsigned char *ARGB, int width,int maxwidth,int LSB)
{

		vector float Yf0,Yf1,Yf2,Yf3,Yf4,Yf5,Yf6,Yf7;
		vector float Uf0,Uf1;
		vector float Vf0,Vf1;
		vector float R00,R01,R02,R03,R10,R11,R12,R13,G00,G01,G02,G03,G10,G11,G12,G13,B00,B01,B02,B03,B10,B11,B12,B13;
		vector float yfv,ufuv,vfu;
		vector float E,D;
		vector unsigned char temp00,temp01,temp02,temp03,temp10,temp11,temp12,temp13;
		vector unsigned char Y0,Y1;
		int i;
		
		int dest=0;
		int inc=0;
		Y0=Y[0];
		Y1=Y[width>>4];
		for (i =0;i < (width>>4) + 1;i++) {
			ARGB[dest*4    ]=temp00;
			ARGB[dest*4 + 1]=temp01;
			ARGB[dest*4 + 2]=temp02;
			ARGB[dest*4 + 3]=temp03;
			ARGB[dest*4 +    (maxwidth>>2)]=temp10;
			ARGB[dest*4 + 1 +(maxwidth>>2)]=temp11;
			ARGB[dest*4 + 2 +(maxwidth>>2)]=temp12;
			ARGB[dest*4 + 3 +(maxwidth>>2)]=temp13;
			vector unsigned char nextY0=Y[i+1];
			vector unsigned char nextY1=Y[i+1+ (width>>4)];

			Yf0=unpackhh(Y0);
			Yf1=unpacklh(Y0);
			Yf2=unpackhl(Y0);
			Yf3=unpackll(Y0);
			Yf4=unpackhh(Y1);
			Yf5=unpacklh(Y1);
			Yf6=unpackhl(Y1);
			Yf7=unpackll(Y1);
	
			if ( LSB ) {
				Uf0=unpackhl(U[0]);
				Uf1=unpackll(U[0]);
				Vf0=unpackhl(V[0]);
				Vf1=unpackll(V[0]);
				LSB=LSB^1;


			} else {

				Uf0=unpackhh(U[0]);
				Uf1=unpacklh(U[0]);
				Vf0=unpackhh(V[0]);
				Vf1=unpacklh(V[0]);
				LSB=LSB^1;
				U++;
				V++;
			}

			E=spu_sub(Vf0,((vector float){128.0,128.0,128.0,128.0}));
			D=spu_sub(Uf0,((vector float){128.0,128.0,128.0,128.0}));

			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));
			
			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
			
			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));

			R00=spu_add(Yf0,unpackfaabb(yfv));
			G00=spu_sub(Yf0,unpackfaabb(ufuv));
			B00=spu_add(Yf0,unpackfaabb(vfu));


			R10=spu_add(Yf4,unpackfaabb(yfv));
			G10=spu_sub(Yf4,unpackfaabb(ufuv));
			B10=spu_add(Yf4,unpackfaabb(vfu));
		
		
			R01=spu_add(Yf1,unpackfccdd(yfv));
			G01=spu_sub(Yf1,unpackfccdd(ufuv));
			B01=spu_add(Yf1,unpackfccdd(vfu));


			R11=spu_add(Yf5,unpackfccdd(yfv));
			G11=spu_sub(Yf5,unpackfccdd(ufuv));
			B11=spu_add(Yf5,unpackfccdd(vfu));


			E=spu_sub(Vf1,((vector float){128.0,128.0,128.0,128.0}));
			D=spu_sub(Uf1,((vector float){128.0,128.0,128.0,128.0}));

			yfv=spu_mul(E,((vector float){1.402,1.402,1.402,1.402}));

			ufuv=spu_madd(D,((vector float){0.34414,0.34414,0.34414,0.34414}) ,(spu_mul(E,((vector float){0.71414,0.71414,0.71414,0.71414}))));
	
			vfu=spu_mul(D,((vector float) {1.772,1.772,1.772,1.772}));

			R02=spu_add(Yf2,unpackfaabb(yfv));
			G02=spu_sub(Yf2,unpackfaabb(ufuv));
			B02=spu_add(Yf2,unpackfaabb(vfu));

			R12=spu_add(Yf6,unpackfaabb(yfv));
			G12=spu_sub(Yf6,unpackfaabb(ufuv));
			B12=spu_add(Yf6,unpackfaabb(vfu));


		
			R03=spu_add(Yf3,unpackfccdd(yfv));
			G03=spu_sub(Yf3,unpackfccdd(ufuv));
			B03=spu_add(Yf3,unpackfccdd(vfu));
	


			R13=spu_add(Yf7,unpackfccdd(yfv));
			G13=spu_sub(Yf7,unpackfccdd(ufuv));
			B13=spu_add(Yf7,unpackfccdd(vfu));
	

			temp00=packfARGB(R00,G00,B00);
			temp01=packfARGB(R01,G01,B01);
			temp02=packfARGB(R02,G02,B02);
			temp03=packfARGB(R03,G03,B03);
			temp10=packfARGB(R10,G10,B10);
			temp11=packfARGB(R11,G11,B11);
			temp12=packfARGB(R12,G12,B12);
			temp13=packfARGB(R13,G13,B13);
			Y0=nextY0;
			Y1=nextY1;
			dest=dest + inc;
			inc=1;
		}
}

#endif
