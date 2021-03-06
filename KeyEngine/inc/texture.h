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
	Texture( Graphics &gph, const std::string &filepath, unsigned slot );
	//===================================================
	//	\function	Texture
	//	\brief  dynamic Texture constructor with CPU per frame update
	//	\date	2021/10/30 3:42
	Texture( Graphics &gph, unsigned width, unsigned height, unsigned slot );

	void bind( Graphics &gph ) cond_noex override;
	void update( Graphics &gph ) cond_noex;
	bool hasAlpha() const noexcept;
	std::string getPath() const noexcept;
	static std::shared_ptr<Texture> fetch( Graphics &gph, const std::string &filepath,
		unsigned slot );
	static std::string generateUid( const std::string &filepath, unsigned slot );
	std::string getUid() const noexcept override;
	static void flipModelNormalMapsGreenChannel( const std::string &objPath );
	//===================================================
	//	\function	flipNormalMapGreenChannel
	//	\brief  flips the normal map green channel of given texture
	//	\date	2022/02/19 17:22
	static void flipNormalMapGreenChannel( const std::string &pathIn,
		const std::string &pathOut );
	static void validateNormalMap( const std::string &pathIn, float thresholdMin,
		float thresholdMax );
	static void makeStripes( const std::string &pathOut, int size, int stripeWidth );
private:
	//===================================================
	//	\function	transformBitmap
	//	\brief  apply function f at every Texel in the Bitmap
	//	\date	2022/02/18 19:14
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
		transformBitmap( bitmap,
			f );
		bitmap.save( pathOut );
	}

	static unsigned calculateNumberOfMipMaps( unsigned width, unsigned height ) noexcept;
};