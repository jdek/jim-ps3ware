#pragma once

#include "XMLUtils.h"

class CColladaImage : public CAutoContainer<CColladaImage>
{
  SStrng fileName;
public:
  CColladaImage(void);
public:
  virtual ~CColladaImage(void);
  static const xmlChar *GetName() { return _X"image"; };
  static CColladaImage *CreateInstance( const xmlNode *node );
  SStrng &GetFileName(){ return fileName; };
};
