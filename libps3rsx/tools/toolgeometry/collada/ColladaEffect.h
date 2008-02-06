#pragma once
#include "../../common/XMLUtils.h"

class CColladaEffect : public CAutoContainer<CColladaEffect>
{
  CColladaImage *diffuseImage;
public:
  CColladaEffect(void);
public:
  virtual ~CColladaEffect(void);
  static const xmlChar *GetName() { return _X"effect"; };
  static CColladaEffect *CreateInstance( const xmlNode *node );
  CColladaImage *GetDiffuse() const { return diffuseImage; };

};


