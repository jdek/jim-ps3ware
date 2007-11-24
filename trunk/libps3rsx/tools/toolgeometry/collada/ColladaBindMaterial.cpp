#include "StdAfx.h"
#include "ColladaBindMaterial.h"
#include "ColladaImage.h"
#include "ColladaEffect.h"

CColladaMaterial::CColladaMaterial(void)
{
}

CColladaMaterial::~CColladaMaterial(void)
{
}

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

CColladaEffect *CColladaMaterial::GetEffect()
{
	return effect;
}

CColladaBindMaterial::CColladaBindMaterial(void)
{
}

CColladaBindMaterial::~CColladaBindMaterial(void)
{
}

CColladaBindMaterial *CColladaBindMaterial::CreateInstance( const xmlNode *node )
{

	const xmlNode *technique;
	const xmlNode *instance;
	const xmlChar *target;
	const xmlChar *symbol;

	technique = Find( node->children, _X"technique_common" );
	instance = technique->children;
	
	while
	(
	  ( instance = Find( instance, _X"instance_material" ) ) &&
	  ( target = LoadStrAttr( instance, _X"target" ) ) &&
	  ( symbol = LoadStrAttr( instance, _X"symbol" ) )	  
	)
	{
		symbolBind[SStrng(symbol)] = target;
		instance = instance->next;	
	}

	return 0;
}

std::map< SStrng, const xmlChar *> CColladaBindMaterial::symbolBind;


const CColladaEffect *CColladaBindMaterial::GetEffectBySymbol( const xmlChar *symbol )
{
	CColladaMaterial *material = CColladaMaterial::Get(symbolBind[SStrng(symbol)]);
	if( material )
	{
		return material->GetEffect();
	}
	return 0;
}