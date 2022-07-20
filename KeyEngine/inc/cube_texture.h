#pragma once

#include <vector>
#include <memory>
#include "bindable.h"


class Bitmap;
class RenderTargetOutput;
class DepthStencilOutput;

class CubeTexture
	: public IBindable
{
	unsigned int m_slot;
protected:
	std::string m_path;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
public:
	CubeTexture( Graphics &gph, const std::string &path, unsigned slot );

	void bind( Graphics &gph ) cond_noex override;
};

class CubeTextureRT
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
	std::vector<std::shared_ptr<RenderTargetOutput>> m_renderTargetViews;
public:
	CubeTextureRT( Graphics &gph, unsigned width, unsigned height,
		unsigned slot, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM );

	void bind( Graphics &gph ) cond_noex override;
	std::shared_ptr<RenderTargetOutput> shareRenderTarget( size_t index ) const;
};

class CubeTextureDS
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
	std::vector<std::shared_ptr<DepthStencilOutput>> m_depthStencilViews;
public:
	CubeTextureDS( Graphics &gph, unsigned size, unsigned slot,
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS );

	void bind( Graphics &gph ) cond_noex override;
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer( size_t index ) const;
};