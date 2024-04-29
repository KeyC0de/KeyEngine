#include "bmp_loader.h"
#include "utils.h"
#include "math_utils.h"
#include "key_random.h"
#include "perlin_noise.h"
#include "dynamic_vertex_buffer.h"
#include "assertions_console.h"


BmpLoader::BmpLoader( const std::string& filename )
	:
	m_filename{filename},
	m_file{filename, std::ios::out | std::ios::in | std::ios::binary}
{
	ASSERT( m_file, "File is missing!" );

	/// <image url="C:/Users/nikla/Pictures/bitmap file structure.png" scale="1" opacity="1" />
	// .bmp file Implementation details:
	// first 3 bytes are:
	//	42 4D sizeOfFile = B M 14 + 40 + bytesOfColorDataIncludingPaddingSinceEachRowOfColorDataNeedsToBeAMultipleOf4Bytes
	// stored in low endian format like so:
	//		RGB(A) per pixel: BB GG RR (AA) -> towards greater offsets/larger addresses
	//
	// padding required per row: [4 - (widthInPixels * 3 (or 4 if using Alpha) % 4)] % 4
	//		= [4 - (w*3 % 4)] % 4
	// padding required for all rows, ie the entire image: padding required per row * heighthInPixels
	//
	// example:
	//		the size of the row in bytes for a 14x12 pixel image is 14 * 3 = 42, which isn't a multiple of 14
	//		so we need 2 more bytes for it to become 44 which is a multiple of 4.
	//		Thus for each row of data we need an additional 2 Bytes of padding
	//		Total size = (14 + 40) + (14 * 12) * 3 + (12 * 2) = 582 B
	//					(12 * 2) = 24 B is the size of the padding required (there are heightInPixels rows)
	//
	// example 2: 3x3 bmp image size would be = (14 + 40) + (3 * 3) * 3 + (3 * 3) = 90 = 5A
	//				row size is 9 so we need 3 more B to be a multiple of 4
	//
	// after the first 3 bytes the next bytes up to and including the tenth byte (09) are filled with 0s
	// the next byte (0A) contains the offset to the starting address of the bitmap image data
	//		which for the 2nd example above should be: 54 = 36H
	// the next bytes up to the 14th are 0
	// starting with the next byte 0E we have 40 Bytes of the DIB header, which go like this:
	// 28 (hex for 40)
	// 00 00 00 (4 B for the header of the 'DIB header' just like the header of the entire bmp file)
	// next goes the width of the image in pixels occupying 4 Bytes
	// so always following the 2nd example above would be like this:
	// 03 00 00 00
	// next goes the height of the image:
	// 03 00 00 00
	// next two bytes is the size of the color planes which has to be 01 00
	// next two bytes are the bit depth which is 24bbp here = 18 00
	// next four bytes occupy the compression method
	// 00 00 00 00 (no compression method here)
	// next (offset 22Hex) goes the size of the image data = (3 * 3) * 3 + (3 * 3) = 36 B = 24 Hex, so:
	// 24 00 00 00 (occupying 4 bytes)
	// next goes the horizontal and the vertical resolution of the image (occupying 4 bytes each), so:
	// 03 00 00 00 03 00 00 00
	// next up goes the number of colors in the color pallet (set to 0 to get the default) - occupying 4 bytes
	// 00 00 00 00
	// last 4 bytes of the dib header are zeros
	// 00 00 00 00
	// now we start with the actual image content (starting at offset 36Hex), which contain 3Bpp
	// the pixels set start from the bottom left corner (first row is bottom row) and ending with the top right pixel
	// in a zig-zag pattern
	//
	// Also note that if the image height is negative the order is reversed (ie the bytes we start reading from belong to the top left pixel,
	// We could examine this by checking the height value of the file first and the code here doesn't account for this.
	//
	// bitmap data is read/written from bottom line to the top by default
	//
	// example completed: "my_first_image.bmp"

	ASSERT( sizeof BITMAPFILEHEADER == 14, "Invalid BITMAPFILEHEADER!" );
	m_file.read( reinterpret_cast<unsigned char*>( &m_bmpFileHeader ), sizeof m_bmpFileHeader );
	// m_bmpFileHeader.bfType		// the file type; must be "BM".
	// m_bmpFileHeader.bfSize		//	size of the bitmap file

	m_file.read( reinterpret_cast<unsigned char*>( &m_bmpInfoHeader ), sizeof m_bmpInfoHeader );
	ASSERT( sizeof BITMAPINFOHEADER == 40, "Invalid BITMAPFILEHEADER" );

	// m_bmpInfoHeader.biBitCount	// bits per pixel
	// biWidth = width of the bitmap, in pixels
	// biHeight = height of the bitmap, in pixels
	m_bitCount = m_bmpInfoHeader.biBitCount;
	ASSERT( m_bitCount == 24 || m_bitCount == 32, "Invalid bits/pixel!" );
	ASSERT( m_bmpInfoHeader.biCompression == BI_RGB, "No RGB compression!" );

	m_width = m_bmpInfoHeader.biWidth;
	m_height = m_bmpInfoHeader.biHeight;

	flush();
}

BmpLoader::BmpLoader( BmpLoader &&rhs ) noexcept
	:
	m_width{rhs.m_width},
	m_height{rhs.m_height},
	m_filename{rhs.m_filename},
	m_bitCount{rhs.m_bitCount},
	m_bmpFileHeader{rhs.m_bmpFileHeader},
	m_bmpInfoHeader{rhs.m_bmpInfoHeader},
	m_file{std::move( rhs.m_file ) }
{
	rhs.m_width = 0;
	rhs.m_height = 0;
}

BmpLoader& BmpLoader::operator=( BmpLoader &&rhs ) noexcept
{
	BmpLoader tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

int BmpLoader::getWidth() const noexcept
{
	return m_width;
}

int BmpLoader::getHeight() const noexcept
{
	return m_height;
}

unsigned short BmpLoader::getBitCount() const noexcept
{
	return m_bitCount;
}

void BmpLoader::flush() noexcept
{
	m_file.flush();
	m_file.close();
}

bool BmpLoader::applyPerlinNoise( const std::string &filename /*= ""*/ )
{
	if ( m_bitCount == 32 )
	{
		ASSERT( false, "BmpLoader not ready for 32 bit depth BGRA file loading!" );
		return false;
	}

	const size_t numImageElements = m_width * m_height;
	ImageData *img = new ImageData[numImageElements];

	auto perlin = [] ( const double x, const double y ) -> double
	{
		static PerlinNoise perlin;
		static KeyRandom r;
		static auto moreRandom = r.getRandomIntInRange( 64, 256 );
		static auto lessRandom = r.getRandomIntInRange( 8, 32 );

		// wood like structure
		double n = lessRandom * perlin.noise( x, y, moreRandom );
		return ( n - floor( n ) ) * 255.0;
	};

	std::function<double(double, double)> perlinNoiseFilter = std::bind( perlin, std::placeholders::_1, std::placeholders::_2 );

	//auto exponent = [] ( const double x, const double y ) -> double
	//{
		//const int power = 2;
		//return val * val;
	//};

	//std::function<double(double, double)> gaussianFilter = std::bind( util::gaussian2d<double>, std::placeholders::_1, std::placeholders::_2, 1.0 );

	std::vector<std::function<double(double, double)>> imageFilters;
	imageFilters.reserve( 1 );
	imageFilters.emplace_back( std::move( perlinNoiseFilter ) );
	//imageFilters.emplace_back( std::move( gaussianFilter ) );

	generateData( img, imageFilters, false, filename );
	writeData( img, filename );

	delete[] img;

	return true;
}

void BmpLoader::readData( _Inout_ ImageData *img,
	const std::string &filename /*= ""*/ )
{
	m_file.open( filename == "" ? m_filename : filename, std::ios::in | std::ios::binary );

	// test for reverse row order and control
	// y loop accordingly
	int yStart;
	int yEnd;
	int dy;
	if ( m_bmpInfoHeader.biHeight < 0 )
	{
		m_height = -m_bmpInfoHeader.biHeight;
		yStart = 0;
		yEnd = m_height;
		dy = 1;
	}
	else
	{
		m_height = m_bmpInfoHeader.biHeight;
		yStart = m_height - 1;
		yEnd = -1;
		dy = -1;
	}

	m_file.seekg( m_bmpFileHeader.bfOffBits );
	const int padding = ( 4 - ( m_width * 3 ) % 4 ) % 4;
	const bool b32Bits = m_bitCount == 32ui16;

	// visit every pixel of the image and assign a color generated with Perlin noise
	for ( int y = yStart; y != yEnd; y += dy )
	{
		for ( int x = 0; x < m_width; ++x )
		{
			const unsigned char blue = (unsigned char) m_file.get();
			const unsigned char green = (unsigned char) m_file.get();
			const unsigned char red = (unsigned char) m_file.get();

			auto &ar = img[y * m_width + x];

			if ( !b32Bits )
			{
				ar._24bit.b = blue;
				ar._24bit.g = green;
				ar._24bit.r = red;
			}
			else
			{
				const unsigned char alpha = (unsigned char) m_file.get();
				ar._32bit = ColorBGRA{red, green, blue, alpha};
			}

			if ( b32Bits )
			{
				m_file.seekg( 1, std::ios::cur );
			}
		}
		if ( !b32Bits )
		{
			m_file.seekg( padding, std::ios::cur );
		}
	}

	flush();
}

void BmpLoader::generateData( _Inout_ ImageData *img,
	std::vector<std::function<double(double, double)>> &imageFilters,
	const bool bTransform,
	const std::string &filename /*= ""*/ )
{
	if ( bTransform )
	{
		m_file.open( filename == "" ? m_filename : filename, std::ios::in | std::ios::binary );
	}
	else
	{
		// trick" open file for writing but don't write anything, just read from it - all reads (like `file.get()`) will read nada values
		m_file.open( filename == "" ? m_filename : filename, std::ios::out | std::ios::binary );
	}

	// test for reverse row order and control
	// y loop accordingly
	int yStart;
	int yEnd;
	int dy;
	if ( m_bmpInfoHeader.biHeight < 0 )
	{
		m_height = -m_bmpInfoHeader.biHeight;
		yStart = 0;
		yEnd = m_height;
		dy = 1;
	}
	else
	{
		m_height = m_bmpInfoHeader.biHeight;
		yStart = m_height - 1;
		yEnd = -1;
		dy = -1;
	}

	m_file.seekg( m_bmpFileHeader.bfOffBits );
	const int padding = ( 4 - ( m_width * 3 ) % 4 ) % 4;
	const bool b32Bits = m_bitCount == 32ui16;

	// visit every pixel of the image and assign a color generated with Perlin noise
	for ( int i = yStart; i != yEnd; i += dy )
	{
		for ( int j = 0; j < m_width; ++j )
		{
			double x = (double) j / ( (double) m_width );
			double y = (double) i / ( (double) m_height );

			// apply the image filters
			double val = 1;
			for ( const auto &filter : imageFilters )
			{
				val *= filter( x, y );
			}

			const unsigned char blue = util::mapToByte( m_file.get() * val );
			const unsigned char green = util::mapToByte( m_file.get() * val );
			const unsigned char red = util::mapToByte( m_file.get() * val );

			auto &ar = img[i * m_width + j];
			if ( !b32Bits )
			{
				ar._24bit.b = blue;
				ar._24bit.g = green;
				ar._24bit.r = red;
			}
			else
			{
				const unsigned char alpha = util::mapToByte( m_file.get() * val );
				ar._32bit = ColorBGRA{red, green, blue, alpha};
			}

			if ( b32Bits )
			{
				m_file.seekg( 1, std::ios::cur );
			}
		}
		if ( !b32Bits )
		{
			m_file.seekg( padding, std::ios::cur );
		}
	}

	flush();
}

void BmpLoader::transformData( _Inout_ ImageData *img,
	std::vector<std::function<double(double, double)>> &imageFilters,
	const std::string &filename /*= ""*/ )
{
	generateData( img, imageFilters, true, filename );
}

void BmpLoader::writeData( _Out_ ImageData *img,
	const std::string &filename /*= ""*/ )
{
	m_file.open( filename == "" ? m_filename : filename, std::ios::out | std::ios::binary );

	unsigned char fileHeader[14] =
	{
		'B','M',		// magic
		0,0,0,0,		// size in bytes
		0,0,			// app data
		0,0,			// app data
		40 + 14,0,0,0	// start of data offset
	};

	unsigned char infoHeader[40] =
	{
		40,0,0,0,		// infoHeader hd size
		0,0,0,0,		// width
		0,0,0,0,		// heigth
		1,0,			// number color planes
		24,0,			// bits per pixel
		0,0,0,0,		// compression is none
		0,0,0,0,		// image bits size
		0x13,0x0B,0,0,	// horz resoluition in pixel / m
		0x13,0x0B,0,0,	// vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
		0,0,0,0,		// colors in pallete
		0,0,0,0,		// important colors
	};

	int w = m_width;
	int h = m_height;

	// padding is for the case of of 24 bit depth only
	int paddingSize  = ( 4 - ( w * 3 ) % 4 ) % 4;
	int sizeData = w * h *3 + h * paddingSize;
	int sizeAll  = sizeData + sizeof( fileHeader ) + sizeof( infoHeader );

	fileHeader[2] = (unsigned char)( sizeAll );
	fileHeader[3] = (unsigned char)( sizeAll >> 8);
	fileHeader[4] = (unsigned char)( sizeAll >> 16);
	fileHeader[5] = (unsigned char)( sizeAll >> 24);

	infoHeader[4] = (unsigned char)( w );
	infoHeader[5] = (unsigned char)( w >> 8);
	infoHeader[6] = (unsigned char)( w >> 16);
	infoHeader[7] = (unsigned char)( w >> 24);

	infoHeader[8] = (unsigned char)( h );
	infoHeader[9] = (unsigned char)( h >> 8);
	infoHeader[10] = (unsigned char)( h >> 16);
	infoHeader[11] = (unsigned char)( h >> 24);

	infoHeader[20] = (unsigned char)( sizeData );
	infoHeader[21] = (unsigned char)( sizeData >> 8);
	infoHeader[22] = (unsigned char)( sizeData >> 16);
	infoHeader[23] = (unsigned char)( sizeData >> 24);

	m_file.write( fileHeader, sizeof( fileHeader ) );
	m_file.write( infoHeader, sizeof( infoHeader ) );

	const bool b32Bits = m_bitCount == 32ui16;

	unsigned char padding[3] = { 0, 0, 0 };

	for ( int y = 0; y < h; ++y )
	{
		for ( int x = 0; x < w; ++x )
		{
			if ( !b32Bits )
			{
				const auto element = img[y * m_width + x]._24bit;
				const unsigned char texel[3] =
				{
					element.b,
					element.g,
					element.r
				};

				m_file.write( texel, 3 );
			}
			else
			{
				const auto element = img[y * m_width + x]._32bit;
				const unsigned char texel[4] =
				{
					element.getBlue(),
					element.getRed(),
					element.getRed(),
					element.getAlpha()
				};

				m_file.write( texel, 4 );
			}
		}

		if ( !b32Bits )
		{
			m_file.write( padding, paddingSize );
		}
	}

	flush();
}

void BmpLoader::normalizeHeightmap( _Inout_ ImageData *img,
	const double value ) noexcept
{
	const bool b32Bits = m_bitCount == 32ui16;

	for ( int y = 0; y < m_height; ++y )
	{
		for ( int x = 0; x < m_width; ++x )
		{
			if ( !b32Bits )
			{
				auto &element = img[y * m_width + x]._24bit;
				element.b /= (unsigned char) value;
				element.g /= (unsigned char) value;
				element.r /= (unsigned char) value;
			}
			else
			{
				auto &element = img[y * m_width + x]._32bit;
				element.set( static_cast<unsigned>( element ) / static_cast<unsigned>( value ) );
			}
		}
	}
}