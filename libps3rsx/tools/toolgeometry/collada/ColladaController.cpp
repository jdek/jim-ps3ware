#include "StdAfx.h"
#include "ColladaController.h"
#include "ColladaSource.h"
#include "ColladaGeometry.h"
#include "ColladaTriangles.h"
#include "ColladaNode.h"


std::map< SStrng,  SAutoInd>  CColladaController::jointMap;
size_t                        CColladaController::jointNumber = 0;
CColladaController           *CColladaController::theGod;

CColladaController::CColladaController(void) : jointArray( 0 )
{}

CColladaController::~CColladaController(void)
{}

void CColladaController::PostInit()
{

  /*
  for( size_t i = 0; i < nodes.size(); ++i )
  {
  	if( usedMatrix[i] != size_t( -1 ) )
  	{
  		size_t parent = parents[i];
  		while( parent != size_t(-1) )
  		{
  			usedMatrix[parent] = 1;
  			parent = parents[parent];
  		};
  	};
  };

  std::vector<mat3x4>          invBindMatricesComp;
  std::vector<mat3x4>          transformMatricesComp;
  std::vector<CColladaNode *>  nodesComp;
  std::vector<size_t>          parentsComp;
  std::vector<CColladaSource*> matrixAnimationsComp;

  for( size_t i = 0; i < nodes.size(); ++i )
  {
  	if( usedMatrix[i] != size_t( -1 ) )
  	{
  		usedMatrix[i] = nodesComp.size();
  		invBindMatricesComp.push_back( invBindMatrices[i] );
  		nodesComp.push_back( nodes[i] );
  		parentsComp.push_back( parents[i] );
  		transformMatricesComp.push_back( transformMatrices[i] );
  		matrixAnimationsComp.push_back( matrixAnimations[i] );
  	}
  }

  invBindMatrices = invBindMatricesComp;
  transformMatrices = transformMatricesComp;
  nodes = nodesComp;
  parents = parentsComp;
  matrixAnimations = matrixAnimationsComp;

  for( size_t i = 0; i < parents.size(); ++i )
  {
  	if( parents[i] != size_t( -1 ) )
  	{
  		size_t index =  usedMatrix[parents[i]];
  		parents[i] = index;
  	}
  }

  for( size_t i = 0; i < weights.size(); ++i )
  {
  	for( size_t j = 0; j < 4; ++j )
  	{
  		size_t index =  usedMatrix[weights[i].indices[j]];
  		weights[i].indices[j] = index;
  	}
  }*/


  for( size_t i = 0; i < nodes.size(); ++i )
  {
    size_t index = remap[i];

    size_t oldSize = GetController()->invBindMatrices.size();

    if( index >= oldSize )
    {
      GetController()->invBindMatrices.resize( index + 1 );
      GetController()->invBindMatrices[index] = invBindMatrices[i];

      GetController()->nodes.resize( index + 1 );
      GetController()->nodes[index] = nodes[i];

      GetController()->parents.resize( index  + 1 );
      GetController()->parents[index] = parents[i] == (size_t)(-1) ? parents[i] : remap[parents[i]];

      GetController()->transformMatrices.resize( index + 1 );
      GetController()->transformMatrices[index] = transformMatrices[i];

      GetController()->matrixAnimations.resize( index + 1 );
      GetController()->matrixAnimations[index] = matrixAnimations[i];

    }
  }



  for( size_t i = 0; i < weights.size(); ++i )
  {
    for( size_t j = 0; j < 4; ++j )
    {
      size_t index =  remap[weights[i].indices[j]];
      weights[i].indices[j] = index;
    }
  }
}

void CColladaController::InitJointInput( const  xmlNode * input )
{
  const xmlChar *semantic;
  const xmlChar *source;

  if
  (
    ( semantic = LoadStrAttr( input, _X"semantic" ) ) &&
    ( source = LoadStrAttr( input, _X"source" ) )
  )
  {

    CColladaSource *sourceArray = CColladaSource::Get( source );

    if( sourceArray != 0 )
    {
      if( !xmlStrcmp( semantic, _X"JOINT" ) )
      {
        assert( jointArray == 0 || jointArray == sourceArray );
        jointArray = sourceArray;
        sourceArray->BuildContent();

        const std::vector<SStrng> &idrefVec = jointArray->GetIDREFArray();

        for( size_t i = 0; i < idrefVec.size(); ++i )
        {
          SAutoInd &ind = jointMap[idrefVec[i]];
          if( ind.value == SAutoInd::GetBadValue() )
          {
            ind.value = (uint16)jointNumber++;
          }
          remap.push_back( ind.value );
          CColladaNode *jointNode = CColladaNode::Get( idrefVec[i].ptr );
          CColladaSource *source = 0;
          mat3x4 mat;
          if( jointNode )
          {
            source = jointNode->matrixAnim;
            mat = jointNode->GetMatrix();
            jointNode->SetIndex( i );
          }
          else
          {
            assert(0);
          }

          matrixAnimations.push_back( source );
          transformMatrices.push_back( mat );
          nodes.push_back( jointNode );
        };

        for( size_t i = 0; i < nodes.size(); ++i )
        {
          size_t parent = (size_t) -1;

          const CColladaNode *parentNode = 0;
          if( nodes[i] && ( parentNode = nodes[i]->GetParent() ) )
          {
            parent = parentNode->GetIndex();

            if( parent == (size_t)-1 )
            {
              while( parentNode )
              {

                assert( parentNode->GetMatrix().GetNormEstimate() < 0.00001f );
                parentNode = parentNode->GetParent();
              }
            };
            //assert( parent != (size_t) -1 );
          }

          parents.push_back( parent );
        }
      }

      if( !xmlStrcmp( semantic, _X"INV_BIND_MATRIX" ) )
      {
        sourceArray->BuildContent();
        const std::vector<float> &matricesVector = sourceArray->GetFloatArray();
        assert( 16 == sourceArray->GetFloatStride() );
        size_t matricesSize =  matricesVector.size() / 16;
        invBindMatrices.resize( matricesSize );

        for( size_t i = 0; i < matricesSize; i ++ )
        {
          sourceArray->BuildContent();
          mat3x4 mat( &matricesVector[i * 16] );
          invBindMatrices[i] = mat;
        }
      }
    }
  }
}


void CColladaController::InitWeightInput( const  xmlNode * input, const xmlNode *vcount, const xmlNode *v, size_t count  )
{
  const xmlChar *semantic;
  const xmlChar *source;

  if
  (
    ( semantic = LoadStrAttr( input, _X"semantic" ) ) &&
    ( source = LoadStrAttr( input, _X"source" ) )
  )
  {

    CColladaSource *sourceArray = CColladaSource::Get( source );

    if( sourceArray != 0 )
    {
      if( !xmlStrcmp( semantic, _X"JOINT" ) )
      {
        assert( jointArray == 0 || jointArray == sourceArray );
        jointArray = sourceArray;
        sourceArray->BuildContent();
      }

      if( !xmlStrcmp( semantic, _X"WEIGHT" ) )
      {
        std::vector<size_t> vVec;
        std::vector<size_t> vcountVec;
        sourceArray->BuildContent();

        {
          const xmlChar *vData = vcount->children->content;
          for( size_t i = 0; i < count; ++i )
          {

            while( IsSpace(*vData) )vData++;
            int val = atoi( (const char *)vData );
            while( !IsSpace(*vData) )vData++;
            vcountVec.push_back( (size_t) val );
            //printf( "%d ", val );
          }
        }

        {
          const xmlChar *vData = v->children->content;

          for( size_t i = 0; i < count; ++i )
          {
            SWeightInfo weightInfo;

            for( size_t j = 0; j < vcountVec[i] * 2; ++j )
            {
              while( IsSpace(*vData) )vData++;
              int val = atoi( (const char *)vData );
              while( !IsSpace(*vData) )vData++;
              vVec.push_back( (size_t) val );
              size_t k = j / 2;
              if( j & 1 )
              {

                if( k < 4 )
                {
                  weightInfo.weights[ j / 2 ] = sourceArray->GetFloatArray()[val];
                }

              }
              else
              {
                if( k < 4 )
                {
                  weightInfo.indices[ j / 2 ] = val;
                }

              }
            }

            weights.push_back( weightInfo );
          }
        }
      }
    }
  }
}

mat3x4 CColladaController::GetMatrix( size_t i, size_t j ) const
{
  if( matrixAnimations[i] )
  {
    size_t animSize =  matrixAnimations[i]->GetFloatSize();
    if( j >= animSize )
    {
      j = animSize - 1;
    };
    mat3x4 m( &matrixAnimations[i]->GetFloatArray()[ j * 16 ] );
    return m;
  };

  return transformMatrices[i];
};

CColladaController *CColladaController::CreateInstance( const xmlNode *node )
{
  const xmlNode *input = node->children;
  const xmlNode *skin;
  //const xmlChar *count;
  //const xmlChar *material;
  CColladaController *triangles = new CColladaController();

  if( ( skin = Find( node->children, _X"skin" ) ) )
  {
    const xmlNode *joints;
    const xmlNode *vertex_weights;
    const xmlChar *source;


    const xmlNode *matrix = Find( skin->children, _X"bind_shape_matrix" );

    if( matrix )
    {
      const xmlChar *v = matrix->children->content;
      std::vector<float> body;
      body.resize( 16 );
      for( size_t i = 0; i < body.size(); ++i )
      {
        while( IsSpace(*v) )v++;
        float val = (float)atof( (const char *)v );
        while( !IsSpace(*v) )v++;
        body[i] = val;
      }
      mat3x4 m( &body[0] );
      triangles->bindMatrix = m;
    }

    CColladaGeometry *geom = 0;
    if( ( source = LoadStrAttr( skin, _X"source" ) ) )
    {
      if ( ( geom = CColladaGeometry::Get( source ) ) )
      {
        geom->SetBindController( triangles );
      }

      //assert( geom );
    }

    const xmlNode *vcount;
    const xmlNode *v;
    const xmlChar *count;

    if
    (
      ( vertex_weights = Find( skin->children, _X"vertex_weights" ) ) &&
      ( count = LoadStrAttr( vertex_weights, _X"count" ) ) &&
      ( v = Find( vertex_weights->children, _X"v" ) ) &&
      ( vcount = Find( vertex_weights->children, _X"vcount" ) )
    )
    {
      input = vertex_weights->children;
      while( input )
      {
        input = Find( input, _X"input" );
        if( input )
        {
          triangles->InitWeightInput( input, vcount, v, atoi( (const char *)count ) );
          input = input->next;
        }
      }
    }

    if( ( joints = Find( skin->children, _X"joints" ) ) )
    {
      input = joints->children;
      while( input )
      {
        input = Find( input, _X"input" );
        if( input )
        {
          triangles->InitJointInput( input );
          input = input->next;
        }
      }
    }

    triangles->PostInit();



  };


  return triangles;
}
