/*
   inttypes.h

   Contributors:
     Created by Marek Michalkiewicz <marekm@linux.org.pl>

   THIS SOFTWARE IS NOT COPYRIGHTED

   This source code is offered for use in the public domain.  You may
   use, modify or distribute it freely.

   This code is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
   DISCLAIMED.  This includes but is not limited to warranties of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef __INTTYPES_H_
#define __INTTYPES_H_

/* Use [u]intN_t if you need exactly N bits.
   XXX - doesn't handle the -mint8 option.  */

#define 	PRId8   "d"
#define 	PRIdLEAST8   "d"
#define 	PRIdFAST8   "d"
#define 	PRIi8   "i"
#define 	PRIiLEAST8   "i"
#define 	PRIiFAST8   "i"
#define 	PRId16   "d"
#define 	PRIdLEAST16   "d"
#define 	PRIdFAST16   "d"
#define 	PRIi16   "i"
#define 	PRIiLEAST16   "i"
#define 	PRIiFAST16   "i"
#define 	PRId32   "ld"
#define 	PRIdLEAST32   "ld"
#define 	PRIdFAST32   "ld"
#define 	PRIi32   "li"
#define 	PRIiLEAST32   "li"
#define 	PRIiFAST32   "li"
#define 	PRIdPTR   PRId16
#define 	PRIiPTR   PRIi16
#define 	PRIo8   "o"
#define 	PRIoLEAST8   "o"
#define 	PRIoFAST8   "o"
#define 	PRIu8   "u"
#define 	PRIuLEAST8   "u"
#define 	PRIuFAST8   "u"
#define 	PRIx8   "x"
#define 	PRIxLEAST8   "x"
#define 	PRIxFAST8   "x"
#define 	PRIX8   "X"
#define 	PRIXLEAST8   "X"
#define 	PRIXFAST8   "X"
#define 	PRIo16   "o"
#define 	PRIoLEAST16   "o"
#define 	PRIoFAST16   "o"
#define 	PRIu16   "u"
#define 	PRIuLEAST16   "u"
#define 	PRIuFAST16   "u"
#define 	PRIx16   "x"
#define 	PRIxLEAST16   "x"
#define 	PRIxFAST16   "x"
#define 	PRIX16   "X"
#define 	PRIXLEAST16   "X"
#define 	PRIXFAST16   "X"
#define 	PRIo32   "lo"
#define 	PRIoLEAST32   "lo"
#define 	PRIoFAST32   "lo"
#define 	PRIu32   "lu"
#define 	PRIuLEAST32   "lu"
#define 	PRIuFAST32   "lu"
#define 	PRIx32   "lx"
#define 	PRIxLEAST32   "lx"
#define 	PRIxFAST32   "lx"
#define 	PRIX32   "lX"
#define 	PRIXLEAST32   "lX"
#define 	PRIXFAST32   "lX"
#define 	PRIoPTR   PRIo16
#define 	PRIuPTR   PRIu16
#define 	PRIxPTR   PRIx16
#define 	PRIXPTR   PRIX16
#define 	SCNd16   "d"
#define 	SCNdLEAST16   "d"
#define 	SCNdFAST16   "d"
#define 	SCNi16   "i"
#define 	SCNiLEAST16   "i"
#define 	SCNiFAST16   "i"
#define 	SCNd32   "ld"
#define 	SCNdLEAST32   "ld"
#define 	SCNdFAST32   "ld"
#define 	SCNi32   "li"
#define 	SCNiLEAST32   "li"
#define 	SCNiFAST32   "li"
#define 	SCNdPTR   SCNd16
#define 	SCNiPTR   SCNi16
#define 	SCNo16   "o"
#define 	SCNoLEAST16   "o"
#define 	SCNoFAST16   "o"
#define 	SCNu16   "u"
#define 	SCNuLEAST16   "u"
#define 	SCNuFAST16   "u"
#define 	SCNx16   "x"
#define 	SCNxLEAST16   "x"
#define 	SCNxFAST16   "x"
#define 	SCNo32   "lo"
#define 	SCNoLEAST32   "lo"
#define 	SCNoFAST32   "lo"
#define 	SCNu32   "lu"
#define 	SCNuLEAST32   "lu"
#define 	SCNuFAST32   "lu"
#define 	SCNx32   "lx"
#define 	SCNxLEAST32   "lx"
#define 	SCNxFAST32   "lx"
#define 	SCNoPTR   SCNo16
#define 	SCNuPTR   SCNu16
#define 	SCNxPTR   SCNx16
#define		PRId64 "lld"
#define		PRIx64 "x"
#define		PRIX64 "X"
#ifndef		PRIu64
#define		PRIu64 "llu"
#endif
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef long int32_t;
typedef unsigned long uint32_t;

#if defined(_MSC_VER)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

#endif