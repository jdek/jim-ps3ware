#include "StdAfx.h"
#include "XMLUtils.h"
#include "ColladaMaterial.h"
#include "ColladaImage.h"
#include "ColladaEffect.h"


CColladaMaterial::CColladaMaterial(void)
{}

CColladaMaterial::~CColladaMaterial(void)
{}

CColladaMaterial *CColladaMaterial::CreateInstance( const xmlNode *node )
{
	const xmlNode *instance;
	const xmlChar *url;
	if(
	    ( instance = Find( node->children, _X"instance_effect" ) ) &&
	    ( url = LoadStrAttr( instance, _X"url" ) )
	)
	{
		CColladaMaterial *material = new CColladaMaterial();
		if( url && *url == '#' )++url;
		material->effect = CColladaEffect::Get( url );
		return material;
	}

	return 0;
}


