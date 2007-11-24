#include "StdAfx.h"
#include "ColladaImage.h"


CColladaImage::CColladaImage(void)
{}

CColladaImage::~CColladaImage(void)
{}

CColladaImage *CColladaImage::CreateInstance( const xmlNode *node )
{
  const xmlNode *from;
  if
  (
    ( from = Find( node->children, _X"init_from" ) )
  )
  {
    CColladaImage *image = new CColladaImage();
    image->fileName = from->children->content;
    printf( "%s \n", image->fileName.ptr );
    return image;
  };
  return 0;
}

