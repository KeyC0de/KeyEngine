#include <sstream>
#include "bitmap.h"
#include "utils.h"
#include "file_utils.h"
#include "assertions_console.h"


namespace dx = DirectX;

Bitmap::Bitmap( const unsigned int width,
	const unsigned int height )
{
	HRESULT hres = m_scratchImg.Initialize2D( s_format, width, height, 1u, 1u );
	if ( FAILED( hres ) )
	{
		THROW_BITMAP_EXCEPTION( "Failed to construct ScratchImage!" );
	}
}

Bitmap::Bitmap( dx::ScratchImage rhs ) noexcept
	:
	m_scratchImg{std::move( rhs )}
{

}

size_t Bitmap::getSize() const noexcept
{
	return m_scratchImg.GetPixelsSize();
}

void Bitmap::clear( const Texel fillValue ) noexcept
{
	const auto width = getWidth();
	const auto height = getHeight();
	auto &imgData = *m_scratchImg.GetImage( 0, 0, 0 );
	for ( size_t y = 0u; y < height; ++y )
	{
		auto rowStart = reinterpret_cast<Texel*>( imgData.pixels + imgData.rowPitch * y );
		std::fill( rowStart, rowStart + imgData.width, fillValue );
	}
}

void Bitmap::setTexel( const unsigned int x,
	const unsigned int y,
	const Texel col ) cond_noex
{
	ASSERT( x >= 0, "x less than 0!" );
	ASSERT( y >= 0, "y less than 0!" );
	ASSERT( x < getWidth(), "x greater than width!" );
	ASSERT( y < getHeight(), "y greater than height!" );
	auto &imgData = *m_scratchImg.GetImage( 0, 0, 0 );
	reinterpret_cast<Texel*>( &imgData.pixels[y * imgData.rowPitch] )[x] = col;
}

const Bitmap::Texel Bitmap::getTexel( const unsigned int x,
	const unsigned int y ) const cond_noex
{
	ASSERT( x >= 0, "x less than 0!" );
	ASSERT( y >= 0, "y less than 0!" );
	ASSERT( x < getWidth(), "x greater than width!" );
	ASSERT( y < getHeight(), "y greater than height!" );
	auto &imgData = *m_scratchImg.GetImage( 0, 0, 0 );
	return reinterpret_cast<Texel*>( &imgData.pixels[y * imgData.rowPitch] )[x];
}

unsigned int Bitmap::getWidth() const noexcept
{
	return static_cast<unsigned>( m_scratchImg.GetMetadata().width );
}

unsigned int Bitmap::getHeight() const noexcept
{
	return static_cast<unsigned>( m_scratchImg.GetMetadata().height );
}

unsigned int Bitmap::getPitch() const noexcept
{
	return static_cast<unsigned>( m_scratchImg.GetImage( 0u, 0u, 0u )->rowPitch );
}

Bitmap::Texel* Bitmap::data() noexcept
{
	return reinterpret_cast<Texel*>( m_scratchImg.GetPixels() );
}

const Bitmap::Texel* Bitmap::getData() const noexcept
{
	return const_cast<Bitmap*>( this )->data();
}

Bitmap Bitmap::loadFromFile( const std::string &name,
	unsigned wicFlags )
{
	dx::ScratchImage scratchImg;
	HRESULT hres = dx::LoadFromWICFile( util::s2ws( name ).c_str(), static_cast<dx::WIC_FLAGS>( wicFlags ), nullptr, scratchImg );
	if ( FAILED( hres ) )
	{
		THROW_BITMAP_EXCEPTION( "Failed to load image from file" );
	}

	// get first image
	if ( scratchImg.GetImage( 0, 0, 0 )->format != s_format )
	{
		dx::ScratchImage convertedImg;
		hres = dx::Convert( *scratchImg.GetImage( 0, 0, 0 ), s_format, dx::TEX_FILTER_DEFAULT, dx::TEX_THRESHOLD_DEFAULT, convertedImg );
		if ( FAILED( hres ) )
		{
			THROW_BITMAP_EXCEPTION( "Failed to convert image" );
		}

		return Bitmap{std::move( convertedImg )};
	}

	return Bitmap{std::move( scratchImg )};
}

const dx::XMVECTOR Bitmap::colorToVector( const Bitmap::Texel col ) noexcept
{
	auto v = dx::XMVectorSet( (float)col.getRed(), (float)col.getGreen(), (float)col.getBlue(), 0.0f );
	const auto all255 = dx::XMVectorReplicate( 2.0f / 255.0f );
	v = dx::XMVectorMultiply( v, all255 );
	const auto all1 = dx::XMVectorReplicate( 1.0f );
	v = dx::XMVectorSubtract( v, all1 );
	return v;
}

Bitmap::Texel Bitmap::vectorToColor( dx::XMVECTOR v ) noexcept
{
	const auto all1 = dx::XMVectorReplicate( 1.0f );
	dx::XMVECTOR nOut = dx::XMVectorAdd( v, all1 );
	const auto all255 = dx::XMVectorReplicate( 255.0f / 2.0f );
	nOut = dx::XMVectorMultiply( nOut, all255 );
	dx::XMFLOAT3 floats{};
	dx::XMStoreFloat3( &floats, nOut );
	return {(unsigned char)round( floats.x ), (unsigned char)round( floats.y ), (unsigned char)round( floats.z )};
}

void Bitmap::save( const std::string &filename,
	const unsigned wicFlags ) const
{
	const auto getCodec = []( const std::string &filename )
	{
		const auto ext = util::getFileExtension( filename );
		if ( ext == ".png" )
		{
			return dx::WIC_CODEC_PNG;
		}
		else if ( ext == ".jpg" )
		{
			return dx::WIC_CODEC_JPEG;
		}
		else if ( ext == ".bmp" )
		{
			return dx::WIC_CODEC_BMP;
		}
		THROW_BITMAP_EXCEPTION( "Image format is not supported" );
	};

	HRESULT hres = dx::SaveToWICFile( *m_scratchImg.GetImage( 0, 0, 0 ), static_cast<dx::WIC_FLAGS>( wicFlags ), GetWICCodec( getCodec( filename ) ), util::s2ws( filename ).c_str() );
	if ( FAILED( hres ) )
	{
		THROW_BITMAP_EXCEPTION( "Failed to save image to file" );
	}
}

bool Bitmap::hasAlpha() const noexcept
{
	return !m_scratchImg.IsAlphaAllOpaque();
}

Bitmap::BitmapException::BitmapException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException(line, file, function, msg)
{

}

std::string Bitmap::BitmapException::getType() const noexcept
{
	return typeid( this ).name();
}

const char* Bitmap::BitmapException::what() const noexcept
{
	return KeyException::what();
}