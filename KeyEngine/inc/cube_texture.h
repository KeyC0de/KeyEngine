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
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
public:
	CubeTexture( Graphics &gph, const std::string &path, const unsigned slot );

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
	CubeTextureRT( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot, const DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM );

	void bind( Graphics &gph ) cond_noex override;
};

class CubeTextureDS
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
	std::vector<std::shared_ptr<DepthStencilOutput>> m_depthStencilViews;
public:
	CubeTextureDS( Graphics &gph, const unsigned size, const unsigned slot, const DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS );

	void bind( Graphics &gph ) cond_noex override;
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer( const size_t index ) const;
};