#pragma once

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
