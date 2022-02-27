#pragma once

#include <array>
#include "bindable.h"


class Rasterizer
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	bool m_bTwoSided;
	bool m_bWireframe;
public:
	Rasterizer( Graphics& gph, bool bTwoSided, bool bWireframe = false );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<Rasterizer> fetch( Graphics& gph, bool bTwoSided,
		bool bWireframe = false );
	static std::string generateUid( bool bTwoSided, bool bWireframe );
	std::string getUid() const noexcept override;
};