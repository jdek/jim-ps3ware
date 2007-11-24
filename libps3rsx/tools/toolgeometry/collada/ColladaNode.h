#pragma once
#include "XMLUtils.h"
#include "vec3.h"

class CColladaSource;

class CColladaNode : public CAutoContainer<CColladaNode>
{

  CColladaNode   *parent;
  mat3x4          transform;
  size_t          nodeIndex;
public:
  CColladaNode(void);
public:
  CColladaSource *matrixAnim;
  virtual ~CColladaNode(void);
  static const xmlChar *GetName() { return _X"node"; };
  static CColladaNode *CreateInstance( const xmlNode * node );

  const CColladaNode *GetParent() const
  {
    return parent;
  }
  size_t GetIndex() const
  {
    return nodeIndex;
  };

  void SetIndex( const size_t index )
  {
    nodeIndex = index;
  };

  const mat3x4 &GetMatrix() const
  {
    return transform;
  }
};
