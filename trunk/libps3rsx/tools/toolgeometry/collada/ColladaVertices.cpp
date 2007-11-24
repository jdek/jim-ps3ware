#include "StdAfx.h"
#include "ColladaVertices.h"
#include "ColladaSource.h"

CColladaVertices::CColladaVertices(void)
{
	positions = 0;
	normals = 0;
}

CColladaVertices::~CColladaVertices(void)
{
}

void CColladaVertices::InitInput( const  xmlNode * input )
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
	
		if( !xmlStrcmp( semantic, _X"POSITION" ) )
		{
			positions = sourceArray;
		}

		if( !xmlStrcmp( semantic, _X"NORMAL" ) )
		{
			normals = sourceArray;
		}

	}
}

CColladaVertices *CColladaVertices::CreateInstance( const xmlNode * node )
{
	const xmlNode *input = node->children;
	
	CColladaVertices *vertices = new CColladaVertices();
	while( input )
	{
		input = Find( input, _X"input" );
		if( input )
		{
			vertices->InitInput( input );
			input = input->next;
		}
	}

	

	return vertices;
}