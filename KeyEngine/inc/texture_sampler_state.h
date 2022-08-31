#pragma once

#include "bindable.h"


class TextureSamplerState
	: public IBindable
{
public:
	enum FilterMode
	{
		Anisotropic,
		Point,
		Bilinear,
		Trilinear
	};

	enum AddressMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border
	};
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	FilterMode m_filteringMode;
	AddressMode m_addressingMode;
	unsigned m_slot;
public:
	TextureSamplerState( Graphics &gph, const unsigned slot, const FilterMode filterMode, const AddressMode addressMode );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<TextureSamplerState> fetch( Graphics &gph, const unsigned slot, const FilterMode filterMode, const AddressMode addressMode );
	static std::string calcUid( const unsigned slot, const FilterMode filterMode, const AddressMode addressMode );
	const std::string getUid() const noexcept override;
};