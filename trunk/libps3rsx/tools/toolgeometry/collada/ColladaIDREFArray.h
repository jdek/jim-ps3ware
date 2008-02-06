#pragma once

#include "../../common/XMLUtils.h"

class CColladaIDREFArray : public CAutoContainer<CColladaIDREFArray>
{
		std::vector<SStrng>  body;
		std::vector<xmlChar> content;
	public:
		CColladaIDREFArray(void);
	public:
		const std::vector<SStrng> &GetNameArray() const;
		virtual ~CColladaIDREFArray(void);
		void BuildContent(){};
		static const xmlChar *GetName() { return _X"Name_array"; };
		static CColladaIDREFArray *CreateInstance( xmlNode *node );
};
