%{
#include <stdio.h>
#include "y.tab.h"


%}

_0 _1 _2 _3 _4 _5 _6 _7 _8 _9 BEG END MARK ARL MOV LIT COM RCP RSQ EXP LOG MUL ADD DP3 DP4 DST MIN MAX SLT SGE MAD DOT X Y Z W  OPOS WGHT NRML COL0 COL1 FOGC TEX0 TEX1 TEX2 TEX3 TEX4 TEX5 TEX6 TEX7 ADDREG R HPOS BFC0 BFC1 FOGC PSIZ


%%

"0"			return _0;
"1"			return _1;
"2"			return _2;
"3"			return _3;
"4"			return _4;
"5"			return _5;
"6"			return _6;
"7"			return _7;
"8"			return _8;
"9"			return _9;


"!!VP1.0"               return BEG;
"END"                   return END;
";"                     return MARK;
","                     return COM;
"."                     return DOT;
"o"                     return 'o';
"c"                     return 'c';
"v"                     return 'v';
"["                     return '[';
"]"                     return ']';

ARL 	                return ARL;
MOV                     return MOV;
LIT                     return LIT;
RCP                     return RCP;
RSQ                     return RSQ;
EXP                     return EXP;
LOG                     return LOG;
MUL                     return MUL;
ADD                     return ADD;
DP3                     return DP3;
DP4                     return DP4;
DST                     return DST;
MIN                     return MIN;
MAX                     return MAX;
SLT                     return SLT;
SGE                     return SGE;
MAD                     return MAD;
x                     	return X;
y                     	return Y;
z                     	return Z;
w                     	return W;
OPOS			return OPOS;
WGHT			return WGHT;
NRML			return NRML;
COL0			return COL0;
COL1			return COL1;
TEX0			return TEX0;
TEX1			return TEX1;
TEX2			return TEX2;
TEX3			return TEX3;
TEX4			return TEX4;
TEX5			return TEX5;
TEX6			return TEX6;
TEX7			return TEX7;
A1                      return ADDREG; 
R 			return R;

HPOS 			return HPOS; 
BFC0			return BFC0;
BFC1 			return BFC1;
FOGC 			return FOGC;
PSIZ			return PSIZ;

\n                      /* ignore end of line */;
[ \t]+                  /* ignore whitespace */;
%%
