#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
#line 2 "crc.y"

#include <stdio.h>
#include <string.h>
#include "nv40_shader.h"
void yyerror(const char *str);

#define uint32 unsigned int


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
	inst_ptr = 0;
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



#line 291 "y.tab.c"
#define YYERRCODE 256
#define _0 257
#define _1 258
#define _2 259
#define _3 260
#define _4 261
#define _5 262
#define _6 263
#define _7 264
#define _8 265
#define _9 266
#define BEG 267
#define END 268
#define COM 269
#define MARK 270
#define ARL 271
#define MOV 272
#define LIT 273
#define RCP 274
#define RSQ 275
#define EXP 276
#define LOG 277
#define MUL 278
#define ADD 279
#define DP3 280
#define DP4 281
#define DST 282
#define MIN 283
#define MAX 284
#define SLT 285
#define SGE 286
#define MAD 287
#define DOT 288
#define PLS 289
#define X 290
#define Y 291
#define Z 292
#define W 293
#define MNS 294
#define OPOS 295
#define WGHT 296
#define NRML 297
#define COL0 298
#define COL1 299
#define FOGC 300
#define TEX0 301
#define TEX1 302
#define TEX2 303
#define TEX3 304
#define TEX4 305
#define TEX5 306
#define TEX6 307
#define TEX7 308
#define ADDREG 309
#define R 310
#define HPOS 311
#define BFC0 312
#define BFC1 313
#define PSIZ 314
const short yylhs[] = {                                        -1,
    0,    2,    2,    4,    5,    5,    5,    5,    5,    6,
    7,    8,    9,   10,   12,   15,   14,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   19,   19,   19,   20,   20,   20,   23,
   23,   25,   30,   30,   30,   30,   30,   30,   30,   30,
   30,   30,   30,   30,   30,   30,   30,   26,   26,   32,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   31,   31,   34,   33,   33,   33,   36,   37,   11,   28,
   27,   29,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   21,   22,   22,
   22,   39,   39,   39,   39,   13,   13,   16,   16,   16,
   16,   17,   17,   17,   17,   17,   17,   17,   17,   17,
   18,    1,    3,
};
const short yylen[] = {                                         2,
    3,    2,    1,    2,    1,    1,    1,    1,    1,    4,
    4,    4,    6,    8,    3,    3,    2,    0,    2,    2,
    3,    2,    3,    3,    4,    2,    3,    3,    4,    3,
    4,    4,    5,    0,    1,    1,    1,    1,    1,    1,
    1,    4,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    4,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    2,    1,    4,    6,    6,    1,    1,    3,    2,
    2,    4,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    0,    2,
    5,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,
};
const short yydefred[] = {                                      0,
  122,    0,    0,    0,  106,  107,  108,  109,  110,  111,
  112,  113,  114,  115,  116,  117,  118,  119,  120,  121,
    0,    3,    0,    5,    6,    7,    8,    9,    0,    0,
    0,    0,    0,    0,  123,    1,    2,    4,    0,    0,
    0,    0,   40,   41,    0,    0,    0,    0,    0,   61,
   62,   63,   64,   65,   66,   67,   68,   69,   70,    0,
   71,    0,    0,    0,   17,    0,    0,    0,   79,   35,
   36,   10,    0,   72,   84,   85,   88,   90,   91,   92,
   93,   94,   95,   96,   97,   83,   86,   87,   89,    0,
   11,    0,    0,    0,    0,   26,   12,    0,    0,    0,
    0,    0,    0,   37,   38,   39,   58,   59,   82,    0,
    0,    0,   27,    0,   28,   30,    0,    0,    0,    0,
    0,    0,   15,    0,   16,    0,   29,   31,   32,   13,
    0,   44,   45,   46,   47,   48,   49,   50,   51,   52,
   53,   54,   55,   56,   57,    0,    0,    0,    0,    0,
  102,  103,  104,  105,   98,    0,   33,    0,   42,    0,
    0,   74,   60,    0,   14,    0,    0,    0,    0,   75,
   76,  101,
};
const short yydgoto[] = {                                       2,
    3,   21,   36,   22,   23,   24,   25,   26,   27,   28,
   34,   72,   29,   41,   91,   30,   31,   32,   92,  103,
  123,  125,   42,   65,  104,  105,  106,   43,   44,  146,
  166,  107,  108,  150,   61,  167,    0,   90,  155,
};
const short yysindex[] = {                                   -237,
    0,    0, -143, -291,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -160,    0, -219,    0,    0,    0,    0,    0, -108, -108,
 -108, -108, -221, -207,    0,    0,    0,    0, -159,   -4,
 -179, -196,    0,    0, -172, -113, -112, -181,    9,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -159,
    0, -153,    9, -264,    0,    9,    9,    9,    0,    0,
    0,    0,  -99,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   69,
    0,  -99, -243, -228, -130,    0,    0, -105, -104, -159,
   75,   76, -120,    0,    0,    0,    0,    0,    0, -119,
 -209, -123,    0, -122,    0,    0,    9,    9, -159, -226,
 -249, -246,    0, -246,    0, -121,    0,    0,    0,    0,
  -96,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   81, -159,  -91, -159,   82,
    0,    0,    0,    0,    0, -246,    0,    9,    0, -159,
 -159,    0,    0, -246,    0, -159,   83,   84, -246,    0,
    0,    0,
};
const short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -90,    0,    0,    0,    0,    0,    0,  -98,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -245,
    0,    0,  -98,    0,    0,  -98,  -98,  -98,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -89,  -88,  -87,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -184,
  -86,  -85,    0,  -84,    0,    0,  -98,  -98, -247,    0,
    0,    0,    0,    0,    0,  -83,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   85,    0,   94,    0,
    0,    0,    0,    0,    0, -175,    0,  -98,    0,    0,
    0,    0,    0,    0,    0,   95,    0,    0,    0,    0,
    0,    0,
};
const short yygindex[] = {                                      0,
    0,    0,    0,  168,    0,    0,    0,    0,    0,    0,
   70,  124,    0,   27,  -62,    0,    0,    0,  -24,  100,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -32,    0,    0,    0,  -56,   32,    0,    0, -103,
};
#define YYTABLESIZE 212
const short yytable[] = {                                     102,
   34,  162,   40,   74,   98,   99,   60,   50,   51,   52,
   53,   54,   55,   56,   57,   58,   59,   33,  101,   34,
  156,   81,   81,   80,   73,   93,   94,   95,   96,    1,
   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,
   81,   73,   80,  151,  152,  153,  154,  111,  112,  113,
   38,   70,  164,   71,  130,  131,   45,   46,   47,   33,
  169,   49,   74,  114,  115,  172,   48,  119,  132,  133,
  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
  144,  145,  126,  127,   99,   99,   62,  147,  149,   63,
   74,   64,   74,  100,  100,  165,   66,   50,   51,   52,
   53,   54,   55,   56,   57,   58,   59,   35,   69,   74,
    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
   14,   15,   16,   17,   18,   19,   20,    4,    5,    6,
    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
   17,   18,   19,   20,   75,   76,   77,   78,   79,   80,
   81,   82,   83,   84,   85,   67,   68,   86,   87,   88,
   89,  109,  116,  117,  118,  120,  121,  122,  124,  128,
  129,  157,  158,  159,  163,  170,  171,   43,   18,   19,
   20,   22,   21,   23,   24,   25,   73,   77,   37,   97,
  148,  110,  168,    0,    0,    0,    0,  160,    0,    0,
    0,   39,  161,    0,    0,    0,    0,    0,    0,    0,
  100,   34,
};
const short yycheck[] = {                                      99,
   99,   93,  111,   60,   67,   68,   39,  257,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  309,  118,  118,
  124,  269,  270,  269,   49,  290,  291,  292,  293,  267,
  257,  258,  259,  260,  261,  262,  263,  264,  265,  266,
  288,   66,  288,  290,  291,  292,  293,  291,  292,  293,
  270,   43,  156,   45,  117,  118,   30,   31,   32,  309,
  164,  269,  119,  292,  293,  169,  288,  100,  295,  296,
  297,  298,  299,  300,  301,  302,  303,  304,  305,  306,
  307,  308,  292,  293,  269,  270,   91,  120,  121,  269,
  147,  288,  149,  269,  270,  158,  269,  257,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  268,  290,  166,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,  285,  286,  287,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,  285,  286,  287,  298,  299,  300,  301,  302,  303,
  304,  305,  306,  307,  308,  269,  269,  311,  312,  313,
  314,   93,  293,  269,  269,   91,   91,  288,  288,  293,
  293,  293,  269,   93,   93,   93,   93,   93,  269,  269,
  269,  269,  269,  269,  269,  269,   93,   93,   21,   66,
  121,   92,  161,   -1,   -1,   -1,   -1,  289,   -1,   -1,
   -1,  310,  294,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  310,  310,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 314
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,"'+'",0,"'-'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,0,0,0,0,"'c'",0,0,0,0,
0,0,0,0,0,0,0,"'o'",0,0,0,0,0,0,"'v'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"_0",
"_1","_2","_3","_4","_5","_6","_7","_8","_9","BEG","END","COM","MARK","ARL",
"MOV","LIT","RCP","RSQ","EXP","LOG","MUL","ADD","DP3","DP4","DST","MIN","MAX",
"SLT","SGE","MAD","DOT","PLS","X","Y","Z","W","MNS","OPOS","WGHT","NRML","COL0",
"COL1","FOGC","TEX0","TEX1","TEX2","TEX3","TEX4","TEX5","TEX6","TEX7","ADDREG",
"R","HPOS","BFC0","BFC1","PSIZ",
};
const char * const yyrule[] = {
"$accept : program",
"program : beg instructionSequence end",
"instructionSequence : instructionSequence instructionLine",
"instructionSequence : instructionLine",
"instructionLine : instruction MARK",
"instruction : ARL_instruction",
"instruction : VECTORop_instruction",
"instruction : SCALARop_instruction",
"instruction : BINop_instruction",
"instruction : TRIop_instruction",
"ARL_instruction : ARL addrReg COM scalarSrcReg",
"VECTORop_instruction : VECTORop maskedDstReg COM swizzleSrcReg",
"SCALARop_instruction : SCALARop maskedDstReg COM scalarSrcReg",
"BINop_instruction : BINop maskedDstReg COM swizzleSrcReg COM swizzleSrcReg",
"TRIop_instruction : TRIop maskedDstReg COM swizzleSrcReg COM swizzleSrcReg COM swizzleSrcReg",
"scalarSrcReg : optionalSign srcReg scalarSuffix",
"swizzleSrcReg : optionalSign srcReg swizzleSuffix",
"maskedDstReg : dstReg optionalMask",
"optionalMask :",
"optionalMask : DOT X",
"optionalMask : DOT Y",
"optionalMask : DOT X Y",
"optionalMask : DOT Z",
"optionalMask : DOT X Z",
"optionalMask : DOT Y Z",
"optionalMask : DOT X Y Z",
"optionalMask : DOT W",
"optionalMask : DOT X W",
"optionalMask : DOT Y W",
"optionalMask : DOT X Y W",
"optionalMask : DOT Z W",
"optionalMask : DOT X Z W",
"optionalMask : DOT Y Z W",
"optionalMask : DOT X Y Z W",
"optionalSign :",
"optionalSign : '+'",
"optionalSign : '-'",
"srcReg : vertexAttribReg",
"srcReg : progParamReg",
"srcReg : temporaryRegSrc",
"dstReg : temporaryRegDst",
"dstReg : vertexResultReg",
"vertexAttribReg : 'v' '[' vertexAttribRegNum ']'",
"vertexAttribRegNum : decCalc",
"vertexAttribRegNum : OPOS",
"vertexAttribRegNum : WGHT",
"vertexAttribRegNum : NRML",
"vertexAttribRegNum : COL0",
"vertexAttribRegNum : COL1",
"vertexAttribRegNum : FOGC",
"vertexAttribRegNum : TEX0",
"vertexAttribRegNum : TEX1",
"vertexAttribRegNum : TEX2",
"vertexAttribRegNum : TEX3",
"vertexAttribRegNum : TEX4",
"vertexAttribRegNum : TEX5",
"vertexAttribRegNum : TEX6",
"vertexAttribRegNum : TEX7",
"progParamReg : absProgParamReg",
"progParamReg : relProgParamReg",
"absProgParamReg : 'c' '[' progParamRegNum ']'",
"NUMBER : _0",
"NUMBER : _1",
"NUMBER : _2",
"NUMBER : _3",
"NUMBER : _4",
"NUMBER : _5",
"NUMBER : _6",
"NUMBER : _7",
"NUMBER : _8",
"NUMBER : _9",
"decCalc : NUMBER",
"decCalc : decCalc NUMBER",
"progParamRegNum : decCalc",
"relProgParamReg : 'c' '[' addrReg ']'",
"relProgParamReg : 'c' '[' addrReg PLS progParamPosOffset ']'",
"relProgParamReg : 'c' '[' addrReg MNS progParamPosOffset ']'",
"progParamPosOffset : decCalc",
"progParamNegOffset : decCalc",
"addrReg : ADDREG DOT X",
"temporaryRegDst : R decCalc",
"temporaryRegSrc : R decCalc",
"vertexResultReg : 'o' '[' vertexResultRegName ']'",
"vertexResultRegName : HPOS",
"vertexResultRegName : COL0",
"vertexResultRegName : COL1",
"vertexResultRegName : BFC0",
"vertexResultRegName : BFC1",
"vertexResultRegName : FOGC",
"vertexResultRegName : PSIZ",
"vertexResultRegName : TEX0",
"vertexResultRegName : TEX1",
"vertexResultRegName : TEX2",
"vertexResultRegName : TEX3",
"vertexResultRegName : TEX4",
"vertexResultRegName : TEX5",
"vertexResultRegName : TEX6",
"vertexResultRegName : TEX7",
"scalarSuffix : DOT component",
"swizzleSuffix :",
"swizzleSuffix : DOT component",
"swizzleSuffix : DOT component component component component",
"component : X",
"component : Y",
"component : Z",
"component : W",
"VECTORop : MOV",
"VECTORop : LIT",
"SCALARop : RCP",
"SCALARop : RSQ",
"SCALARop : EXP",
"SCALARop : LOG",
"BINop : MUL",
"BINop : ADD",
"BINop : DP3",
"BINop : DP4",
"BINop : DST",
"BINop : MIN",
"BINop : MAX",
"BINop : SLT",
"BINop : SGE",
"TRIop : MAD",
"beg : BEG",
"end : END",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
#line 484 "crc.y"


void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n",str);
}
 
  
int yywrap()
{
        return 1;
} 
  
main()
{
        yyparse();
} 







#line 704 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 282 "crc.y"
{ print(); }
break;
case 4:
#line 286 "crc.y"
{ dump(); clean(); }
break;
case 15:
#line 311 "crc.y"
{ set( v, src_type ); }
break;
case 16:
#line 314 "crc.y"
{ set( v, src_type ); }
break;
case 17:
#line 317 "crc.y"
{ reg( v, dst_type ); }
break;
case 18:
#line 320 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 1, 1, 1 ) ); }
break;
case 19:
#line 321 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 0, 0, 0 ) ); }
break;
case 20:
#line 322 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 1, 0, 0 ) ); }
break;
case 21:
#line 323 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 1, 0, 0 ) ); }
break;
case 22:
#line 324 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 0, 1, 0 ) ); }
break;
case 23:
#line 325 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 0, 1, 0 ) ); }
break;
case 24:
#line 326 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 1, 1, 0 ) ); }
break;
case 25:
#line 327 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 1, 1, 0 ) ); }
break;
case 26:
#line 328 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 0, 0, 1 ) ); }
break;
case 27:
#line 329 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 0, 0, 1 ) ); }
break;
case 28:
#line 330 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 1, 0, 1 ) ); }
break;
case 29:
#line 331 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 1, 0, 1 ) ); }
break;
case 30:
#line 332 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 0, 1, 1 ) ); }
break;
case 31:
#line 333 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 0, 1, 1 ) ); }
break;
case 32:
#line 334 "crc.y"
{ msk( NV40_VP_DEST_MASK( 0, 1, 1, 1 ) ); }
break;
case 33:
#line 335 "crc.y"
{ msk( NV40_VP_DEST_MASK( 1, 1, 1, 1 ) ); }
break;
case 36:
#line 340 "crc.y"
{ neg = 1; }
break;
case 37:
#line 343 "crc.y"
{ src_type = 2; }
break;
case 38:
#line 344 "crc.y"
{ src_type = 1; }
break;
case 39:
#line 345 "crc.y"
{ src_type = 0; }
break;
case 40:
#line 348 "crc.y"
{ dst_type = 0; }
break;
case 41:
#line 349 "crc.y"
{ dst_type = 1; }
break;
case 44:
#line 356 "crc.y"
{ v = 0; }
break;
case 45:
#line 357 "crc.y"
{ v = 1; }
break;
case 46:
#line 358 "crc.y"
{ v = 2; }
break;
case 47:
#line 359 "crc.y"
{ v = 3; }
break;
case 48:
#line 360 "crc.y"
{ v = 4; }
break;
case 49:
#line 361 "crc.y"
{ v = 5; }
break;
case 50:
#line 362 "crc.y"
{ v = 8; }
break;
case 51:
#line 363 "crc.y"
{ v = 9; }
break;
case 52:
#line 364 "crc.y"
{ v = 10; }
break;
case 53:
#line 365 "crc.y"
{ v = 11; }
break;
case 54:
#line 366 "crc.y"
{ v = 12; }
break;
case 55:
#line 367 "crc.y"
{ v = 13; }
break;
case 56:
#line 368 "crc.y"
{ v = 14; }
break;
case 57:
#line 369 "crc.y"
{ v = 15; }
break;
case 61:
#line 379 "crc.y"
{ n = 0; }
break;
case 62:
#line 380 "crc.y"
{ n = 1; }
break;
case 63:
#line 381 "crc.y"
{ n = 2; }
break;
case 64:
#line 382 "crc.y"
{ n = 3; }
break;
case 65:
#line 383 "crc.y"
{ n = 4; }
break;
case 66:
#line 384 "crc.y"
{ n = 5; }
break;
case 67:
#line 385 "crc.y"
{ n = 6; }
break;
case 68:
#line 386 "crc.y"
{ n = 7; }
break;
case 69:
#line 387 "crc.y"
{ n = 8; }
break;
case 70:
#line 388 "crc.y"
{ n = 9; }
break;
case 71:
#line 393 "crc.y"
{ v = n; }
break;
case 72:
#line 394 "crc.y"
{ v = v * 10 + n; }
break;
case 83:
#line 424 "crc.y"
{ v = 0; }
break;
case 84:
#line 425 "crc.y"
{ v = 1; }
break;
case 85:
#line 426 "crc.y"
{ v = 2; }
break;
case 86:
#line 427 "crc.y"
{ v = 3; }
break;
case 87:
#line 428 "crc.y"
{ v = 4; }
break;
case 88:
#line 429 "crc.y"
{ v = 5; }
break;
case 89:
#line 430 "crc.y"
{ v = 6; }
break;
case 90:
#line 431 "crc.y"
{ v = 7; }
break;
case 91:
#line 432 "crc.y"
{ v = 8; }
break;
case 92:
#line 433 "crc.y"
{ v = 9; }
break;
case 93:
#line 434 "crc.y"
{ v = 11; }
break;
case 94:
#line 435 "crc.y"
{ v = 11; }
break;
case 95:
#line 436 "crc.y"
{ v = 12; }
break;
case 96:
#line 437 "crc.y"
{ v = 13; }
break;
case 97:
#line 438 "crc.y"
{ v = 14; }
break;
case 102:
#line 449 "crc.y"
{ swz( 0 ); }
break;
case 103:
#line 450 "crc.y"
{ swz( 1 ); }
break;
case 104:
#line 451 "crc.y"
{ swz( 2 ); }
break;
case 105:
#line 452 "crc.y"
{ swz( 3 ); }
break;
case 106:
#line 455 "crc.y"
{ opv( NV40_VP_INST_OP_MOV ); }
break;
case 107:
#line 456 "crc.y"
{ opv( NV40_VP_INST_OP_LIT ); }
break;
case 108:
#line 458 "crc.y"
{ ops( NV40_VP_INST_OP_RCP ); }
break;
case 109:
#line 459 "crc.y"
{ ops( NV40_VP_INST_OP_RSQ ); }
break;
case 110:
#line 460 "crc.y"
{ ops( NV40_VP_INST_OP_EXP ); }
break;
case 111:
#line 461 "crc.y"
{ ops( NV40_VP_INST_OP_LOG ); }
break;
case 112:
#line 464 "crc.y"
{ opv( NV40_VP_INST_OP_MUL ); }
break;
case 113:
#line 465 "crc.y"
{ opv( NV40_VP_INST_OP_ADD ); }
break;
case 114:
#line 466 "crc.y"
{ opv( NV40_VP_INST_OP_DP3 ); }
break;
case 115:
#line 467 "crc.y"
{ opv( NV40_VP_INST_OP_DP4 ); }
break;
case 116:
#line 468 "crc.y"
{ opv( NV40_VP_INST_OP_DST ); }
break;
case 117:
#line 469 "crc.y"
{ opv( NV40_VP_INST_OP_MIN ); }
break;
case 118:
#line 470 "crc.y"
{ opv( NV40_VP_INST_OP_MAX ); }
break;
case 119:
#line 471 "crc.y"
{ opv( NV40_VP_INST_OP_SLT ); }
break;
case 120:
#line 472 "crc.y"
{ opv( NV40_VP_INST_OP_SGE ); }
break;
case 121:
#line 475 "crc.y"
{ opv( NV40_VP_INST_OP_MAD ); }
break;
case 122:
#line 480 "crc.y"
{ printf("\tVP\n");  clean(); }
break;
case 123:
#line 482 "crc.y"
{ printf("\tEND\n"); }
break;
#line 1259 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
