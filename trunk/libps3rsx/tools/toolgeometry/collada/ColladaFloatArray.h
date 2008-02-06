#pragma once

#include "../../common/XMLUtils.h"

class CColladaFloatArray : public CAutoContainer<CColladaFloatArray>
{
  std::vector<float> body;
  xmlChar *content;
public:
  CColladaFloatArray(void);
public:
  const std::vector<float> &GetFloatArray() const { return body; };
  virtual ~CColladaFloatArray(void);
  void BuildContent();
  static const xmlChar *GetName() { return _X"float_array"; };
  static CColladaFloatArray *CreateInstance( xmlNode *node );
};
