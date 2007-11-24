#pragma once

#include "XMLUtils.h"
class CColladaFloatArray;
class �ColladaIDREFArray;

class CColladaSource : public CAutoContainer<CColladaSource>
{
	CColladaFloatArray  *fArray;
	�ColladaIDREFArray  *sArray;
	size_t              floatStride;
public:
	CColladaSource(void);
public:
	void BuildContent();
	size_t GetFloatStride() const { return floatStride; };
	size_t GetFloatSize() const { return GetFloatArray().size() / GetFloatStride(); };
	const std::vector<float> &GetFloatArray() const;
	const std::vector<SStrng> &GetIDREFArray() const;
	virtual ~CColladaSource(void);
	static const xmlChar *GetName() { return _X"source"; };
	static CColladaSource *CreateInstance( xmlNode *node );
	

};