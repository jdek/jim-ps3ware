#include <squish.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>

#include "../../src/types.h"
#include "../../src/textures/textures.h"
#include "../common/conv.h"


enum ETGAImageType
{
    TGAIT_NOIMAGEDATA = 0,
    TGAIT_COLOR_MAPPED = 1,
    TGAIT_TRUE_COLOR = 2,
    TGAIT_BLACK_WHITE = 3,
    TGAIT_RLE_COLOR_MAPPED = 9,
    TGAIT_RLE_TRUE_COLOR = 10,
    TGAIT_RLE_BLACK_WHITE = 11
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack( 1 )
// describe the color map (if any) used for the image
struct SColorMapSpecification
{
	uint16 wFirstEntryIndex;// Index of the first color map entry. Index refers to the starting entry in loading the color map.
	uint16 wColorMapLength; // Total number of color map entries included
	uint8 cColorMapEntrySize;// Establishes the number of bits per entry. Typically 15, 16, 24 or 32-bit values are used
};
//
struct SImageDescriptor
{
	uint8 cAlphaChannelBits : 4; // the number of attribute bits per pixel
	uint8 cLeftToRightOrder : 1; // left-to-right ordering
	uint8 cTopToBottomOrder : 1; // top-to-bottom ordering
	uint8 cUnused : 2; // Must be zero to insure future compatibility
};

// describe the image screen location, size and pixel depth

struct SImageSpecification
{
	uint16 wXOrigin; // absolute horizontal coordinate for the lower left corner of the image as it is positioned on a display device having an origin at the lower left of the screen
	uint16 wYOrigin; // absolute vertical coordinate for the lower left corner of the image as it is positioned on a display device having an origin at the lower left of the screen
	uint16 wImageWidth; // width of the image in pixels
	uint16 wImageHeight; // height of the image in pixels
	uint8 cPixelDepth; // number of bits per pixel. This number includes the Attribute or Alpha channel bits. Common values are 8, 16, 24 and 32 but other pixel depths could be used.
	SImageDescriptor descriptor; //
};

struct STGAFileHeader
{
	uint8 cIDLength; // identifies the number of bytes contained in Field 6, the Image ID Field
	uint8 cColorMapType; // indicates the type of color map (if any) included with the image
	uint8 cImageType; // TGA File Format can be used to store Pseudo-Color, True-Color and Direct-Color images of various pixel depths
	SColorMapSpecification colormap; //
	SImageSpecification imagespec; //
};

struct STGAFileFooter
{
	uint32 dwExtensionAreaOffset; // offset from the beginning of the file to the start of the Extension Area
	uint32 dwDeveloperDirectoryOffset;
	// offset from the beginning of the file to the start of the Developer Directory
	uint8 cSignature[16]; // "TRUEVISION-XFILE"
	uint8 cReservedCharacter; // must be '.'
	uint8 cBinaryZeroStringTerminator; // '\0'
};

#pragma pack()

struct Rectangle
{
	size_t width;
	size_t height;
	size_t x;
	size_t y;
};

struct SortR
{
	size_t  price;
	size_t	order;

	bool operator < ( const SortR &s ) const
	{
		return price > s.price;
	}
};

void Sort( const Rectangle *rects, size_t size, size_t *order )
{
	std::vector<SortR> sorts( size );

	for( size_t i = 0; i < size; ++i )
	{
		order[i] = i;
		const Rectangle &rect = rects[i];
		sorts[i].price = ( rect.width << 16 ) + rect.height;
		sorts[i].order = i;
	}

	std::sort( sorts.begin(), sorts.end() );

	for( size_t i = 0; i < size; ++i )
	{
		order[i] = sorts[i].order;
	}
}

bool Pack( Rectangle *rects, size_t size, const size_t *order, size_t width, size_t height )
{
	std::vector<size_t> line( width, 0 );

	for( size_t i = 0; i < size; ++i )
	{
		Rectangle &rect = !order ? rects[i] : rects[order[i]];

		size_t minMaxHeight = height;
		size_t minMaxIndex = 0;

		for( size_t k = 0; k + rect.width <= width; ++k )
		{
			size_t maxHeight = 0;
			for( size_t j = k; j < k + rect.width; ++j )
			{
				if( maxHeight < line[j] )
				{
					maxHeight = line[j];
				}
			}
			if( minMaxHeight > maxHeight )
			{
				minMaxHeight = maxHeight;
				minMaxIndex = k;
			}
		}

		if( minMaxHeight + rect.height > height )
		{
			return false;
		}

		rect.y = minMaxHeight;
		rect.x = minMaxIndex;

		for( size_t j = minMaxIndex; j < minMaxIndex + rect.width; ++j )
		{
			line[j] = minMaxHeight + rect.height;
		}
	}

	printf( "%d %d\n", width, height );
	for( size_t i = 0; i < size; ++i )
	{
		Rectangle &rect = !order ? rects[i] : rects[order[i]];
		printf( "w = %d h = %d x = %d y = %d \n",  rect.width, rect.height, rect.x, rect.y );
	}

	return true;
}

struct FOpen
{
	FILE *fp;
	FOpen( const char *file, bool read )
	{
		fp = fopen( file, read ? "rb" : "wb" );
	}
	~FOpen()
	{
		fclose( fp );
	}
};

struct Notifier
{
	virtual void Length( int add ) = 0;
	virtual void Mip() = 0;
};

uint8 noise( uint8 v )
{
    return v;
}

void ConvertTexture2D( const texture_desc_t *desc, const uint8_t *user_rgba, uint8_t *out, float gamma, Notifier *note )
{
	uint8 pixels[4][4][4];  // 16 pixels of input
	uint8 dxt[16];      // 8-16 bytes of output

	if( desc->mips == 0 )
	{
		return;
	}

	note->Mip();

	int block = -1;
	int comp = -1;

	if( desc->format == DXT1 )
	{
		block = 8;
		comp =  squish::kDxt1;
	}
	if( desc->format == DXT3 )
	{
		printf( "3" );
		block = 16;
		comp =  squish::kDxt3;
	}
	if( desc->format == DXT5 )
	{
		printf( "5" );
		block = 16;
		comp =  squish::kDxt5;
	}


	for( int i = 0; i < desc->height; i += 4 )
	{
		for( int j = 0; j < desc->width; j += 4 )
		{

			if( user_rgba )
			{
				for( int i1 = 0; i1 < 4; ++i1 )
				{
					for( int j1 = 0; j1 < 4; ++j1 )
					{
						int offset = ( ( j + j1 ) + ( i + i1 ) * desc->width ) * 4;
						
						pixels[i1][j1][0] = noise( user_rgba[offset + 2] );
						pixels[i1][j1][1] = noise( user_rgba[offset + 1] );
						pixels[i1][j1][2] = noise( user_rgba[offset + 0] );
						pixels[i1][j1][3] = noise( user_rgba[offset + 3] );

					}
				}
				squish::Compress( &pixels[0][0][0], dxt, comp );
				memcpy( out, dxt, block );
				out += block;
			}
			note->Length( block );
		}
	}

	if( desc->height > 4 && desc->width > 4 )
	{

		std::vector<uint8_t> newData;

		if( user_rgba )
		{
			
			newData.resize( desc->width * desc->height );
			float inv = 1.0f / gamma;
			for( int i = 0; i < desc->height; i += 2 )
			{
				for( int j = 0; j < desc->width; j += 2 )
				{
					float r = 0.0f;
					float g = 0.0f;
					float b = 0.0f;
					float a = 0.0f;

					for( int i1 = 0; i1 < 2; ++i1 )
					{
						for( int j1 = 0; j1 < 2; ++j1 )
						{
							int offset = ( ( j + j1 ) + ( i + i1 ) * desc->width ) * 4;
							r += 0.25f * pow( user_rgba[offset + 0], gamma );
							g += 0.25f * pow( user_rgba[offset + 1], gamma );
							b += 0.25f * pow( user_rgba[offset + 2], gamma );
							a += 0.25f * pow( user_rgba[offset + 3], gamma );

						}
					}

					int offset = ( j * 2 + i * desc->width );
					newData[offset + 0] = (uint8_t)pow ( r, inv );
					newData[offset + 1] = (uint8_t)pow ( g, inv );
					newData[offset + 2] = (uint8_t)pow ( b, inv );
					newData[offset + 3] = (uint8_t)pow ( a, inv );

				}
			}
		}
		texture_desc_t newDesc = *desc;
		newDesc.width /= 2;
		newDesc.height /= 2;
		newDesc.mips--;
		ConvertTexture2D( &newDesc, user_rgba ? &newData[0] : 0, out, gamma, note );
		
	}
}

struct LengthMips : public Notifier
{
	int length;
	int mips;

	LengthMips() : length ( 0 ), mips( 0 ){};

	void Length( int add )
	{
		length += add;
	}
	void Mip()
	{
		++mips;
	}
};

struct Counter : public Notifier
{
	int mips;
	int length;
	int total;
	int percent;

	Counter() : length ( 0 ), mips( 0 ), percent( -1 ) {};

	void Length( int add )
	{

		length += add;
		int newPercent = ( length * 10 ) / total;
		if( newPercent != percent )
		{
			printf( "percent = %d \n", newPercent * 10 );
			percent = newPercent;
		}

	}
	void Mip()
	{}
}
;

uint16 endian( uint16 a )
{
	return ( a >> 8 ) | ( ( a & 0xff ) << 8 );
}


struct TgaFile
{
	std::vector<uint8>  data;
	int width;
	int height;

	uint8 *GetPixel( size_t i, size_t j ) 
	{	
		return &data[ ( i * width + j ) * 4 ];

	}
	void SetSize( size_t size )
	{
		width = height = size;
		data.resize( width * height * 4, 255 );
	}
	void Pack( size_t i, size_t j, TgaFile &src )
	{
		for( size_t ii = 0; ii < src.width; ++ii )
		{
			
			for( size_t jj = 0; jj < src.height; ++jj )
			{
				const uint8 *from = src.GetPixel( ii, jj );
				uint8 *to = GetPixel( ii + i, jj + j );
				to[0] = from[0];
				to[1] = from[1];
				to[2] = from[2];
				to[3] = from[3];
					
			}
		}
	}
	bool Init( const char *file )
	{
		std::vector<uint8>  scan;

		FOpen fp( file, true );

		if( !fp.fp )
		{
			printf( "can not open file %s \n", file );
			return false;
		}

		STGAFileHeader hdr;

		if( !fread( &hdr, sizeof( hdr ), 1, fp.fp ) )
		{
			printf( "cannot read tga header from file %s \n", file );
			return false;
		}

		if( hdr.cImageType != TGAIT_TRUE_COLOR )
		{
			printf( "not TGAIT_TRUE_COLOR file %s \n", file );
			return file;
		}

		int bpp = -1;

		if( hdr.imagespec.cPixelDepth == 24 )
		{
			bpp = 3;
		}

		if( hdr.imagespec.cPixelDepth == 32 )
		{
			bpp = 4;
		}

		if( bpp == -1 )
		{
			printf( "not 24 or 32 image depth for file %s \n", file );
			return -1;
		}



		width = endian( hdr.imagespec.wImageWidth );
		height = endian( hdr.imagespec.wImageHeight );
		scan.resize( width * 4 );
		data.resize( width * height * 4, 255 );

		printf( "file %s width %d height %d bpp %d\n", file, width, height, bpp );

		for( int i = 0; i < height; ++i )
		{

			memset( &scan[0], 255, width * 4 );
			fread( &scan[0], width * bpp, 1, fp.fp );
			int ioff = hdr.imagespec.descriptor.cTopToBottomOrder ? height - 1 - i : i;
			ioff = ioff * width * 4;

			for( int j = 0; j < width; ++j )
			{
				int off = hdr.imagespec.descriptor.cLeftToRightOrder ? j : width - 1 - j;
				off = ioff + off * 4;
				for( int k = 0; k < bpp; ++k )
				{
					data[off + k] = scan[j * bpp + k];
				}
			}
		}

		return true;

	}
};

int main( int argc, const char *argv[] )
{
	texture_desc_t desc;
	if( argc < 5 )
	{
		printf( "params: [-dxt1:-dxt3:-dxt5] [mips num] infile outfile\n" );

		return -1;

	}

	int mode = -1;

	if( !strcmp( argv[1], "-dxt1" ) )
	{
		mode = 1;
		desc.format = DXT1;
	}

	if( !strcmp( argv[1], "-dxt3" ) )
	{
		mode = 3;
		desc.format = DXT3;
	}

	if( !strcmp( argv[1], "-dxt5" ) )
	{
		mode = 5;
		desc.format = DXT5;
	}

	if( mode ==- 1 )
	{
		printf( "specify [-dxt1:-dxt3:-dxt5]\n" );
		return -1;
	}

	FOpen out_file( argv[argc - 1], false );

	TgaFile tf;

	if( argc == 5 )
	{

		if( !tf.Init( argv[3] ) )
		{
			return -1;
		}
	}
	else
	{
		std::vector<TgaFile>  files;
		files.resize( argc - 4 );

		for( size_t i = 0; i < files.size(); ++i )
		{
			if( !files[i].Init( argv[3 + i] ) )
			{
				return -1;
			}
		}

		std::vector<Rectangle> rects;
		std::vector<size_t> order;
		order.resize( files.size() );
		rects.resize( files.size() );
		for( size_t i = 0; i < files.size(); ++i )
		{
			rects[i].width = files[i].width;
			rects[i].height = files[i].height;
		}
		Sort( &rects[0], files.size(), &order[0] );

		size_t size = 0;
		for( size_t j = 0; j < 16; ++j )
		{
			if( Pack( &rects[0], files.size(), &order[0], 1 << j, 1 << j ) )
			{
				
				FOpen out_atlas( ( std::string( argv[argc - 1 ] ) + ".atlas" ).c_str(), false );
				size = 1 << j;
				tf.SetSize( size );
				float mul = 1.0f / (float)size;
				for( size_t i = 0; i < files.size(); ++i )
				{
					uint16 v[8];
					

					float x = ( rects[i].x + 1.0f ) * mul;
					float y = ( rects[i].y + 1.0f ) * mul;
					float dx = ( rects[i].width  - 2.0f ) * mul;
					float dy = ( rects[i].height - 2.0f ) * mul;
					
					
					v[1] = floatToHalf( x );
					v[0] = floatToHalf( y );
					v[3] = floatToHalf( x + dx );
					v[2] = floatToHalf( y );
					v[5] = floatToHalf( x + dx );
					v[4] = floatToHalf( y + dy );
					v[7] = floatToHalf( x );
					v[6] = floatToHalf( y + dy );
					
					fwrite( v, 16, 1, out_atlas.fp );
					tf.Pack( rects[i].x, rects[i].y, files[i] );	
				}
				break;
			}
		}
		if( size == 0 )
		{
			printf( "too big atlas \n" );
			return -1;
		}
		
	}

	printf( "packing...\n" );
	
	int mips = atoi( argv[2] );
	LengthMips lm;
	Counter cnt;
	desc.width = tf.width;
	desc.mips = mips;
	desc.height = tf.height;


	ConvertTexture2D( &desc, 0, 0, 1.2f, &lm );

	desc.length = lm.length;
	std::vector<uint8> out( lm.length );
	cnt.total = lm.length;


	ConvertTexture2D( &desc, &tf.data[0], &out[0], 1.2f, &cnt );

	printf( "result mips %d result size = %d \n", lm.mips, lm.length );

	desc.mips = lm.mips;

	fwrite( &desc, sizeof( desc ), 1, out_file.fp );
	fwrite( &out[0], out.size(), 1, out_file.fp );


	return 0;

}
