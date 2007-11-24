#pragma once

#include "XMLUtils.h"
#include "..//MATH//mathLib.h"

class CColladaFloatArray;
class CColladaSource;
class CColladaNode;

struct SWeightInfo
{
	float   weights[4];
	size_t  indices[4];

	SWeightInfo()
	{
		for( size_t i = 0; i < 4; ++i )
		{
			indices[i] = 0;
			weights[i] = i == 0 ? 1.0f : 0.0f;
		}
	};
};


class CColladaController : public CAutoContainer<CColladaController>
{
	std::vector<SWeightInfo>     weights;
	std::vector<mat3x4>          invBindMatrices;
	std::vector<mat3x4>          transformMatrices;
	std::vector<CColladaNode *>  nodes;
	std::vector<size_t>          parents;
	std::vector<CColladaSource*> matrixAnimations;
	CColladaSource              *jointArray;
	mat3x4                       bindMatrix;
	std::vector<size_t>          remap;
	static std::map< SStrng,  SAutoInd>  jointMap; 
	static size_t                        jointNumber;
	static CColladaController   *theGod;
	
	void InitJointInput( const  xmlNode * input );
	void InitWeightInput( const  xmlNode * input, const xmlNode *vcount, const xmlNode *v, size_t count );
	void PostInit();
public:
	CColladaController(void);
public:
	static CColladaController *GetController()
	{
		if( theGod == 0 )
		{
			theGod = new CColladaController;
		}
		return theGod;
	}
	static void ClearInner() 
	{
		delete theGod;
		theGod = 0;
		jointMap.clear();
		jointNumber = 0;
	}
	const mat3x4 &GetMatrix() const
	{
		return bindMatrix;
	}
	virtual ~CColladaController(void);
	static const xmlChar *GetName() { return _X"controller"; };
	static CColladaController *CreateInstance( const xmlNode *node );
	size_t GetSize() const
	{
		return nodes.size();
	}
	size_t GetParentIndex( size_t i )
	{
		size_t index = parents[i];
		if( index == size_t( - 1 ) )
		{
			return GetSize();
		}
		return index;
	}
	const mat3x4 GetInvBindMat( size_t i ) const
	{
		return invBindMatrices[i];
	}
	mat3x4 GetMatrix( size_t i, size_t j ) const;
	const std::vector<CColladaSource*> &GetAnimations() const
	{
		return matrixAnimations;
	}
	const std::vector<SWeightInfo> &GetWeights() const
	{
		return weights;
	};
};
