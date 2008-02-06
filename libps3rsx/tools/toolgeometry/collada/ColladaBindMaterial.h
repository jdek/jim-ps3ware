#pragma once

#include "../../common/XMLUtils.h"

class CColladaEffect;




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

