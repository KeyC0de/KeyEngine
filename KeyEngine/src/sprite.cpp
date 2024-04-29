#ifdef D2D_INTEROP
#include "sprite.h"
#include "os_utils.h"

#pragma comment( lib, "windowscodecs.lib" )


namespace mwrl = Microsoft::WRL;

Sprite::Sprite( const std::wstring& filename,
	Graphics &gph )
{
	// create the wic factory
	HRESULT hres = CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &m_pWicFactory ) );
	ASSERT_HRES_IF_FAILED;

	// create the decoder to decode the image files
	hres = m_pWicFactory->CreateDecoderFromFilename( filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &m_pWicDecoder );
	ASSERT_HRES_IF_FAILED;

	// get frame from image
	hres = m_pWicDecoder->GetFrame( 0, &m_pBitmapFrame );
	ASSERT_HRES_IF_FAILED;

	// create wic format converter
	hres = m_pWicFactory->CreateFormatConverter( &m_pWicConverter );
	ASSERT_HRES_IF_FAILED;

	// setup the converter to create 32bpp BGRA bitmaps
	hres = m_pWicConverter->Initialize( m_pBitmapFrame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom );
	ASSERT_HRES_IF_FAILED;

	// create the D2D1 bitmap
	hres = gph.renderTarget2d()->CreateBitmapFromWicBitmap( m_pWicConverter.Get(), nullptr, &m_pBitmap );
	ASSERT_HRES_IF_FAILED;
}

void Sprite::render( Graphics &gph,
	const D2D1_RECT_F& srcRect,
	const D2D1_RECT_F& destRect,
	const float alpha )
{
	gph.renderTarget2d()->DrawBitmap( m_pBitmap.Get(), destRect, alpha, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, 		srcRect );
}

float Sprite::getWidth() const noexcept
{
	return m_pBitmap->GetSize().width;
}

float Sprite::getHeight() const noexcept
{
	return m_pBitmap->GetSize().height;
}


SpriteSheet::SpriteSheet( const std::wstring& filename,
	Graphics &gph,
	const int spriteWidth,
	const int spriteHeight )
	:
	Sprite{filename, gph}
{
	m_spriteWidth = spriteWidth;
	m_spriteHeight = spriteHeight;
	m_nSpritesAcross = (int)getWidth() / spriteWidth;
}

void SpriteSheet::render( Graphics &gph,
	const int index,
	const int x,
	const int y,
	const float alpha )
{
	float srcRectX = (float)( index % m_nSpritesAcross ) * m_spriteWidth;
	float srcRectY = (float)( index / m_nSpritesAcross ) * m_spriteHeight;
	D2D1_RECT_F srcRect = D2D1::RectF( (float) srcRectX, (float) srcRectY, (float) srcRectX + m_spriteWidth, (float) srcRectY + m_spriteHeight );
	D2D1_RECT_F destRect = D2D1::RectF( (float) x, (float) y, (float) x + m_spriteWidth, (float) y + m_spriteHeight );

	Sprite::render( gph, srcRect, destRect, alpha );
}
#endif