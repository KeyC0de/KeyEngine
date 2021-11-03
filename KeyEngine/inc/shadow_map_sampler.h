#pragma once

#include "bindable.h"


class ShadowMapSampler
	: public IBindable
{
	unsigned m_samplerMask;
	// 0 : point filtering
	// 1 : trilinear filtering		- Tick
	// 2 : hardware pcf	filtering	- Tick
	// 3 : software pcf filtering
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplers[4];
public:
	ShadowMapSampler( Graphics& gph );

	void bind( Graphics& gph ) cond_noex override;
	void setTrilinearFiltering( bool bEnable );
	bool isTrilinearFiltering() const;
	void setHwPcfFiltering( bool bEnable );
	bool isHwPcfFiltering() const;
private:
	unsigned getCurrentSlot() const;
	// gets the bitmap index of each filtering method
	static size_t getIndex( bool bTrilin, bool bHwPcf );
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> make( Graphics& gph, bool bTrilinear,
		bool bHwPcf );
};