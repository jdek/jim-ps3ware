#define M( a, i, j) a[ ( i ) * 4 + ( j )]

#define DEG2RAD ( M_PI / 180.0f )

void identity( float *matrix )
{
	int i, j;
	for( i = 0; i < 4; ++i )
	{
		for( j = 0; j < 4; ++j )
		{
			M( matrix, i, j ) = ( i == j ) ? 1.0f : 0.0f;
		}
	}
}


void multiply( float *a, float *b, float *matrix )
{
	int i, j;
	for( i = 0; i < 4; ++i )
	{
		for( j = 0; j < 4; ++j )
		{
			M( matrix, i, j ) = M( a, i, 0 ) * M( b, 0, j ) +
			                    M( a, i, 1 ) * M( b, 1, j ) +
			                    M( a, i, 2 ) * M( b, 2, j ) +
			                    M( a, i, 3 ) * M( b, 3, j );
		}
	}

}

void translatef( float x, float y, float z, float *matrix )
{
	float a[16];
	float b[16];
	identity( b );
	M( b, 0, 3 ) = x;
	M( b, 1, 3 ) = y;
	M( b, 2, 3 ) = z;
	multiply( matrix, b, a );
	memcpy( matrix, a, sizeof( a ) );
}


void rotatef( float angle, float x, float y, float z, float *matrix )
{
	float a[16];
	float b[16];
	identity( b );

	float s = (float)sin( angle * DEG2RAD );
	float c = (float)cos( angle * DEG2RAD );

	float mag = x * x + y * y + z * z;
	if( mag < 0.0001f )
	{
		return;
	}

	mag = 1.0f / sqrtf( mag );
	x *= mag;
	y *= mag;
	z *= mag;

	float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
	xx = x * x;
	yy = y * y;
	zz = z * z;
	xy = x * y;
	yz = y * z;
	zx = z * x;
	xs = x * s;
	ys = y * s;
	zs = z * s;
	one_c = 1.0f - c;

	M( b, 0, 0 ) = (one_c * xx) + c;
	M( b, 0, 1 ) = (one_c * xy) - zs;
	M( b, 0, 2 ) = (one_c * zx) + ys;

	M( b, 1, 0 ) = (one_c * xy) + zs;
	M( b, 1, 1 ) = (one_c * yy) + c;
	M( b, 1, 2 ) = (one_c * yz) - xs;

	M( b, 2, 0 ) = (one_c * zx) - ys;
	M( b, 2, 1 ) = (one_c * yz) + xs;
	M( b, 2, 2 ) = (one_c * zz) + c;

	multiply( matrix, b, a );
	memcpy( matrix, a, sizeof( a ) );
}

void frustrum( float left, float right, float bottom,float top, float nearval, float farval, float *matrix )
{
	float x, y, a, b, c, d;
	float m[16];
	float r[16];

	x = (2.0F*nearval) / (right-left);
	y = (2.0F*nearval) / (top-bottom);
	a = (right+left) / (right-left);
	b = (top+bottom) / (top-bottom);
	c = -(farval+nearval) / ( farval-nearval);
	d = -(2.0F*farval*nearval) / (farval-nearval);  /* error? */


	M( m, 0, 0 ) = x;     M( m, 0, 1 ) = 0.0f;  M( m, 0, 2 ) = a;      M( m, 0, 3 ) = 0.0f;
	M( m, 1, 0 ) = 0.0f;  M( m, 1, 1 ) = y;     M( m, 1, 2 ) = b;      M( m, 1, 3 ) = 0.0f;
	M( m, 2, 0 ) = 0.0f;  M( m, 2, 1 ) = 0.0f;  M( m, 2, 2 ) = c;      M( m, 2, 3 ) = d;
	M( m, 3, 0 ) = 0.0f;  M( m, 3, 1 ) = 0.0f;  M( m, 3, 2 ) = -1.0f;  M( m, 3, 3 ) = 0.0f;

	multiply( matrix, m, r );
	memcpy( matrix, r, sizeof( r ) );

}

