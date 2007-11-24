#include "StdAfx.h"

#include "XMLUtils.h"
#include "ColladaTriangles.h"
#include "ColladaImage.h"
#include "ColladaEffect.h"
#include "ColladaFloatArray.h"
#include "ColladaIDREFArray.h"
#include "ColladaSource.h"
#include "ColladaVertices.h"
#include "ColladaGeometry.h"
#include "ColladaController.h"
#include "ColladaMaterial.h"
#include "ColladaBindMaterial.h"
#include "ColladaNode.h"
#include "ColladaAnimation.h"




void ConvertModel( const char *fileNameOut, const char *fileNameIn )
{
  xmlDoc *doc = NULL;
  xmlNode *root = NULL;
  doc = xmlReadFile( fileNameIn, NULL, 0 );

  if (doc == NULL)
  {
    printf("error: could not parse file ");
  }

  root = xmlDocGetRootElement(doc);
  CXMLLoader<CColladaImage> imageLoader;
  CXMLLoader<CColladaEffect> effectLoader;
  CXMLLoader<CColladaMaterial> materialLoader;
  CXMLLoader<CColladaBindMaterial> bindMaterialLoader;
  CXMLLoader<CColladaFloatArray> floatLoader;
  CXMLLoader<CColladaIDREFArray> idrefLoader;
  CXMLLoader<CColladaSource> sourceLoader;
  CXMLLoader<CColladaTriangles> triangleLoader;
  CXMLLoader<CColladaVertices> vertexLoader;
  CXMLLoader<CColladaGeometry> geometryLoader;
  CXMLLoader<CColladaController> controllerLoader;
  CXMLLoader<CColladaNode> nodeLoader;
  CXMLLoader<CColladaAnimation> animLoader;

  imageLoader.Load( root, false );
  effectLoader.Load( root, false );
  materialLoader.Load( root, false );
  bindMaterialLoader.Load( root, false );
  floatLoader.Load( root, false );
  idrefLoader.Load( root, false );
  sourceLoader.Load( root, false );
  vertexLoader.Load( root, false );
  geometryLoader.Load( root, false);
  triangleLoader.Load( root, false );
  nodeLoader.Load( root, true );
  animLoader.Load( root, false );
  controllerLoader.Load( root, false );


  CColladaBindMaterial::ClearInner();
  CColladaController::ClearInner();
  CColladaImage::Clear();
  CColladaEffect::Clear();
  CColladaMaterial::Clear();
  CColladaBindMaterial::Clear();
  CColladaFloatArray::Clear();
  CColladaIDREFArray::Clear();
  CColladaSource::Clear();
  CColladaVertices::Clear();
  CColladaGeometry::Clear();
  CColladaController::Clear();
  CColladaNode::Clear();
  CColladaAnimation::Clear();


  xmlFreeDoc(doc);
  xmlCleanupParser();

}

int main( int argc, const char *argv[2] )
{
  
  if( argc < 3 )
  {
    printf( "params: infile outfile\n" );

    return -1;

  }

  ConvertModel( argv[1], argv[2] );

 


  return 0;

}

