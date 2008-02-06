#pragma once
/** Converts float in uint32 format to a a half in uint16 format*/
static inline uint16 floatToHalfI(uint32 i)
{
    register int s =  (i >> 16) & 0x00008000;
    register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
    register int m =   i        & 0x007fffff;
        
    if (e <= 0)
    {
        if (e < -10)
        {
            return 0;
        }
        m = (m | 0x00800000) >> (1 - e);
        
        return s | (m >> 13);
    }
    else if (e == 0xff - (127 - 15))
    {
        if (m == 0) // Inf
        {
            return s | 0x7c00;
        } 
        else    // NAN
        {
            m >>= 13;
            return s | 0x7c00 | m | (m == 0);
        }
    }
    else
    {
        if (e > 30) // Overflow
        {
            return s | 0x7c00;
        }
        
        return s | (e << 10) | (m >> 13);
    }
}


static inline uint16 floatToHalf(float i)
{
    union { float f; uint32 i; } v;
    v.f = i;
    return floatToHalfI(v.i);
}

