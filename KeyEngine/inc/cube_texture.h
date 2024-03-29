#pragma once

#include <vector>
#include <memory>
#include "bindable.h"


class RenderTargetOutput;
class DepthStencilOutput;

class CubeTexture
	: public IBindable
{
	unsigned int m_slot;
protected:
	std::string m_path;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
public:
	CubeTexture( Graphics &gph, const std::string &path, const unsigned slot );

	void bind( Graphics &gph ) cond_noex override;
};

class CubeTextureOffscreenRT
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
	std::vector<std::shared_ptr<RenderTargetOutput>> m_renderTargetViews;
public:
	CubeTextureOffscreenRT( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot, const DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM );

	void bind( Graphics &gph ) cond_noex override;
	std::shared_ptr<RenderTargetOutput> shareRenderTarget( const size_t index ) const;
	RenderTargetOutput* renderTarget( const size_t index );
};

class CubeTextureOffscreenDS
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
	std::vector<std::shared_ptr<DepthStencilOutput>> m_depthStencilViews;
public:
	CubeTextureOffscreenDS( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot, const DepthStencilViewMode dsMode );

	void bind( Graphics &gph ) cond_noex override;
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer( const size_t index ) const;
	DepthStencilOutput* depthBuffer( const size_t index );
};