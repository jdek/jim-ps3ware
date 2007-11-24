#pragma once

class CRenderObject;
struct SCameraInfo;
class CPortalFrustrumTest;

struct SObjectInfo
{
	bool isVisible;
	float phase;
	vec3 pos;
	vec3 startPos;
	D3DXMATRIX matTrans;
};

class CRenderObjectPlacer
{
	std::vector<SObjectInfo> infos;
	CRenderObject *parent;
public:
	void PushRequests();
	void UpdatePositions( float radius, float speed, float domainSize );
	void InitCircle( CRenderObject *parentObject, int number, float radius, float domainSize );
	void Init( CRenderObject *parentObject, int number, float offset, float domainSize );
	void InitSingle( CRenderObject *parentObject, float x, float y, float domainSize, float angle = 0.0f );
	void Render( LPDIRECT3DDEVICE9 device, const CPortalFrustrumTest &frustrum, float x, float y, float lodDist, float lodWidth, const SCameraInfo &matMVP );
	void SetCompensationSpeed( const vec3 &value );
};

CRenderObject *LoadModel(  const char *fileName,  LPDIRECT3DDEVICE9 device  );
void ConvertModel( const char *fileNameOut, const char *fileNameIn, bool animated,  LPDIRECT3DDEVICE9 device, size_t first, size_t last );
void ConvertTexture( const char *fileNameOut, const char *fileNameIn, LPDIRECT3DDEVICE9 device );
LPDIRECT3DTEXTURE9 LoadTextureBinary( const char *file, LPDIRECT3DDEVICE9 device );