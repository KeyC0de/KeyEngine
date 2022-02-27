#pragma once

#include "bindable.h"


class TextureSampler
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
	FilterMode m_filtering;
	AddressMode m_addressing;
	unsigned m_slot;
public:
	TextureSampler( Graphics& gph, unsigned slot, FilterMode filt, AddressMode addr );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<TextureSampler> fetch( Graphics& gph, unsigned slot,
		FilterMode filt, AddressMode addr );
	static std::string generateUid( unsigned slot, FilterMode filt, AddressMode addr );
	std::string getUid() const noexcept override;
};