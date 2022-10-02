#pragma once

#include "bindable.h"


//=============================================================
//	\class	ShadowMapSamplerState
//
//	\author	KeyC0de
//	\date	2022/02/19 22:12
//
//	\brief	Comparison sampler mainly used for Shadow Maps
//			the bound slot is equal to 1 if bHwPcf, otherwise equal to 2
//=============================================================
class ShadowMapSamplerState
	: public IBindable
{
public:
	enum FilterMode
	{
		Linear,
		Bilinear,
		Trilinear,
	};
private:
	unsigned m_slot;
	bool m_bHwPcf;
	FilterMode m_filterMode;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
public:
	ShadowMapSamplerState( Graphics &gph, const bool bHwPcf, const FilterMode filterMode, const unsigned slot_autoCalcedDontSet = -1 );

	void bind( Graphics &gph ) cond_noex override;
	const FilterMode getFilterMode() const noexcept;
	bool isHwPcfFiltering() const noexcept;
	const unsigned getSlot() const noexcept;
	static std::shared_ptr<ShadowMapSamplerState> fetch( Graphics &gph, const bool bHwPcf, const FilterMode filterMode, const unsigned slot );
	static std::string calcUid( const bool bHwPcf, const FilterMode filterMode, const unsigned slot );
	const std::string getUid() const noexcept override;
};