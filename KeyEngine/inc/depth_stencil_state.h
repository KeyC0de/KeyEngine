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
		DepthOffStencilWriteFF,
		DepthOnStencilReadFF,
		DepthOffStencilReadFF,
		DepthOnStencilWriteBF,	// BF: Back Face
		DepthOffStencilWriteBF,
		DepthOnStencilReadBF,
		DepthOffStencilReadBF,
		DepthReversed,			// only things that are occluded should be drawn
		DepthEquals1,
	};
private:
	Mode m_mode;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
public:
	DepthStencilState( Graphics& gph, Mode mode );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<DepthStencilState> fetch( Graphics& gph, Mode mode );
	static std::string generateUID( Mode mode );
	std::string getUID() const noexcept override;
};