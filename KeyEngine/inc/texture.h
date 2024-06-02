#pragma once

#include "key_wrl.h"
#include "bindable.h"
#include "bitmap.h"
#include "render_target.h"
#include "depth_stencil_view.h"


class Graphics;

using TextureOp = DirectX::XMVECTOR(*)(Bitmap::Texel);

class Texture
	: public IBindable
{
	bool m_bAlpha = false;
	bool m_bDynamic = false;
	std::string m_path;
	unsigned m_width;
	unsigned m_height;
	unsigned int m_slot;
	TextureOp m_op = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
public:
	Texture( Graphics &gfx, const std::string &filepath, const unsigned slot, TextureOp op = nullptr );
	//	\function	Texture	||	\date	2021/10/30 3:42
	//	\brief	Texture constructor with dynamic CPU per frame update
	Texture( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, TextureOp op = nullptr );

	void paintTextureWithBitmap( Graphics &gfx, ID3D11Texture2D *tex, const Bitmap &bitmap, const D3D11_BOX *destPortion = nullptr );
	void bind( Graphics &gfx ) cond_noex override;
	void update( Graphics &gfx ) cond_noex;
	bool hasAlpha() const noexcept;
	const std::string& getPath() const noexcept;
	unsigned getWidth() const noexcept;
	unsigned getHeight() const noexcept;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& getD3dSrv();
	static std::shared_ptr<Texture> fetch( Graphics &gfx, const std::string &filepath, const unsigned slot, TextureOp op = nullptr );
	static std::string calcUid( const std::string &filepath, const unsigned slot, TextureOp op = nullptr );
	std::string getUid() const noexcept override;
};

//=============================================================
//	\class	TextureOffscreenRT
//	\author	KeyC0de
//	\date	2022/10/13 11:08
//	\brief	Flexible Render Target View Wrapper that will be used EITHER for input OR for output (offscreen/RTT/back buffer rendering)
//=============================================================
class TextureOffscreenRT
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
	std::shared_ptr<RenderTargetOutput> m_pRtv;
public:
	TextureOffscreenRT( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const RenderTargetViewMode rtvMode );

	void bind( Graphics &gfx ) cond_noex override;
	std::shared_ptr<RenderTargetOutput> shareRenderTarget() const;
	std::shared_ptr<RenderTargetOutput>& rtv() noexcept;
	unsigned getSlot() const noexcept;
};

//=============================================================
//	\class	TextureOffscreenDS
//	\author	KeyC0de
//	\date	2022/10/13 11:08
//	\brief	Flexible Depth Stencil View Wrapper that will be used EITHER for input OR for output (offscreen/RTT/back buffer rendering)
//=============================================================
class TextureOffscreenDS
	: public IBindable
{
	unsigned int m_slot;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
	std::shared_ptr<DepthStencilOutput> m_pDsv;
public:
	TextureOffscreenDS( Graphics &gfx, const unsigned width, const unsigned height, const unsigned slot, const DepthStencilViewMode dsvMode );

	void bind( Graphics &gfx ) cond_noex override;
	std::shared_ptr<DepthStencilOutput> shareDepthBuffer() const;
	std::shared_ptr<DepthStencilOutput>& dsv() noexcept;
	unsigned getSlot() const noexcept;
};
