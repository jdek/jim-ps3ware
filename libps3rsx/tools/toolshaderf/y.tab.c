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
int creg = 0;

const char *outfile;

typedef union
{
	float f;
	int i;
}f2i;

int inst = 0;

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

	++inst;
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
	    desc.aux[0] = inst;
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
	if( creg == 0 && reg_num > max_reg )
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
	creg = 0;
}

void cnd_swz()
{
	fp_inst[1] &= ~NV40_FP_OP_COND_SWZ_ALL_MASK;
	fp_inst[1] |= ( get_swizzle() << NV40_FP_OP_COND_SWZ_ALL_SHIFT );
	
}

#line 335 "y.tab.c"
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
   53,   53,   53,   53,   53,   54,   54,   51,   51,   52,
   52,   46,   55,   55,   55,   37,   41,   41,   41,   41,
   41,   56,   56,   56,   56,   28,   50,   57,   57,   57,
   57,   57,   36,   36,   35,   35,   35,    7,   58,   59,
   59,
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
    1,    1,    1,    1,    1,    1,    2,    2,    2,    2,
    2,    4,    1,    1,    1,    2,    5,    4,    3,    2,
    0,    1,    1,    1,    1,    7,    2,    1,    1,    1,
    1,    1,    1,    1,    0,    1,    1,    4,    1,    0,
    2,
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
    0,    0,  169,    0,    2,    0,   30,    0,    0,    0,
    0,    0,    0,   91,    0,    0,    0,  168,    0,    0,
    0,    0,    0,   87,   88,  124,  125,   32,   34,    0,
    0,    0,    0,  152,  153,  154,  155,    0,    0,    0,
  163,  164,  171,  126,  127,  128,  129,  130,  131,  132,
  133,  134,  135,  141,  136,    0,  139,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  137,
  143,  144,  145,    0,    0,   18,    0,    0,    0,  107,
    0,   86,    0,   19,    0,    0,    0,    0,    0,    0,
  142,  166,  167,    0,    0,    0,    0,    0,  108,    0,
  109,  111,    0,    0,    0,    0,    0,    0,    0,  147,
    0,    0,    0,    0,    0,    0,  117,  116,   81,   82,
    0,  110,  112,  113,   89,   76,   77,    0,    0,    0,
   20,   21,    0,    0,   24,    0,    0,    0,    0,   84,
   85,  114,    0,    0,    0,   79,   80,    0,    0,    0,
    0,   83,  119,  120,  121,  122,    0,  123,   78,  146,
   22,    0,    0,    0,  118,    0,    0,    0,   25,  158,
  159,  160,  161,  162,    0,  156,
};
const short yydgoto[] = {                                       2,
  165,   47,   69,   48,   68,   49,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   75,  112,  166,   60,
  174,   61,   62,   63,   64,   89,   65,  225,   66,   76,
   98,  119,  194,  195,  184,  205,  236,  206,  185,  186,
  104,  113,  152,  172,  114,  115,   90,  208,  247,  240,
  116,  117,  145,  146,  164,  128,  265,   94,  108,
};
const short yysindex[] = {                                   -245,
 -203,    0,    0,  -32, -198,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  -32,    0,   22,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -239, -239,
 -239, -239, -239,   26, -239, -239,    0, -182, -194,    0,
    0,    0,    0,    0, -198, -174, -198, -198, -198, -198,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -152,
 -198, -198,    0,   63,    0,  -79,    0, -141,  -79,  -79,
  -79,  -79, -106,    0,  -79,  -79, -208,    0, -155, -118,
   46,  121,  -89,    0,    0,    0,    0,    0,    0,  146,
  148,  153,  167,    0,    0,    0,    0, -106,  169,  177,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -102,    0, -102, -134, -203,
  -75,  191, -203, -203, -203, -203, -106, -203, -203,    0,
    0,    0,    0,  155,   78,    0,  -67,  -90,  -63,    0,
   26,    0,   78,    0,  208,  209,  210, -106,  217,  221,
    0,    0,    0,  -76, -203, -203,  -70,  -54,    0,  -26,
    0,    0,  225, -203, -203,  -69, -203, -203, -203,    0,
 -203, -203,   78,  176, -152, -152,    0,    0,    0,    0,
  -25,    0,    0,    0,    0,    0,    0,   78,   -2,   -2,
    0,    0,  227,  -46,    0,  257, -100,  183, -244,    0,
    0,    0,  186, -100, -106,    0,    0, -203, -102, -203,
 -203,    0,    0,    0,    0,    0,  224,    0,    0,    0,
    0, -102,  274,  275,    0, -203, -203, -191,    0,    0,
    0,    0,    0,    0, -203,    0,
};
const short yyrindex[] = {                                      0,
   42,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   52,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -214, -214,
 -214, -214, -214,    0, -214, -214,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -222,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  -13,
    0,    0,    0,  261,    0,    0,    0,   58,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   76,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -35,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -36,    0,  -30,    0,  -42,
    0,  288,  -42,  -42,  -42,  -42,  -29,  -42,  -42,    0,
    0,    0,    0,    0,  -68,    0,   86,   89,   92,    0,
    0,    0,  -68,    0,    0,    0,    0,  -28,    0,    0,
    0,    0,    0,    0,   53,   53,  130,  136,    0,  138,
    0,    0,    0,   53,   53,    0,  -42,  -42,  -42,    0,
  -12,  -42,  -61,    0,  -37,  -37,    0,    0,    0,    0,
  139,    0,    0,    0,    0,    0,    0,  -61,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -42,    0,  290,
  -42,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -27,    0,    0,    0, -154,  -12,    0,    0,    0,
    0,    0,    0,    0,  276,    0,
};
const short yygindex[] = {                                      0,
   -1,    0,    0,  289,   -6,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  197,   67, -119,    0,
 -133,    0,    0,    0,    0,  166,    0,   81,    0,    0,
    0,    0,    0,  122, -153,  -96,  119, -177,    0,  140,
   45,    0,    0,    0, -135,    0,    0,    0,    0,  112,
    0,    0, -121, -105,    0, -110,    0,    0,    0,
};
#define YYTABLESIZE 389
const short yytable[] = {                                       4,
    5,  204,    5,  140,  148,  150,  151,  140,  150,  138,
  133,  149,  148,  138,  149,  148,  157,  157,  220,  196,
  175,  151,  140,  150,  160,  204,  160,  151,  138,  149,
  148,  111,  204,  165,    1,  176,  177,   72,  179,  180,
  165,  243,  244,  245,  246,  151,  178,  203,  207,  227,
   31,   73,   74,   31,  218,  165,  220,   29,   29,    5,
  207,   29,  131,  221,  234,  157,  132,  200,   96,    3,
   99,  100,  101,  102,   67,  260,  261,  262,  222,  223,
   71,    5,  226,   95,  105,  106,  151,  140,  150,  263,
  264,  207,   93,  138,  149,  148,    5,   97,  207,  219,
  239,  134,  135,  136,  137,  138,  139,  140,  141,  142,
  143,    5,    5,    5,    5,  115,  144,  103,  251,  115,
  182,  254,  183,  107,  250,  100,    5,    5,  101,  100,
  160,  103,  101,  252,  118,  103,  149,  219,  134,  135,
  136,  137,  138,  139,  140,  141,  142,  143,  161,  162,
  163,  173,  173,  147,  134,  135,  136,  137,  138,  139,
  140,  141,  142,  143,  150,  120,  121,  122,  123,  102,
  131,  129,  130,  102,  132,  104,  109,  105,  106,  104,
  151,  105,  106,  209,  210,  124,  125,  126,  127,  153,
  110,  154,  216,  217,  131,  173,  155,  109,  132,  224,
  109,  131,  165,  190,  191,  132,  165,  109,  165,  165,
  156,  110,  158,  165,  110,  165,  167,  168,  169,  170,
  159,  110,  165,  211,  212,  187,  188,  189,    5,  165,
  171,  192,    5,  140,    5,  151,  140,  150,  253,  138,
  213,    5,  138,  149,  148,  157,    6,  181,    5,  230,
  231,  197,  198,  199,  258,  224,   77,   78,   79,   80,
  201,   91,   92,  266,  202,  215,  229,  235,  214,  232,
  238,    7,    8,    9,   10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
   26,   27,   28,   29,   30,   31,   32,   33,  239,   34,
  241,   35,   36,   37,   38,   39,  242,   40,   41,  249,
   42,   43,   44,   45,   46,    5,  255,  256,  257,  170,
    5,   81,   82,   83,   84,   85,   86,   87,   88,    1,
   33,   90,    5,    5,    5,   70,  193,  259,  237,  233,
  248,    0,  228,    0,    0,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    0,    5,    0,    5,    5,    5,    5,    5,
    0,    5,    5,    0,    5,    5,    5,    5,    5,
};
const short yycheck[] = {                                       1,
   43,  102,   45,   40,  110,   41,   44,   44,   44,   40,
  107,   41,   41,   44,   44,   44,   44,  128,  196,  173,
  154,   59,   59,   59,  146,  102,  148,   41,   59,   59,
   59,  111,  102,  102,  280,  155,  156,  277,  158,  159,
  102,  286,  287,  288,  289,   59,  157,  124,  184,  203,
  273,  291,  292,  276,  124,  124,  234,  272,  273,  102,
  196,  276,  271,  197,  218,   93,  275,  178,   75,  273,
   77,   78,   79,   80,  273,  267,  268,  269,  198,  199,
   59,  124,  202,  278,   91,   92,  124,  124,  124,  281,
  282,  227,  275,  124,  124,  124,   44,  272,  234,  196,
  345,  257,  258,  259,  260,  261,  262,  263,  264,  265,
  266,   59,  267,  268,  269,   40,  272,  270,  238,   44,
   43,  241,   45,   61,  235,   40,  281,  282,   40,   44,
  252,   40,   44,  239,  276,   44,   91,  234,  257,  258,
  259,  260,  261,  262,  263,  264,  265,  266,  283,  284,
  285,  153,  154,  272,  257,  258,  259,  260,  261,  262,
  263,  264,  265,  266,   44,   99,  100,  101,  102,   40,
  271,  105,  106,   44,  275,   40,  277,   40,   40,   44,
  270,   44,   44,  185,  186,  292,  293,  294,  295,   44,
  291,   44,  194,  195,  271,  197,   44,  277,  275,  201,
  277,  271,  271,  294,  295,  275,  275,  277,  277,  271,
   44,  291,   44,  275,  291,  277,  292,  293,  294,  295,
   44,  291,  291,  294,  295,  293,  294,  295,  271,  291,
   40,  295,  275,  270,  277,  273,  273,  273,  240,  270,
  295,  274,  273,  273,  273,  273,  279,   93,  291,  205,
  206,   44,   44,   44,  256,  257,   60,   61,   62,   63,
   44,   65,   66,  265,   44,   41,   91,  270,  295,  295,
   44,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  345,  332,
   44,  334,  335,  336,  337,  338,  124,  340,  341,  124,
  343,  344,  345,  346,  347,  274,   93,   44,   44,   59,
  279,  296,  297,  298,  299,  300,  301,  302,  303,  278,
  273,   44,  345,   44,   59,   47,  171,  257,  220,  218,
  229,   -1,  203,   -1,   -1,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,   -1,  332,   -1,  334,  335,  336,  337,  338,
   -1,  340,  341,   -1,  343,  344,  345,  346,  347,
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
"fragF32Reg : R C",
"fragF16Reg : H decCalc",
"fragF16Reg : H C",
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
#line 652 "crc.y"


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
	printf( "inst slots %d \n", inst );
	return 0;
} 







#line 915 "y.tab.c"
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
#line 332 "crc.y"
{ print(); }
break;
case 8:
#line 347 "crc.y"
{ dump(); clean();}
break;
case 26:
#line 388 "crc.y"
{ prc( 0 ); }
break;
case 27:
#line 389 "crc.y"
{ prc( 1 ); }
break;
case 28:
#line 390 "crc.y"
{ prc( 3 ); }
break;
case 29:
#line 391 "crc.y"
{ prc( 0 );	}
break;
case 30:
#line 395 "crc.y"
{ cnd( 1 ); }
break;
case 31:
#line 396 "crc.y"
{ cnd( 0 ); }
break;
case 32:
#line 399 "crc.y"
{ sat( 1 ); }
break;
case 33:
#line 400 "crc.y"
{ sat( 0 ); }
break;
case 34:
#line 403 "crc.y"
{ type = 0; }
break;
case 35:
#line 407 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DDX ); }
break;
case 36:
#line 408 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DDY ); }
break;
case 37:
#line 409 "crc.y"
{ opv( NV40_FP_OP_OPCODE_FLR ); }
break;
case 38:
#line 410 "crc.y"
{ opv( NV40_FP_OP_OPCODE_FRC ); }
break;
case 39:
#line 411 "crc.y"
{ opv( 0xff ); }
break;
case 40:
#line 412 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MOV ); }
break;
case 41:
#line 413 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK2H ); }
break;
case 42:
#line 414 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK2US ); }
break;
case 43:
#line 415 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK4B ); }
break;
case 44:
#line 416 "crc.y"
{ opv( NV40_FP_OP_OPCODE_PK4UB ); }
break;
case 45:
#line 419 "crc.y"
{ opv( NV40_FP_OP_OPCODE_COS ); }
break;
case 46:
#line 420 "crc.y"
{ opv( NV40_FP_OP_OPCODE_EX2 ); }
break;
case 47:
#line 421 "crc.y"
{ opv( NV40_FP_OP_OPCODE_LG2 ); }
break;
case 48:
#line 422 "crc.y"
{ opv( NV40_FP_OP_OPCODE_RCP ); }
break;
case 49:
#line 423 "crc.y"
{ opv( NV40_FP_OP_OPCODE_RSQ ); }
break;
case 50:
#line 424 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SIN ); }
break;
case 51:
#line 425 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP2H ); }
break;
case 52:
#line 426 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP2US ); }
break;
case 53:
#line 427 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP4B ); }
break;
case 54:
#line 428 "crc.y"
{ opv( NV40_FP_OP_OPCODE_UP4UB ); }
break;
case 56:
#line 434 "crc.y"
{ opv( NV40_FP_OP_OPCODE_ADD ); }
break;
case 57:
#line 435 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DP3 ); }
break;
case 58:
#line 436 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DP4 ); }
break;
case 59:
#line 437 "crc.y"
{ opv( NV40_FP_OP_OPCODE_DST ); }
break;
case 60:
#line 438 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MAX ); }
break;
case 61:
#line 439 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MIN ); }
break;
case 62:
#line 440 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MUL ); }
break;
case 63:
#line 441 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SEQ ); }
break;
case 64:
#line 442 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SGE ); }
break;
case 65:
#line 443 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SGT ); }
break;
case 66:
#line 444 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SLE ); }
break;
case 67:
#line 445 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SLT ); }
break;
case 68:
#line 446 "crc.y"
{ opv( NV40_FP_OP_OPCODE_SNE ); }
break;
case 69:
#line 447 "crc.y"
{ opv( NV40_FP_OP_OPCODE_ADD ); }
break;
case 70:
#line 452 "crc.y"
{ opv( NV40_FP_OP_OPCODE_MAD ); }
break;
case 71:
#line 453 "crc.y"
{ opv( 0xff ); }
break;
case 72:
#line 456 "crc.y"
{ opv( NV40_FP_OP_OPCODE_KIL ); }
break;
case 73:
#line 460 "crc.y"
{ opv( NV40_FP_OP_OPCODE_TEX ); }
break;
case 74:
#line 461 "crc.y"
{ opv( NV40_FP_OP_OPCODE_TXP ); }
break;
case 75:
#line 464 "crc.y"
{ opv( NV40_FP_OP_OPCODE_TXD ); }
break;
case 76:
#line 468 "crc.y"
{src_reg( v ); }
break;
case 77:
#line 469 "crc.y"
{src_reg( v ); }
break;
case 81:
#line 480 "crc.y"
{src_reg( v ); }
break;
case 82:
#line 481 "crc.y"
{src_reg( v ); }
break;
case 83:
#line 485 "crc.y"
{ mod = 1; }
break;
case 86:
#line 494 "crc.y"
{ out_reg( v );  cnd_swz(); type = 0; h = 0; printf( "\n" ); }
break;
case 90:
#line 504 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_TR ); }
break;
case 92:
#line 510 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_EQ ); }
break;
case 93:
#line 511 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_GE ); }
break;
case 94:
#line 512 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_GT ); }
break;
case 95:
#line 513 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_LE ); }
break;
case 96:
#line 514 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_LT ); }
break;
case 97:
#line 515 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_NE ); }
break;
case 98:
#line 516 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_TR ); }
break;
case 99:
#line 517 "crc.y"
{ cnd_msk( NV40_FP_OP_COND_FL ); }
break;
case 100:
#line 521 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 0, 0 ) ); }
break;
case 101:
#line 522 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 0, 0 ) ); }
break;
case 102:
#line 523 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 0, 0 ) ); }
break;
case 103:
#line 524 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 0, 1, 0 ) ); }
break;
case 104:
#line 525 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 1, 0 ) ); }
break;
case 105:
#line 526 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 1, 0 ) ); }
break;
case 106:
#line 527 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 1, 0 ) ); }
break;
case 107:
#line 528 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 0, 0, 1 ) ); }
break;
case 108:
#line 529 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 0, 1 ) ); }
break;
case 109:
#line 530 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 0, 1 ) ); }
break;
case 110:
#line 531 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 0, 1 ) ); }
break;
case 111:
#line 532 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 0, 1, 1 ) ); }
break;
case 112:
#line 533 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 0, 1, 1 ) ); }
break;
case 113:
#line 534 "crc.y"
{ msk( NV40_FP_DEST_MASK( 0, 1, 1, 1 ) ); }
break;
case 114:
#line 535 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 1, 1 ) ); }
break;
case 115:
#line 536 "crc.y"
{ msk( NV40_FP_DEST_MASK( 1, 1, 1, 1 ) ); }
break;
case 119:
#line 548 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_POSITION ); }
break;
case 120:
#line 549 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_COL0 ); }
break;
case 121:
#line 550 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_COL1 ); }
break;
case 122:
#line 551 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_FOGC ); }
break;
case 123:
#line 552 "crc.y"
{ set_input( NV40_FP_OP_INPUT_SRC_TC0 + v ); }
break;
case 126:
#line 559 "crc.y"
{ n = 0; }
break;
case 127:
#line 560 "crc.y"
{ n = 1; }
break;
case 128:
#line 561 "crc.y"
{ n = 2; }
break;
case 129:
#line 562 "crc.y"
{ n = 3; }
break;
case 130:
#line 563 "crc.y"
{ n = 4; }
break;
case 131:
#line 564 "crc.y"
{ n = 5; }
break;
case 132:
#line 565 "crc.y"
{ n = 6; }
break;
case 133:
#line 566 "crc.y"
{ n = 7; }
break;
case 134:
#line 567 "crc.y"
{ n = 8; }
break;
case 135:
#line 568 "crc.y"
{ n = 9; }
break;
case 136:
#line 573 "crc.y"
{ v = n; }
break;
case 137:
#line 574 "crc.y"
{ v = v * 10 + n; }
break;
case 138:
#line 578 "crc.y"
{ h = 0; }
break;
case 139:
#line 579 "crc.y"
{ h = 0; v = 31; creg = 1; }
break;
case 140:
#line 582 "crc.y"
{ h = 1; }
break;
case 141:
#line 583 "crc.y"
{ h = 1; v = 63; creg = 1; }
break;
case 143:
#line 590 "crc.y"
{ v = 0; h = 0; }
break;
case 144:
#line 591 "crc.y"
{ v = 0; h = 1; }
break;
case 152:
#line 607 "crc.y"
{ com( 0 ); }
break;
case 153:
#line 608 "crc.y"
{ com( 1 ); }
break;
case 154:
#line 609 "crc.y"
{ com( 2 ); }
break;
case 155:
#line 610 "crc.y"
{ com( 3 ); }
break;
case 156:
#line 613 "crc.y"
{ set_tex( v ); }
break;
case 163:
#line 629 "crc.y"
{ set_cns(); cnst = 1;}
break;
case 164:
#line 630 "crc.y"
{ set_cns(); cnst = 1;}
break;
case 165:
#line 634 "crc.y"
{ neg = 0; }
break;
case 166:
#line 635 "crc.y"
{ neg = 0; }
break;
case 167:
#line 636 "crc.y"
{ neg = 1; }
break;
#line 1570 "y.tab.c"
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
