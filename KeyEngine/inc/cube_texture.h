#pragma once

#include "key_wrl.h"
#include <vector>
#include <memory>
#include "bindable.h"
#include "texture_desc.h"


static constexpr unsigned nCubeFaces = 6u;

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
	unsigned getSlot() const noexcept;
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
	RenderTargetOutput* accessRenderTarget( const size_t index );
};

//=============================================================
//	\class	TextureArrayOffscreenDS
//	\author	KeyC0de
//	\date	2024/09/03 22:46
//	\brief	A TextureCube (HLSL), SRV for HLSL access & Depth Stencil View all wrapped up; can be used either for input OR for output
//=============================================================
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
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer( const size_t faceIndex ) const;
	DepthStencilOutput* accessDepthBuffer( const size_t faceIndex );
	unsigned getSlot() const noexcept;
};

//=============================================================
//	\class	CubeTextureArrayOffscreenDS
//	\author	KeyC0de
//	\date	2024/09/03 22:45
//	\brief	A TextureCubeArray (HLSL), SRV for HLSL access and Depth Stencil View all wrapped up; can be used either for input OR for output
//=============================================================
class CubeTextureArrayOffscreenDS
	: public IBindable
{
	unsigned int m_slot;
	unsigned int m_nCubeTextures;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pD3dSrv;
	std::vector<std::array<std::shared_ptr<DepthStencilOutput>, nCubeFaces>> m_depthStencilViews;
public:
	CubeTextureArrayOffscreenDS( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const DepthStencilViewMode dsvMode, const unsigned nCubeTextures );

	void bind( Graphics &gfx ) cond_noex override;
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer( const size_t index, const size_t faceIndex ) const;
	DepthStencilOutput* accessDepthBuffer( const size_t index, const size_t faceIndex );
	unsigned getSlot() const noexcept;
	//	\function	getTextureCount	||	\date	2024/09/06 17:01
	//	\brief	returns amount of Cube Textures, which is in fact 6 times the total texture count
	unsigned getTextureCount() const noexcept;
};
