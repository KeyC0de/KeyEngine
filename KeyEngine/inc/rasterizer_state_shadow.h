#pragma once

#include <array>
#include "bindable.h"


class RasterizerStateShadow
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	int m_depthBias;
	float m_slopeScaleDepthBias;
	float m_clamp;
public:
	RasterizerStateShadow( Graphics &gph, const int depthBias, const float slopeBias, const float clamp );

	void bind( Graphics &gph ) cond_noex override;
	void setParameters( const int depthBias, const float slopeBias, const float clamp ) noexcept;
	const int getDepthBias() const noexcept;
	const float getSlopeScaleDepthBias() const noexcept;
	const float getClamp() const noexcept;
};