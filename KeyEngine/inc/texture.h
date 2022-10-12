#pragma once

#include "bindable.h"
#include "bitmap.h"


class Texture
	: public IBindable
{
	bool m_bAlpha = false;
	bool m_bDynamic = false;
	std::string m_path;
	unsigned m_width;
	unsigned m_height;
	unsigned int m_slot;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSrv;
public:
	Texture( Graphics &gph, const std::string &filepath, const unsigned slot );
	//	\function	Texture	||	\date	2021/10/30 3:42
	//	\brief	Texture constructor with dynamic CPU per frame update
	Texture( Graphics &gph, const unsigned width, const unsigned height, const unsigned slot );

	void paintTextureToBitmap( Graphics &gph, ID3D11Texture2D *tex, const Bitmap &bitmap, const D3D11_BOX *destPortion = nullptr );
	void bind( Graphics &gph ) cond_noex override;
	void update( Graphics &gph ) cond_noex;
	bool hasAlpha() const noexcept;
	const std::string& getPath() const noexcept;
	static std::shared_ptr<Texture> fetch( Graphics &gph, const std::string &filepath, const unsigned slot );
	static std::string calcUid( const std::string &filepath, const unsigned slot );
	const std::string getUid() const noexcept override;

public:
	static void flipModelNormalMapsGreenChannel( const std::string &objPath );
	//	\function	flipNormalMapGreenChannel	||	\date	2022/02/19 17:22
	//	\brief  flips the normal map green channel of given texture
	static void flipNormalMapGreenChannel( const std::string &pathIn, const std::string &pathOut );
	static void validateNormalMap( const std::string &pathIn, const float thresholdMin, const float thresholdMax );
	static void makeStripes( const std::string &pathOut, const int size, const int stripeWidth );
private:
	//	\function	transformBitmap	||	\date	2022/02/18 19:14
	//	\brief  apply function f at every Texel in the Bitmap
	template<typename F>
	static void transformBitmap( Bitmap &bitmap,
		F &&f )
	{
		const unsigned width = bitmap.getWidth();
		const unsigned height = bitmap.getHeight();
		for ( unsigned y = 0; y < height; ++y )
		{
			for ( unsigned x = 0; x < width; ++x )
			{
				const auto v = Bitmap::colorToVector( bitmap.getTexel( x,
					y ) );
				bitmap.setTexel( x,
					y,
					Bitmap::vectorToColor( f( v, x, y ) ) );
			}
		}
	}

	template<typename F>
	static void transformFile( const std::string &pathIn,
		const std::string &pathOut,
		F &&f )
	{
		auto bitmap = Bitmap::loadFromFile( pathIn );
		Texture::transformBitmap( bitmap,
			f );
		bitmap.save( pathOut );
	}

	static const unsigned calculateNumberOfMipMaps( const unsigned width, const unsigned height ) noexcept;
};

/*
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
	std::shared_ptr<RenderTargetOutput> shareRenderTarget( const size_t index ) const;
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
};*/