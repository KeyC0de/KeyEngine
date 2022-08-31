#pragma once

#include "bindable.h"


//=============================================================
//	\class	PixelShaderNull
//
//	\author	KeyC0de
//	\date	2021/06/25 20:21
//
//	\brief	writes nothing to color buffer
//=============================================================
class PixelShaderNull
	: public IBindable
{
public:
	PixelShaderNull( Graphics &gph );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<PixelShaderNull> fetch( Graphics &gph );
	static std::string calcUid();
	const std::string getUid() const noexcept override;
};