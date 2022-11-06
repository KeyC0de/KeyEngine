#pragma once

#include <array>
#include "bindable.h"


class RasterizerState
	: public IBindable
{
public:
	enum CullMode
	{
		FrontSided,
		BackSided,
		TwoSided,
	};
	enum FillMode
	{
		Solid,
		Wireframe,
	};
protected:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	CullMode m_cullMode;
	FillMode m_fillMode;
public:
	RasterizerState( Graphics &gph, const CullMode cullMode, const FillMode fillMode = FillMode::Solid );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<RasterizerState> fetch( Graphics &gph, const CullMode cullMode, const FillMode fillMode = FillMode::Solid );
	static std::string calcUid( const CullMode cullMode, const FillMode fillMode );
	const std::string getUid() const noexcept override;
	const CullMode& getCullMode() const noexcept;
	const FillMode& getFillMode() const noexcept;
};