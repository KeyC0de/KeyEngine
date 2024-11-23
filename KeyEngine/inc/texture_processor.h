#pragma once

#include <string>
#include "bitmap.h"


using TextureOp = DirectX::XMVECTOR(*)(Bitmap::Texel);

class TextureProcessor final
{
	template<typename F>
	static void transformFile( const std::string &pathIn,
		const std::string &pathOut,
		F &&f )
	{
		auto bitmap = Bitmap::loadFromFile( pathIn );
		TextureProcessor::transformBitmap( bitmap, f );
		bitmap.save( pathOut );
	}

	static const unsigned calculateNumberOfMipMaps( const unsigned width, const unsigned height ) noexcept;
public:
	static void flipModelNormalMapsGreenChannel( const std::string &objPath );
	/// \brief	flips the normal map green channel of given texture
	static void flipNormalMapGreenChannel( const std::string &pathIn, const std::string &pathOut );
	static void validateNormalMap( const std::string &pathIn, const float thresholdMin, const float thresholdMax );
	static void makeStripes( const std::string &pathOut, const int size, const int stripeWidth );
	/// \brief	apply function f at every Texel in the Bitmap
	static void transformBitmap( Bitmap &bitmap, TextureOp f );
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
				const auto v = Bitmap::colorToVector( bitmap.getTexel( x, y ) );
				bitmap.setTexel( x, y, Bitmap::vectorToColor( f( v, x, y ) ) );
			}
		}
	}
};