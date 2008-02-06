#include "common_spu.h"
#include "structs.h"
#include <math.h>

#include <float.h>


uint16 ia = 0, ib = 0, ic = 0;

vec_uint4 inline uint16_uload( qword address )
{
    vec_uint4 qw = (vec_uint4)si_lqd( address, 0 );
    vec_uint4 ao = spu_and( (vec_uint4)address, spu_splats( (unsigned int )0xf ) );
    vec_uint4 am = spu_shuffle( ao, ao, (vec_uchar16)spu_splats( (unsigned int) 0x80800303 ) );
    am = spu_add( am, spu_splats( (unsigned int)0x80800001 ) );
    return spu_shuffle( qw, qw, (vec_uchar16)am );
}

vec_uint4 inline f2h( vec_uint4 h )
{
    vec_uint4 s = spu_and( spu_rlmask( h, -16 ), spu_splats( (unsigned int)0x8000) );
    vec_uint4 e = spu_and( spu_rlmask( h, -23 ), spu_splats( (unsigned int)0xff) );
    vec_uint4 z = spu_cmpgt( e,  spu_splats( (unsigned int)112 ) );
    vec_uint4 m = spu_and( h, spu_splats( (unsigned int)0x7fffff) );
    
    e = spu_sub( e, spu_splats( (unsigned int)112 ) );
    
    e = spu_sel( spu_splats( (unsigned int)0x0 ), e, z );
    
    e = spu_rl( e, 10 );

    m = spu_rlmask( m, -13 );

    return spu_or( e, spu_or( m, s ) ); 

}

vec_float4 inline h2f( vec_uint4 h )
{

    vec_uint4 s = spu_and( spu_rlmask( h, -15 ), spu_splats( (unsigned int)0x1) );
    vec_uint4 e = spu_and( spu_rlmask( h, -10 ), spu_splats( (unsigned int)0x1f) );
    vec_uint4 m = spu_and( h, spu_splats( (unsigned int)0x3ff) );
    
    e = spu_add( e, spu_splats( (unsigned int)112 ) );
    
    e = spu_rl( e, 23 );
    m = spu_rl( m, 13 );
    s = spu_rl( s, 31 );
    
    return (vec_float4)spu_or( e, spu_or( m, s ) ); 
}


struct sin_cos
{
    vec_float4 scsc[257];
    
    sin_cos()
    {
	vec_float4 start = (vec_float4){ +1.0f, 0.0f, -1.0f, 0.0f };
	vec_float4 co = (vec_float4){ +0.999698819f, +0.999698819f, +0.999698819f, +0.999698819f };
	vec_float4 si = (vec_float4){ +0.024541228f, -0.024541228f, +0.024541228f, -0.024541228f };
	
	for( size_t i = 0; i < 257; ++i )
	{
	    scsc[i] = start;
	    start = start * co + si * YXWZ( start );
	    
	}
	
	//printf( "%f %f\n", ((float *)scsc)[1026], ((float *)scsc)[1027] );
	
    }	
    
    vec_float4 value( vec_float4 f )
    {
	vec_int4  i = spu_convts( f, 8 );
	vec_float4 fi = spu_convtf( i, 8 );
	vec_float4 d = spu_mul( spu_sub( f, fi ), spu_splats( 256.0f ) );

	unsigned int ind = si_to_uint( (qword)i ) & 255;
	vec_float4 a = scsc[ind + 0];
	vec_float4 b = scsc[ind + 1];
	
	return spu_madd( spu_sub( b, a ), d, a );
	
    }
};


sin_cos table;

	    
void inline process_linear_spline( qword &address, vec_float4 &value, vec_float4 time, vec_uint4 startFrame, vec_uint4 endFrame )
{
    vec_uint4 num = spu_rl( uint16_uload( address ), 1 );
    vec_uint4 dec = spu_sub( num, spu_splats( (unsigned int)0x2) );
    vec_uint4 ptr = spu_sel( num, dec, spu_cmpgt( num, (unsigned int)0x2 ) );
    vec_uint4 fptr = spu_add( (vec_uint4)address, ptr );
    
    vec_uint4 beg = spu_splats( (unsigned int)0x0 );
    vec_uint4 end = dec;
    vec_uint4 ibeg = startFrame;
    vec_uint4 iend = endFrame;
    
    while( 1 )
    {
	vec_uint4 bega = spu_add( beg, spu_splats( (unsigned int)0x4 ) );
	vec_uint4 tst = spu_cmpgt( bega, end );
	
	unsigned int ret = si_to_uint( (qword)tst );
	
	if( ret == 0xffffffff )
	{
	    break;
	}
	
	vec_uint4 med = spu_rl( spu_rlmask( spu_add( beg, end ), -2 ), 1 );
	vec_uint4 i = uint16_uload( (qword)spu_add( med, (vec_uint4)address ) );
	
	vec_float4 fi = spu_convtf( i, 0 );
	vec_uint4 b = spu_cmpgt( time, fi );
	
	beg = spu_sel( beg, med, b );
	end = spu_sel( med, end, b );
	
	ibeg = spu_sel( ibeg, i, b );
	iend = spu_sel( i, iend, b );
	
    }
    
    vec_uint4 f1 = uint16_uload( (qword)spu_add( beg, fptr) ); 
    vec_uint4 f2 = uint16_uload( (qword)spu_add( end, fptr) ); 
    
    vec_float4 f = h2f( spu_shuffle( f1, f2, SWZ{ F_X, S_X, E_4, E_4 } ) );
    vec_float4 times = spu_convtf( spu_shuffle( ibeg, iend, SWZ{ S_X, F_X, E_4, E_4 } ), 0 );
    vec_float4 d = spu_sub( times, time );
    
    vec_float4 mul = spu_sub( times, YX( times ) );
    mul = spu_mul( spu_re( mul ), spu_mul( f, d ) );
    value = spu_add( mul, Y( mul ) );
    address = (qword)spu_add( fptr, num );
}

vec_float4 inline conv(
    vec_uint4 kbeg,
    vec_uint4 kend,
    vec_uint4 vbeg,
    vec_uint4 vend,
    vec_float4 time )
{
    vec_float4 fbeg = h2f( vbeg );
    vec_float4 fend = h2f( vend );
    
    vec_float4 tbeg = spu_convtf( kbeg, 0 );
    vec_float4 tend = spu_convtf( kend, 0 );
    
    vec_float4 dtbeg = spu_sub( time, tbeg );
    vec_float4 dtend = spu_sub( tend, time );
    vec_float4 mul = spu_re( spu_sub( tend, tbeg ) );
    
    vec_float4 xyzw = spu_add( spu_mul( dtbeg, fend ), spu_mul( dtend, fbeg ) );
    xyzw = spu_mul( mul, xyzw );
    return xyzw;
}

void inline process_linear_spline( 
    qword &address, 
    vec_uint4 &kbeg,
    vec_uint4 &kend,
    vec_uint4 &vbeg,
    vec_uint4 &vend,
    vec_float4 time, 
    vec_uint4 startFrame, 
    vec_uint4 endFrame )
{
    vec_uint4 num = spu_rl( uint16_uload( address ), 1 );
    vec_uint4 dec = spu_sub( num, spu_splats( (unsigned int)0x2) );
    vec_uint4 ptr = spu_sel( num, dec, spu_cmpgt( num, (unsigned int)0x2 ) );
    vec_uint4 fptr = spu_add( (vec_uint4)address, ptr );
    
    vec_uint4 beg = spu_splats( (unsigned int)0x0 );
    vec_uint4 end = dec;
    vec_uint4 ibeg = startFrame;
    vec_uint4 iend = endFrame;
    
    //printf( "%d \n", si_to_uint( (qword)num ) );
    while( 1 )
    {
	vec_uint4 bega = spu_add( beg, spu_splats( (unsigned int)0x4 ) );
	vec_uint4 tst = spu_cmpgt( bega, end );
	
	unsigned int ret = si_to_uint( (qword)tst );
	
	if( ret == 0xffffffff )
	{
	    break;
	}
	
	vec_uint4 med = spu_rl( spu_rlmask( spu_add( beg, end ), -2 ), 1 );
	vec_uint4 i = uint16_uload( (qword)spu_add( med, (vec_uint4)address ) );
	
	vec_float4 fi = spu_convtf( i, 0 );
	vec_uint4 b = spu_cmpgt( time, fi );
	
	beg = spu_sel( beg, med, b );
	end = spu_sel( med, end, b );
	
	ibeg = spu_sel( ibeg, i, b );
	iend = spu_sel( i, iend, b );
	
    }

    address = (qword)spu_add( fptr, num );
    
    vec_uint4 f1 = uint16_uload( (qword)spu_add( beg, fptr) ); 
    vec_uint4 f2 = uint16_uload( (qword)spu_add( end, fptr) ); 
    
    vbeg = spu_shuffle( vbeg, f1, SWZ{ F_Y, F_Z, F_W, S_X } );
    vend = spu_shuffle( vend, f2, SWZ{ F_Y, F_Z, F_W, S_X } );
    
    kbeg = spu_shuffle( kbeg, ibeg, SWZ{ F_Y, F_Z, F_W, S_X } );
    kend = spu_shuffle( kend, iend, SWZ{ F_Y, F_Z, F_W, S_X } );
    
    
}


instance_data_t	insts[256] __attribute__((aligned(128)));;
BinaryParticle 	particles[1024] __attribute__((aligned(128)));
char	       	chunks[4][2048] __attribute__((aligned(128)));
char	       	fly[4] = { 0 };
uint16         	startFrames[4];
uint16         	endFrames[4];
float 	       	realTimes[4];
float 	       	dst[11];


uint16 number = 0;


struct Vertex
{
    float x, z, y;
    short u, v;
    short r,g,b,a;
};


Vertex   outBuffer[512];
size_t   outPtr = 0;
uint16	 coords[256][8];

void inline DoParticle( size_t next, size_t inst )
{

    //printf( "a4 \n" );

    float time = realTimes[next];
    char *data = chunks[next];
    vec_uint4 startFrame = (vec_uint4)si_from_uint( startFrames[next] );
    vec_uint4 endFrame = (vec_uint4)si_from_uint( endFrames[next] );
    mfc_write_tag_mask( 1 << next  );
    mfc_read_tag_status_any();
    
    float  x, y, z, u, v;
    short  r, g, b, a;
    
    qword dptr = si_from_ptr( data );
    
    vec_float4 vtime = spu_splats( time );
    vec_float4 vx, vy, vz, vu, vv, vp, vs;
    
    
    process_linear_spline( dptr, vx, vtime, startFrame, endFrame );
    process_linear_spline( dptr, vy, vtime, startFrame, endFrame );
    process_linear_spline( dptr, vz, vtime, startFrame, endFrame );
    
    vec_uint4  kbeg = spu_splats( (unsigned int)0 );
    vec_uint4  kend = spu_splats( (unsigned int)0 );
    vec_uint4  vbeg = spu_splats( (unsigned int)0 );
    vec_uint4  vend = spu_splats( (unsigned int)0 );
    
    //printf( "a7 \n" );

    
    process_linear_spline( dptr, kbeg, kend, vbeg, vend, vtime, startFrame, endFrame );
    process_linear_spline( dptr, kbeg, kend, vbeg, vend, vtime, startFrame, endFrame );
    process_linear_spline( dptr, kbeg, kend, vbeg, vend, vtime, startFrame, endFrame );
    process_linear_spline( dptr, kbeg, kend, vbeg, vend, vtime, startFrame, endFrame );
    
    //printf( "a6 \n" );

    
    vec_float4 rgba = conv( kbeg, kend, vbeg, vend, vtime );
    rgba = spu_mul( *(vec_float4 *)insts[inst].col, rgba );
    vec_uint4  colh = f2h( ( vec_uint4 ) rgba );
    
    process_linear_spline( dptr, vu, vtime, startFrame, endFrame );
    process_linear_spline( dptr, vv, vtime, startFrame, endFrame );
    process_linear_spline( dptr, vp, vtime, startFrame, endFrame );
    process_linear_spline( dptr, vs, vtime, startFrame, endFrame );

    float ix, iy, iz;
    
    ix = *(float *)&vx;
    iy = *(float *)&vy;
    iz = *(float *)&vz;
    
    
    x = ix * insts[inst].mat[0] + iy * insts[inst].mat[1] + iz * insts[inst].mat[2] + insts[inst].mat[3];
    y = ix * insts[inst].mat[4] + iy * insts[inst].mat[5] + iz * insts[inst].mat[6] + insts[inst].mat[7];
    z = ix * insts[inst].mat[8] + iy * insts[inst].mat[9] + iz * insts[inst].mat[10] + insts[inst].mat[11];
    
    
    
    r = ((unsigned  int *)&colh)[0];
    g = ((unsigned  int *)&colh)[1];
    b = ((unsigned  int *)&colh)[2];
    a = ((unsigned  int *)&colh)[3];
    
    
    u = *(float *)&vu;
    v = *(float *)&vv;

    u *= insts[inst].scale;
    v *= insts[inst].scale;
            

    
    Vertex &a00 = outBuffer[(outPtr + 0 ) & 511];
    Vertex &a01 = outBuffer[(outPtr + 1 ) & 511];
    Vertex &a11 = outBuffer[(outPtr + 2 ) & 511];
    Vertex &a10 = outBuffer[(outPtr + 3 ) & 511];
    
    vec_uint4 ind = spu_convtu( vs, 0 );
    uint32_t sprite = si_to_uint( (qword)ind ) & 255;    
    
    
    vec_float4 rot = table.value( XXXX( vp ) );
    
    float si = ((float *)&rot)[0];
    float co = ((float *)&rot)[1];
    
    a00.x = x + co * u - si * v;
    a01.x = x + co * u + si * v;
    a11.x = x - co * u + si * v;
    a10.x = x - co * u - si * v;
    
    a00.y = y - si * u - co * v;
    a01.y = y - si * u + co * v;
    a11.y = y + si * u + co * v;
    a10.y = y + si * u - co * v;
    
    a00.z = z;
    a01.z = z;
    a11.z = z;
    a10.z = z;
    
    a00.u = coords[sprite][0];
    a00.v = coords[sprite][1];
    a01.u = coords[sprite][2];
    a01.v = coords[sprite][3];
    a11.u = coords[sprite][4];
    a11.v = coords[sprite][5];
    a10.u = coords[sprite][6];
    a10.v = coords[sprite][7];
        
    a00.r = r;
    a01.r = r;
    a11.r = r;
    a10.r = r;
    
    a00.g = g;
    a01.g = g;
    a11.g = g;
    a10.g = g;
    
    a00.b = b;
    a01.b = b;
    a11.b = b;
    a10.b = b;
    
    a00.a = a;
    a01.a = a;
    a11.a = a;
    a10.a = a;
    
    
    outPtr += 4;
    ++number;
}



void SampleParticles( float _time, uint32 num, unsigned long long base, uint16 loopFrame, size_t inst )
{
	size_t j = 0;
    	for( size_t i = 0; i < num; ++i )
	{
	    uint16 startFrame = particles[i].startFrame;
	    uint16 endFrame = particles[i].endFrame;
	    
	    
	    float realTime = 0.0f;
	    bool calc = false;
	    
	    float time = _time;
	    
	    if( time >= startFrame && time <= endFrame )
	    {
		realTime = time;
		calc = true;
	    }
	    else
	    {
	    
		time += loopFrame;
	    
		if( time >= startFrame && time <= endFrame )
		{
		    realTime = time;
		    calc = true;
		}
	    }
	
	    if( calc )
	    {
	
		mfc_get( &chunks[j][0], base + particles[i].qwordOffset * 16, particles[i].qwordSize * 16, j, 0, 0 );
		fly[j] = 1;
		
		//printf( "%d \n", particles[i].qwordSize * 16 );
		startFrames[j] = startFrame;
		endFrames[j] = endFrame;
		realTimes[j] = realTime;
		size_t next = ( j + 1 ) & 3;
		if( fly[next] == 1 )
		{
		    fly[next] = 0;
		    DoParticle( next, inst );
		}
		j = next;
	    }
	}
	
	for( size_t i = 0; i < 4; ++i )
	{
	    if( fly[i] == 1 )
	    {
		fly[i] = 0;
		DoParticle( i, inst );
	    }
	
	}
}




int main(unsigned long long spe_id, unsigned long long program_data_ea, unsigned long long env)
{
	//unsigned long long te;
	spu_write_decrementer( 0xffffffff );
	
	
	while( 1 )
	{

		spu_read_in_mbox();
		ParticleHeader header;
		program_data_t data;
		
		mfc_get( &data, program_data_ea, sizeof( data ), 0, 0, 0 );
		mfc_write_tag_mask( 1 );
		mfc_read_tag_status_any();
		
		//printf( "%x %x \n", data.atlas, data.asize );
		
		mfc_get( &coords[0], data.atlas, data.asize, 0, 0, 0 );
		mfc_get( &insts[0], data.insts, data.isize * sizeof( instance_data_t ), 0, 0, 0 );
		mfc_get( &header, data.fx, sizeof( header ), 0, 0, 0 );
		mfc_write_tag_mask( 1 );
		mfc_read_tag_status_any();

		int particleSize =  header.trackOffset - header.particleOffset;
		
		
		mfc_get( &particles[0], data.fx + header.particleOffset, particleSize, 0, 0, 0 );
		mfc_write_tag_mask( 1 );
		mfc_read_tag_status_any();
		
		//printf( "a1\n" );
		

		data.quads = 0;
		
		for( size_t i = 0; i < data.isize; ++i )
		{
		    outPtr = 0;
		    
		    uint32_t t = insts[i].time;
		    
		    float ftime = ( t >> 8 ) % ( header.loopFrame ) + ( t & 255 ) / 256.0f;
		    
		    SampleParticles( ftime, header.particles, data.fx + header.trackOffset, header.loopFrame, i );			
		    mfc_put( outBuffer, data.dynamic, 24 * outPtr, 0, 0, 0 );
		    mfc_write_tag_mask( 1 );
		    mfc_read_tag_status_any();
		    data.quads += outPtr / 4;   
		    data.dynamic += 24 * outPtr;
		}

		mfc_put( &data, program_data_ea, sizeof( data ), 0, 0, 0 );
		mfc_write_tag_mask( 1 );
		mfc_read_tag_status_any();
		
		
		spu_write_out_mbox( 3 );
		
	}
	return 0;
}
