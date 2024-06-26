#pragma once

#include "key_wrl.h"
#include "bindable.h"


class Graphics;

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
		DepthReadOnlyStencilOff,		// useful for Transparency
		DepthReadOnlyEquals1StencilOff,		// useful for Skybox
	};
private:
	Mode m_mode;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
public:
	DepthStencilState( Graphics &gfx, const Mode mode );

	void bind( Graphics &gfx ) cond_noex override;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& getD3dDepthStencilState();
	static std::shared_ptr<DepthStencilState> fetch( Graphics &gfx, const Mode mode );
	static std::string calcUid( const Mode mode );
	std::string getUid() const noexcept override;
};