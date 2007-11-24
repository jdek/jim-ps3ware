#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "consts.h"
#include "vec2.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define CHECK_FOR_NANS( v ) { assert( !IS_NAN((v).x) && !IS_NAN((v).y) && !IS_NAN((v).z) ); }
#else
#define CHECK_FOR_NANS( v )
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class vec3
{
public:
	float x;
	float y;
	float z;

	vec3()
	{
#ifdef _DEBUG
		MAKE_NAN( x );
		MAKE_NAN( y );
		MAKE_NAN( z );
#endif
	}

	vec3 &maximize(const vec3 &v)
	{
		if( x < v.x ) x = v.x;
		if( y < v.y ) y = v.y;
		if( z < v.z ) z = v.z;
		return *this;
	}

	vec3 &minimize(const vec3 &v)
	{
		if( x > v.x ) x = v.x;
		if( y > v.y ) y = v.y;
		if( z > v.z ) z = v.z;
		return *this;	
	}

	vec3( const float _x, const float _y, const float _z ) : x(_x), y(_y), z(_z)
	{
		CHECK_FOR_NANS( *this );
	}

	vec3( const vec3 &a ) : x( a.x ), y( a.y ), z( a.z )
	{
		CHECK_FOR_NANS( *this );
	}

	vec3( const vec2 &v, float _z ) : x( v.x ), y( v.y), z( _z )
	{
		CHECK_FOR_NANS( *this );
	}

	// mathematical operations
	vec3& operator+=( const vec3 &v ) 
	{ 
		CHECK_FOR_NANS( *this );
		CHECK_FOR_NANS( v );

		x += v.x; 
		y += v.y; 
		z += v.z; 
		return *this; 
	}

	vec3& sub( const vec3 &v ) 
	{
		return (*this) -= v;
	}

	vec3& add( const vec3 &v ) 
	{
		return (*this) += v;
	}

	vec3& div( const float &d ) 
	{
		return (*this) /= d;
	}

	vec3& mul( const float &d ) 
	{
		return (*this) *= d;
	}

	vec3& operator-=( const vec3 &v ) 
	{ 
		CHECK_FOR_NANS( *this );
		CHECK_FOR_NANS( v );

		x -= v.x; 
		y -= v.y; 
		z -= v.z; 
		return *this; 
	}

	vec3& operator*=( const float d ) 
	{ 
		x *= d; 
		y *= d; 
		z *= d;

		CHECK_FOR_NANS( *this );		
		return *this; 
	}

	vec3& operator/=( const float d ) 
	{ 
		float d1 = 1.0f / d; 
		x *= d1; 
		y *= d1; 
		z *= d1; 

		CHECK_FOR_NANS( *this );
		return *this; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const vec3 operator -( const vec3 &a) { return vec3(-a.x, -a.y, -a.z); }
inline const vec3 operator+( const vec3 &a, const vec3 &b ) { return vec3( a.x + b.x, a.y + b.y, a.z + b.z ); }
inline const vec3 operator-( const vec3 &a, const vec3 &b ) { return vec3( a.x - b.x, a.y - b.y, a.z - b.z ); }
inline float dot( const vec3 &a, const vec3 &b ) { CHECK_FOR_NANS(a); CHECK_FOR_NANS(b); return ( a.x*b.x + a.y*b.y + a.z*b.z ); }
inline const vec3 operator*( const vec3 &a, const float b ) { return vec3( a.x*b, a.y*b, a.z*b ); }
inline const vec3 operator*( const float a, const vec3 &b ) { return vec3( b.x*a, b.y*a, b.z*a ); }
inline const vec3 operator/( const vec3 &a, const float b ) { float b1 = 1.0f/b; return vec3( a.x*b1, a.y*b1, a.z*b1 ); }
inline const vec3 cross( const vec3 &a, const vec3 &b ) { return vec3( a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y ); }
inline float fabs2( const vec3 &a ) { CHECK_FOR_NANS(a); return dot( a, a ); }


//inline float fabs( const vec3 &a ) { CHECK_FOR_NANS(a); return sqrt( dot( a, a ) ); }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct mat3x4
{
	float vec[3][4];
	mat3x4()
	{
		for( int i = 0; i < 12; ++i )
		{
			vec[0][i] = 0.0f;
		};
		vec[0][0] = vec[1][1] = vec[2][2] = 1.0f;
	};
	
	float GetNormEstimate() const
	{
		mat3x4 m;
		float error = 0.0f;

		for( size_t i = 0; i < 3; ++i )
		{
			for( size_t j = 0; j < 4; ++j )
			{
				float d = m.vec[i][j] - vec[i][j];
				error += d * d;
			}
		}

		return error;
	}

	void FromPtr( const float *ptr )
	{
		for( size_t i = 0; i < 3; ++i )
		{
			for( size_t j = 0; j < 4; ++j )
			{
				vec[i][j] = ptr[ i + j * 4];
			}
		}
	}

	mat3x4( const float *ptr )
	{
		for( int i = 0; i < 12; ++i )
		{
			vec[0][i] = ptr[i];
		};
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void inline combine( const mat3x4 &a, const mat3x4 &b, mat3x4 *res )
{
	for( size_t i = 0; i < 3; ++i )
	{
		for( size_t j = 0; j < 3; ++j )
		{
			res->vec[i][j] = a.vec[i][0] * b.vec[0][j] +
				               a.vec[i][1] * b.vec[1][j] +
											 a.vec[i][2] * b.vec[2][j];

		};

		res->vec[i][3]   = a.vec[i][0] * b.vec[0][3] +
				               a.vec[i][1] * b.vec[1][3] +
											 a.vec[i][2] * b.vec[2][3] +
											 a.vec[i][3];
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#undef CHECK_FOR_NANS



