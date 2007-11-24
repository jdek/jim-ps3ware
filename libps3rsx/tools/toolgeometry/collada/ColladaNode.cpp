#include "StdAfx.h"
#include "ColladaImage.h"
#include "ColladaNode.h"

CColladaNode::CColladaNode(void) : nodeIndex( (size_t)-1 ), matrixAnim( 0 )
{}

CColladaNode::~CColladaNode(void)
{}

CColladaNode *CColladaNode::CreateInstance( const xmlNode *node )
{
  CColladaNode *colladaNode = new CColladaNode;
  const xmlChar *parentId = LoadStrAttr( node->parent, _X"id" );
  colladaNode->parent =  parentId ? CColladaNode::Get( parentId ) : 0;

  const xmlNode *matrix = Find( node->children, _X"matrix" );

  if( matrix )
  {
    const xmlChar *v = matrix->children->content;
    if( v == 0 )
    {
      return colladaNode;
    }
    std::vector<float> body;
    body.resize( 16 );
    for( size_t i = 0; i < body.size(); ++i )
    {
      while( IsSpace(*v) )v++;
      float val = (float)atof( (const char *)v );
      while( !IsSpace(*v) )v++;
      body[i] = val;
    }
    mat3x4 m( &body[0] );
    colladaNode->transform = m;
  }


  return colladaNode;
}
