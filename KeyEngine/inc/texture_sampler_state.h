#pragma once

#include "bindable.h"


//=============================================================
//	\class	TextureSamplerState
//	\author	KeyC0de
//	\date	2024/04/28 18:00
//	\brief	Texture sampler state class
//			the bound slot is equal to 0 for default TS operation, yet it's > 0 for Shadow operations (automatically set to 1 if bHwPcf, otherwise equal to 2)
//			Currently for a ShadowTS HW-PCF is automatically used
//=============================================================
class TextureSamplerState
	: public IBindable
{
public:
	enum TextureSamplerMode
	{
		DefaultTS,
		ShadowTS,
	};
	enum FilterMode
	{
		Anisotropic,	// can't be used for Shadow Maps
		Point,			// or Linear
		Bilinear,
		Trilinear,
	};
	enum AddressMode
	{
		Wrap,			// default for DefaultTS
		Mirror,
		Clamp,			// d3d11 default
		Border,			// default for ShadowTS
	};
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	unsigned m_slot;
	TextureSamplerMode m_mode;
	FilterMode m_filteringMode;
	AddressMode m_addressingMode;
	bool m_bHwPcf;
public:
	TextureSamplerState( Graphics &gph, const TextureSamplerMode samplingMode, const FilterMode filterMode, const AddressMode addressMode );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<TextureSamplerState> fetch( Graphics &gph, const TextureSamplerMode samplingMode, const FilterMode filterMode, const AddressMode addressMode );
	static std::string calcUid( const TextureSamplerMode samplingMode, const FilterMode filterMode, const AddressMode addressMode );
	std::string getUid() const noexcept override;
	static unsigned calcSlot( const TextureSamplerMode samplingMode ) noexcept;
private:
	unsigned getSlot() const noexcept;
};