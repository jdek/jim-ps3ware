#pragma once

#include "XMLUtils.h"

class CColladaEffect;

class CColladaMaterial : public CAutoContainer<CColladaMaterial>
{
	CColladaEffect *effect;
public:
	CColladaMaterial(void);
public:
	virtual ~CColladaMaterial(void);
	static const xmlChar *GetName() { return _X"material"; };
	static CColladaMaterial *CreateInstance( const xmlNode *node );
	CColladaEffect *GetEffect();
};


class CColladaBindMaterial : public CAutoContainer<CColladaBindMaterial>
{
	static std::map< SStrng, const xmlChar *> symbolBind;
public:
	CColladaBindMaterial(void);
public:
	virtual ~CColladaBindMaterial(void);
	static const xmlChar *GetName() { return _X"bind_material"; };
	static CColladaBindMaterial *CreateInstance( const xmlNode *node );
	static const CColladaEffect *GetEffectBySymbol( const xmlChar *symbol );
	static void ClearInner() 
	{
		symbolBind.clear();
	}
};
