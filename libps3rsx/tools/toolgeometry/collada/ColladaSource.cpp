#include "StdAfx.h"
#include "ColladaSource.h"
#include "ColladaFloatArray.h"
#include "ColladaIDREFArray.h"

CColladaSource::CColladaSource()
{
}

CColladaSource::~CColladaSource()
{
}

void CColladaSource::BuildContent()
{
	if( fArray )
	{
		fArray->BuildContent();
	}

	if( sArray )
	{
		sArray->BuildContent();
	}
}

const std::vector<float> &CColladaSource::GetFloatArray() const 
{ 
	static std::vector<float> aux; 
	return fArray ? fArray->GetFloatArray() : aux; 
};

const std::vector<SStrng> &CColladaSource::GetIDREFArray() const
{
	static std::vector<SStrng> aux; 
	return sArray ? sArray->GetNameArray() : aux; 
}



CColladaSource *CColladaSource::CreateInstance( xmlNode *node )
{
	const xmlNode *common;
	const xmlNode *accessor;
	const xmlChar *source;
	const xmlChar *stride;
	if
	( 
		( common = Find( node->children, _X"technique_common" ) )  && 
		( accessor = Find( common->children, _X"accessor" ) )  && 
		( source = LoadStrAttr( accessor, _X"source" ) ) 

	)
	{
		CColladaSource *colladaSource = new CColladaSource;
		CColladaFloatArray *fArray = CColladaFloatArray::Get( source );
		colladaSource->fArray = fArray;

		CColladaIDREFArray *sArray = CColladaIDREFArray::Get( source );
		colladaSource->sArray = sArray;


		stride = LoadStrAttr( accessor, _X"stride" );
		size_t nStride = 0;
		if( stride )
		{
			nStride = atoi( (const char *)stride );
		}
		colladaSource->floatStride = nStride;
		return colladaSource;
	}
	return 0;
}
