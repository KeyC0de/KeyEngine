#pragma once

#include "key_wrl.h"
#include <vector>
#include <memory>
#include "bindable.h"
#include "texture_desc.h"


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
	CubeTexture( Graphics &gfx, const std::string &path, const unsigned slot );

	void bind( Graphics &gfx ) cond_noex override;
	const std::string& getPath() const noexcept;
	static std::shared_ptr<CubeTexture> fetch( Graphics &gfx, const std::string &filepath, const unsigned slot );
	static std::string calcUid( const std::string &filepath, const unsigned slot );
	std::string getUid() const noexcept override;
};

class CubeTextureOffscreenRT
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
	std::vector<std::shared_ptr<RenderTargetOutput>> m_renderTargetViews;
public:
	CubeTextureOffscreenRT( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const DXGI_FORMAT format );

	void bind( Graphics &gfx ) cond_noex override;
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
	CubeTextureOffscreenDS( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const DepthStencilViewMode dsvMode );

	void bind( Graphics &gfx ) cond_noex override;
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer( const size_t index ) const;
	DepthStencilOutput* depthBuffer( const size_t index );
};