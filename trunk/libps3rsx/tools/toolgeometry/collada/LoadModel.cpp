#include "StdAfx.h"

#include "..//COLLADA//ColladaTriangles.h"
#include "..//COLLADA//ColladaImage.h"
#include "..//COLLADA//ColladaEffect.h"
#include "..//COLLADA//ColladaFloatArray.h"
#include "..//COLLADA//ColladaIDREFArray.h"
#include "..//COLLADA//ColladaSource.h"
#include "..//COLLADA//ColladaVertices.h"
#include "..//COLLADA//ColladaGeometry.h"
#include "..//COLLADA//ColladaController.h"
#include "..//COLLADA//ColladaBindMaterial.h"
#include "..//COLLADA//ColladaNode.h"
#include "..//COLLADA//ColladaAnimation.h"
#include "..//DXUT//TextureManager.h"
#include "..//LOD//ProgressiveMesh.h"
#include "..//Animation//Animation.h"
#include "..//UTILS\\stream.h"
#include "..//UTILS\\timer.h"
#include "..//Terrain//QNode.h"
#include "..//Visibility//Visibility.h"

#include "LoadModel.h"

float GetHeightMax( float x, float y, float domainSize, float d )
{
	float m = 100000.0f;
	for( size_t i = 0; i < 5; ++i )
	{
		for( size_t j = 0; j < 5; ++j )
		{
			float v = GetHeight( x + 0.5f * d * ( (float)i - 2 ), y + 0.5f * d * ( (float)j - 2 ), domainSize );
			if( m > v )
			{
				m = v;
			}
		}
	}

	return m;
};


class CRenderObject
{
public:
	LPDIRECT3DVERTEXBUFFER9	            vertexBuffer;
	LPDIRECT3DVERTEXBUFFER9	            vertexBufferLOD;
	std::vector<LPDIRECT3DINDEXBUFFER9> indexBuffers;
	std::vector<LPDIRECT3DINDEXBUFFER9> indexBuffersLOD;
	std::vector<size_t>                 indexCounts;
	std::vector<size_t>                 indexCountsLOD;
	std::vector<size_t>                 subMeshCounts; 
	
	std::vector<size_t>                 indexOffsets;
	std::vector<size_t>                 indexOffsetsLOD;
	std::vector<LPDIRECT3DTEXTURE9>     textures;
	std::vector<CAABox>                 boxes;  
	size_t                              vertexCount;
	size_t                              vertexSize;
	std::vector<size_t>                 controllerBind;
	std::vector<SAnimController*>       controllers;
	vec3                                center;
	vec3                                extends; 
	vec3                                compensationSpeed;

	void ReInit()
	{
		size_t offset = 0, offsetLOD = 0;
		for( size_t i = 0; i < indexCounts.size(); ++i )
		{
			indexOffsets.push_back( offset );
			indexOffsetsLOD.push_back( offsetLOD );
			offset += indexCounts[i];
			offsetLOD += indexCountsLOD[i];
		}
		/*
		std::vector<LPDIRECT3DTEXTURE9> newTex;
		std::vector<CAABox>             newBoxes; 
	    std::vector<size_t>             newIndexCounts;
	    std::vector<size_t>             newIndexCountsLOD;
		std::vector<size_t>             newIndexOffsets;
	    std::vector<size_t>             newIndexOffsetsLOD;
		std::vector<bool>               used;
		used.resize( boxes.size(), true );

		size_t offset = 0, offsetLOD = 0;
		for( size_t i = 0; i < indexCounts.size(); ++i )
		{
			indexOffsets.push_back( offset );
			indexOffsetsLOD.push_back( offsetLOD );
			offset += indexCounts[i];
			offsetLOD += indexCountsLOD[i];
		}

		for( size_t i = 0; i < textures.size(); ++i )
		{
			if( used[i] )
			for( size_t j = i; j < textures.size(); ++j )
			{
				if( textures[i] == textures[j] && used[j] )
				{
					newBoxes.push_back( boxes[j] );
					newIndexCounts.push_back( indexCounts[j] );
					newIndexCountsLOD.push_back( indexCountsLOD[j] );
					newTex.push_back( textures[i] );
					newIndexOffsets.push_back( indexOffsets[j] );
					newIndexOffsetsLOD.push_back( indexOffsetsLOD[j] );
                    used[j] = false;
				};
			}
		}

		textures = newTex;
		boxes = newBoxes;
		indexCounts = newIndexCounts;
		indexCountsLOD = newIndexCountsLOD;
		indexOffsets = newIndexOffsets;
		indexOffsetsLOD = newIndexOffsetsLOD;
		*/


	}

	void Render( LPDIRECT3DDEVICE9 device,  bool lod, const D3DXMATRIX &matMV, float phase, bool first )
	{
		static LPDIRECT3DTEXTURE9 settedTexture;
		
		if( first )
		{
			settedTexture = 0;
			if( !lod )
			{
				device->SetStreamSource( 0, vertexBuffer, 0, (UINT)vertexSize );
				device->SetStreamSource( 1, vertexBufferLOD, 0, (UINT)vertexSize  );
				device->SetIndices( indexBuffers[0] );
			}
			else
			{
				device->SetStreamSource( 0, vertexBufferLOD, 0, (UINT)vertexSize  );
				device->SetIndices( indexBuffersLOD[0] );
			}
		}
	
		
		size_t k = 0, newK;

		if( !controllers.empty() )
		{
			mat3x4 root;
			root.FromPtr( (float *)&matMV );
			static mat3x4 matArray[80];
			controllers[0]->GetMatrices( phase + (float)GetTickCountInner() * 0.03f, matArray, root, compensationSpeed );
			device->SetVertexShaderConstantF( 0, (float *)matArray, (UINT)3 * controllers[0]->indices.size() );
		}

		for( size_t i = 0; i < subMeshCounts.size(); ++i )
		{
			bool bRender = false;
			size_t length = 0;
			newK = k;
			for( size_t j = 0; j < subMeshCounts[i]; ++j )
			{
				bRender = true;
				length += ( (!lod ) ? indexCounts[newK] :  indexCountsLOD[newK] );
				newK++;
			}

			if( bRender )
			{
				if( textures[i] )
				{
					if( settedTexture != textures[i] )
					{
						device->SetTexture( 0, textures[i] );
						settedTexture = textures[i];
					}

					extern float trnNum;
					if( !lod )
					{
						trnNum +=  length / 3;
						device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, UINT( vertexCount ), indexOffsets[k], UINT ( length / 3 ) );
					}
					else
					{
						trnNum +=  length / 3;
						device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, UINT( vertexCount ), indexOffsetsLOD[k], UINT ( length / 3 ) );
					}
				}
			}	

		  k = newK;

		}
	}
};

void SetObject( LPDIRECT3DDEVICE9 device, bool forceSync, float morphFactor, float fogFactor )
{
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0    , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3*4  , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6*4  , D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 1, 0    , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
		{ 1, 3*4  , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1 },
		{ 1, 6*4  , D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};


	D3DVERTEXELEMENT9 declSimple[] =
	{
		{ 0, 0    , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3*4  , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6*4  , D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	static LPDIRECT3DVERTEXDECLARATION9    vd = 0;
	static LPDIRECT3DVERTEXDECLARATION9    vdSimple = 0;
	static SAPIShader objectPixel;
	static SAPIShader objectMorph;
	static SAPIShader objectSimple;
	static int morphConstant;
	static int fogConstant;

	if( vd == 0 )
	{
		CShaderCompiler("ps_1_1").Compile( device, "..//psObject.hlsl",  objectPixel );
		device->CreateVertexDeclaration( decl, &vd);
		device->CreateVertexDeclaration( declSimple, &vdSimple);


		CShaderCompiler("vs_1_1").
			Set( "morphFactor", &morphConstant ).
			Set( "fogFactor", &fogConstant ).
			Define( "MORPH", "true" ).
			Compile( device, "..//vsObject.hlsl",  objectMorph );

		CShaderCompiler("vs_1_1").
			Compile( device, "..//vsObject.hlsl",  objectSimple );

		device->CreateVertexDeclaration( decl, &vd);

	}

	if( forceSync )
	{
		device->SetPixelShader( objectPixel.pixelShader );
		
	}

	static bool morphOld;
	
	bool morphNew = morphFactor > 0.0f && morphFactor < 1.0f;

	float fogV[] = { fogFactor, fogFactor, fogFactor, fogFactor };

	if( morphNew )
	{
		float morphV[] = { morphFactor, morphFactor, morphFactor, morphFactor };
		device->SetVertexShaderConstantF( morphConstant, morphV, 1 );
	}
	device->SetVertexShaderConstantF( fogConstant, fogV, 1 );
	if( morphOld != morphNew || forceSync )
	{
		morphOld = morphNew;
		
		if( morphNew )
		{
			device->SetVertexShader( objectMorph.vertexShader );
			device->SetVertexDeclaration( vd );
		}
		else
		{
			device->SetVertexShader( objectSimple.vertexShader );
			device->SetVertexDeclaration( vdSimple );
		}
	}
}


void SetAnimObject( LPDIRECT3DDEVICE9 device, bool forceSync, float morphFactor, float fogFactor,  const SCameraInfo &matMVP )
{
	D3DVERTEXELEMENT9 decl[] =
	{

		{ 0, 0    , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3*4  , D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6*4  , D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32   , D3DDECLTYPE_UBYTE4N,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ 0, 36   , D3DDECLTYPE_UBYTE4,   D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0 },
		
		D3DDECL_END()
	};

	static LPDIRECT3DVERTEXDECLARATION9    vd = 0;
	static SAPIShader objectPixel;
	static SAPIShader objectAnim;
	static SAPIShader objectAnim2Bones;

	static int scleConstant;
	static int cnstConstant;
	static int fogConstant;

	if( vd == 0 )
	{
		CShaderCompiler("ps_1_1").Compile( device, "..//psObject.hlsl",  objectPixel );
		device->CreateVertexDeclaration( decl, &vd);

		CShaderCompiler("vs_2_0").
			Set( "scle", &scleConstant ).
			Set( "cnst", &cnstConstant ).
			Set( "fogConstant", &fogConstant ).
			Define( "FOUR", "true" ).
			Compile( device, "..//vsAnimation.hlsl",  objectAnim );

		CShaderCompiler("vs_2_0").
			Set( "fogConstant", &fogConstant ).
			Compile( device, "..//vsAnimation.hlsl",  objectAnim2Bones );



		device->CreateVertexDeclaration( decl, &vd );

	}

	if( forceSync )
	{
		device->SetPixelShader( objectPixel.pixelShader );
		device->SetVertexShader( objectAnim.vertexShader );
		device->SetVertexDeclaration(vd);
	}

	if( morphFactor >= 1.0f )
	{
		device->SetVertexShader( objectAnim2Bones.vertexShader );
	}


	const D3DXMATRIX &pr = matMVP.matP;
	float fogV[] = { fogFactor, fogFactor, fogFactor, fogFactor };
	float scle[] = { pr._11, pr._22, pr._33, pr._34 };
	float cnst[] = { 0, 0, pr._43, 0 };

	device->SetVertexShaderConstantF( fogConstant, fogV, 1 );
	device->SetVertexShaderConstantF( scleConstant, scle, 1 );
	device->SetVertexShaderConstantF( cnstConstant, cnst, 1 );
	

}

void CRenderObjectPlacer::UpdatePositions( float radius, float speed, float domainSize  )
{
	speed /= ( radius  * 2 );
	for ( size_t i = 0; i < infos.size(); ++i )
	{
		SObjectInfo &info = infos[i];
		
		float angle = ( (float)GetTickCountInner() + info.phase ) * speed * 0.03f;
		float x0 = sin( angle ) * radius + info.startPos.x;
		float y0 = cos( angle ) * radius + info.startPos.y;
		
		//info.phase = 1000.0f * rand() / (float)RAND_MAX;
		info.pos.x = x0;
		info.pos.y = y0;
		info.pos.z = GetHeightMax( x0, y0, domainSize, 0.5f );
	
		D3DXMATRIX matRot, matTrans, rotZ;
		D3DXMatrixRotationX( &matRot, -atanf( 1.0f ) * 4.0f );
		D3DXMatrixRotationZ( &rotZ,   -angle - atanf( 1.0f ) * 2.0f );

		D3DXMatrixTranslation( &matTrans, info.pos.x, info.pos.y, info.pos.z );		
		info.matTrans = matRot * rotZ * matTrans;

	}
}

void CRenderObjectPlacer::InitCircle( CRenderObject *parentObject, int number, float radius, float domainSize )
{
	parent = parentObject;

	for ( int j = 0; j < number; ++j )
	{
		float angle = j / (float) number * atan( 1.0f ) * 8.0f;
		float x0 = sin( angle ) * radius;
		float y0 = cos( angle ) * radius;
		SObjectInfo info;
		info.phase = 1000.0f * rand() / (float)RAND_MAX;
		info.pos.x = x0;
		info.pos.y = y0;
		info.pos.z = GetHeightMax( x0, y0, domainSize, 0.5f );
		info.startPos = info.pos;
		//angle = 0.0f;
		D3DXMATRIX matRot, matTrans, rotZ;
		D3DXMatrixRotationX( &matRot, -atan( 1.0f ) * 4.0f );
		D3DXMatrixRotationZ( &rotZ,   -angle );

		D3DXMatrixTranslation( &matTrans, info.pos.x, info.pos.y, info.pos.z );		
		info.matTrans = matRot * rotZ * matTrans;
		infos.push_back( info );
	}
}

void  CRenderObjectPlacer::InitSingle( CRenderObject *parentObject, float x0, float y0, float domainSize, float angle )
{
	parent = parentObject;
	SObjectInfo info;
	info.phase = 1000.0f * rand() / (float)RAND_MAX;
	info.pos.x = x0;
	info.pos.y = y0;
	info.pos.z = GetHeightMax( x0, y0, domainSize, 0.5f );
	info.startPos = info.pos;
	D3DXMATRIX matRot, matTrans, rotZ;
	D3DXMatrixRotationX( &matRot, -atan( 1.0f ) * 4.0f );
	D3DXMatrixTranslation( &matTrans, info.pos.x, info.pos.y, info.pos.z );		
	D3DXMatrixRotationZ( &rotZ,   -angle );
	info.matTrans = matRot * rotZ * matTrans;
	infos.push_back( info );
	
}

void CRenderObjectPlacer::Init( CRenderObject *parentObject, int number, float offset, float domainSize )
{
	parent = parentObject;

	for( int j = -number; j <= number; ++j )
	{
		for( int i = -number; i <= number; ++i )
		{
			float x0 = ( rand() & 15 ) - (float)number + i * offset;
			float y0 = ( rand() & 15 ) - (float)number + j * offset;
			SObjectInfo info;
			info.phase = 1000.0f * rand() / (float)RAND_MAX;
			info.pos.x = x0;
			info.pos.y = y0;
			info.pos.z = GetHeightMax( x0, y0, domainSize, 1.0f );
			info.startPos = info.pos;
			D3DXMATRIX matRot, matTrans;
			D3DXMatrixRotationX( &matRot, -atan( 1.0f ) * 4.0f );
			D3DXMatrixTranslation( &matTrans, info.pos.x, info.pos.y, info.pos.z );		
			info.matTrans = matRot * matTrans;
			infos.push_back( info );
		}
	}
}

void CRenderObjectPlacer::PushRequests()
{
	for( size_t i = 0; i < infos.size(); ++i )
	{
		infos[i].isVisible = true;
		PushVisibilityRequest( infos[i].pos, parent->extends, false, &infos[i].isVisible );
	}
}

void CRenderObjectPlacer::SetCompensationSpeed( const vec3 &value )
{
	parent->compensationSpeed = value;
}
void  CRenderObjectPlacer::Render(  LPDIRECT3DDEVICE9 device, const CPortalFrustrumTest &frustrum, float x, float y, float lodDist, float lodWidth, const SCameraInfo &matMVP )
{
	struct SRenderReqInner
	{
		float distance;
		int num;
		bool operator < ( const SRenderReqInner &other ) const
		{
			return distance < other.distance;
		};
	};

	std::vector<SRenderReqInner> inReqs;

	for( size_t i = 0; i < infos.size(); ++i )
	{
		CAABox box;
		box.center = infos[i].pos + parent->center;
		box.extends = parent->extends;
		if ( !frustrum.Test( box ) )
			continue;

		float dx, dy;
		dx = infos[i].pos.x - x;
		dy = infos[i].pos.y - y;

		float sqDist;

		sqDist = dx * dx + dy * dy;

		SRenderReqInner request;
		request.distance = sqDist;
		request.num = (int)i;
		
		if( infos[i].isVisible )
		{		
			inReqs.push_back( request );
		};
	}

	if( inReqs.empty() )
	{
		return;
	};
	std::sort( &inReqs[0], &inReqs[inReqs.size() - 1] + 1 );

	for( size_t j = 0; j < inReqs.size(); ++j )
	{

		float sqDist = inReqs[j].distance;
		float fogFactor = 1.0f - 0.00001f * sqDist;
		float scale = ( sqrt( sqDist ) - lodDist ) / lodWidth;

		
		if( scale < 0.0f )
		{
			scale = 0.0f;
		}
		if( scale > 1.0f )
		{
			scale = 1.0f;
		}


		size_t i = inReqs[j].num;
		//printf("%f \n", scale );

		D3DXMATRIX newMatMVP = infos[i].matTrans * matMVP.matMVP;
		D3DXMATRIX newMatMV = infos[i].matTrans * matMVP.matMV;
		
		if( parent->controllers.empty() )
		{
			SetObject( device, j == 0, scale, fogFactor );
			device->SetVertexShaderConstantF( 0, (float *)&newMatMVP, 4 );
		}
		else
		{
			SetAnimObject( device, j == 0, scale, fogFactor, matMVP );
		};

		if( infos[i].isVisible )
		{
			extern bool daMatrix;
			parent->Render( device, scale == 1.0f, newMatMV, infos[i].phase, j == 0 || daMatrix );
		}
	}
}

LPDIRECT3DTEXTURE9 LoadTexture( const char *file, LPDIRECT3DDEVICE9 device )
{
	if( file && !strncmp( file, "file:///", strlen( "file:///" ) ) )
	{
		file += strlen( "file:///" );
	}
	LPDIRECT3DTEXTURE9 texture = 0;

	char buff[1024];
	size_t ptr = 0;

	memset( buff, 0, 1024 );
	for( size_t i = 0; i < 1024; ++i )
	{
		if( file[i] == 0 )
		{
			break;
		};

		if( file[i] == '%' && file[i + 1] == '2' && file[i + 2] == '0' )
		{
			buff[ptr++] = ' ';
			i += 2;
			continue;
		}

		buff[ptr++] = file[i];

	};


	HRESULT hr = D3DXCreateTextureFromFileEx
		(device, 
		buff, 
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_DXT1,
		D3DPOOL_MANAGED,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		0,
		0,
		&texture );
	return texture;
}

LPDIRECT3DTEXTURE9 LoadTextureBinary( const char *file, LPDIRECT3DDEVICE9 device )
{
	FileReader *fileReader = CreateZipFileReader( file );

	size_t size = 0, width, height;

	fileReader->Read( (char *)&size, 4 );
	if( size == 0 )
	{
		return 0;
	}

	fileReader->Read( (char *)&width, 4 );
	fileReader->Read( (char *)&height, 4 );


	LPDIRECT3DTEXTURE9 texture; 
	device->CreateTexture( (UINT)width, (UINT)height, (UINT)size, 0, D3DFMT_DXT1, D3DPOOL_MANAGED, &texture, 0 );

	if( texture == 0 )
	{
		return texture;
	};

	for( size_t j = 0; j < size; ++j )
	{
		size_t length;
		fileReader->Read( (char *)&length, 4 );
		fileReader->Read( (char *)&width, 4 );
		fileReader->Read( (char *)&height, 4 );
		D3DLOCKED_RECT rect;
		texture->LockRect( (UINT)j, &rect, 0, 0);
		fileReader->Read( (char *)rect.pBits, length );
		texture->UnlockRect( (UINT)j );
	}

	delete fileReader;
	return texture;
};
void ConvertTexture( const char *fileNameOut, const char *fileNameIn, LPDIRECT3DDEVICE9 device  )
{
	LPDIRECT3DTEXTURE9 texture = LoadTexture( fileNameIn, device );

	FileWriter *fileWriter = CreateZipFileWriter( fileNameOut );
	size_t s = 0;

	if( texture == 0 )
	{
		fileWriter->Write( (char *)&s, 4 );
		return;
	};

	D3DSURFACE_DESC desc;
	texture->GetLevelDesc( 0, &desc );
	size_t i;
	for( i = 1; i < 10; ++i )
	{
		if( ( desc.Width >> i ) <= 4 )
		{
			break;
		};
		if( ( desc.Height >> i ) <= 4 )
		{
			break;
		};
	};

	s = i;
	
	fileWriter->Write( (char *)&s, 4 );
	fileWriter->Write( (char *)&desc.Width, 4 );
	fileWriter->Write( (char *)&desc.Height, 4 );



	for( size_t j = 0; j < s; ++j )
	{
		D3DSURFACE_DESC desc2;
		texture->GetLevelDesc( (UINT)j, &desc2 );
		D3DLOCKED_RECT rect;
		texture->LockRect( (UINT)j, &rect, 0, 0);
		size_t w = max( desc2.Width, 4 );
		size_t h = max( desc2.Height, 4 );
		size_t size = ( w * h * 8 ) / 16;

		fileWriter->Write( (char *)&size, 4 );
	  fileWriter->Write( (char *)&desc2.Width, 4 );
	  fileWriter->Write( (char *)&desc2.Height, 4 );

		fileWriter->Write( (char *)rect.pBits, size );
		texture->UnlockRect( (UINT)j );
	}

	delete fileWriter;
}

CRenderObject *LoadModel(  const char *fileName,  LPDIRECT3DDEVICE9 device  )
{
	CRenderObject *object = new CRenderObject;
	FileReader *fileReader = CreateZipFileReader( fileName );

	size_t size;
	fileReader->Read( (char *)&size, 4 );

	size_t textures;
	fileReader->Read( (char *)&textures, 4 );
	assert( textures == MAKEFOURCC('T','X','T','R') );

	size_t texSize;
	fileReader->Read( (char *)&texSize, 4 );
	

	for( size_t i = 0; i < texSize; ++i )
	{
		char buff[1024];
		size_t strLen, texNums;
		fileReader->Read( (char *)&strLen, 4 );
		//if( strLen )
		{
			fileReader->Read( buff, strLen + 1 );
			object->textures.push_back( LoadTextureBinary( buff, device ) );	
			printf( "%s \n", buff );
			fileReader->Read( (char *)&texNums, 4 );
			object->subMeshCounts.push_back( texNums );
		

		}
		/*else 
		{
			object->textures.push_back( 0 );
		}*/
	}

	size_t ind0 = 0;
	fileReader->Read( (char *)&ind0, 4 );
	assert( ind0 == MAKEFOURCC('I','N','D','0') );

	static uint16 buffer[65536];
	size_t ind = 0;
	
	for( size_t i = 0; i < size; ++i )
	{
		size_t indexSize;
		fileReader->Read( (char*)&indexSize, 4 );
		//indexBuffer->Lock( 0, (UINT)size, (void**)&data, 0 );
		fileReader->Read( (char *)(buffer + ind), 2 * indexSize );
		object->indexCounts.push_back( indexSize );
		ind += indexSize;
	}
	
	{
	LPDIRECT3DINDEXBUFFER9 indexBuffer = CreateIB( ind, buffer, device );
	object->indexBuffers.push_back( indexBuffer );
	}	

	size_t ind1 = 0;
	fileReader->Read( (char *)&ind1, 4 );
	assert( ind1 == MAKEFOURCC('I','N','D','1') );
	
	ind = 0;
	
	for( size_t i = 0; i < size; ++i )
	{
		size_t indexSize;
		fileReader->Read( (char*)&indexSize, 4 );
		fileReader->Read( (char *)(buffer + ind), 2 * indexSize );
		object->indexCountsLOD.push_back( indexSize );
		ind += indexSize;
	}

	{
	LPDIRECT3DINDEXBUFFER9 indexBuffer = CreateIB( ind, buffer, device );
	object->indexBuffersLOD.push_back( indexBuffer );
	}


	size_t aabb = 0; // = 
	fileReader->Read( (char *)&aabb, 4 );
	assert( aabb == MAKEFOURCC('A','A','B','B') );

	vec3 minV( +1000000.0f, +1000000.0f, +1000000.0f );
	vec3 maxV( -1000000.0f, -1000000.0f, -1000000.0f );
	for( size_t i = 0; i < size; ++i )
	{
		CAABox box;// = triangles[i]->box;
		fileReader->Read( (char *)&box, sizeof( CAABox ) );
		object->boxes.push_back( box );
		minV.minimize( box.center - box.extends );
		maxV.maximize( box.center + box.extends );
	}

	//minV.z = 0.0f;

	object->center  = ( maxV + minV ) * 0.5f;
	object->extends = ( maxV - minV ) * 0.5f;
	object->extends.z = maxV.z;
	object->center.z = 0;


	size_t type;
	fileReader->Read( (char *)&type, 4 );

	if( type == MAKEFOURCC('S','M','P','1') )
	{
		size_t verticesSize;
		size_t vertexSize;
		fileReader->Read( (char *)&verticesSize, 4 );
		fileReader->Read( (char *)&vertexSize, 4 ); 	
		object->vertexCount = verticesSize;


		assert( vertexSize == sizeof( SSimpleVertex ) );
		object->vertexSize = vertexSize;

		{
			LPDIRECT3DVERTEXBUFFER9 vertexBuffer = CreateVB( verticesSize, vertexSize, 0, device );
			void *data;
			size_t size = verticesSize * vertexSize;
			vertexBuffer->Lock( 0, (UINT)size, (void**)&data, 0 );
			fileReader->Read( (char *)data, size );
			vertexBuffer->Unlock();
			object->vertexBuffer = vertexBuffer;
		}

		{
			LPDIRECT3DVERTEXBUFFER9 vertexBuffer = CreateVB( verticesSize, vertexSize, 0, device );
			void *data;
			size_t size = verticesSize * vertexSize;
			vertexBuffer->Lock( 0, (UINT)size, (void**)&data, 0 );
			fileReader->Read( (char *)data, size );
			vertexBuffer->Unlock();
			object->vertexBufferLOD = vertexBuffer;
		}
	}
	else if( type == MAKEFOURCC('A','N','M','1') )
	{
		size_t verticesSize;
		size_t vertexSize;
		fileReader->Read( (char *)&verticesSize, 4 );
		fileReader->Read( (char *)&vertexSize, 4 ); 	
		object->vertexCount = verticesSize;

		assert( vertexSize == sizeof( SFatVertex ) );

		object->vertexSize = vertexSize;

		{
			LPDIRECT3DVERTEXBUFFER9 vertexBuffer = CreateVB( verticesSize, vertexSize, 0, device );
			void *data;
			size_t size = verticesSize * vertexSize;
			vertexBuffer->Lock( 0, (UINT)size, (void**)&data, 0 );
			fileReader->Read( (char *)data, size );
			vertexBuffer->Unlock();
			object->vertexBuffer = vertexBuffer;
		}

		{
			LPDIRECT3DVERTEXBUFFER9 vertexBuffer = CreateVB( verticesSize, vertexSize, 0, device );
			void *data;
			size_t size = verticesSize * vertexSize;
			vertexBuffer->Lock( 0, (UINT)size, (void**)&data, 0 );
			fileReader->Read( (char *)data, size );
			vertexBuffer->Unlock();
			object->vertexBufferLOD = vertexBuffer;
		}

		size_t indc;
		fileReader->Read( (char *)&indc, 4 );
		assert( indc == MAKEFOURCC('C','N','T','B') );
		for( size_t i = 0; i < size; ++i )
		{
			size_t ind;
			fileReader->Read( (char *)&ind, 4 );
			object->controllerBind.push_back( ind );
		}

		size_t indr;// = MAKEFOURCC('C','N','T','R');
		fileReader->Read( (char *)&indr, 4 );
		assert( indr == MAKEFOURCC('C','N','T','R') );
		size_t controllers;
		fileReader->Read( (char *)&controllers, 4 );
		for(  size_t i = 0; i < controllers; ++i )
		{
			size_t animSize;
			fileReader->Read( (char *)&animSize, 4 );
			char *buffer = new char[animSize]; 
			fileReader->Read( (char *)buffer, animSize );
			object->controllers.push_back( ( SAnimController *)buffer );
		}

	}

	delete fileReader;
	object->ReInit();

	return object;
}



void ConvertModel( const char *fileNameOut, const char *fileNameIn, bool animated, LPDIRECT3DDEVICE9 device, size_t first, size_t last )
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
	CXMLLoader<ÑColladaIDREFArray> idrefLoader;
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

	//CColladaAnimation::Freeeezzzze( _X"Hip_matrix" );

	controllerLoader.Load( root, false );


	SVertexMetric weakMetric = { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	SVertexMetric strongMetric = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f };
	CProgressiveMesh *progressiveMesh = new CProgressiveMesh( weakMetric, strongMetric, 0.001f );

	const std::vector<CColladaTriangles*> &trianglesIn = CColladaTriangles::GetArrayContainer();
	std::vector< SFatVertex > vertices;
	std::vector< SFatVertex > verticesLOD;
	std::vector< std::vector< uint16> > indicesVector;
	std::vector<CColladaController *> controllers;
	std::vector< size_t>              controllerInd;  
	std::vector<SAnimController *>    anims;
	std::vector<size_t>               animSizes;

	FileWriter *fileWriter = CreateZipFileWriter( fileNameOut );


	std::map<std::string, std::vector<CColladaTriangles*>  > textureMap;
  size_t trianglesSize = 0;
	for( size_t i = 0; i < trianglesIn.size(); ++i )
	{
		const CColladaEffect *effect = trianglesIn[i]->GetEffect();		
		size_t  strLen = 0; 
		const char   *strng  = 0;
		if( effect && effect->GetDiffuse() )
		{
			strng = (const char *)effect->GetDiffuse()->GetFileName().ptr;
			if( strng )
			{
				trianglesSize++;
				textureMap[std::string(strng)].push_back( trianglesIn[i] );
			}
		}
	}

 
	fileWriter->Write( (char *)&trianglesSize, 4  );

	size_t textures = MAKEFOURCC('T','X','T','R');
	fileWriter->Write( (char *)&textures, 4 );


	size_t texNums = textureMap.size();
	fileWriter->Write(  (char *)&texNums, 4 );
	std::vector<CColladaTriangles*> triangles;

	size_t index = 0;

	for( std::map<std::string, std::vector<CColladaTriangles*> >::iterator it = textureMap.begin(); it != textureMap.end(); ++it )
	{
		static char buff[1024];
    sprintf_s( buff, 1024, "%s.texture.%d", fileNameOut, index++ );
	  ConvertTexture( buff, it->first.c_str(), device );
		size_t strLen = strlen( buff );	
		fileWriter->Write( (char *)&strLen, 4 );
		fileWriter->Write( buff, strLen + 1 );
		std::vector<CColladaTriangles*> &vec = it->second;
		texNums = vec.size();
	  fileWriter->Write(  (char *)&texNums, 4 );

		for( size_t i = 0; i < vec.size(); ++i )
		{
			triangles.push_back( vec[i] );
		}
	}

	size_t inds = MAKEFOURCC('I','N','D','0');
	fileWriter->Write( (char *)&inds, 4 );

	trianglesSize = triangles.size();

	for ( size_t i = 0; i < trianglesSize; ++i )
	{
		std::vector< uint16 > indices;
		std::vector< SFatVertex > verticesLocal;
		std::vector< SFatVertex > verticesLocalAlt;
		triangles[i]->GetFatVertices( &verticesLocal, &indices );
		std::vector< uint16 > indicesAlt;
		std::vector<DWORD> result;
		result.resize( indices.size() / 3 );
		indicesAlt.resize( indices.size() );

		D3DXOptimizeFaces( &indices[0], (UINT)indices.size() / 3, (UINT)verticesLocal.size(), false, &result[0] );
		for( size_t j = 0; j < result.size(); ++j )
		{
			size_t src = result[j] * 3;
			size_t dst = j * 3;
			indicesAlt[dst + 0] = indices[src + 0];
			indicesAlt[dst + 1] = indices[src + 1];
			indicesAlt[dst + 2] = indices[src + 2];
		}



		result.resize( verticesLocal.size() );
		verticesLocalAlt.resize( verticesLocal.size() );
		std::vector<DWORD> remap;
		remap.resize( result.size() );

		D3DXOptimizeVertices( &indicesAlt[0], (UINT)indicesAlt.size() / 3, (UINT)verticesLocal.size(), false, &result[0] ); 

		for( size_t j = 0; j < result.size(); ++j )
		{
			verticesLocalAlt[j] = verticesLocal[result[j]];
			remap[result[j]] = (DWORD)j; 
		}

		verticesLocal = verticesLocalAlt;
		for( size_t j = 0; j < indicesAlt.size(); ++j )
		{
			indices[j] = (uint16) ( remap[indicesAlt[j]] + vertices.size() );
		}

		indicesVector.push_back( indices );
		CColladaController *colladaController = triangles[i]->GetBindController();
		if( colladaController )
		{
			colladaController = colladaController->GetController();
			if( find( controllers.begin(), controllers.end(), colladaController ) == controllers.end() )
			{
				size_t size;
				SAnimController * controller = CreateController( colladaController, &size, first, last );
				controllers.push_back( colladaController );
				animSizes.push_back( size );
				anims.push_back( controller );

			}
			controllerInd.push_back( find( controllers.begin(), controllers.end(), colladaController ) - controllers.begin() );
		}
		else	
		{
			controllerInd.push_back( size_t( -1 ) );
		}

		size_t indicesSize = indices.size();
		fileWriter->Write( (char *)&indicesSize, 4 );
		fileWriter->Write( (char *)&indices[0], indicesSize * 2 );

		//object.indexBuffers.push_back( CreateIB( , &indices[0], device ) );

		//object.indexCounts.push_back( indices.size() );

		//object.textures.push_back( texture );

		//object.boxes.push_back( triangles[i]->box );

		float f = 1.0f;// i == 0 ? 1.0f : 2.0f;
		for( size_t j = 0; j < verticesLocal.size(); ++j )
		{

			SFatVertex &vertex = verticesLocal[j];
			vertices.push_back( vertex );
			SVertexInfo info = { 
				vertex.coo[0] * f,
				vertex.coo[1] * f,
				vertex.coo[2] * f,
				vertex.nor[0] * f,
				vertex.nor[1] * f,
				vertex.nor[2] * f,
				vertex.tx0[0] * f,
				vertex.tx0[1] * f,
				(float)i, 
				0.0f, 0.0f, 0.0f };
				progressiveMesh->PushBack( info );
		}
	}

	/*

	for( size_t i = 0; i < trianglesSize; ++i )
	{
		const CColladaEffect *effect = triangles[i]->GetEffect();		
		size_t  strLen = 0; 
		char   *strng  = 0;
		if( effect && effect->GetDiffuse() )
		{
			strng = (char *)effect->GetDiffuse()->GetFileName().ptr;
			if( strng )
			{
				
				static char buff[1024];
				sprintf_s( buff, 1024, "%s.texture.%d", fileNameOut, i );
				ConvertTexture( buff, strng, device );
				strng = buff;
				strLen = strlen( strng );	
				
			}
		}

		fileWriter->Write(  (char *)&strLen, 4 );
		if( strng )
		{
			fileWriter->Write( strng, strLen + 1 );
		}
	}

	size_t inds = MAKEFOURCC('I','N','D','0');
	fileWriter->Write( (char *)&inds, 4 );

	for( size_t i = 0; i < trianglesSize; ++i )
	{
		
		std::vector< uint16 > indices;
		size_t sizeOld = vertices.size();
		triangles[i]->GetFatVertices( &vertices, &indices );
		indicesVector.push_back( indices );
		size_t sizeNew = vertices.size();
			

		CColladaController *colladaController =  triangles[i]->GetBindController();
		if( colladaController )
		{
			if( find( controllers.begin(), controllers.end(), colladaController ) == controllers.end() )
			{
				size_t   size;
				SAnimController * controller = CreateController(  colladaController, &size, first, last );
				controllers.push_back( colladaController );
				animSizes.push_back( size );
				anims.push_back( controller );
			}

			controllerInd.push_back( find( controllers.begin(), controllers.end(), colladaController ) - controllers.begin() );
		}
		else
		{
			controllerInd.push_back( size_t( -1 ) );
		}

		size_t indicesSize = indices.size();
		fileWriter->Write( (char *)&indicesSize, 4 );
		fileWriter->Write( (char *)&indices[0], indicesSize * 2 );
	

		//object.indexBuffers.push_back( CreateIB( , &indices[0], device ) );
		//object.indexCounts.push_back( indices.size() );
		//object.textures.push_back( texture );
		//object.boxes.push_back( triangles[i]->box );

		float f = 1.0f;// i == 0 ? 1.0f : 2.0f;

		for( size_t j = sizeOld; j < sizeNew; ++j )
		{
			SVertexInfo info = { 
				vertices[j].coo[0] * f,
				vertices[j].coo[1] * f,
				vertices[j].coo[2] * f,
				vertices[j].nor[0] * f,
				vertices[j].nor[1] * f,
				vertices[j].nor[2] * f,
				vertices[j].tx0[0] * f,
				vertices[j].tx0[1] * f,
				(float)i, 
				0.0f, 0.0f, 0.0f };

				progressiveMesh->PushBack( info );
		}
	}
	*/

	for( size_t i = 0; i < indicesVector.size(); ++i )
	{
		std::vector< uint16 > &indices = indicesVector[i];
		for( size_t j = 0; j < indices.size(); j += 3 )
		{
			size_t triangles[] = { indices[j + 0], indices[j + 1], indices[j + 2] };
			progressiveMesh->MountTriangle( triangles );
		}
	}

	for( size_t i = 0; i < 12; ++i )
	{
		progressiveMesh->Optimize();
	}

	verticesLOD.resize( vertices.size() );

	progressiveMesh->Remap( (char *)&vertices[0], (char *)&verticesLOD[0], sizeof( vertices[0] ) );

	size_t indl = MAKEFOURCC('I','N','D','1');
	fileWriter->Write( (char *)&indl, 4 );
	for( size_t i = 0; i < indicesVector.size(); ++i )
	{

		std::vector< uint16 > indices;
		progressiveMesh->Remap( indicesVector[i], &indices );

		size_t indicesSize = indices.size();
		fileWriter->Write( (char *)&indicesSize, 4 );
		fileWriter->Write( (char *)&indices[0], indicesSize * 2 );
	}

	size_t aabb = MAKEFOURCC('A','A','B','B');
	fileWriter->Write( (char *)&aabb, 4 );
	for( size_t i = 0; i < trianglesSize; ++i )
	{
		CAABox box = triangles[i]->box;
		fileWriter->Write( (char *)&box, sizeof( CAABox ) );
	}

	for( size_t i = 0; i < verticesLOD.size(); ++i )
	{
		float a = verticesLOD[i].weights[0];
		float b = verticesLOD[i].weights[1];

		float s = 255.0f / ( a + b );

		a *= s;
		b *= s;

		verticesLOD[i].weights[0] = (uint8)a;
		verticesLOD[i].weights[1] = (uint8)b;
		verticesLOD[i].weights[2] = 0;
		verticesLOD[i].weights[3] = 0;
	};

	if( animated )
	{
		size_t type = MAKEFOURCC('A','N','M','1');
		size_t verticesSize = vertices.size();
		size_t vertexSize = sizeof( SFatVertex );
		fileWriter->Write( (char *)&type, 4 );
		fileWriter->Write( (char *)&verticesSize, 4 );
		fileWriter->Write( (char *)&vertexSize, 4 ); 
		fileWriter->Write( (char *)&vertices[0], verticesSize * vertexSize );
		fileWriter->Write( (char *)&verticesLOD[0], verticesSize * vertexSize );

		size_t indc = MAKEFOURCC('C','N','T','B');
		fileWriter->Write( (char *)&indc, 4 );
		for( size_t i = 0; i < indicesVector.size(); ++i )
		{
			fileWriter->Write( (char *)&controllerInd[i], 4 );
		}

		size_t indr = MAKEFOURCC('C','N','T','R');
		fileWriter->Write( (char *)&indr, 4 );
		size_t controllers = anims.size();
		fileWriter->Write( (char *)&controllers, 4 );
		for(  size_t i = 0; i < controllers; ++i )
		{
			fileWriter->Write( (char *)&animSizes[i], 4 );
			fileWriter->Write( (char *)anims[i], animSizes[i] );
		}
	}
	else
	{
		size_t type = MAKEFOURCC('S','M','P','1');
		size_t verticesSize = vertices.size();
		size_t vertexSize = sizeof( SSimpleVertex );
		fileWriter->Write( (char *)&type, 4 );
		fileWriter->Write( (char *)&verticesSize, 4 );
		fileWriter->Write( (char *)&vertexSize, 4 ); 
		for( size_t i = 0; i < verticesSize; ++i )
		{
			fileWriter->Write( (char *)&vertices[i], vertexSize );
		}
		for( size_t i = 0; i < verticesSize; ++i )
		{
			fileWriter->Write( (char *)&verticesLOD[i], vertexSize );
		}
	}


	delete fileWriter;
	delete progressiveMesh;

	CColladaBindMaterial::ClearInner();
	CColladaController::ClearInner();
	CColladaImage::Clear();
	CColladaEffect::Clear();
	CColladaMaterial::Clear();
	CColladaBindMaterial::Clear();
	CColladaFloatArray::Clear();
	ÑColladaIDREFArray::Clear();
	CColladaSource::Clear();
	CColladaVertices::Clear();
	CColladaGeometry::Clear();
	CColladaController::Clear();
	CColladaNode::Clear();
	CColladaAnimation::Clear();


	xmlFreeDoc(doc);
	xmlCleanupParser();

}



