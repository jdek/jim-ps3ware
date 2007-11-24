#include "StdAfx.h"
#include "ColladaFloatArray.h"

CColladaFloatArray::CColladaFloatArray()
{
}

CColladaFloatArray::~CColladaFloatArray()
{
}



CColladaFloatArray *CColladaFloatArray::CreateInstance( xmlNode *node )
{
	const xmlChar *count;
	if( ( count = LoadStrAttr( node, _X"count" ) ) )
	{
		CColladaFloatArray *fArray = new CColladaFloatArray;
		size_t nCount = atoi( (const char *)count );
		fArray->content = node->children->content;
		fArray->body.resize( nCount );
		return fArray;
	}

	return 0;
};

void CColladaFloatArray::BuildContent()
{
	const xmlChar *v = content;
	if( content == 0 )
	{
		return;
	}
	for( size_t i = 0; i < body.size(); ++i )
	{
			while( IsSpace(*v) )v++; 
			float val = (float)atof( (const char *)v );
			while( !IsSpace(*v) )v++; 
			body[i] = val;
	}
	content = 0;
};