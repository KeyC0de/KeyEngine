#pragma once

#ifdef D2D_INTEROP
#include "graphics.h"
#include <wincodec.h>
#include "non_copyable.h"

// https://docs.microsoft.com/en-us/windows/win32/wic/jpeg-ycbcr-support
class Sprite
{
	Microsoft::WRL::ComPtr<IWICImagingFactory> m_pWicFactory;
	Microsoft::WRL::ComPtr<IWICBitmapDecoder> m_pWicDecoder;
	Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> m_pBitmapFrame;
	Microsoft::WRL::ComPtr<IWICFormatConverter> m_pWicConverter;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_pBitmap;
public:
	Sprite( const std::wstring &filename, Graphics &gph );
	virtual ~Sprite() noexcept = default;

	//	\function	render	||	\date	2022/09/11 20:33
	//	\brief  draw the Sprite bitmap to the render target
	void render( Graphics &gph, const D2D1_RECT_F &srcRect, const D2D1_RECT_F &destRect, const float alpha = 1.0f );

	const float getWidth() const noexcept;
	const float getHeight() const noexcept;
};

class SpriteSheet
	: public Sprite
{
	int m_spriteWidth;
	int m_spriteHeight;
	int m_nSpritesAcross;
public:
	SpriteSheet( const std::wstring& filename, Graphics &gph, const int spriteWidth, const int spriteHeight );

	void render( Graphics &gph, const int index, const int x, const int y, const float alpha = 1.0f );
};
#endif