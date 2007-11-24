#include "StdAfx.h"
#include "ColladaIDREFArray.h"

CColladaIDREFArray::CColladaIDREFArray()
{}

CColladaIDREFArray::~CColladaIDREFArray()
{}

const std::vector<SStrng> &CColladaIDREFArray::GetNameArray() const
{
  return body;
}

CColladaIDREFArray *CColladaIDREFArray::CreateInstance( xmlNode *node )
{
  const xmlChar *count;
  if( ( count = LoadStrAttr( node, _X"count" ) ) )
  {

    CColladaIDREFArray *arrayIDREF = new CColladaIDREFArray;
    const xmlChar *v = node->children->content;
    arrayIDREF->content.insert( arrayIDREF->content.begin(), v, v + strlen( (char *)v ) + 1 );
    bool dumped = true;
    for( size_t i = 0; i < arrayIDREF->content.size(); ++i )
    {
      xmlChar &v = arrayIDREF->content[i];
	//printf( "%c %i\n", v, v );
      if( v == (xmlChar)' ' )
      {
        v = 0;
        dumped = true;
      }
      else
      {
        if( dumped )
        {
          arrayIDREF->body.push_back( SStrng( &v ) );
          dumped = false;
        }
      }
    }

    size_t cnt = (size_t)atoi( (char *)count );

    //printf( "sizes %i %i \n", cnt, arrayIDREF->body.size() );

    assert(  cnt == arrayIDREF->body.size() );


    return arrayIDREF;
  }

  return 0;
};

