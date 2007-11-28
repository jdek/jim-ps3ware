// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <math.h>
#include <float.h>
#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>
#include <typeinfo>
#include <time.h>


typedef unsigned char uint8;
typedef   signed char sint8;

typedef unsigned short uint16;
typedef   signed short sint16;

typedef unsigned int uint32;
typedef   signed int sint32;

typedef unsigned long long uint64;
typedef   signed long long sint64;


typedef unsigned char uint8_t;
typedef   signed char sint8_t;

typedef unsigned short uint16_t;
typedef   signed short sint16_t;

typedef unsigned int uint32_t;
typedef   signed int sint32_T;

typedef unsigned long long uint64_t;
typedef   signed long long sint64_t;

#define LENGTH(X) (sizeof(X)/sizeof(X[0]))

struct SAutoInd
{
	uint16 value;
	static uint16 GetBadValue(){ return 0xfFfF;}
	SAutoInd():value( GetBadValue() ){};
};

struct SFatVertex
{
	float  coo[3];
	float  nor[3];
	float  tx0[2];
	unsigned char weights[4];
	unsigned char indices[4];
};

struct SSimpleVertex
{
	float  coo[3];
	float  nor[3];
	float  tx0[2];
};


