#pragma once

#include <array>
#include <optional>
#include "bindable.h"


//=============================================================
//	\class	BlendState
//	\author	KeyC0de
//	\date	2022/02/19 10:23
//	\brief	blending factors are optional : apply separate values for R, G, B, A channels
//=============================================================
class BlendState
	: public IBindable
{
public:
	enum Mode
	{
		NoBlend,
		Additive,
		Multiplicative,
		DoubleMultiplicative,
		Alpha,
		AlphaToCoverage
	};
private:
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_pBlendState;
	Mode m_mode;
	unsigned m_renderTargetSlot;
	std::optional<std::array<float, 4>> m_blendFactors;	//
public:
	BlendState( Graphics &gph, const Mode mode, const unsigned renderTargetSlot, std::optional<float> blendFactors = {} );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<BlendState> fetch( Graphics &gph, const Mode mode, const unsigned renderTargetSlot, std::optional<float> blendFactors = {} );
	static std::string calcUid( const Mode mode, const unsigned renderTargetSlot, std::optional<float> blendFactors );
	const std::string getUid() const noexcept override;
	void fillBlendFactors( const float sameBlendFactor ) cond_noex;
	void setBlendFactors( std::array<float, 4> blendFactors ) cond_noex;
	void setBlendFactorRed( const float blendFactorRed ) cond_noex;
	void setBlendFactorGreen( const float blendFactorGreen ) cond_noex;
	void setBlendFactorBlue( const float blendFactorBlue ) cond_noex;
	void setBlendFactorAlpha( const float blendFactorAlpha ) cond_noex;
	const float getBlendFactorRed() const noexcept;
	const float getBlendFactorGreen() const noexcept;
	const float getBlendFactorBlue() const noexcept;
	const float getBlendFactorAlpha() const noexcept;
	const std::array<float, 4>& getBlendFactors() const noexcept;
	std::array<float, 4>& blendFactors();
};