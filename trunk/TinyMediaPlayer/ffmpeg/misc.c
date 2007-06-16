#include <inttypes.h>

float rintf(float x)
{
    __asm 
	{
        fld x
        frndint
    }
}

double rint(double x)
{
    __asm 
	{
        fld x
        frndint
	}
}

long int lrintf(float flt)
{
	int intgr;

	_asm
	{       
		fld flt
		fistp intgr
	}

	return intgr;
}

double getNan()
{
	union
	{
		double d;
		int64_t x;
	} v;

	v.x = 0x7FF80000;	// gcc returns this for 0.0 / 0.0

	return v.d;
}
