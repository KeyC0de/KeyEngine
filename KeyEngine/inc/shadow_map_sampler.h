#pragma once

#include "bindable.h"


//=============================================================
//	\class	ShadowMapSampler
//
//	\author	KeyC0de
//	\date	2022/02/19 22:12
//
//	\brief	Creating 3 samplers just in case
//			0 : point filtering
//			1 : trilinear filtering
//			2 : hardware pcf filtering
//=============================================================
class ShadowMapSampler
	: public IBindable
{
	unsigned m_samplerMask;
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
	//===================================================
	//	\function	getIndex
	//	\brief  gets the bitmap index of each filtering method
	//	\date	2022/02/19 22:05
	static size_t getIndex( bool bTrilin, bool bHwPcf );
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> make( Graphics& gph, bool bTrilinear,
		bool bHwPcf );
};