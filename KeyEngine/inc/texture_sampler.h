#pragma once

#include "bindable.h"


class TextureSampler
	: public IBindable
{
public:
	enum Type
	{
		Anisotropic,
		Point,
		Bilinear,
		Trilinear
	};
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	Type m_filterType;
	bool m_bMirrorAddr;
	bool m_bClampAddr;
	unsigned m_slot;
public:
	TextureSampler( Graphics& gph, unsigned slot, Type filterType, bool bMirrorTexAddr,
		bool bClampTexAddr );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<TextureSampler> fetch( Graphics& gph, unsigned slot,
		Type filterType, bool bMirrorTexAddr = false, bool bClampTexAddr = false );
	static std::string generateUID( unsigned slot, Type filterType, bool bMirrorTexAddr,
		bool bClampTexAddr );
	std::string getUID() const noexcept override;
};