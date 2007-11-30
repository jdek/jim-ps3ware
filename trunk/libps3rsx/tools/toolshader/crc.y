
%{
#include <stdio.h>
#include <string.h>
#include "nv40_shader.h"

#define uint32 unsigned int
#define uint32_t unsigned int

#include "../../src/shaders/vertex.h"
void yyerror(const char *str);




#define NV40_VP_DEST_MASK(x,y,z,w)  ((x<<3)|(y<<2)|(z<<1)|w)
#define NV40VP_MAKE_SWIZZLE(x, y, z, w) ((x<<6)|(y<<4)|(z<<2)|(w))

uint32 vp_inst[4];

uint32 inst_stack[256][4];

int vec;
int sca;
int swizzles[4];
int v;
int n;
int swnum;
int neg;
int numreg;
int dst_type;
int src_type;
int inst_ptr = 0;
int vp_out = 0;
int vp_in = 0;

const char *outfile;

#define OUT_REG_COL0   (1<<0)
#define OUT_REG_COL1   (1<<1)
#define OUT_REG_BFC0   (1<<2)
#define OUT_REG_BFC1   (1<<3)
#define OUT_REG_FOGC   (1<<4)
#define OUT_REG_PSIZ    (1<<5)
#define OUT_REG_TEX(N)   (1<<( 14 + N ) )


void out( uint32 feature )
{
	vp_out |= feature; 
}

void set_s0( uint32 src, uint32 abs )
{
	uint32 lo = (((src) & NV40_VP_SRC0_LOW_MASK) << NV40_VP_INST_SRC0L_SHIFT);
	uint32 hi = ((((src) & NV40_VP_SRC0_HIGH_MASK) >> NV40_VP_SRC0_HIGH_SHIFT) << NV40_VP_INST_SRC0H_SHIFT);
	vp_inst[0] = (vp_inst[0] & ~NV40_VP_INST_SRC0_ABS) | (abs ? NV40_VP_INST_SRC0_ABS : 0);
	vp_inst[1] = (vp_inst[1] & ~NV40_VP_INST_SRC0H_MASK) | hi;	
	vp_inst[2] = (vp_inst[2] & ~NV40_VP_INST_SRC0L_MASK) | lo;
 
}

void set_s1( uint32 src, uint32 abs )
{	
	vp_inst[0] = (vp_inst[0] & ~NV40_VP_INST_SRC1_ABS) | (abs ? NV40_VP_INST_SRC1_ABS : 0); \
 	vp_inst[2] = (vp_inst[2] & ~NV40_VP_INST_SRC1_MASK) | ((src) << NV40_VP_INST_SRC1_SHIFT); \
}

void set_s2( uint32 src, uint32 abs )
{	
	uint32 lo = (((src) & NV40_VP_SRC2_LOW_MASK) << NV40_VP_INST_SRC2L_SHIFT); 
	uint32 hi = ((((src) & NV40_VP_SRC2_HIGH_MASK) >> NV40_VP_SRC2_HIGH_SHIFT) << NV40_VP_INST_SRC2H_SHIFT);
   	vp_inst[0] = (vp_inst[0] & ~NV40_VP_INST_SRC2_ABS) | (abs ? NV40_VP_INST_SRC2_ABS : 0);\
  	vp_inst[2] = (vp_inst[2] & ~NV40_VP_INST_SRC2H_MASK) | hi;
	vp_inst[3] = (vp_inst[3] & ~NV40_VP_INST_SRC2L_MASK) | lo;
}

void set_s( uint32 src, uint32 abs )
{
	if( numreg == 0 )
	{
		set_s0( src, abs );
	}
	if( numreg == 1 )
	{
		set_s1( src, abs );
	}
	if( numreg == 2 )
	{
		set_s2( src, abs );
	}
	++numreg;
	neg = 0;
	swnum = 0;
}

uint32 get_swizzle()
{
	uint32 real_swizzle = 0;
	if( swnum == 0 )
	{
		real_swizzle = NV40VP_MAKE_SWIZZLE( 0, 1, 2, 3 );
	}
	else if( swnum == 1 )
	{
		real_swizzle = NV40VP_MAKE_SWIZZLE( swizzles[0], swizzles[0], swizzles[0], swizzles[0] );
	}
	else
	{
		real_swizzle = NV40VP_MAKE_SWIZZLE( swizzles[0], swizzles[1], swizzles[2], swizzles[3] );
	}

	swnum = 0;
	return real_swizzle;

}

void set_temp( uint32 id )
{
	uint32 s =  get_swizzle();
	uint32 src = (NV40_VP_SRC_REG_TYPE_TEMP<<NV40_VP_SRC_REG_TYPE_SHIFT);
	src |= ((id) << NV40_VP_SRC_TEMP_SRC_SHIFT);
	if( neg )
	{
		src |=  NV40_VP_SRC_NEGATE;
	}
	src |= ( s << NV40_VP_SRC_SWZ_ALL_SHIFT);
	printf( "...%s temp %4d swz %x %x %x %x \n", neg ? "-" : "+", id, ( s >> 6 ) & 3, ( s >> 4 ) & 3, ( s >> 2 ) & 3, ( s >> 0 ) & 3 );
	set_s( src, 0 );
}

void set_const( uint32 id )
{
	uint32 s =  get_swizzle();
	vp_inst[1] &= ~(NV40_VP_INST_CONST_SRC_MASK);
	vp_inst[1] |= ((id) << NV40_VP_INST_CONST_SRC_SHIFT); 
	
	uint32 src = (NV40_VP_SRC_REG_TYPE_CONST<<NV40_VP_SRC_REG_TYPE_SHIFT);
	if( neg )
	{
		src |=  NV40_VP_SRC_NEGATE;
	}
	src |= ( s << NV40_VP_SRC_SWZ_ALL_SHIFT);
	printf( "...%s cnst %4d swz %x %x %x %x \n", neg ? "-" : "+", id, ( s >> 6 ) & 3, ( s >> 4 ) & 3, ( s >> 2 ) & 3, ( s >> 0 ) & 3 );
	set_s( src, 0 );
}

void set_input( uint32 id )
{
	uint32 s =  get_swizzle();
	vp_inst[1] &= ~( NV40_VP_INST_INPUT_SRC_MASK );
	vp_inst[1] |= ((id) << NV40_VP_INST_INPUT_SRC_SHIFT);
	
	uint32 src = (NV40_VP_SRC_REG_TYPE_INPUT<<NV40_VP_SRC_REG_TYPE_SHIFT);
	if( neg )
	{
		src |=  NV40_VP_SRC_NEGATE;
	}
	src |= ( s << NV40_VP_SRC_SWZ_ALL_SHIFT);
	printf( "...%s inpt %4d swz %x %x %x %x \n", neg ? "-" : "+", id, ( s >> 6 ) & 3, ( s >> 4 ) & 3, ( s >> 2 ) & 3, ( s >> 0 ) & 3 );
	set_s( src, 0 );
}


void set( uint32 id, uint32 type )
{
	if( type == 0 )
	{
		set_temp( id );
	}
	if( type == 1 )
	{
		set_const( id );
	}
	if( type == 2 )
	{
		set_input( id );
	}
}

void dump()
{
	inst_stack[inst_ptr][0] = vp_inst[0];
	inst_stack[inst_ptr][1] = vp_inst[1];
	inst_stack[inst_ptr][2] = vp_inst[2];
	inst_stack[inst_ptr][3] = vp_inst[3];
	
	++inst_ptr;
	
	
}

void print()
{
	inst_stack[inst_ptr - 1][3] |= NV40_VP_INST_LAST;
	uint32 i;
	for( i = 0; i < inst_ptr; ++i )
	{
		printf( "%8x %8x %8x %8x \n", inst_stack[i][0], inst_stack[i][1], inst_stack[i][2], inst_stack[i][3]  );
	}
	printf( "vp_out %8x \n", vp_out );
	printf( "vp_in  %8x \n", vp_in );
	
	vertex_shader_desc_t desc;
	
	FILE *out = fopen( outfile, "wb" );
	if( out )
	{
	    desc.vp_in = vp_in;
	    desc.vp_out = vp_out;
	    desc.aux = 0xcafebabe;
	    desc.dword_length = inst_ptr * 4;
	    fwrite( &desc, sizeof( desc ), 1, out );
	    fwrite( &inst_stack[0][0], 4 * desc.dword_length, 1, out );
	    fclose( out );
	    	
	}
	inst_ptr = 0;
	vp_out = 0;
}

void clean()
{
	vp_inst[0] = 0x401f9c6c;
	vp_inst[1] = 0x0000000d;
	vp_inst[2] = 0x8106c083;
	vp_inst[3] = 0x60401f80;
	vec = 0;
	sca = 0;
	swnum = 0;
	neg = 0;
	numreg = 0;
}

void opv( uint32 op )
{
	vec = 1;
	printf( "opv code %x \n", op );
	vp_inst[1] &= ~NV40_VP_INST_VEC_OPCODE_MASK; 
  	vp_inst[1] |= ( op << NV40_VP_INST_VEC_OPCODE_SHIFT); 
}

void ops( uint32 op )
{
	sca = 1;
	printf( "ops code %x \n", op );
	vp_inst[1] &= ~NV40_VP_INST_SCA_OPCODE_MASK;
  	vp_inst[1] |= ( op << NV40_VP_INST_SCA_OPCODE_SHIFT);
}

void reg( uint32 dr, int res )
{
	if( vec )
	{
		if( res )
		{
			vp_inst[0] |= (NV40_VP_INST_VEC_DEST_TEMP_MASK | NV40_VP_INST_VEC_RESULT ); 
   			vp_inst[3] &= ~NV40_VP_INST_DEST_MASK; 
   			vp_inst[3] |= (dr << NV40_VP_INST_DEST_SHIFT); 
		}
		else
		{
			vp_inst[0] &= ~(NV40_VP_INST_VEC_DEST_TEMP_MASK | NV40_VP_INST_VEC_RESULT );
  			vp_inst[0] &= ~NV40_VP_INST_VEC_DEST_TEMP_MASK;
			vp_inst[0] |= (dr << NV40_VP_INST_VEC_DEST_TEMP_SHIFT);
		} 
	}
	if( sca )
	{
		if( res )
		{
			vp_inst[3] |= NV40_VP_INST_SCA_DEST_TEMP_MASK; 
   			vp_inst[3] &= ~NV40_VP_INST_DEST_MASK; 
   			vp_inst[3] |= (dr << NV40_VP_INST_DEST_SHIFT); 
		}
		{
			vp_inst[3] &= ~NV40_VP_INST_SCA_DEST_TEMP_MASK;
			vp_inst[3] &= ~NV40_VP_INST_SCA_DEST_TEMP_MASK; 
   			vp_inst[3] |= (dr << NV40_VP_INST_SCA_DEST_TEMP_SHIFT);
		}
	}
	printf( "%s register %x \n", res ? "tmp" : "out", dr );
	
	
}


void msk( uint32 m )
{
	if( vec )
	{
		vp_inst[3] &= ~NV40_VP_INST_VEC_WRITEMASK_MASK; 
   		vp_inst[3] |= (m << NV40_VP_INST_VEC_WRITEMASK_SHIFT); 
	
	}
	if( sca )
	{
		vp_inst[3] &= ~NV40_VP_INST_SCA_WRITEMASK_MASK; 
   		vp_inst[3] |= (m << NV40_VP_INST_SCA_WRITEMASK_SHIFT); 
	}

	printf( "msk %x %x %x %x ", ( m >> 3 ) & 1, ( m >> 2 ) & 1, ( m >> 1 ) & 1, ( m >> 0 ) & 1 );
}


void swz( uint32 co )
{
	swizzles[swnum] = co;
	++swnum;
}



%}

%token _0 _1 _2 _3 _4 _5 _6 _7 _8 _9  BEG END COM MARK ARL MOV LIT RCP RSQ EXP LOG MUL ADD DP3 DP4 DST MIN MAX SLT SGE MAD DOT PLS X Y Z W MNS PLS OPOS WGHT NRML COL0 COL1 FOGC TEX0 TEX1 TEX2 TEX3 TEX4 TEX5 TEX6 TEX7 ADDREG R HPOS BFC0 BFC1 FOGC PSIZ



%%
program: beg instructionSequence end { print(); };

instructionSequence: instructionSequence instructionLine |  instructionLine;

instructionLine: instruction MARK { dump(); clean(); };

instruction:   ARL_instruction
              | VECTORop_instruction
              | SCALARop_instruction
              | BINop_instruction
              | TRIop_instruction;

ARL_instruction:      
	ARL addrReg COM scalarSrcReg;

VECTORop_instruction:
	VECTORop maskedDstReg COM swizzleSrcReg;

SCALARop_instruction: 
	SCALARop maskedDstReg COM scalarSrcReg;

BINop_instruction: 
	BINop maskedDstReg COM swizzleSrcReg COM swizzleSrcReg;

TRIop_instruction: 
	TRIop maskedDstReg  COM swizzleSrcReg COM  swizzleSrcReg COM swizzleSrcReg;


scalarSrcReg:
	optionalSign srcReg scalarSuffix { set( v, src_type ); };

swizzleSrcReg:
	optionalSign srcReg swizzleSuffix { set( v, src_type ); };

maskedDstReg:
	dstReg optionalMask { reg( v, dst_type ); };

optionalMask:
	/* empty */ { msk( NV40_VP_DEST_MASK( 1, 1, 1, 1 ) ); }| 
	DOT X 	    { msk( NV40_VP_DEST_MASK( 1, 0, 0, 0 ) ); }|
	DOT Y 	    { msk( NV40_VP_DEST_MASK( 0, 1, 0, 0 ) ); }|
	DOT X Y     { msk( NV40_VP_DEST_MASK( 1, 1, 0, 0 ) ); }|
	DOT Z       { msk( NV40_VP_DEST_MASK( 0, 0, 1, 0 ) ); }|
	DOT X Z     { msk( NV40_VP_DEST_MASK( 1, 0, 1, 0 ) ); }|
	DOT Y Z     { msk( NV40_VP_DEST_MASK( 0, 1, 1, 0 ) ); }|
	DOT X Y Z   { msk( NV40_VP_DEST_MASK( 1, 1, 1, 0 ) ); }|
	DOT W 	    { msk( NV40_VP_DEST_MASK( 0, 0, 0, 1 ) ); }| 
	DOT X W     { msk( NV40_VP_DEST_MASK( 1, 0, 0, 1 ) ); }|
	DOT Y W     { msk( NV40_VP_DEST_MASK( 0, 1, 0, 1 ) ); }|
	DOT X Y W   { msk( NV40_VP_DEST_MASK( 1, 1, 0, 1 ) ); }|
	DOT Z  W    { msk( NV40_VP_DEST_MASK( 0, 0, 1, 1 ) ); }| 
	DOT X Z W   { msk( NV40_VP_DEST_MASK( 1, 0, 1, 1 ) ); }|
	DOT Y Z W   { msk( NV40_VP_DEST_MASK( 0, 1, 1, 1 ) ); }|
	DOT X Y Z W { msk( NV40_VP_DEST_MASK( 1, 1, 1, 1 ) ); };

optionalSign:       
	/* empty */ |
	'+'  |
	'-' { neg = 1; } ;
                            
srcReg:            
	vertexAttribReg { src_type = 2; }|
        progParamReg 	{ src_type = 1; }|
	temporaryRegSrc { src_type = 0; };

dstReg:
	temporaryRegDst { dst_type = 0; } |
        vertexResultReg { dst_type = 1; } ;

vertexAttribReg:      
	'v' '[' vertexAttribRegNum ']' { vp_in |= ( 1 << v ); };

vertexAttribRegNum: 
	decCalc |
	OPOS { v = 0; }|
        WGHT { v = 1; }|
	NRML { v = 2; }|
	COL0 { v = 3; }|
	COL1 { v = 4; }|
	FOGC { v = 5; }|
	TEX0 { v = 8; }|
	TEX1 { v = 9; }|
	TEX2 { v = 10; }|
	TEX3 { v = 11; }|
	TEX4 { v = 12; }|
	TEX5 { v = 13; }|
	TEX6 { v = 14; }|
	TEX7 { v = 15; };

progParamReg:
	absProgParamReg |
	relProgParamReg;

absProgParamReg:      
	'c' '[' progParamRegNum ']';
	
NUMBER:
	_0 { n = 0; } |
	_1 { n = 1; } |
	_2 { n = 2; } |
	_3 { n = 3; } |
	_4 { n = 4; } |
	_5 { n = 5; } |
	_6 { n = 6; } |
	_7 { n = 7; } |
	_8 { n = 8; } |
	_9 { n = 9; };
	    
		
	
decCalc:
	NUMBER { v = n; } |
	decCalc NUMBER { v = v * 10 + n; };    

progParamRegNum:
	decCalc;

relProgParamReg: 
	'c' '[' addrReg ']' | 
	'c' '[' addrReg PLS progParamPosOffset ']' |
	'c' '[' addrReg MNS progParamPosOffset ']';

progParamPosOffset:
	decCalc;

progParamNegOffset:
	decCalc;

addrReg:	
	ADDREG DOT X;

temporaryRegDst:         
	R decCalc;
	
temporaryRegSrc:
	R decCalc;
	
	
vertexResultReg:
	'o' '[' vertexResultRegName ']';

vertexResultRegName: 
	HPOS { v = 0; } |
       	COL0 { v = 1; out( OUT_REG_COL0 ); } |
	COL1 { v = 2; out( OUT_REG_COL1 ); } |
	BFC0 { v = 3; out( OUT_REG_BFC0 ); } |
	BFC1 { v = 4; out( OUT_REG_BFC1 ); } |
	FOGC { v = 5; out( OUT_REG_FOGC ); } |
	PSIZ { v = 6; out( OUT_REG_PSIZ ); }  |
	TEX0 { v = 7; out( OUT_REG_TEX( 0 ) ); } |
	TEX1 { v = 8; out( OUT_REG_TEX( 1 ) ); } |
	TEX2 { v = 9; out( OUT_REG_TEX( 2 ) ); } |
	TEX3 { v = 11; out( OUT_REG_TEX( 3 ) ); } |
	TEX4 { v = 11; out( OUT_REG_TEX( 4 ) );	} |
	TEX5 { v = 12; out( OUT_REG_TEX( 5 ) );	} |
	TEX6 { v = 13; out( OUT_REG_TEX( 6 ) );	} |
	TEX7 { v = 14; out( OUT_REG_TEX( 7 ) );	};

scalarSuffix:
	DOT component;

swizzleSuffix:
	/* empty */ |
	DOT component |
	DOT component component component component;
	
component:
	X { swz( 0 ); } |
	Y { swz( 1 ); } |
 	Z { swz( 2 ); } |
	W { swz( 3 ); }; 
	
VECTORop:
	MOV { opv( NV40_VP_INST_OP_MOV ); } | 
	LIT { opv( NV40_VP_INST_OP_LIT ); };
SCALARop:          
	RCP { ops( NV40_VP_INST_OP_RCP ); } | 
	RSQ { ops( NV40_VP_INST_OP_RSQ ); } | 
	EXP { ops( NV40_VP_INST_OP_EXP ); } | 
	LOG { ops( NV40_VP_INST_OP_LOG ); };

BINop:             
	MUL { opv( NV40_VP_INST_OP_MUL ); } | 
	ADD { opv( NV40_VP_INST_OP_ADD ); } | 
	DP3 { opv( NV40_VP_INST_OP_DP3 ); } | 
	DP4 { opv( NV40_VP_INST_OP_DP4 ); } | 
	DST { opv( NV40_VP_INST_OP_DST ); } | 
	MIN { opv( NV40_VP_INST_OP_MIN ); } | 
	MAX { opv( NV40_VP_INST_OP_MAX ); } | 
	SLT { opv( NV40_VP_INST_OP_SLT ); } | 
	SGE { opv( NV40_VP_INST_OP_SGE ); };

TRIop:             
	MAD { opv( NV40_VP_INST_OP_MAD ); };


            
beg:
        BEG{ printf("\tVP\n");  clean(); };
end:
        END{ printf("\tEND\n"); };
              
%%

void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n",str);
}
 
  
int yywrap()
{
        return 1;
} 
  
int main( int argn, const char *argv[] )
{
	if( argn != 2 )
	{
	    printf( "specify outfile \n" );
	    return 1;
	}
	outfile = argv[1];
        yyparse();
	return 0;
} 







