#pragma once

#include "bindable.h"


class DepthStencilState
	: public IBindable
{
public:
	enum Mode
	{
		Default,				// DepthOnStencilOff
		DepthOffStencilOff,
		DepthOnStencilWriteFF,	// FF: Front Face
		DepthOnStencilReadFF,
		DepthOffStencilWriteFF,
		DepthOffStencilReadFF,
		DepthOnStencilWriteBF,	// BF: Back Face
		DepthOnStencilReadBF,
		DepthOffStencilWriteBF,
		DepthOffStencilReadBF,
		DepthReversedStencilOff,// only things that are occluded should be drawn
		DepthEquals1,
	};
private:
	Mode m_mode;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
public:
	DepthStencilState( Graphics &gph, const Mode mode );

	void bind( Graphics &gph ) cond_noex override;
	static std::shared_ptr<DepthStencilState> fetch( Graphics &gph, const Mode mode );
	static std::string calcUid( const Mode mode );
	std::string getUid() const noexcept override;
};