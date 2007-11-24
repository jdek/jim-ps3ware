#include "StdAfx.h"
#include "ColladaImage.h"
#include "ColladaEffect.h"

CColladaEffect::CColladaEffect(void)
{
}

CColladaEffect::~CColladaEffect(void)
{
}

CColladaEffect *CColladaEffect::CreateInstance( const xmlNode *node )
{
	const xmlNode *common;
	const xmlNode *param = 0;
	const xmlNode *surface;
	const xmlNode *from;
	const xmlNode *sampler;
	const xmlNode *source;
	const xmlNode *technique;
	const xmlNode *diffuse;
	const xmlNode *lambert;
	const xmlNode *texture;
	const xmlChar *sid;
	const xmlChar *image;
	std::map< SStrng, xmlChar *> samplers;
	std::map< SStrng, SStrng > sources;
	if
	( 
		( common = Find( node->children, _X("profile_COMMON") ) ) 
	)
	while( 1 )
	{
		if( param == 0 )
		{
			param = Find( common->children, _X("newparam" ) );
		}
		else
		{
			param = Find( param->next, _X("newparam" ) );
		}

		if( param == 0 )
		{
			break;
		}

		if( !( sid = LoadStrAttr( param, _X("sid" ) ) ) )
		{
			break;
		}

		if
		( 
			( surface = Find( param->children, _X("surface") ) ) &&
			( from = Find( surface->children, _X("init_from") ) )
		)
		{
			samplers[SStrng(sid)] = from->children->content;
		}
		if
		( 
			( sampler = Find( param->children, _X("sampler2D") ) ) &&
			( source = Find( sampler->children, _X("source") ) )
		)
		{
			sources[SStrng(sid)] = SStrng( source->children->content );
		}

	}
	if
	( 
	   common &&
	   ( technique = Find( common->children, _X("technique") ) ) &&
	   ( lambert = Find( technique->children, _X("lambert") ) ) &&
	   ( diffuse = Find( lambert->children, _X("diffuse") ) ) &&
	   ( texture = Find( diffuse->children, _X("texture") ) ) && 
	   ( image = LoadStrAttr( texture, _X("texture") ) ) 
	)
	{
		CColladaEffect *effect = new CColladaEffect();
		SStrng source = sources[SStrng(image)];
		effect->diffuseImage = CColladaImage::Get( samplers[source] );
		printf("find bind for %s\n", effect->diffuseImage->GetFileName() );
		return effect;
	}
	return 0;
}
