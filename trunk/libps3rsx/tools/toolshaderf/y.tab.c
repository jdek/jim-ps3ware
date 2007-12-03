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

#define uint32 unsigned int
#define uint32_t unsigned int

#include "../../src/shaders/fragment.h"
void yyerror(const char *str);

uint32 endian_fp( uint32 v )
{
  return ( ( ( v >> 16 ) & 0xffff ) << 0 ) |
         ( ( ( v >> 0 ) & 0xffff ) << 16 );

}


#define NV40_FP_DEST_MASK(x,y,z,w)  ((w<<3)|(z<<2)|(y<<1)|x)
#define NV40_FP_SWIZZLE(x,y,z,w)  ((w<<6)|(z<<4)|(y<<2)|x)


uint32 fp_inst[4];
float  f[256];


uint32 inst_stack[256][4];
uint32 swizzles[4];
int swz_ptr = 0;

int inst_ptr = 0;
int n;
int v;
int h;
int type = 0;
int src_ptr = 0;
int mod = 0;
int neg = 0;
int cnst = 0;
int last_cmd = 0;
int max_reg = 0;

const char *outfile;

typedef union
{
	float f;
	int i;
}f2i;


void dump()
{
	if( fp_inst[0] == 0 )
	{
		cnst = 0;
		return;
	}
	inst_stack[inst_ptr][0] = fp_inst[0];
	inst_stack[inst_ptr][1] = fp_inst[1];
	inst_stack[inst_ptr][2] = fp_inst[2];
	inst_stack[inst_ptr][3] = fp_inst[3];
	
	++inst_ptr;
	last_cmd = 1;

	if( cnst )
	{
		f2i c;
		c.f = f[0];
		inst_stack[inst_ptr][0] = c.i;
		c.f = f[1];
		inst_stack[inst_ptr][1] = c.i;
		c.f = f[2];
		inst_stack[inst_ptr][2] = c.i;
		c.f = f[3];
		inst_stack[inst_ptr][3]	= c.i;
		++inst_ptr;
		last_cmd = 2;
		
	}
	cnst = 0;
	
	
}

void print()
{
	/*printf( "asdasdasd" );*/
	/*inst_stack[inst_ptr - 1][3] |= NV40_VP_INST_LAST;*/
	uint32 i;
	inst_stack[inst_ptr - last_cmd][0] |= NV40_FP_OP_PROGRAM_END;
	for( i = 0; i < inst_ptr; ++i )
	{
		printf( "%8x %8x %8x %8x \n", inst_stack[i][0], inst_stack[i][1], inst_stack[i][2], inst_stack[i][3]  );
		inst_stack[i][0] = endian_fp( inst_stack[i][0] );
		inst_stack[i][1] = endian_fp( inst_stack[i][1] );
		inst_stack[i][2] = endian_fp( inst_stack[i][2] );
		inst_stack[i][3] = endian_fp( inst_stack[i][3] );
		
	}

	


	
	fragment_shader_desc_t desc;
	
	
	FILE *out = fopen( outfile, "wb" );
	if( out )
	{
	    desc.aux[0] = 0xcafebabe;
	    desc.dword_length = inst_ptr * 4;
	    desc.num_regs = max_reg;
	    fwrite( &desc, sizeof( desc ), 1, out );
	    fwrite( &inst_stack[0][0], 4 * desc.dword_length, 1, out );
	    fclose( out );
	    printf( "fp regs %x \n", max_reg );
	    	
	} 
	inst_ptr = 0;
}

void clean()
{
	
	fp_inst[0] = 0;
	fp_inst[1] = 0;
	fp_inst[2] = 0;
	fp_inst[3] = 0;
	src_ptr = 0;
	type = 0;
	mod = 0;
	neg = 0;
}

void opv( uint32 op )
{
	if( op == 0xff )
	{
	 	op = 0;
	}
	
	printf( "opv code 0x%2x ", op );
	fp_inst[0] &= ~NV40_FP_OP_OPCODE_MASK; 
  	fp_inst[0] |= ( op <<  NV40_FP_OP_OPCODE_SHIFT); 
	
}

void msk( uint32 ms )
{
	printf( "ms %x %x %x %x ", ( ms >> 0 ) & 1, ( ms >> 1 ) & 1, ( ms >> 2 ) & 1, ( ms >> 3 ) & 1  );
	fp_inst[0] &= ~NV40_FP_OP_OUTMASK_MASK; 
  	fp_inst[0] |= ( ms << NV40_FP_OP_OUTMASK_SHIFT ); 

}

void cnd_msk( uint32 cn )
{
	fp_inst[1] &= ~NV40_FP_OP_COND_MASK;
	fp_inst[1] |= ( cn << NV40_FP_OP_COND_SHIFT );
	printf( "cnd %d ", cn );
	
}

void sat( uint32 s )
{
	printf( "sat %x ", s );
	fp_inst[0] &= ~NV40_FP_OP_OUT_SAT;
	if( s )
	{	
		fp_inst[0] |= NV40_FP_OP_OUT_SAT;	
	}

}

void cnd( uint32 s )
{
	printf( "cnd %x \n", s );
	fp_inst[0] &= ~NV40_FP_OP_COND_WRITE_ENABLE;
	if( s )
	{	
		fp_inst[0] |= NV40_FP_OP_COND_WRITE_ENABLE;	
	}

}
	

void com( uint32 s )
{
	swizzles[swz_ptr++] = s;
}

void set_input( uint32 in )
{
	type = 1;
	fp_inst[0] &= ~NV40_FP_OP_INPUT_SRC_MASK;
	fp_inst[0] |= ( in << NV40_FP_OP_INPUT_SRC_SHIFT );
	printf( "inp %d ", in  );
	
}

uint32 get_swizzle()
{
	int st;
	uint32 r;
	if( swz_ptr == 0 )
	{
		swizzles[0] = 0;
		swizzles[1] = 1;
		swizzles[2] = 2;
		swizzles[3] = 3;
		               	
	}
	else
	{
		for( st = swz_ptr; st < 4; ++st )
		{
			swizzles[st] = swizzles[swz_ptr - 1];
		} 
	}
	swz_ptr = 0;
	
	r = NV40_FP_SWIZZLE( swizzles[0], swizzles[1], swizzles[2], swizzles[3] );
	printf( "swz = %x %x %x %x ", swizzles[0], swizzles[1], swizzles[2], swizzles[3] );
	return r;
	
}

void prc( uint32 s )
{
	printf( "prc %x ", s );
	fp_inst[0] &= ~NV40_FP_OP_PRECISION_MASK;
	if( s )
	{	
		fp_inst[0] |= ( s << NV40_FP_OP_PRECISION_SHIFT );	
	}
}

void set_cns()
{
	type = 2;
}

void set_tex( uint32 r )
{
	fp_inst[0] &= ~( NV40_FP_OP_TEX_UNIT_MASK );
	fp_inst[0] |= ( r << NV40_FP_OP_TEX_UNIT_SHIFT );
	
	printf( "tex %d \n", r );
}

void src_reg( uint32 reg )
{

	fp_inst[src_ptr + 1] &= ~NV40_FP_REG_TYPE_MASK;
	fp_inst[src_ptr + 1] |= ( type << NV40_FP_REG_TYPE_SHIFT );

	fp_inst[src_ptr + 1] &= ~NV40_FP_REG_UNK_0;
	if( h )
	{
		fp_inst[src_ptr + 1] |= NV40_FP_REG_UNK_0;
	
	}
	
	fp_inst[src_ptr + 1] &= ~NV40_FP_REG_NEGATE;
	if( neg )
	{
		fp_inst[src_ptr + 1] |= NV40_FP_REG_NEGATE;
	}
	fp_inst[src_ptr + 1] &= ~NV40_FP_REG_SRC_MASK;
	fp_inst[src_ptr + 1] |= ( reg << NV40_FP_REG_SRC_SHIFT );
	
	fp_inst[src_ptr + 1] &= ~NV40_FP_REG_SWZ_ALL_MASK;
	fp_inst[src_ptr + 1] |= ( get_swizzle() << NV40_FP_REG_SWZ_ALL_SHIFT );
	
	printf( "... src %s %2d type %d neg %d \n", h ? "h" : "r", reg, type, neg  );
	src_ptr++;
	type = 0;
	neg = 0;
		
}

        
void out_reg( uint32 reg )
{
	uint32 reg_num =  h ? reg / 2 + 2: reg + 2;
	if( reg_num > max_reg )
	{
		max_reg = reg_num;
	}
	fp_inst[0] &= ~NV40_FP_OP_UNK0_7;
	if( h )
	{
		fp_inst[0] |= NV40_FP_OP_UNK0_7;
	
	}
	
	
	fp_inst[0] &= ~NV40_FP_OP_OUT_REG_MASK;
	fp_inst[0] |= ( reg << NV40_FP_OP_OUT_REG_SHIFT );
	printf( "out %s %2d ", h ? "h" : "r", reg  );	
}

void cnd_swz()
{
	fp_inst[1] &= ~NV40_FP_OP_COND_SWZ_ALL_MASK;
	fp_inst[1] |= ( get_swizzle() << NV40_FP_OP_COND_SWZ_ALL_SHIFT );
	
}

#line 331 "y.tab.c"
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
#define _1D 267
#define _2D 268
#define _3D 269
#define DOT 270
#define CONST 271
#define C 272
#define WHITE 273
#define DEC 274
#define WORD 275
#define SAT 276
#define H 277
#define END 278
#define POW 279
#define BEG 280
#define CUBE 281
#define RECT 282
#define COLR 283
#define COLH 284
#define DEPR 285
#define WPOS 286
#define COL0 287
#define COL1 288
#define FOGC 289
#define F 290
#define R 291
#define X 292
#define Y 293
#define Z 294
#define W 295
#define EQ 296
#define GE 297
#define GT 298
#define LE 299
#define LT 300
#define NE 301
#define TR 302
#define FL 303
#define DDX 304
#define DDY 305
#define FLR 306
#define FRC 307
#define LIT 308
#define MOV 309
#define PK2H 310
#define PK2US 311
#define PK4B 312
#define PK4UB 313
#define COS 314
#define EXP 315
#define LG2 316
#define RCP 317
#define RSQ 318
#define SIN 319
#define UP2H 320
#define UP2US 321
#define UP4B 322
#define UP4UB 323
#define ADD 324
#define DP3 325
#define DP4 326
#define DST 327
#define MAX 328
#define MIN 329
#define MUL 330
#define RFL 331
#define SEQ 332
#define SFL 333
#define SGE 334
#define SGT 335
#define SLE 336
#define SLT 337
#define SNE 338
#define STR 339
#define SUB 340
#define MAD 341
#define LRP 342
#define X2D 343
#define KIL 344
#define TEX 345
#define TXP 346
#define TXD 347
const short yylhs[] = {                                        -1,
    3,    0,    2,    2,    1,    1,    5,    4,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    8,    9,   10,
   11,   12,   13,   14,   15,   30,   30,   30,   30,   31,
   31,   32,   32,   17,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   22,   23,   23,   23,   23,   23,
   23,   23,   23,   23,   23,   23,   23,   23,   23,   24,
   24,   25,   27,   27,   29,   21,   21,   33,   34,   34,
   19,   19,   39,   40,   40,   18,   42,   42,   44,   44,
   26,   47,   47,   47,   47,   47,   47,   47,   47,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   38,   38,   48,   49,   49,
   49,   49,   49,   45,   45,   53,   53,   53,   53,   53,
   53,   53,   53,   53,   53,   54,   54,   51,   52,   46,
   55,   55,   55,   37,   41,   41,   41,   41,   41,   56,
   56,   56,   56,   28,   50,   57,   57,   57,   57,   57,
   36,   36,   35,   35,   35,    7,   58,   59,   59,
};
const short yylen[] = {                                         2,
    0,    5,    2,    1,    0,    1,    1,    2,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    6,    6,    8,
    8,   10,    2,    8,   12,    1,    1,    1,    0,    1,
    0,    1,    0,    3,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    3,    3,    4,    3,    3,
    3,    3,    4,    3,    3,    3,    1,    1,    3,    0,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    2,
    2,    3,    2,    3,    3,    4,    2,    3,    3,    4,
    3,    4,    4,    5,    0,    1,    1,    4,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    2,    2,    2,    4,
    1,    1,    1,    2,    5,    4,    3,    2,    0,    1,
    1,    1,    1,    7,    2,    1,    1,    1,    1,    1,
    1,    1,    0,    1,    1,    4,    1,    0,    2,
};
const short yydefred[] = {                                      0,
    0,    0,    6,    0,    0,   55,   35,   36,   37,   38,
   39,   40,   41,   42,   43,   44,   45,   46,   47,   48,
   49,   50,   51,   52,   53,   54,   56,   57,   58,   59,
   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,
   70,   71,   72,   73,   74,   75,    0,    4,    0,    9,
   10,   11,   12,   13,   14,   15,   16,   17,    0,    0,
    0,    0,    0,    0,    0,    0,    7,    0,    0,    3,
    8,   27,   26,   28,    0,    0,    0,    0,    0,    0,
   92,   93,   94,   95,   96,   97,   98,   99,   23,    0,
    0,    0,  167,    0,    2,    0,   30,    0,    0,    0,
    0,    0,    0,   91,    0,    0,    0,  166,    0,    0,
    0,    0,    0,   87,   88,  124,  125,   32,   34,    0,
    0,    0,    0,  150,  151,  152,  153,    0,    0,    0,
  161,  162,  169,  126,  127,  128,  129,  130,  131,  132,
  133,  134,  135,  136,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  137,  141,  142,
  143,    0,    0,   18,    0,    0,    0,  107,    0,   86,
    0,   19,    0,    0,    0,    0,    0,    0,  140,  164,
  165,    0,    0,    0,    0,    0,  108,    0,  109,  111,
    0,    0,    0,    0,    0,    0,    0,  145,    0,    0,
    0,    0,    0,    0,  117,  116,   81,   82,    0,  110,
  112,  113,   89,   76,   77,    0,    0,    0,   20,   21,
    0,    0,   24,    0,    0,    0,    0,   84,   85,  114,
    0,    0,    0,   79,   80,    0,    0,    0,    0,   83,
  119,  120,  121,  122,    0,  123,   78,  144,   22,    0,
    0,    0,  118,    0,    0,    0,   25,  156,  157,  158,
  159,  160,    0,  154,
};
const short yydgoto[] = {                                       2,
  163,   47,   69,   48,   68,   49,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   75,  112,  164,   60,
  172,   61,   62,   63,   64,   89,   65,  223,   66,   76,
   98,  119,  192,  193,  182,  203,  234,  204,  183,  184,
  104,  113,  150,  170,  114,  115,   90,  206,  245,  238,
  116,  117,  144,  145,  162,  128,  263,   94,  108,
};
const short yysindex[] = {                                   -240,
 -208,    0,    0,  -24, -197,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  -24,    0,   32,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -229, -229,
 -229, -229, -229, -161, -229, -229,    0, -178, -175,    0,
    0,    0,    0,    0, -197, -154, -197, -197, -197, -197,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -123,
 -197, -197,    0,  110,    0, -102,    0, -130, -102, -102,
 -102, -102, -162,    0, -102, -102, -224,    0, -105, -105,
   75,  128,  -96,    0,    0,    0,    0,    0,    0,  132,
  134,  136,  137,    0,    0,    0,    0, -162,  144,  146,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -105, -105,  -57, -208, -108,  155,
 -208, -208, -208, -208, -162, -208, -208,    0,    0,    0,
    0,  103,   67,    0,  -37, -242,  -95,    0, -161,    0,
   67,    0,  160,  163,  168, -162,  169,  171,    0,    0,
    0,  -98, -208, -208, -236,  -64,    0,  -63,    0,    0,
  199, -208, -208,  -74, -208, -208, -208,    0, -208, -208,
   67,  108, -123, -123,    0,    0,    0,    0,  -53,    0,
    0,    0,    0,    0,    0,   67,  -23,  -23,    0,    0,
  204,  -94,    0,  208,  -66,  135, -244,    0,    0,    0,
  139,  -66, -162,    0,    0, -208, -105, -208, -208,    0,
    0,    0,    0,    0,  167,    0,    0,    0,    0, -105,
  217,  220,    0, -208, -208, -195,    0,    0,    0,    0,
    0,    0, -208,    0,
};
const short yyrindex[] = {                                      0,
   50,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  -13,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -193, -193,
 -193, -193, -193,    0, -193, -193,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -106,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   -2,
    0,    0,    0,  207,    0,    0,    0,   -6,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   44,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -39,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -34,  -32,    0,  -42,    0,  224,
  -42,  -42,  -42,  -42,  -30,  -42,  -42,    0,    0,    0,
    0,    0,  -69,    0,   49,   58,   60,    0,    0,    0,
  -69,    0,    0,    0,    0,  -28,    0,    0,    0,    0,
    0,    0,    5,    5,   79,   85,    0,  104,    0,    0,
    0,    5,    5,    0,  -42,  -42,  -42,    0,  -76,  -42,
  -61,    0,  -29,  -29,    0,    0,    0,    0,  105,    0,
    0,    0,    0,    0,    0,  -61,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -42,    0,  226,  -42,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  -27,
    0,    0,    0, -160,  -76,    0,    0,    0,    0,    0,
    0,    0,  212,    0,
};
const short yygindex[] = {                                      0,
   -1,    0,    0,  225,   36,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  158,   63, -119,    0,
 -120,    0,    0,    0,    0,  106,    0,   18,    0,    0,
    0,    0,    0,   61, -147,  -89,   56, -171,    0,   77,
 -133,    0,    0,    0, -126,    0,    0,    0,    0,   52,
    0,    0, -124, -103,    0, -109,    0,    0,    0,
};
#define YYTABLESIZE 397
const short yytable[] = {                                       4,
    5,  148,    5,  202,  148,  139,  146,  138,  111,  139,
  147,  138,  146,  147,  149,  146,  155,  133,  155,  148,
  158,  158,  218,  194,  139,  201,  138,  202,  147,  149,
  146,  173,  163,  174,  175,  202,  177,  178,  149,    1,
  163,  241,  242,  243,  244,  176,  131,   72,    5,  216,
  132,  188,  189,  225,  163,  205,  149,  209,  210,    5,
  218,   73,   74,    5,    3,  155,  198,  205,  232,  228,
  229,  258,  259,  260,  219,   67,  220,  221,   29,   29,
  224,    5,   29,  115,  148,  261,  262,  115,  100,  139,
   71,  138,  100,  147,  149,  146,   93,  101,  205,  103,
  237,  101,   95,  103,  217,  205,    5,    5,    5,  180,
   96,  181,   99,  100,  101,  102,  249,   97,  102,  252,
    5,    5,  102,  248,  104,  158,  105,  106,  104,  124,
  125,  126,  127,  250,   81,   82,   83,   84,   85,   86,
   87,   88,  217,  105,  106,  118,  103,  105,  106,  171,
  171,  134,  135,  136,  137,  138,  139,  140,  141,  142,
  143,  120,  121,  122,  123,  147,   31,  129,  130,   31,
  107,  148,  131,  149,  109,  151,  132,  152,  109,  153,
  154,  207,  208,  165,  166,  167,  168,  156,  110,  157,
  214,  215,  110,  171,  169,  179,  131,  222,  227,  190,
  132,  163,  109,  195,  131,  163,  196,  163,  132,  163,
  109,  197,  199,  163,  200,  163,  110,   77,   78,   79,
   80,  163,   91,   92,  110,  159,  160,  161,    5,  163,
  211,  212,    5,  148,    5,  139,  251,  138,  139,  213,
  138,  230,  147,  149,  146,  155,  233,  236,    5,    5,
  237,  239,  256,  222,    6,  185,  186,  187,  240,  253,
  254,  264,  247,  255,    1,  168,   33,   90,    5,    5,
    5,   70,  257,  235,  191,    0,  231,  226,  246,    7,
    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
   28,   29,   30,   31,   32,   33,    0,   34,    0,   35,
   36,   37,   38,   39,    0,   40,   41,    0,   42,   43,
   44,   45,   46,    5,    0,    0,    0,    0,    5,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
    0,    5,    0,    5,    5,    5,    5,    5,    0,    5,
    5,    0,    5,    5,    5,    5,    5,
};
const short yycheck[] = {                                       1,
   43,   41,   45,  102,   44,   40,  110,   40,  111,   44,
   41,   44,   41,   44,   44,   44,   44,  107,  128,   59,
  145,  146,  194,  171,   59,  124,   59,  102,   59,   59,
   59,  152,  102,  153,  154,  102,  156,  157,   41,  280,
  102,  286,  287,  288,  289,  155,  271,  277,   44,  124,
  275,  294,  295,  201,  124,  182,   59,  294,  295,  102,
  232,  291,  292,   59,  273,   93,  176,  194,  216,  203,
  204,  267,  268,  269,  195,  273,  196,  197,  272,  273,
  200,  124,  276,   40,  124,  281,  282,   44,   40,  124,
   59,  124,   44,  124,  124,  124,  275,   40,  225,   40,
  345,   44,  278,   44,  194,  232,  267,  268,  269,   43,
   75,   45,   77,   78,   79,   80,  236,  272,   40,  239,
  281,  282,   44,  233,   40,  250,   91,   92,   44,  292,
  293,  294,  295,  237,  296,  297,  298,  299,  300,  301,
  302,  303,  232,   40,   40,  276,  270,   44,   44,  151,
  152,  257,  258,  259,  260,  261,  262,  263,  264,  265,
  266,   99,  100,  101,  102,   91,  273,  105,  106,  276,
   61,   44,  271,  270,  277,   44,  275,   44,  277,   44,
   44,  183,  184,  292,  293,  294,  295,   44,  291,   44,
  192,  193,  291,  195,   40,   93,  271,  199,   91,  295,
  275,  271,  277,   44,  271,  275,   44,  277,  275,  271,
  277,   44,   44,  275,   44,  277,  291,   60,   61,   62,
   63,  291,   65,   66,  291,  283,  284,  285,  271,  291,
  295,  295,  275,  273,  277,  270,  238,  270,  273,   41,
  273,  295,  273,  273,  273,  273,  270,   44,  291,  274,
  345,   44,  254,  255,  279,  293,  294,  295,  124,   93,
   44,  263,  124,   44,  278,   59,  273,   44,  345,   44,
   59,   47,  255,  218,  169,   -1,  216,  201,  227,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,   -1,  332,   -1,  334,
  335,  336,  337,  338,   -1,  340,  341,   -1,  343,  344,
  345,  346,  347,  274,   -1,   -1,   -1,   -1,  279,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
   -1,  332,   -1,  334,  335,  336,  337,  338,   -1,  340,
  341,   -1,  343,  344,  345,  346,  347,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 347
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,"'+'","','","'-'",0,0,0,0,0,0,0,0,0,0,0,0,0,"';'",0,
"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,
0,0,0,0,0,0,0,"'f'",0,0,0,0,0,0,0,0,"'o'",0,0,0,0,0,0,0,0,0,0,0,0,"'|'",0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_1D","_2D",
"_3D","DOT","CONST","C","WHITE","DEC","WORD","SAT","H","END","POW","BEG","CUBE",
"RECT","COLR","COLH","DEPR","WPOS","COL0","COL1","FOGC","F","R","X","Y","Z","W",
"EQ","GE","GT","LE","LT","NE","TR","FL","DDX","DDY","FLR","FRC","LIT","MOV",
"PK2H","PK2US","PK4B","PK4UB","COS","EXP","LG2","RCP","RSQ","SIN","UP2H",
"UP2US","UP4B","UP4UB","ADD","DP3","DP4","DST","MAX","MIN","MUL","RFL","SEQ",
"SFL","SGE","SGT","SLE","SLT","SNE","STR","SUB","MAD","LRP","X2D","KIL","TEX",
"TXP","TXD",
};
const char * const yyrule[] = {
"$accept : program",
"$$1 :",
"program : BEG space instructionSequence $$1 END",
"instructionSequence : instructionSequence instructionStatement",
"instructionSequence : instructionStatement",
"space :",
"space : WHITE",
"empty : WHITE",
"instructionStatement : instruction ';'",
"instruction : localDeclaration",
"instruction : VECTORop_instruction",
"instruction : SCALARop_instruction",
"instruction : BINSCop_instruction",
"instruction : BINop_instruction",
"instruction : TRIop_instruction",
"instruction : KILop_instruction",
"instruction : TEXop_instruction",
"instruction : TXDop_instruction",
"VECTORop_instruction : VECTORop pref empty maskedDstReg ',' vectorSrc",
"SCALARop_instruction : SCALARop pref empty maskedDstReg ',' scalarSrc",
"BINSCop_instruction : BINSCop pref empty maskedDstReg ',' scalarSrc ',' scalarSrc",
"BINop_instruction : BINop pref empty maskedDstReg ',' vectorSrc ',' vectorSrc",
"TRIop_instruction : TRIop pref empty maskedDstReg ',' vectorSrc ',' vectorSrc ',' vectorSrc",
"KILop_instruction : KILop ccMask",
"TEXop_instruction : TEXop pref empty maskedDstReg ',' vectorSrc ',' texImageId",
"TXDop_instruction : TXDop pref empty maskedDstReg ',' vectorSrc ',' vectorSrc ',' vectorSrc ',' texImageId",
"prec : R",
"prec : H",
"prec : X",
"prec :",
"cond : C",
"cond :",
"sat : SAT",
"sat :",
"pref : prec cond sat",
"VECTORop : DDX",
"VECTORop : DDY",
"VECTORop : FLR",
"VECTORop : FRC",
"VECTORop : LIT",
"VECTORop : MOV",
"VECTORop : PK2H",
"VECTORop : PK2US",
"VECTORop : PK4B",
"VECTORop : PK4UB",
"SCALARop : COS",
"SCALARop : EXP",
"SCALARop : LG2",
"SCALARop : RCP",
"SCALARop : RSQ",
"SCALARop : SIN",
"SCALARop : UP2H",
"SCALARop : UP2US",
"SCALARop : UP4B",
"SCALARop : UP4UB",
"BINSCop : POW",
"BINop : ADD",
"BINop : DP3",
"BINop : DP4",
"BINop : DST",
"BINop : MAX",
"BINop : MIN",
"BINop : MUL",
"BINop : SEQ",
"BINop : SGE",
"BINop : SGT",
"BINop : SLE",
"BINop : SLT",
"BINop : SNE",
"BINop : SUB",
"TRIop : MAD",
"TRIop : X2D",
"KILop : KIL",
"TEXop : TEX",
"TEXop : TXP",
"TXDop : TXD",
"scalarSrc : space absScalarSrc space",
"scalarSrc : space baseScalarSrc space",
"absScalarSrc : negate '|' baseScalarSrc '|'",
"baseScalarSrc : negate vectorConstant scalarSuffix",
"baseScalarSrc : negate srcRegister scalarSuffix",
"vectorSrc : space absVectorSrc space",
"vectorSrc : space baseVectorSrc space",
"absVectorSrc : negate '|' baseVectorSrc '|'",
"baseVectorSrc : negate vectorConstant swizzleSuffix",
"baseVectorSrc : negate srcRegister swizzleSuffix",
"maskedDstReg : dstRegister optionalWriteMask optionalCCMask",
"dstRegister : fragTempReg",
"dstRegister : fragOutputReg",
"optionalCCMask : '(' ccMask ')'",
"optionalCCMask :",
"ccMask : ccMaskRule swizzleSuffix",
"ccMaskRule : EQ",
"ccMaskRule : GE",
"ccMaskRule : GT",
"ccMaskRule : LE",
"ccMaskRule : LT",
"ccMaskRule : NE",
"ccMaskRule : TR",
"ccMaskRule : FL",
"optionalWriteMask : DOT X",
"optionalWriteMask : DOT Y",
"optionalWriteMask : DOT X Y",
"optionalWriteMask : DOT Z",
"optionalWriteMask : DOT X Z",
"optionalWriteMask : DOT Y Z",
"optionalWriteMask : DOT X Y Z",
"optionalWriteMask : DOT W",
"optionalWriteMask : DOT X W",
"optionalWriteMask : DOT Y W",
"optionalWriteMask : DOT X Y W",
"optionalWriteMask : DOT Z W",
"optionalWriteMask : DOT X Z W",
"optionalWriteMask : DOT Y Z W",
"optionalWriteMask : DOT X Y Z W",
"optionalWriteMask :",
"srcRegister : fragAttribReg",
"srcRegister : fragTempReg",
"fragAttribReg : 'f' '[' fragAttribRegId ']'",
"fragAttribRegId : WPOS",
"fragAttribRegId : COL0",
"fragAttribRegId : COL1",
"fragAttribRegId : FOGC",
"fragAttribRegId : texImageUnit",
"fragTempReg : fragF32Reg",
"fragTempReg : fragF16Reg",
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
"fragF32Reg : R decCalc",
"fragF16Reg : H decCalc",
"fragOutputReg : 'o' '[' fragOutputRegName ']'",
"fragOutputRegName : COLR",
"fragOutputRegName : COLH",
"fragOutputRegName : DEPR",
"scalarSuffix : DOT component",
"swizzleSuffix : DOT component component component component",
"swizzleSuffix : DOT component component component",
"swizzleSuffix : DOT component component",
"swizzleSuffix : DOT component",
"swizzleSuffix :",
"component : X",
"component : Y",
"component : Z",
"component : W",
"texImageId : space texImageUnit space ',' space texImageTarget space",
"texImageUnit : TEX decCalc",
"texImageTarget : _1D",
"texImageTarget : _2D",
"texImageTarget : _3D",
"texImageTarget : CUBE",
"texImageTarget : RECT",
"vectorConstant : CONST",
"vectorConstant : WORD",
"negate :",
"negate : '+'",
"negate : '-'",
"localDeclaration : DEC empty namedLocalParameter optionalLocalValue",
"namedLocalParameter : WORD",
"optionalLocalValue :",
"optionalLocalValue : '=' vectorConstant",
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
#line 647 "crc.y"


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







#line 908 "y.tab.c"
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
#line 328 "crc.y"
{ print(); }
break;
case 8:
#line 343 "crc.y"
{ dump(); clean();}
break;
case 26:
#line 384 "crc.y"
{ prc( 0 ); }
break;
case 27:
#line 385 "crc.y"
{ prc( 1 ); }
break;
case 28:
#line 386 "crc.y"
{ prc( 2 ); }
break;
case 29:
#line 387 "crc.y"
{ prc( 0 );	}
break;
case 30:
#line 391 "crc.y"
{ cnd( 1 ); }
break;
case 31:
#line 392 "crc.y"
{ cnd( 0 ); }
break;
case 32:
#line 395 "crc.y"
{ sat( 1 ); }
break;
case 33:
#line 396 "crc.y"
{ sat( 0 ); }
break;
case 34:
#line 399 "crc.y"
{ type = 0; }
break;
case 35:
#line 403 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DDX ); }
break;
case 36:
#line 404 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DDY ); }
break;
case 37:
#line 405 "crc.y"
{ opv( NV40_FP_OP_OPCODE_FLR ); }
break;
case 38:
#line 406 "crc.y"
{ opv( NV40_FP_OP_OPCODE_FRC ); }
break;
case 39:
#line 407 "crc.y"
{ opv( 0xff ); }
break;
case 40:
#line 408 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MOV ); }
break;
case 41:
#line 409 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK2H ); }
break;
case 42:
#line 410 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK2US ); }
break;
case 43:
#line 411 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK4B ); }
break;
case 44:
#line 412 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK4UB ); }
break;
case 45:
#line 415 "crc.y"
{ opv( NV40_FP_OP_OPCODE_COS ); }
break;
case 46:
#line 416 "crc.y"
{ opv( NV40_FP_OP_OPCODE_EX2 ); }
break;
case 47:
#line 417 "crc.y"
{ opv( NV40_FP_OP_OPCODE_LG2 ); }
break;
case 48:
#line 418 "crc.y"
{ opv( NV40_FP_OP_OPCODE_RCP ); }
break;
case 49:
#line 419 "crc.y"
{ opv( 0xff ); }
break;
case 50:
#line 420 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SIN ); }
break;
case 51:
#line 421 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP2H ); }
break;
case 52:
#line 422 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP2US ); }
break;
case 53:
#line 423 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP4B ); }
break;
case 54:
#line 424 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP4UB ); }
break;
case 56:
#line 430 "crc.y"
{ opv( NV40_FP_OP_OPCODE_ADD ); }
break;
case 57:
#line 431 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DP3 ); }
break;
case 58:
#line 432 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DP4 ); }
break;
case 59:
#line 433 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DST ); }
break;
case 60:
#line 434 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MAX ); }
break;
case 61:
#line 435 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MIN ); }
break;
case 62:
#line 436 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MUL ); }
break;
case 63:
#line 437 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SEQ ); }
break;
case 64:
#line 438 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SGE ); }
break;
case 65:
#line 439 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SGT ); }
break;
case 66:
#line 440 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SLE ); }
break;
case 67:
#line 441 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SLT ); }
break;
case 68:
#line 442 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SNE ); }
break;
case 69:
#line 443 "crc.y"
{ opv( NV40_FP_OP_OPCODE_ADD ); }
break;
case 70:
#line 448 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MAD ); }
break;
case 71:
#line 449 "crc.y"
{ opv( 0xff ); }
break;
case 72:
#line 452 "crc.y"
{ opv( NV40_FP_OP_OPCODE_KIL ); }
break;
case 73:
#line 456 "crc.y"
{ opv( NV40_FP_OP_OPCODE_TEX ); }
break;
case 74:
#line 457 "crc.y"
{ opv( NV40_FP_OP_OPCODE_TXP ); }
break;
case 75:
#line 460 "crc.y"
{ opv( NV40_FP_OP_OPCODE_TXD ); }
break;
case 76:
#line 464 "crc.y"
{src_reg( v ); }
break;
case 77:
#line 465 "crc.y"
{src_reg( v ); }
break;
case 81:
#line 476 "crc.y"
{src_reg( v ); }
break;
case 82:
#line 477 "crc.y"
{src_reg( v ); }
break;
case 83:
#line 481 "crc.y"
{ mod = 1; }
break;
case 86:
#line 490 "crc.y"
{ out_reg( v );  cnd_swz(); type = 0; h = 0; printf( "\n" ); }
break;
case 90:
#line 500 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_TR ); }
break;
case 92:
#line 506 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_EQ ); }
break;
case 93:
#line 507 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_GE ); }
break;
case 94:
#line 508 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_GT ); }
break;
case 95:
#line 509 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_LE ); }
break;
case 96:
#line 510 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_LT ); }
break;
case 97:
#line 511 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_NE ); }
break;
case 98:
#line 512 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_TR ); }
break;
case 99:
#line 513 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_FL ); }
break;
case 100:
#line 517 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 0, 0 ) ); }
break;
case 101:
#line 518 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 0, 0 ) ); }
break;
case 102:
#line 519 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 0, 0 ) ); }
break;
case 103:
#line 520 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 0, 1, 0 ) ); }
break;
case 104:
#line 521 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 1, 0 ) ); }
break;
case 105:
#line 522 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 1, 0 ) ); }
break;
case 106:
#line 523 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 1, 0 ) ); }
break;
case 107:
#line 524 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 0, 0, 1 ) ); }
break;
case 108:
#line 525 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 0, 1 ) ); }
break;
case 109:
#line 526 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 0, 1 ) ); }
break;
case 110:
#line 527 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 0, 1 ) ); }
break;
case 111:
#line 528 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 0, 1, 1 ) ); }
break;
case 112:
#line 529 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 1, 1 ) ); }
break;
case 113:
#line 530 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 1, 1 ) ); }
break;
case 114:
#line 531 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 1, 1 ) ); }
break;
case 115:
#line 532 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 1, 1 ) ); }
break;
case 119:
#line 544 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_POSITION ); }
break;
case 120:
#line 545 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_COL0 ); }
break;
case 121:
#line 546 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_COL1 ); }
break;
case 122:
#line 547 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_FOGC ); }
break;
case 123:
#line 548 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_TC0 + v ); }
break;
case 126:
#line 555 "crc.y"
{ n = 0; }
break;
case 127:
#line 556 "crc.y"
{ n = 1; }
break;
case 128:
#line 557 "crc.y"
{ n = 2; }
break;
case 129:
#line 558 "crc.y"
{ n = 3; }
break;
case 130:
#line 559 "crc.y"
{ n = 4; }
break;
case 131:
#line 560 "crc.y"
{ n = 5; }
break;
case 132:
#line 561 "crc.y"
{ n = 6; }
break;
case 133:
#line 562 "crc.y"
{ n = 7; }
break;
case 134:
#line 563 "crc.y"
{ n = 8; }
break;
case 135:
#line 564 "crc.y"
{ n = 9; }
break;
case 136:
#line 569 "crc.y"
{ v = n; }
break;
case 137:
#line 570 "crc.y"
{ v = v * 10 + n; }
break;
case 138:
#line 574 "crc.y"
{ h = 0; }
break;
case 139:
#line 577 "crc.y"
{ h = 1; }
break;
case 141:
#line 585 "crc.y"
{ v = 0; h = 0; }
break;
case 142:
#line 586 "crc.y"
{ v = 0; h = 1; }
break;
case 150:
#line 602 "crc.y"
{ com( 0 ); }
break;
case 151:
#line 603 "crc.y"
{ com( 1 ); }
break;
case 152:
#line 604 "crc.y"
{ com( 2 ); }
break;
case 153:
#line 605 "crc.y"
{ com( 3 ); }
break;
case 154:
#line 608 "crc.y"
{ set_tex( v ); }
break;
case 161:
#line 624 "crc.y"
{ set_cns(); cnst = 1;}
break;
case 162:
#line 625 "crc.y"
{ set_cns(); cnst = 1;}
break;
case 163:
#line 629 "crc.y"
{ neg = 0; }
break;
case 164:
#line 630 "crc.y"
{ neg = 0; }
break;
case 165:
#line 631 "crc.y"
{ neg = 1; }
break;
#line 1555 "y.tab.c"
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
