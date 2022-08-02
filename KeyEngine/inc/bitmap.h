#pragma once

#include "winner.h"
#include <string>
#include <optional>
#include <DirectxMath.h>
#include <dxtex/DirectXTex.h>
#include "key_exception.h"
#include "color.h"


//=============================================================
//	\class
//
//	\author	KeyC0de
//	\date	2021/10/04 12:56
//
//	\brief	uses DirectX Tex to load images into
//=============================================================
class Bitmap final
{
	static constexpr DXGI_FORMAT m_format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	DirectX::ScratchImage m_scratchImg;
public:
	using Texel = ColorBGRA;
private:
	class BitmapException final
		: public KeyException
	{
	public:
		BitmapException( int line, const char *file, const char *function,
			const std::string &msg ) noexcept;

		const std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};
private:
	Bitmap( DirectX::ScratchImage rhs ) noexcept;
public:
	static Bitmap loadFromFile( const std::string &filename,
		unsigned wicFlags = DirectX::WIC_FLAGS_IGNORE_SRGB );
	//===================================================
	//	\function	colorToVector
	//	\brief  convert from Color(0..255) to vector(-1..1)
	//	\date	2022/02/18 17:49
	static DirectX::XMVECTOR colorToVector( Bitmap::Texel col ) noexcept;
	static Bitmap::Texel vectorToColor( const DirectX::XMVECTOR &v ) noexcept;
public:
	Bitmap( unsigned int width, unsigned int height );
	~Bitmap() = default;
	Bitmap( Bitmap &rhs ) = delete;
	Bitmap& operator=( const Bitmap &rhs ) = delete;
	Bitmap( Bitmap &&rhs ) noexcept = default;
	Bitmap& operator=( Bitmap &&rhs ) noexcept = default;

	void clear( Texel fillValue ) noexcept;
	void setTexel( unsigned int x, unsigned int y, Texel col ) cond_noex;
	Texel getTexel( unsigned int x, unsigned int y ) const cond_noex;
	unsigned int getWidth() const noexcept;
	unsigned int getHeight() const noexcept;
	//===================================================
	//	\function	getPitch
	//	\brief  === getWidth() * sizeof Texel
	//	\date	2022/02/19 19:27
	unsigned int getPitch() const noexcept;
	Texel* data() noexcept;
	const Texel* dataConst() const noexcept;
	void save( const std::string &filename, unsigned wicFlags = DirectX::WIC_FLAGS_NONE ) const;
	bool hasAlpha() const noexcept;
};


#define throwBitmapException( msg ) throw BitmapException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg )