#pragma once
#include "XMLUtils.h"


class CColladaTriangles;
class CColladaController;

class CColladaGeometry :  public CAutoContainer<CColladaGeometry>
{
  std::vector<CColladaTriangles *> meshes;
public:
  CColladaGeometry(void);
public:
  void AddMesh( CColladaTriangles * );
  virtual ~CColladaGeometry(void);
  static const xmlChar *GetName() { return _X"geometry"; };
  static CColladaGeometry *CreateInstance( xmlNode *node );
  void GetFatVertices( std::vector<SFatVertex> *vertices, std::vector<uint16> *indices ) const;
  void SetBindController( CColladaController * controller );
};


