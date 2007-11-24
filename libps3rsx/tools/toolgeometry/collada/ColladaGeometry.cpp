#include "StdAfx.h"
#include "ColladaGeometry.h"
#include "ColladaTriangles.h"

CColladaGeometry::CColladaGeometry(void)
{}

CColladaGeometry::~CColladaGeometry(void)
{
}

void CColladaGeometry::SetBindController( CColladaController * controller )
{
  for( size_t i = 0; i < meshes.size(); ++i )
  {
    if( meshes[i] )
    {
      meshes[i]->SetBindController( controller );
    }
  }
}

void CColladaGeometry::AddMesh( CColladaTriangles *triangle )
{
  meshes.push_back( triangle );
}

void CColladaGeometry::GetFatVertices( std::vector<SFatVertex> *vertices, std::vector<uint16> *indices ) const
{
  for( size_t i = 0; i < meshes.size(); ++i )
  {
    if( meshes[i] )
    {
      meshes[i]->GetFatVertices( vertices, indices );
    }
  }
}

CColladaGeometry *CColladaGeometry::CreateInstance( xmlNode *node )
{
  return new CColladaGeometry();
}
