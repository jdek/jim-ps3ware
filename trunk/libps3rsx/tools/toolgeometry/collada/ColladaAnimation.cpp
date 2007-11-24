#include "StdAfx.h"
#include "ColladaImage.h"
#include "ColladaAnimation.h"
#include "ColladaSource.h"
#include "ColladaNode.h"

CColladaAnimation::CColladaAnimation(void) : matrixAnim( 0 )
{}

CColladaAnimation::~CColladaAnimation(void)
{}

void CColladaAnimation::InitInput( const  xmlNode * input )
{
  const xmlChar *semantic;
  const xmlChar *source;


  if
  (
    ( semantic = LoadStrAttr( input, _X"semantic" ) ) &&
    ( source = LoadStrAttr( input, _X"source" ) )
  )
  {
    CColladaSource *sourceArray = CColladaSource::Get( source );

    if( sourceArray != 0 )
    {
      if( !xmlStrcmp( semantic, _X"OUTPUT" ) )
      {
        sourceArray->BuildContent();
        matrixAnim = sourceArray;
      }
    }
  }
}



CColladaAnimation *CColladaAnimation::CreateInstance( const xmlNode *node )
{
  CColladaAnimation *animation = new CColladaAnimation;
  const xmlNode *sampler;
  const xmlNode *target;
  const xmlChar *nodeName;


  if( ( sampler = Find( node->children, _X"sampler" ) ) )
  {
    const xmlNode *input = sampler->children;
    while( input )
    {
      input = Find( input, _X"input" );
      if( input )
      {
        animation->InitInput( input );
        input = input->next;
      }
    }
  }

  if
  (
    ( target = Find( node->children, _X"channel" ) ) &&
    ( nodeName = LoadStrAttr( target, _X"target" ) )
  )
  {
    xmlChar name[1024];
    memset( name, 0, 1024 );
    for( size_t i = 0; i < 1024; ++i )
    {
      if( nodeName[i] == '/' || nodeName[i] == 0 )
        break;
      name[i] = nodeName[i];
    };

    CColladaNode *targetNode = CColladaNode::Get( name );
    if( targetNode )
    {
      targetNode->matrixAnim = animation->matrixAnim;
    }
  }

  return animation;
}
