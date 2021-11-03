#pragma once

#include <array>
#include <optional>
#include "bindable.h"


class BlendState
	: public IBindable
{
public:
	enum Mode
	{
		NoBlend,
		Additive,
		Multiplicative,
		Alpha,
		AlphaToCoverage
	};
private:
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_pBlendState;
	Mode m_mode;
	unsigned m_renderTargetSlot;
	// optional blending factors - apply separate values for R, G, B, A channels
	std::optional<std::array<float, 4>> m_blendFactors;
public:
	BlendState( Graphics& gph, Mode mode, unsigned renderTargetSlot,
		std::optional<float> blendFactors = {} );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<BlendState> fetch( Graphics& gph, Mode mode,
		unsigned renderTargetSlot, std::optional<float> blendFactors = {} );
	void setBlendFactors( float blendFactors ) cond_noex;
	float getBlendFactor() const cond_noex;
	static std::string generateUID( Mode mode, unsigned renderTargetSlot,
		std::optional<float> blendFactors );
	std::string getUID() const noexcept override;
};
