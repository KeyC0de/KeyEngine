#pragma once

#include "winner.h"
#include <string>
#include <DirectxMath.h>
#include <dxtex/DirectXTex.h>
#include "key_exception.h"
#include "color.h"
#include "non_copyable.h"


//=============================================================
//	\class	Bitmap
//	\author	KeyC0de
//	\date	2021/10/04 12:56
//	\brief	uses DirectX Tex to load images into
//			rule-of-0
//=============================================================
class Bitmap final
	: public NonCopyable
{
	static constexpr DXGI_FORMAT s_format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	DirectX::ScratchImage m_scratchImg;

	class BitmapException final
		: public KeyException
	{
	public:
		BitmapException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};

	Bitmap( DirectX::ScratchImage rhs ) noexcept;
public:
	using Texel = ColorBGRA;

	static Bitmap loadFromFile( const std::string &filename, unsigned wicFlags = DirectX::WIC_FLAGS_IGNORE_SRGB );
	//	\function	colorToVector	||	\date	2022/02/18 17:49
	//	\brief	convert from Color(0..255) to vector(-1..1)
	static const DirectX::XMVECTOR colorToVector( const Bitmap::Texel col ) noexcept;
	static Bitmap::Texel vectorToColor( DirectX::XMVECTOR v ) noexcept;
public:
	Bitmap( const unsigned int width, const unsigned int height );

	size_t getSize() const noexcept;
	void clear( const Texel fillValue ) noexcept;
	void setTexel( const unsigned int x, const unsigned int y, const Texel col ) cond_noex;
	const Texel getTexel( const unsigned int x, const unsigned int y ) const cond_noex;
	unsigned int getWidth() const noexcept;
	unsigned int getHeight() const noexcept;
	//	\function	getPitch	||	\date	2022/02/19 19:27
	//	\brief	=== getWidth() * sizeof Texel
	unsigned int getPitch() const noexcept;
	Texel* data() noexcept;
	const Texel* getData() const noexcept;
	void save( const std::string &filename, const unsigned wicFlags = DirectX::WIC_FLAGS_NONE ) const;
	bool hasAlpha() const noexcept;
};


#define THROW_BITMAP_EXCEPTION( msg ) __debugbreak();\
	throw BitmapException( __LINE__, __FILE__, __FUNCTION__, msg );