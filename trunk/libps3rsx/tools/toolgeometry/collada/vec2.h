#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "consts.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define CHECK_FOR_NANS( v ) { assert( !IS_NAN((v).x) && !IS_NAN((v).y) /*, "vec2 operation with Not A Number"*/ ); }
#else
#define CHECK_FOR_NANS( v )
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class vec2
{
public:
	float x;
	float y;

	vec2()
	{
#ifdef _DEBUG
		MAKE_NAN( x );
		MAKE_NAN( y );
#endif
	}

	vec2( const float _x, const float _y ) : x(_x), y(_y)
	{
		CHECK_FOR_NANS( *this );
	}

	vec2( const vec2 &a ) : x( a.x ), y( a.y )
	{
		CHECK_FOR_NANS( *this );
	}

	// mathematical operations
	vec2& operator+=( const vec2 &v ) 
	{ 
		CHECK_FOR_NANS( *this );
		CHECK_FOR_NANS( v );

		x += v.x; 
		y += v.y; 
		return *this; 
	}

	vec2& operator-=( const vec2 &v ) 
	{ 
		CHECK_FOR_NANS( *this );
		CHECK_FOR_NANS( v );

		x -= v.x; 
		y -= v.y; 
		return *this; 
	}

	vec2& operator*=( const float d ) 
	{ 
		x *= d; 
		y *= d; 

		CHECK_FOR_NANS( *this );		
		return *this; 
	}

	vec2& operator/=( const float d ) 
	{ 
		float d1 = 1.0f / d; 
		x *= d1; 
		y *= d1; 

		CHECK_FOR_NANS( *this );
		return *this; 
	}

	float Normalize()
	{
		CHECK_FOR_NANS( *this );

		float length2 = x*x + y*y;
		if ( length2 < FP_EPSILON2 )
		{
			x = 0;
			y = 0;
			return 0.0f;
		}

		float length = sqrt( length2 );
		(*this) /= length;
		return length;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const vec2 operator -( const vec2 &a) { return vec2(-a.x, -a.y); }
inline const vec2 operator+( const vec2 &a, const vec2 &b ) { return vec2( a.x + b.x, a.y + b.y ); }
inline const vec2 operator-( const vec2 &a, const vec2 &b ) { return vec2( a.x - b.x, a.y - b.y ); }
inline float dot( const vec2 &a, const vec2 &b ) { CHECK_FOR_NANS(a); CHECK_FOR_NANS(b); return ( a.x*b.x + a.y*b.y ); }
inline const vec2 operator*( const vec2 &a, const float b ) { return vec2( a.x*b, a.y*b ); }
inline const vec2 operator*( const float a, const vec2 &b ) { return vec2( b.x*a, b.y*a ); }
inline const vec2 operator/( const vec2 &a, const float b ) { float b1 = 1.0f/b; return vec2( a.x*b1, a.y*b1 ); }
inline float fabs2( const vec2 &a ) { CHECK_FOR_NANS(a); return dot( a, a ); }
inline float fabs( const vec2 &a ) { CHECK_FOR_NANS(a); return sqrt( dot( a, a ) ); }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#undef CHECK_FOR_NANS
