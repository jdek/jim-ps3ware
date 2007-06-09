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

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef int int16_t;
typedef unsigned int uint16_t;

typedef long int32_t;
typedef unsigned long uint32_t;

#if defined(_MSC_VER)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

typedef int16_t intptr_t;
typedef uint16_t uintptr_t;

typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

#endif