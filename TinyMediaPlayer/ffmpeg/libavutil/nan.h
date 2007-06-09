#ifndef _NAN_H
#define _NAN_H

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

#endif