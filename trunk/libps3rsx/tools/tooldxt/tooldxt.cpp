#include <squish.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../../src/types.h"
#include "../../src/textures/textures.h"


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

void convert_texture_2D( const texture_desc_t *desc, const uint8_t *user_rgba, uint8_t *out, float gamma, Notifier *note )
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
    block = 16;
    comp =  squish::kDxt3;
  }
  if( desc->format == DXT5 )
  {
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
            pixels[i1][j1][0] = user_rgba[offset + 2];
            pixels[i1][j1][1] = user_rgba[offset + 1];
            pixels[i1][j1][2] = user_rgba[offset + 0];
            pixels[i1][j1][3] = user_rgba[offset + 3];

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

    uint8_t *newData = 0;

    if( user_rgba )
    {
      newData = new uint8_t[ desc->width * desc->height ];

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
    convert_texture_2D( &newDesc, newData, out, gamma, note );
    delete[] newData;

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
  {
  }
};

uint16 endian( uint16 a )
{
	return ( a >> 8 ) | ( ( a & 0xff ) << 8 );
}

int main( int argc, const char *argv[2] )
{
  texture_desc_t desc;
  if( argc < 4 )
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

  int mips = atoi( argv[2] );

  FOpen fp( argv[3], true );
  FOpen out_file( argv[4], false );
  

  if( !fp.fp )
  {
    printf( "cannot open file %s \n", argv[3] );
    return -1;
  }

  STGAFileHeader hdr;

  if( !fread( &hdr, sizeof( hdr ), 1, fp.fp ) )
  {
    printf( "cannot read tga header from file %s \n", argv[3] );
    return -1;
  }

  //fseek( fp.fp, hdr.cIDLength, SEEK_CUR );

  if( hdr.cImageType != TGAIT_TRUE_COLOR )
  {
    printf( "not TGAIT_TRUE_COLOR file %s \n", argv[3] );
    return -1;
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
    printf( "not 24 or 32 image depth for file %s \n", argv[3] );
    return -1;
  }



  int width = endian( hdr.imagespec.wImageWidth );
  int height = endian( hdr.imagespec.wImageHeight );
  uint8 *scan = new uint8[ width * 4 ];
  uint8 *data = new uint8[ width * height * 4 ];
  memset( data, 255, width * height * 4 );

  printf( "file %s width %d height %d bpp %d\n", argv[3], width, height, bpp );

  for( int i = 0; i < height; ++i )
  {

    memset( scan, 255, width * 4 );
    fread( scan, width * bpp, 1, fp.fp );
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


  LengthMips lm;
  Counter cnt;
  desc.width = width;
  desc.mips = mips;
  desc.height = height;
  

  convert_texture_2D( &desc, 0, 0, 1.2f, &lm );
    
  desc.length = lm.length;
  uint8 *out = new uint8[lm.length];
  cnt.total = lm.length;
 

  convert_texture_2D( &desc, data, out, 1.2f, &cnt );
  
  printf( "result mips %d result size = %d \n", lm.mips, lm.length );
  
  desc.mips = lm.mips;
  
  fwrite( &desc, sizeof( desc ), 1, out_file.fp );
  fwrite( out, lm.length, 1, out_file.fp );
  


  delete out;
  delete data;
  delete scan;
  return 0;

}
