#pragma once


namespace
{
using Byte = uint8_t;
}

struct ColorBGR final
{
	Byte b;
	Byte g;
	Byte r;
};

///=============================================================
/// \class	ColorBGRA
/// \author	KeyC0de
/// \date	2024/11/23 19:56
/// \brief For any DXGI format, the byte order is the order of the components in the format name.
/// \brief For example R8G8B8A8, R should be the first (lowest) byte and A should be the last (highest) byte.
/// \brief Endianness really doesn't have anything to do with it in this particular case, since the order is explicit.
/// \brief https://www.khronos.org/opengl/wiki/Direct3D_Compatibility#BGRA_vertex_format
/// \brief OpenGL has the reversed color components ie D3D RGBA = GL ABGR
/// \brief
/// \brief KeyEngine uses DXGI BGRA color order
///=============================================================
struct ColorBGRA final
{
	union
	{
		unsigned int m_dword;
		struct
		{
			Byte b;
			Byte g;
			Byte r;
			Byte a;
		} _;
	};

	constexpr ColorBGRA()
		:
		m_dword(0)
	{

	}

	constexpr ColorBGRA( unsigned int dw )
		:
		m_dword(dw)
	{

	}

	constexpr ColorBGRA( const Byte r,
		const Byte g,
		const Byte b,
		const Byte a )
		:
		m_dword(( a << 24u ) | ( r << 16u ) | ( g << 8u ) | b)
	{

	}

	constexpr ColorBGRA( const Byte r,
		const Byte g,
		const Byte b )
		:
		m_dword((255 << 24u) | ( r << 16u ) | ( g << 8u ) | b)
	{

	}

	//ColorBGRA( const Byte r,
	//	const Byte g,
	//	const Byte b )
	//	:
	//	_{b,g,r,255u}
	//{
	//
	//}

	constexpr ColorBGRA( const ColorBGRA &col )
		:
		m_dword{col.m_dword}
	{

	}

	constexpr ColorBGRA( const ColorBGRA col,
		const Byte a )
		:
		ColorBGRA(( a << 24u ) | col.m_dword )
	{

	}

	ColorBGRA& operator*=( const int scale )
	{
		this->m_dword *= scale;
		return *this;
	}

	ColorBGRA operator*( const int scale )
	{
		return *this *= scale;
	}

	ColorBGRA& operator=( ColorBGRA rhs )
	{
		m_dword = rhs.m_dword;
		return *this;
	}

	bool operator==( const ColorBGRA &rhs ) const noexcept
	{
		return m_dword == rhs.m_dword;
	}

	bool operator!=( const ColorBGRA &rhs ) const noexcept
	{
		return !( *this == rhs );
	}

	operator unsigned int() const noexcept
	{
		return m_dword;
	}

	constexpr Byte getRed() const noexcept
	{
		return ( m_dword >> 16u ) & 0xFFu;
	}

	constexpr Byte getGreen() const noexcept
	{
		return ( m_dword >> 8u ) & 0xFFu;
	}

	constexpr Byte getBlue() const noexcept
	{
		return m_dword & 0xFFu;
	}

	constexpr Byte getAlpha() const noexcept
	{
		return m_dword >> 24u;
	}

	void setRed( const Byte r )
	{
		m_dword = ( m_dword & 0xFF00FFFFu ) | ( r << 16u );
	}

	void setGreen( const Byte g )
	{
		m_dword = ( m_dword & 0xFFFF00FFu ) | ( g << 8u );
	}

	void setBlue( const Byte b )
	{
		m_dword = ( m_dword & 0xFFFFFF00u ) | b;
	}

	void setAlpha( const Byte a )
	{
		m_dword = ( m_dword & 0xFFFFFFu ) | ( a << 24u );
	}

	void set( const unsigned int value ) noexcept
	{
		m_dword = value;
	}
};

namespace col
{
// order reversions
static unsigned rgbaToAbgr( const unsigned col )
{
	/// \brief Memory (low address >towards> high address)
	/// \brief RGBA
	/// \brief ABGR
	unsigned r = col;
	unsigned g = col >> 8u;
	unsigned b = col >> 16u;
	unsigned a = col >> 24u;
	return ( r << 24u ) | ( g << 16u ) | ( b << 8u ) | a ;
}

static unsigned bgraToArgb( const ColorBGRA col )
{
	// Memory (low address >towards> high address)
	// BGRA
	// ARGB
	return 0;
}

// Conversions
static unsigned bgraToRgba( const ColorBGRA col )
{
	// best do the conversion of the color components in the shader directly
	// Memory (low address >towards> high address)
	// RGBA
	// BGRA
	unsigned r = col;
	unsigned g = col >> 8u;
	unsigned b = col >> 16u;
	unsigned a = col >> 24u;
	return ( a << 24u ) | ( r << 16u ) | ( g << 8u ) | b;
}

static ColorBGRA rgbaToBgra( const unsigned col )
{
	/// \brief best do the conversion of the color components in the shader directly
	/// \brief Memory (low address >towards> high address)
	/// \brief RGBA
	/// \brief BGRA
	unsigned r = col;
	unsigned g = col >> 8u;
	unsigned b = col >> 16u;
	unsigned a = col >> 24u;
	return ( a << 24u ) | ( r << 16u ) | ( g << 8u ) | b;
}

/*
// convenience functions
static ColorBGRA toRgba( const Byte r,
	const Byte g,
	const Byte b,
	const Byte a = 255u ) noexcept
{
	return ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r;
}

static ColorBGRA toRgb( const Byte r,
	const Byte g,
	const Byte b ) noexcept
{
	return toRgba( r,
		g,
		b,
		255u );
}
*/

static ColorBGRA toBgra( const Byte r,
	const Byte g,
	const Byte b,
	const Byte a ) noexcept
{
	return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

static ColorBGRA toBgr( const Byte r,
	const Byte g,
	const Byte b ) noexcept
{
	return toBgra( r, g, b, 255u );
}


static const ColorBGRA White = toBgra( 255u, 255u, 255u, 255u );
static const ColorBGRA Black = toBgra( 0u, 0u, 0u, 255u );
static const ColorBGRA Gray = toBgr( 0x80u, 0x80u, 0x80u );
static const ColorBGRA LightGray = toBgr( 0xD3u, 0xD3u, 0xD3u );
static const ColorBGRA Red = toBgr( 255u, 0u, 0u );
static const ColorBGRA Green = toBgr( 0u, 255u, 0u );
static const ColorBGRA DarkGreen = toBgr( 0u, 170u, 0u );
static const ColorBGRA Blue = toBgr( 0u, 0u, 255u );
static const ColorBGRA Yellow = toBgr( 255u, 255u, 0u );
static const ColorBGRA Cyan = toBgr( 0u, 255u, 255u );
static const ColorBGRA Magenta = toBgr( 255u, 0u, 255u );
static const ColorBGRA PinkHot = toBgr( 0xFFu, 0x69u, 0xB4u );
static const ColorBGRA Orange = toBgr( 0xFFu, 0x8Cu, 0x00u );
static const ColorBGRA Gold = toBgr( 255u, 215u, 0u );
static const ColorBGRA Silver = toBgr( 0xc0u, 0xc0u, 0xc0u );
static const ColorBGRA SaddleBrown = toBgr( 0x8bu, 0x45u, 0x13u );
static const ColorBGRA Sienna = toBgr( 0xa0u, 0x52u, 0x2du );
static const ColorBGRA YellowGreen = toBgr( 0x9au, 0xcdu, 0x32u );
static const ColorBGRA DeepPink = toBgr( 0xFFu, 0x14u, 0x93u );
static const ColorBGRA Ivory = toBgr( 0xFFu, 0xffu, 0xf0u );
static const ColorBGRA Beige = toBgr( 0xF5u, 0xf5u, 0xdcu );
static const ColorBGRA MistyRose = toBgr( 0xFFu, 0xe4u, 0xe1u );
static const ColorBGRA Olive = toBgr( 0x80u, 0x80u, 0x00u );
static const ColorBGRA Chocolate = toBgr( 0xd2u, 0x69u, 0x1eu );
static const ColorBGRA RosyBrown = toBgr( 0xbcu, 0x8fu, 0x8fu );
static const ColorBGRA Brown = toBgr( 0xa5u, 0x2au, 0x2au );
static const ColorBGRA Maroon = toBgr( 0x80u, 0x00u, 0x00u );
static const ColorBGRA MediumBlue = toBgr( 0x00u, 0x00u, 0xcdu );
static const ColorBGRA DarkBlue = toBgr( 0x00u, 0x00u, 0x8bu );
static const ColorBGRA IndianRed = toBgr( 0xcdu, 0x5cu, 0x5cu );
static const ColorBGRA Fuchsia = toBgr( 0xFFu, 0x00u, 0xffu );
static const ColorBGRA Pink = toBgr( 0xFFu, 0xc0u, 0xcbu );
static const ColorBGRA LightSkyBlue = toBgr( 0x87u, 0xceu, 0xfau );
static const ColorBGRA SkyBlue = toBgr( 0x87u, 0xceu, 0xebu );
static const ColorBGRA LavenderBlush = toBgr( 0xffu, 0xf0u, 0xf5u );


}// namespace col