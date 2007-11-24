#pragma once

#include "XMLUtils.h"

class �ColladaIDREFArray : public CAutoContainer<�ColladaIDREFArray>
{
	std::vector<SStrng>  body;
	std::vector<xmlChar> content;
public:
	�ColladaIDREFArray(void);
public:
	const std::vector<SStrng> &GetArray() const;
	virtual ~�ColladaIDREFArray(void);
	void BuildContent(){};
	static const xmlChar *GetName() { return _X"IDREF_array"; };
	static �ColladaIDREFArray *CreateInstance( xmlNode *node );
};
