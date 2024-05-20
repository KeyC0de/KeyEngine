#pragma once

#include <string>
#include <fstream>
#include <functional>
#include <vector>
#include "winner.h"
#include "color.h"


namespace ver
{

class VBuffer;

}

//=============================================================
//	\class	BmpLoader
//	\author	KeyC0de
//	\date	2022/11/19 14:15
//	\brief	BMPs BGRA color order if bit count is 32bit, or BGR if 24bit
//			Windows only, as Linux doesn't provide facets/locales for basic_fstream of "unsigned char"
//=============================================================
class BmpLoader final
{
public:
	union ImageData
	{
		ColorBGR _24bit;
		ColorBGRA _32bit;

		// default ctor
		ImageData()
			:
			_24bit()
		{

		}

		ImageData( int )
			:
			_32bit()
		{

		}

		ImageData( unsigned char r, unsigned char g, unsigned char b )
		{
			_24bit.b = b;
			_24bit.g = g;
			_24bit.r = r;
		}

		ImageData( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
			:
			_32bit{r, g, b, a}
		{

		}

		ColorBGRA toBGRA()
		{
			unsigned v( ( 255 << 24u ) | ( _24bit.r << 16u ) | ( _24bit.g << 8u ) | _24bit.b );
			return v;
		}
	};
private:
	int m_width = 0;
	int m_height = 0;
	std::string m_filename;
	unsigned short m_bitCount;
	BITMAPFILEHEADER m_bmpFileHeader;
	BITMAPINFOHEADER m_bmpInfoHeader;
	using ufstream = std::basic_fstream<unsigned char, std::char_traits<unsigned char>>;
	ufstream m_file;
public:
	BmpLoader() = default;
	//	\function	ctor	||	\date	2022/11/19 14:15
	//	\brief	store sprite (.bmp format) from file into data structure
	BmpLoader( const std::string& filename );
	BmpLoader( const BmpLoader& rhs ) = delete;
	BmpLoader& operator=( const BmpLoader& rhs ) = delete;
	BmpLoader( BmpLoader&& rhs ) noexcept;
	BmpLoader& operator=( BmpLoader&& rhs ) noexcept;

	int getWidth() const noexcept;
	int getHeight() const noexcept;
	unsigned short getBitCount() const noexcept;
	//	\function	flush	||	\date	2022/11/19 15:59
	//	\brief	writing after reading requires flushing of the file stream
	void flush() noexcept;
	bool applyPerlinNoise( const std::string &filename = "" );
	void readData( _Inout_ ImageData *img, const std::string &filename = "" );
	//	\function	normalizeHeightmap	||	\date	2022/11/20 17:02
	//	\brief	goes through the image data and divides each height value so that the terrain doesn't look too spikey
	void normalizeHeightmap( _Inout_ ImageData *img, const double value ) noexcept;
private:
	//	\function	generateData	||	\date	2022/11/20 13:32
	//	\brief	generates data from a combination of filters and outputs it in `img`, essentially creates an image anew
	void generateData( _Inout_ ImageData *img, std::vector<std::function<double(double, double)>> &imageFilters, const bool bTransform = false, const std::string &filename = "" );
	//	\function	transformData	||	\date	2022/11/20 13:33
	//	\brief	transforms the data of file and outputs it in `img`, transformData = readData + generateData
	void transformData( _Inout_ ImageData *img, std::vector<std::function<double(double, double)>> &imageFilters, const std::string &filename = "" );
	void writeData( ImageData *img, const std::string &filename = "" );
};