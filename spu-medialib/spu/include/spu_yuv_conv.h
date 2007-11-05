#ifndef __SPU_YUV_CONV_H
#define __SPU_YUV_CONV_H

static inline void yuv420_to_yuyv422(vector unsigned char *InputY0,vector unsigned char *InputY1,vector unsigned char *InputU,vector unsigned char *InputV, vector unsigned char *Output0,vector unsigned char *Output1, int width,int high)
{

	int i=0;
	int even=1;
	
	if (high)
		even=0;
	
	static const vector unsigned char h={0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
	static const vector unsigned char l={8,24,9,25,10,26,11,27,12,28,13,29,14,30,15,31};

	vector unsigned char tmpUV0;
	vector unsigned char tmpUV1;
	vector unsigned char U,V,Y0,Y1;
	vector unsigned char Out0,Out1,Out2,Out3;
	vector unsigned char nextU,nextV;
	U=InputU[0];
	V=InputV[0];
	Y0=InputY0[0];
	Y1=InputY1[1];
	
	nextU=U;
	nextV=V;
	
	for (i=0;i<(width>>4);i++)
	{
		if (!even)
		{
			nextU=InputU[(i+1+high)/2];
			nextV=InputV[(i+1+high)/2];
		}

		if (even)
		{
			tmpUV0=spu_shuffle(U,V,h);	
		} else { 
			tmpUV0=spu_shuffle(U,V,l);
		}

		vector unsigned char nextY0=InputY0[i+1];
		vector unsigned char nextY1=InputY1[i+1];

		Output0[2*i]=Out0;
		Output0[2*i+1]=Out1;
		Output1[2*i]=Out2;
		Output1[2*i+1]=Out3;

		Out0=spu_shuffle(Y0,tmpUV0,h);
		Out1=spu_shuffle(Y0,tmpUV0,l);
		Out2=spu_shuffle(Y1,tmpUV0,h);
		Out3=spu_shuffle(Y1,tmpUV0,l);
		Y0=nextY0;
		Y1=nextY1;
		U=nextU;
		V=nextV;
		even=even^1;
		
	}

}


static inline void yuv422_to_argb(vector unsigned char *Input,vector unsigned char *Output, int width)
{
	int i=0;
	const vector signed short max={255,255,255,255,255,255,255,255};
	const vector signed short min={0,0,0,0,0,0,0,0};
	
	const vector unsigned char RG={1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
	const vector unsigned char HRGB={0xc0,0,8,17,0xc0,1,9,19,0xc0,2,10,21,0xc0,3,11,23};
	const vector unsigned char LRGB={0xc0,4,12,25,0xc0,5,13,27,0xc0,6,14,29,0xc0,7,15,31};

	const vector unsigned char Yshuf={0x80,0,0x80,2,0x80,4,0x80,6,0x80,8,0x80,10,0x80,12,0x80,14};
	const vector unsigned char UVshuf={0x80,1,0x80,3,0x80,5,0x80,7,0x80,9,0x80,11,0x80,13,0x80,15};
	const vector unsigned char nothing={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//	vector unsigned char 
	const vector unsigned char ItoS={2,3,2,3,6,7,6,7,10,11,10,11,14,15,14,15};

	const static vector signed short s128={128,128,128,128,128,128,128,128};
	const static vector signed short s2871={2871,2871,2871,2871,2871,2871,2871,2871};	//1.402*2048
	const static vector signed short s705={705,705,705,705,705,705,705,705}; //0.344414*2048
	const static vector signed short s1463={1462,1462,1462,1462,1462,1462,1462,1462};//0.71414*2048
	const static vector signed short s3629={3629,3629,3629,3629,3629,3629,3629,3629};//1.722*2048
	//vector unsigned char Vshuf={0x80,3,0x80,3,0x80,7,0x80,7,0x80,11,0x80,11,0x80,15,0x80,15}
	vector unsigned char In;
	vector signed short Y;
	vector signed short UV,DE;
	vector signed int yfv,ufuv0,ufuv1,ufuv,vfu;
	vector signed short R,G,B;
	vector unsigned char tmp0;
	vector unsigned char Out0;
	vector unsigned char Out1;
	vector unsigned char tmp1,Out2,Out3;
	vector unsigned char In1=Input[1];
	vector signed short Y1,UV1,DE1;
	vector signed int yfv1,ufuv2,ufuv3,ufuv01,vfu1;
	vector signed short R1,G1,B1;
	vector unsigned short Rmax,Rmin,Gmax,Gmin,Bmax,Bmin,Rmax1,Rmin1,Gmax1,Gmin1,Bmax1,Bmin1;
	vector signed int Rshi,Gshi,Bshi, R1shi,G1shi,B1shi;
	vector signed short Rshu,Gshu,Bshu,R1shu,G1shu,B1shu;	
	int inc=0;
	int dest=1;

	In=Input[0];
	In1=Input[1];
	for (i=0;i<(width>>4) +1 ;i++ ) // this loop does 2 and 2 vectors
	{
		//start loop start pipe
		vector unsigned char nextYUV0=Input[2*i+2];
		vector unsigned char nextYUV1=Input[2*i+3];
		Output[dest*4]=Out0;
		Output[dest*4+1]=Out1;
		Output[dest*4+2]=Out2;
		Output[dest*4+3]=Out3;
		//end loop start pipe

		Y=(vector signed short)spu_shuffle(In,nothing,Yshuf);
		UV=(vector signed short)spu_shuffle(In,nothing,UVshuf);

		Y1=(vector signed short)spu_shuffle(In1,nothing,Yshuf);
		UV1=(vector signed short)spu_shuffle(In1,nothing,UVshuf);

		DE=spu_sub(UV,s128); // U And V -128
		DE1=spu_sub(UV1,s128); // U And V -128

		yfv=spu_mulo(DE,s2871);
		yfv1=spu_mulo(DE1,s2871);

		ufuv0=spu_mulo(DE,s1463);
		ufuv2=spu_mulo(DE1,s1463);

		ufuv1=spu_mule(DE,s705);
		ufuv3=spu_mule(DE1,s705);

		ufuv=spu_add(ufuv0,ufuv1);
		ufuv01=spu_add(ufuv2,ufuv3);

		vfu=spu_mule(DE,s3629);
		vfu1=spu_mule(DE1,s3629);

		Rshi=spu_rlmaska(yfv,-11);
		Gshi=spu_rlmaska(ufuv,-11);
		Bshi=spu_rlmaska(vfu,-11);	

		R1shi=spu_rlmaska(yfv1,-11);
		G1shi=spu_rlmaska(ufuv01,-11);
		B1shi=spu_rlmaska(vfu1,-11);	

		Rshu=(vector signed short)spu_shuffle((vector unsigned char)Rshi,nothing,ItoS);
		Gshu=(vector signed short)spu_shuffle((vector unsigned char)Gshi,nothing,ItoS);
		Bshu=(vector signed short)spu_shuffle((vector unsigned char)Bshi,nothing,ItoS);	

		R1shu=(vector signed short)spu_shuffle((vector unsigned char)R1shi,nothing,ItoS);
		G1shu=(vector signed short)spu_shuffle((vector unsigned char)G1shi,nothing,ItoS);
		B1shu=(vector signed short)spu_shuffle((vector unsigned char)B1shi,nothing,ItoS);	

		R=spu_add(Y,Rshu);
		G=spu_sub(Y,Gshu);
		B=spu_add(Y,Bshu);
		
		R1=spu_add(Y1,R1shu);
		G1=spu_sub(Y1,G1shu);
		B1=spu_add(Y1,B1shu);
		
		Rmax=spu_cmpgt(R,max);
		Rmin=spu_cmpgt(min,R);
		Gmax=spu_cmpgt(G,max);
		Gmin=spu_cmpgt(min,G);
		Bmax=spu_cmpgt(B,max);
		Bmin=spu_cmpgt(min,B);
		
		Rmax1=spu_cmpgt(R1,max);
		Rmin1=spu_cmpgt(min,R1);
		Gmax1=spu_cmpgt(G1,max);
		Gmin1=spu_cmpgt(min,G1);
		Bmax1=spu_cmpgt(B1,max);
		Bmin1=spu_cmpgt(min,B1);
	
		R=spu_sel(R,max,Rmax);
		R=spu_sel(R,min,Rmin);

		G=spu_sel(G,max,Gmax);
		G=spu_sel(G,min,Gmin);

		B=spu_sel(B,max,Bmax);
		B=spu_sel(B,min,Bmin);

		R1=spu_sel(R1,max,Rmax1);
		R1=spu_sel(R1,min,Rmin1);

		G1=spu_sel(G1,max,Gmax1);
		G1=spu_sel(G1,min,Gmin1);

		B1=spu_sel(B1,max,Bmax1);
		B1=spu_sel(B1,min,Bmin1);

		tmp0=spu_shuffle((vector unsigned char)R,(vector unsigned char)G,RG);
		tmp1=spu_shuffle((vector unsigned char)R1,(vector unsigned char)G1,RG);

		Out0=spu_shuffle(tmp0,(vector unsigned char)B,HRGB);
		Out1=spu_shuffle(tmp0,(vector unsigned char)B,LRGB);

		Out2=spu_shuffle(tmp1,(vector unsigned char)B1,HRGB);
		Out3=spu_shuffle(tmp1,(vector unsigned char)B1,LRGB);
		
		//start loop end pipe
		In=nextYUV0;
		In1=nextYUV1;
		dest=dest+inc;
		inc=1;
		//end loop end pipe
	}


}

static inline void yuv420_to_argb(vector unsigned char *InputY0,vector unsigned char *InputY1,vector unsigned char *InputU,vector unsigned char *InputV, vector unsigned char *Output0,vector unsigned char *Output1, int width,int high)
{
	int i=0;
	static const vector signed short max={255,255,255,255,255,255,255,255};
	static const vector signed short min={0,0,0,0,0,0,0,0};
	
	static const vector unsigned char RG={1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
	static const vector unsigned char HRGB={0xc0,0,8,17,0xc0,1,9,19,0xc0,2,10,21,0xc0,3,11,23};
	static const vector unsigned char LRGB={0xc0,4,12,25,0xc0,5,13,27,0xc0,6,14,29,0xc0,7,15,31};

	static const vector unsigned char Y0shuf={0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7};
	static const vector unsigned char Y1shuf={0x80,8,0x80,9,0x80,10,0x80,11,0x80,12,0x80,13,0x80,14,0x80,15};

	static const vector unsigned char UV0shuf={0x80,0,0x80,16,0x80,1,0x80,17,0x80,2,0x80,18,0x80,3,0x80,19};
	static const vector unsigned char UV1shuf={0x80,4,0x80,20,0x80,5,0x80,21,0x80,6,0x80,22,0x80,7,0x80,23};
	static const vector unsigned char UV2shuf={0x80,8,0x80,24,0x80,9,0x80,25,0x80,10,0x80,26,0x80,11,0x80,27};
	static const vector unsigned char UV3shuf={0x80,12,0x80,28,0x80,13,0x80,29,0x80,14,0x80,30,0x80,15,0x80,31};

	static const vector unsigned char nothing={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static const vector unsigned char ItoS={2,3,2,3,6,7,6,7,10,11,10,11,14,15,14,15};

	const static vector signed short s128={128,128,128,128,128,128,128,128};
	const static vector signed short s2871={2871,2871,2871,2871,2871,2871,2871,2871};	//1.402*2048
	const static vector signed short s705={705,705,705,705,705,705,705,705}; //0.344414*2048
	const static vector signed short s1463={1462,1462,1462,1462,1462,1462,1462,1462};//0.71414*2048
	const static vector signed short s3629={3629,3629,3629,3629,3629,3629,3629,3629};//1.722*2048

	vector unsigned char InY0,InY1,InU,InV;
	vector signed short Y0,Y1,Y2,Y3;//U,V;
	vector signed short UV,DE;
	vector signed int yfv,ufuv0,ufuv1,ufuv,vfu;
	vector signed short R0,G0,B0,R1,G1,B1,R2,G2,B2,R3,G3,B3;

	vector unsigned char tmp0,tmp1,tmp2,tmp3;

	vector signed short UV1,DE1;
	vector signed int yfv1,ufuv2,ufuv3,ufuv01,vfu1;
	vector unsigned short Rmax0,Rmin0,Gmax0,Gmin0,Bmax0,Bmin0,Rmax1,Rmin1,Gmax1,Gmin1,Bmax1,Bmin1;
	vector unsigned short Rmax2,Rmin2,Gmax2,Gmin2,Bmax2,Bmin2,Rmax3,Rmin3,Gmax3,Gmin3,Bmax3,Bmin3;
	vector signed int Rshi,Gshi,Bshi, R1shi,G1shi,B1shi;
	vector signed short Rshu,Gshu,Bshu,R1shu,G1shu,B1shu;	
	int even=1;
		
	if (high)
		even=0;

	InY0=InputY0[0];
	InY1=InputY1[0];
	InU=InputU[0];
	InV=InputV[0];

	for (i=0;i<(width>>4) ;i++ ) // this loop does 2 and 2 vectors
	{
	
		
		if (!even)
		{
			InU=InputU[(i+high)/2];
			InV=InputV[(i+high)/2];
		}


		Y0=(vector signed short)spu_shuffle(InY0,nothing,Y0shuf);
		Y1=(vector signed short)spu_shuffle(InY0,nothing,Y1shuf);
		
		Y2=(vector signed short)spu_shuffle(InY1,nothing,Y0shuf);
		Y3=(vector signed short)spu_shuffle(InY1,nothing,Y1shuf);
		
		if (even)
		{
			UV=(vector signed short)spu_shuffle(InU,InV,UV0shuf);
			UV1=(vector signed short)spu_shuffle(InU,InV,UV1shuf);
		} else {
			UV=(vector signed short)spu_shuffle(InU,InV,UV2shuf);
			UV1=(vector signed short)spu_shuffle(InU,InV,UV3shuf);	
		}

		
		DE=spu_sub(UV,s128); // U And V -128
		DE1=spu_sub(UV1,s128); // U And V -128

		yfv=spu_mulo(DE,s2871);
		yfv1=spu_mulo(DE1,s2871);

		ufuv0=spu_mulo(DE,s1463);
		ufuv2=spu_mulo(DE1,s1463);

		ufuv1=spu_mule(DE,s705);
		ufuv3=spu_mule(DE1,s705);

		ufuv=spu_add(ufuv0,ufuv1);
		ufuv01=spu_add(ufuv2,ufuv3);

		vfu=spu_mule(DE,s3629);
		vfu1=spu_mule(DE1,s3629);

		Rshi=spu_rlmaska(yfv,-11);
		Gshi=spu_rlmaska(ufuv,-11);
		Bshi=spu_rlmaska(vfu,-11);	

		R1shi=spu_rlmaska(yfv1,-11);
		G1shi=spu_rlmaska(ufuv01,-11);
		B1shi=spu_rlmaska(vfu1,-11);	

		Rshu=(vector signed short)spu_shuffle((vector unsigned char)Rshi,nothing,ItoS);
		Gshu=(vector signed short)spu_shuffle((vector unsigned char)Gshi,nothing,ItoS);
		Bshu=(vector signed short)spu_shuffle((vector unsigned char)Bshi,nothing,ItoS);	

		R1shu=(vector signed short)spu_shuffle((vector unsigned char)R1shi,nothing,ItoS);
		G1shu=(vector signed short)spu_shuffle((vector unsigned char)G1shi,nothing,ItoS);
		B1shu=(vector signed short)spu_shuffle((vector unsigned char)B1shi,nothing,ItoS);	

		R0=spu_add(Y0,Rshu);
		G0=spu_sub(Y0,Gshu);
		B0=spu_add(Y0,Bshu);
		
		R1=spu_add(Y1,R1shu);
		G1=spu_sub(Y1,G1shu);
		B1=spu_add(Y1,B1shu);
		
		R2=spu_add(Y2,Rshu);
		G2=spu_sub(Y2,Gshu);
		B2=spu_add(Y2,Bshu);
			
		R3=spu_add(Y3,R1shu);
		G3=spu_sub(Y3,G1shu);
		B3=spu_add(Y3,B1shu);
		
		Rmax0=spu_cmpgt(R0,max);
		Rmin0=spu_cmpgt(min,R0);
		Gmax0=spu_cmpgt(G0,max);
		Gmin0=spu_cmpgt(min,G0);
		Bmax0=spu_cmpgt(B0,max);
		Bmin0=spu_cmpgt(min,B0);
		
		Rmax1=spu_cmpgt(R1,max);
		Rmin1=spu_cmpgt(min,R1);
		Gmax1=spu_cmpgt(G1,max);
		Gmin1=spu_cmpgt(min,G1);
		Bmax1=spu_cmpgt(B1,max);
		Bmin1=spu_cmpgt(min,B1);
	
		Rmax2=spu_cmpgt(R2,max);
		Rmin2=spu_cmpgt(min,R2);
		Gmax2=spu_cmpgt(G2,max);
		Gmin2=spu_cmpgt(min,G2);
		Bmax2=spu_cmpgt(B2,max);
		Bmin2=spu_cmpgt(min,B2);	

		Rmax3=spu_cmpgt(R3,max);
		Rmin3=spu_cmpgt(min,R3);
		Gmax3=spu_cmpgt(G3,max);
		Gmin3=spu_cmpgt(min,G3);
		Bmax3=spu_cmpgt(B3,max);
		Bmin3=spu_cmpgt(min,B3);

		R0=spu_sel(R0,max,Rmax0);
		R0=spu_sel(R0,min,Rmin0);

		G0=spu_sel(G0,max,Gmax0);
		G0=spu_sel(G0,min,Gmin0);

		B0=spu_sel(B0,max,Bmax0);
		B0=spu_sel(B0,min,Bmin0);

		R1=spu_sel(R1,max,Rmax1);
		R1=spu_sel(R1,min,Rmin1);

		G1=spu_sel(G1,max,Gmax1);
		G1=spu_sel(G1,min,Gmin1);

		B1=spu_sel(B1,max,Bmax1);
		B1=spu_sel(B1,min,Bmin1);

		R2=spu_sel(R2,max,Rmax2);
		R2=spu_sel(R2,min,Rmin2);

		G2=spu_sel(G2,max,Gmax2);
		G2=spu_sel(G2,min,Gmin2);

		B2=spu_sel(B2,max,Bmax2);
		B2=spu_sel(B2,min,Bmin2);

		R3=spu_sel(R3,max,Rmax3);
		R3=spu_sel(R3,min,Rmin3);

		G3=spu_sel(G3,max,Gmax3);
		G3=spu_sel(G3,min,Gmin3);

		B3=spu_sel(B3,max,Bmax3);
		B3=spu_sel(B3,min,Bmin3);

		tmp0=spu_shuffle((vector unsigned char)R0,(vector unsigned char)G0,RG);
		tmp1=spu_shuffle((vector unsigned char)R1,(vector unsigned char)G1,RG);

		tmp2=spu_shuffle((vector unsigned char)R2,(vector unsigned char)G2,RG);
		tmp3=spu_shuffle((vector unsigned char)R3,(vector unsigned char)G3,RG);


		Output0[i*4]=spu_shuffle(tmp0,(vector unsigned char)B0,HRGB);
		Output0[i*4+1]=spu_shuffle(tmp0,(vector unsigned char)B0,LRGB);

		Output0[i*4+2]=spu_shuffle(tmp1,(vector unsigned char)B1,HRGB);
		Output0[i*4+3]=spu_shuffle(tmp1,(vector unsigned char)B1,LRGB);

		Output1[i*4]=spu_shuffle(tmp2,(vector unsigned char)B2,HRGB);
		Output1[i*4+1]=spu_shuffle(tmp2,(vector unsigned char)B2,LRGB);

		Output1[i*4+2]=spu_shuffle(tmp3,(vector unsigned char)B3,HRGB);
		Output1[i*4+3]=spu_shuffle(tmp3,(vector unsigned char)B3,LRGB);
		
		//start loop end pipe
		InY0=InputY0[i+1];
		InY1=InputY1[i+1];

		even=even^1;
		//end loop end pipe
	}


}

static inline void yuv420_vs_to_argb(vector short *InputY0,vector short*InputY1,vector short *InputU,vector short *InputV, vector unsigned char *Output0,vector unsigned char *Output1, int width)
{
	int i=0;
	static const vector signed short max={255,255,255,255,255,255,255,255};
	static const vector signed short min={0,0,0,0,0,0,0,0};
	
	static const vector unsigned char RG={1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
	static const vector unsigned char HRGB={0xc0,0,8,17,0xc0,1,9,19,0xc0,2,10,21,0xc0,3,11,23};
	static const vector unsigned char LRGB={0xc0,4,12,25,0xc0,5,13,27,0xc0,6,14,29,0xc0,7,15,31};

// 	static const vector unsigned char Y0shuf={0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7};
// 	static const vector unsigned char Y1shuf={0x80,8,0x80,9,0x80,10,0x80,11,0x80,12,0x80,13,0x80,14,0x80,15};
// 
// 	static const vector unsigned char UV0shuf={0x80,0,0x80,16,0x80,1,0x80,17,0x80,2,0x80,18,0x80,3,0x80,19};
// 	static const vector unsigned char UV1shuf={0x80,4,0x80,20,0x80,5,0x80,21,0x80,6,0x80,22,0x80,7,0x80,23};
// 	static const vector unsigned char UV2shuf={0x80,8,0x80,24,0x80,9,0x80,25,0x80,10,0x80,26,0x80,11,0x80,27};
// 	static const vector unsigned char UV3shuf={0x80,12,0x80,28,0x80,13,0x80,29,0x80,14,0x80,30,0x80,15,0x80,31};

	static const vector unsigned char UVshuf0={0,1,16,17,2,3,18,19,4,5,20,21,6,7,22,23};
	static const vector unsigned char UVshuf1={8,9,24,25,10,11,26,27,12,13,28,29,14,15,30,31};

	static const vector unsigned char nothing={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static const vector unsigned char ItoS={2,3,2,3,6,7,6,7,10,11,10,11,14,15,14,15};

	const static vector signed short s128={128,128,128,128,128,128,128,128};
	const static vector signed short s2871={2871,2871,2871,2871,2871,2871,2871,2871};	//1.402*2048
	const static vector signed short s705={705,705,705,705,705,705,705,705}; //0.344414*2048
	const static vector signed short s1463={1462,1462,1462,1462,1462,1462,1462,1462};//0.71414*2048
	const static vector signed short s3629={3629,3629,3629,3629,3629,3629,3629,3629};//1.722*2048

// 	vector unsigned char InY0,InY1,InY2,InY3,
	vector signed short InU,InV;
	vector signed short Y0,Y1,Y2,Y3;//U,V;
	vector signed short UV,DE;
	vector signed int yfv,ufuv0,ufuv1,ufuv,vfu;
	vector signed short R0,G0,B0,R1,G1,B1,R2,G2,B2,R3,G3,B3;

	vector unsigned char tmp0,tmp1,tmp2,tmp3;

	vector signed short UV1,DE1;
	vector signed int yfv1,ufuv2,ufuv3,ufuv01,vfu1;
	vector unsigned short Rmax0,Rmin0,Gmax0,Gmin0,Bmax0,Bmin0,Rmax1,Rmin1,Gmax1,Gmin1,Bmax1,Bmin1;
	vector unsigned short Rmax2,Rmin2,Gmax2,Gmin2,Bmax2,Bmin2,Rmax3,Rmin3,Gmax3,Gmin3,Bmax3,Bmin3;
	vector signed int Rshi,Gshi,Bshi, R1shi,G1shi,B1shi;
	vector signed short Rshu,Gshu,Bshu,R1shu,G1shu,B1shu;	
	int even=1;
		
// 	if (high)
// 		even=0;

// 	InY0=InputY0[0];
// 	InY1=InputY1[0];
// 	InU=InputU[0];
// 	InV=InputV[0];

// 	InY0=InputY0[0];
// 	InY1=InputY0[1];
// 	InY2=InputY1[0];
// 	InY3=InputY1[1];

	for (i=0;i<(width>>4) ;i++ ) // this loop does 2 and 2 vectors
	{
	
		
	//	if (!even)
	//	{
	//		InU=InputU[(i+high)/2];
	//		InV=InputV[(i+high)/2];
	//	}

		Y0=InputY0[2*i];
		Y1=InputY0[2*i+1];
		Y2=InputY1[2*i];
		Y3=InputY1[2*i+1];
		InU=InputU[i];
		InV=InputV[i];
/*
		Y0=InY0;
		Y1=InY1;
		Y2=InY2;
		Y3=InY3;*/
// 		Y0=(vector signed short)spu_shuffle(InY0,nothing,Y0shuf);
// 		Y1=(vector signed short)spu_shuffle(InY0,nothing,Y1shuf);
// 		
// 		Y2=(vector signed short)spu_shuffle(InY1,nothing,Y0shuf);
// 		Y3=(vector signed short)spu_shuffle(InY1,nothing,Y1shuf);
		
	//	if (even)
	//	{
		UV=(vector signed short)spu_shuffle(InU,InV,UVshuf0);
		UV1=(vector signed short)spu_shuffle(InU,InV,UVshuf1);
	//	} else {
	//		UV=(vector signed short)spu_shuffle(InU,InV,UV2shuf);
	//		UV1=(vector signed short)spu_shuffle(InU,InV,UV3shuf);	
	//	}

		
		DE=spu_sub(UV,s128); // U And V -128
		DE1=spu_sub(UV1,s128); // U And V -128

		yfv=spu_mulo(DE,s2871);
		yfv1=spu_mulo(DE1,s2871);

		ufuv0=spu_mulo(DE,s1463);
		ufuv2=spu_mulo(DE1,s1463);

		ufuv1=spu_mule(DE,s705);
		ufuv3=spu_mule(DE1,s705);

		ufuv=spu_add(ufuv0,ufuv1);
		ufuv01=spu_add(ufuv2,ufuv3);

		vfu=spu_mule(DE,s3629);
		vfu1=spu_mule(DE1,s3629);

		Rshi=spu_rlmaska(yfv,-11);
		Gshi=spu_rlmaska(ufuv,-11);
		Bshi=spu_rlmaska(vfu,-11);	

		R1shi=spu_rlmaska(yfv1,-11);
		G1shi=spu_rlmaska(ufuv01,-11);
		B1shi=spu_rlmaska(vfu1,-11);	

		Rshu=(vector signed short)spu_shuffle((vector unsigned char)Rshi,nothing,ItoS);
		Gshu=(vector signed short)spu_shuffle((vector unsigned char)Gshi,nothing,ItoS);
		Bshu=(vector signed short)spu_shuffle((vector unsigned char)Bshi,nothing,ItoS);	

		R1shu=(vector signed short)spu_shuffle((vector unsigned char)R1shi,nothing,ItoS);
		G1shu=(vector signed short)spu_shuffle((vector unsigned char)G1shi,nothing,ItoS);
		B1shu=(vector signed short)spu_shuffle((vector unsigned char)B1shi,nothing,ItoS);	

		R0=spu_add(Y0,Rshu);
		G0=spu_sub(Y0,Gshu);
		B0=spu_add(Y0,Bshu);
		
		R1=spu_add(Y1,R1shu);
		G1=spu_sub(Y1,G1shu);
		B1=spu_add(Y1,B1shu);
		
		R2=spu_add(Y2,Rshu);
		G2=spu_sub(Y2,Gshu);
		B2=spu_add(Y2,Bshu);
			
		R3=spu_add(Y3,R1shu);
		G3=spu_sub(Y3,G1shu);
		B3=spu_add(Y3,B1shu);
		
		Rmax0=spu_cmpgt(R0,max);
		Rmin0=spu_cmpgt(min,R0);
		Gmax0=spu_cmpgt(G0,max);
		Gmin0=spu_cmpgt(min,G0);
		Bmax0=spu_cmpgt(B0,max);
		Bmin0=spu_cmpgt(min,B0);
		
		Rmax1=spu_cmpgt(R1,max);
		Rmin1=spu_cmpgt(min,R1);
		Gmax1=spu_cmpgt(G1,max);
		Gmin1=spu_cmpgt(min,G1);
		Bmax1=spu_cmpgt(B1,max);
		Bmin1=spu_cmpgt(min,B1);
	
		Rmax2=spu_cmpgt(R2,max);
		Rmin2=spu_cmpgt(min,R2);
		Gmax2=spu_cmpgt(G2,max);
		Gmin2=spu_cmpgt(min,G2);
		Bmax2=spu_cmpgt(B2,max);
		Bmin2=spu_cmpgt(min,B2);	

		Rmax3=spu_cmpgt(R3,max);
		Rmin3=spu_cmpgt(min,R3);
		Gmax3=spu_cmpgt(G3,max);
		Gmin3=spu_cmpgt(min,G3);
		Bmax3=spu_cmpgt(B3,max);
		Bmin3=spu_cmpgt(min,B3);

		R0=spu_sel(R0,max,Rmax0);
		R0=spu_sel(R0,min,Rmin0);

		G0=spu_sel(G0,max,Gmax0);
		G0=spu_sel(G0,min,Gmin0);

		B0=spu_sel(B0,max,Bmax0);
		B0=spu_sel(B0,min,Bmin0);

		R1=spu_sel(R1,max,Rmax1);
		R1=spu_sel(R1,min,Rmin1);

		G1=spu_sel(G1,max,Gmax1);
		G1=spu_sel(G1,min,Gmin1);

		B1=spu_sel(B1,max,Bmax1);
		B1=spu_sel(B1,min,Bmin1);

		R2=spu_sel(R2,max,Rmax2);
		R2=spu_sel(R2,min,Rmin2);

		G2=spu_sel(G2,max,Gmax2);
		G2=spu_sel(G2,min,Gmin2);

		B2=spu_sel(B2,max,Bmax2);
		B2=spu_sel(B2,min,Bmin2);

		R3=spu_sel(R3,max,Rmax3);
		R3=spu_sel(R3,min,Rmin3);

		G3=spu_sel(G3,max,Gmax3);
		G3=spu_sel(G3,min,Gmin3);

		B3=spu_sel(B3,max,Bmax3);
		B3=spu_sel(B3,min,Bmin3);

		tmp0=spu_shuffle((vector unsigned char)R0,(vector unsigned char)G0,RG);
		tmp1=spu_shuffle((vector unsigned char)R1,(vector unsigned char)G1,RG);

		tmp2=spu_shuffle((vector unsigned char)R2,(vector unsigned char)G2,RG);
		tmp3=spu_shuffle((vector unsigned char)R3,(vector unsigned char)G3,RG);


		Output0[i*4]=spu_shuffle(tmp0,(vector unsigned char)B0,HRGB);
		Output0[i*4+1]=spu_shuffle(tmp0,(vector unsigned char)B0,LRGB);

		Output0[i*4+2]=spu_shuffle(tmp1,(vector unsigned char)B1,HRGB);
		Output0[i*4+3]=spu_shuffle(tmp1,(vector unsigned char)B1,LRGB);

		Output1[i*4]=spu_shuffle(tmp2,(vector unsigned char)B2,HRGB);
		Output1[i*4+1]=spu_shuffle(tmp2,(vector unsigned char)B2,LRGB);

		Output1[i*4+2]=spu_shuffle(tmp3,(vector unsigned char)B3,HRGB);
		Output1[i*4+3]=spu_shuffle(tmp3,(vector unsigned char)B3,LRGB);
		
// 		//start loop end pipe
// 		InY0=InputY0[2*i+2];
// 		InY1=InputY0[2*i+3];
// 		InY1=InputY1[2*i+2];
// 		InY1=InputY1[2*i+3];
		even=even^1;
		//end loop end pipe
	}


}

static inline void global_blend(vector unsigned char *Input,vector unsigned char *Output, uint8_t blend,int bytes)
{
	int i;

	static const vector unsigned char nothing={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	static const vector unsigned char H={0x80,0,0x80,1,0x80,2,0x80,3,0x80,4,0x80,5,0x80,6,0x80,7};
	static const vector unsigned char L={0x80,8,0x80,9,0x80,10,0x80,11,0x80,12,0x80,13,0x80,14,0x80,15};
	vector signed short plus={blend,blend,blend,blend,blend,blend,blend,blend};
	vector signed short minus=spu_sub(((vector signed short){255,255,255,255,255,255,255,255}),plus);

	for( i=0;i<(bytes>>4) + 1;i++)
	{
 		vector signed short tmpI0=(vector signed short)spu_shuffle(Input[i],nothing,H);
 		vector signed short tmpI1=(vector signed short)spu_shuffle(Input[i],nothing,L);
 		vector signed short tmpO0=(vector signed short)spu_shuffle(Output[i],nothing,H);
 		vector signed short tmpO1=(vector signed short)spu_shuffle(Output[i],nothing,L);	
		
		vector signed int InEven0=spu_mule(tmpI0,plus);
		vector signed int InOdd0=spu_mulo(tmpI0,plus);
		vector signed int InEven1=spu_mule(tmpI1,plus);
		vector signed int InOdd1=spu_mulo(tmpI1,plus);	

		vector signed int OutEven0=spu_mule(tmpO0,minus);
		vector signed int OutOdd0=spu_mulo(tmpO0,minus);
		vector signed int OutEven1=spu_mule(tmpO1,minus);
		vector signed int OutOdd1=spu_mulo(tmpO1,minus);

		vector signed int newOut0=spu_add(InEven0,OutEven0);
		vector signed int newOut1=spu_add(InOdd0,OutOdd0);
		vector signed int newOut2=spu_add(InEven1,OutEven1);
		vector signed int newOut3=spu_add(InOdd1,OutOdd1);	

		newOut0=spu_rlmaska(newOut0,-8); //TODO shuffeling these back into shorts before rightshift will save some 
		newOut1=spu_rlmaska(newOut1,-8);
		newOut2=spu_rlmaska(newOut2,-8);
		newOut3=spu_rlmaska(newOut3,-8);			

		vector unsigned char tmpOut0=(vector unsigned char)spu_shuffle(newOut0,newOut1,((vector unsigned char){3,19,7,23,8,11,27,15,31,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80}));
		vector unsigned char tmpOut1=(vector unsigned char)spu_shuffle(newOut2,newOut3,((vector unsigned char){3,19,7,23,8,11,27,15,31,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80}));
		Output[i]=spu_shuffle(tmpOut0,tmpOut1,((vector unsigned char){0,1,2,3,4,5,6,7,15,16,17,18,19,20,21,22,23}));


	}

}

#endif

