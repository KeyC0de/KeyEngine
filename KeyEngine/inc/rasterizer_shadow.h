#pragma once

#include <array>
#include "bindable.h"


class RasterizerShadow
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	int m_depthBias;
	float m_slopeScaleDepthBias;
	float m_clamp;
public:
	RasterizerShadow( Graphics &gph, int depthBias, float slopeBias, float clamp );

	void setParameters( Graphics &gph, int depthBias, float slopeBias,
		float clamp );
	void bind( Graphics &gph ) cond_noex override;
	int getDepthBias() const;
	float getSlopeScaleDepthBias() const;
	float getClamp() const;
};