#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <vector>
#include <map>
#include "StdAfx.h"
#include "../common/XMLUtils.h"
#include "../common/conv.h"
#include "../../src/hilevel/fx.h"

template<class T> void BinDump( const T &value, std::vector<uint8> &data )
{
    for( size_t i = 0; i < sizeof( T ); ++i )
    {
	data.push_back( ((uint8 *)&value)[i] );
    }
}

typedef std::pair<float, uint16> Pair;

enum Type
{
    FLOAT,
    UINT8
};




uint8 saturateub( float v )
{
    int iv = int( v * 255.0f );
    if( iv < 0 )
    {
	return 0;
    } 
    if( iv > 255 )
    {
	return 255;
    }
    return uint8(iv);
}

struct CTrack
{
	
	std::vector<Pair>  pairs;
	
	void DumpBinary( std::vector<uint8> &data, Type type )
	{
	    uint16 size = (uint16)pairs.size();
	    
	    printf( "%d \n", size );
	    
	    BinDump( size, data );
	    for( size_t i = 1; i + 1 < pairs.size(); ++i )
	    {
		BinDump( pairs[i].second, data );
	    }
	    for( size_t i = 0; i < pairs.size(); ++i )
	    {
		if( type == FLOAT )
		{

		    BinDump( floatToHalf (pairs[i].first ), data );
		}
		if( type == UINT8 )
		{
		    BinDump( saturateub( pairs[i].first ), data );
		}
	    }
	}
	
	void Add( float value, uint16 key )
	{
		pairs.push_back( Pair( value, key ) );
	}
	
	void Out( const char * name )
	{
	    printf( "%s  ", name );
	    for( size_t i = 0; i < pairs.size(); ++i )
	    {
		printf( "%2.4f:%d ", pairs[i].first, pairs[i].second );
	    }
	    printf( "\n" );
	}
	
	void Compress( float delta, CTrack &out )
	{
	    uint16 first = 0;
	    uint16 last = 2;
	    
	    
	    float start = pairs[0].first; 
	    out.pairs.push_back( pairs[0] );
	    
	    
	    bool ret = true;
	    for( size_t i = 0; i < pairs.size(); ++i )
	    {
		if( fabs( start - pairs[i].first ) > delta )
		{
		    ret = false;
		    break;
		}
		
	    }
	    
	    if( ret )
	    {
		return;
	    }
	    
	    while( 1 )
	    {
		++last;
		if( last >= pairs.size() )
		{
		    out.pairs.push_back( pairs.back() );
		    return;
		}
		
		float firstValue = pairs[first].first;
		float lastValue = pairs[last].first;
		float dist = 1.0f / ( last - first );
		bool dump = false;
		
		for( uint16 n = first + 1; n + 1 < last; ++n )
		{
		
		    if( fabs( pairs[n].first - firstValue * ( n - first ) * dist  - lastValue * ( last - n ) * dist ) > delta )
		    {
			dump = true;
			break;
		    }
		    
		}
		
		if( dump )
		{
		    out.pairs.push_back( pairs[last - 1] );
		    first = last;
		    last = first + 2;
		}
	    }
	}
};


enum Mode
{
	XYZ, RGBA, SCALE, ANGLE, SPRITE,
};



struct ParticleTrack
{
	CTrack data[11];
	CTrack optimized[11];
	std::vector<uint8> bin;
	
	uint16 startFrame;
	uint16 endFrame;
	
	
	void Out()
	{
	
	    int local = 0;
	    static float deltas[11] = { 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f };
	    Type types[11] = { FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT };
	    for( size_t i = 0; i < 11; ++i )
	    {
		data[i].Compress( deltas[i], optimized[i] );
		optimized[i].DumpBinary( bin, types[i] );
	    }
	    
	    local = bin.size();
	    if( local & 15 )
	    {
		local += ( 16 - local & 15 );
	    }
	    bin.resize( local, 0 );
	}

	
	
	void ParseXYZ(const xmlNode *xmlKey, uint16 ind )
	{
		data[0].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"x" ) ), ind );
		data[1].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"y" ) ), ind );
		data[2].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"z" ) ), ind );
	}
	
	void ParseRGBA(const xmlNode *xmlKey, uint16 ind )
	{

		data[3].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"r" ) ), ind );
		data[4].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"g" ) ), ind );
		data[5].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"b" ) ), ind );
		data[6].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"a" ) ), ind );
	}

	void ParseScale(const xmlNode *xmlKey, uint16 ind )
	{
		data[7].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"x" ) ) * 0.5f, ind );
		data[8].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"y" ) ) * 0.5f, ind );
	}

	
	void ParseAngle(const xmlNode *xmlKey, uint16 ind )
	{
		data[9].Add( atof( (const char *)LoadStrAttr( xmlKey, _X"value" ) ) / 6.28318531f, ind );
	}
	
	void ParseSprite(const xmlNode *xmlKey, uint16 ind )
	{
		data[10].Add( (float)atoi( (const char *)LoadStrAttr( xmlKey, _X"value" ) ) + 0.5f, ind );
	}
	
	void Parse( const xmlNode *xmlTrack, Mode mode )
	{
	
		for ( xmlNode *xmlKey = xmlTrack->children; xmlKey; xmlKey = xmlKey->next )
		{
			if ( xmlStrcmp( xmlKey->name, _X"key" ) != 0 )
				continue;

			int time = atoi( (const char *)LoadStrAttr( xmlKey, _X"time" ) );
			
			if( mode == XYZ )
			{
				ParseXYZ( xmlKey, (uint16)time );
			}
			else if( mode == RGBA )
			{
				ParseRGBA( xmlKey, (uint16)time );
			}
			else if( mode == SCALE )
			{
				ParseScale( xmlKey, (uint16)time );
			}
			else if( mode == ANGLE )
			{
				ParseAngle( xmlKey, (uint16)time );
			}
			if( mode == SPRITE )
			{
				ParseSprite( xmlKey, (uint16)time );
			}
			
		}
	}
	
	void ParseInfo( xmlNode *xmlParticle )
	{
		startFrame = atoi( (const char *)LoadStrAttr( xmlParticle, _X"startFrame" ) );
		endFrame = atoi( (const char *)LoadStrAttr( xmlParticle, _X"endFrame" ) );

	}
	

	void Parse( xmlNode *xmlParticle )
	{
		Parse( Find( xmlParticle->children, _X"position" ), XYZ );
		Parse( Find( xmlParticle->children, _X"color" ), RGBA );
		Parse( Find( xmlParticle->children, _X"scale" ), SCALE );
		Parse( Find( xmlParticle->children, _X"rotation" ), ANGLE );		
		Parse( Find( xmlParticle->children, _X"sprite" ), SPRITE );

	}

};




struct FOpen
{
  FILE *fp;
  FOpen( const char *file, bool read )
  {
    fp = fopen( file, read ? "rb" : "wb" );
  }
  ~FOpen()
  {
    if( fp == 0 )
    {
	return;
    }
    fclose( fp );
  }
};


bool LoadParticleTemplate( const xmlNode *root, const char *fileDump, int loop )
{
	

	const xmlNode *particlesSet = Find( root->children, _X"particles" );
	//int numFrames = atoi( (const char *)LoadStrAttr( particlesSet, _X"frames" ) );
	//int numTracks = atoi( (const char *)LoadStrAttr( particlesSet, _X"tracks" ) );

	std::vector<BinaryParticle> particles;
	std::vector<uint8> bin;	
		
	for ( ; particlesSet; particlesSet = particlesSet->next )
	{
		for( xmlNode *xmlParticle = particlesSet->children; xmlParticle; xmlParticle = xmlParticle->next )
		{
			if ( xmlStrcmp( xmlParticle->name, _X"particle" ) != 0 )
				continue;

			
			ParticleTrack track;
			
			track.ParseInfo( xmlParticle );
			
			if( track.startFrame <= loop )
			{
			
			    uint16 length = bin.size();
			    bin.swap( track.bin );
			    track.Parse( xmlParticle );

			    track.Out();
			    BinaryParticle bp;
			    bp.startFrame = track.startFrame;
			    bp.endFrame = track.endFrame;
			    bp.qwordOffset = length;
			    bp.qwordSize = track.bin.size() - length;

			    assert(	( bp.qwordSize & 15 ) == 0 );
			    assert(	( bp.qwordOffset & 15 ) == 0 );
			
			    bp.qwordSize /= 16;
			    bp.qwordOffset /= 16;
						
			    particles.push_back( bp );
			    bin.swap( track.bin );
			}
		}
	}
	
	printf( "particles %d  pack size %d \n", particles.size(), bin.size() );
	
	uint32 length = particles.size();
	if( length & 1 )
	{
	    particles.resize( length + 1 );
	}
	
	ParticleHeader header;
	header.particles = (uint16)length;
	header.loopFrame = loop;
	header.trackSize = bin.size();
	header.particleOffset = sizeof( header );
	header.trackOffset = sizeof( header) + sizeof( particles[0] ) * particles.size();
	
	FOpen fp( fileDump, false );
	
	if( fp.fp == 0 )
	{
	    printf( "file %s can not be opened \n", fileDump );
	    return false;
	}
	
	fwrite(	&header, 1, sizeof( header ), fp.fp );
	fwrite( &particles[0], particles.size(), sizeof( particles[0] ), fp.fp );
	fwrite( &bin[0], 1, bin.size(), fp.fp );
	return true;
}





int main( int argc, const char *argv[2] )
{
  	if( argc != 4 )
  	{
    		printf( "params: infile outfile loopFrame \n" );
		return -1;
	}

  	xmlDoc *doc = NULL;
	xmlNode *root = NULL;
	
	int loop = atoi( argv[3] );

	doc = xmlReadFile( argv[1], NULL, 0 );

	if (doc == NULL) 
	{
		printf( "error: could not parse file %s", argv[1] );
		return -1;
	}

	//printf( "%d \n", size );
	root = xmlDocGetRootElement(doc);
	if( LoadParticleTemplate( root, argv[2], loop ) )
	{
	    printf( "file dumped \n" );
	}

 	return 0;

}
