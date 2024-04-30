#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include "assertions_console.h"
#include "utils.h"


namespace util
{

constexpr float PI = 3.14159265f;
constexpr double PI_D = 3.1415926535897932;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T abs( const T val )
{
	return ( val >= 0 ) ?
		val :
		-val;
}

//	\function	truncate	||	\date	2023/05/09 18:37
//	\brief	truncate the supplied number val; handles negative numbers as well
template<typename T, typename>
constexpr int truncate( const T val )
{
	return static_cast<int>( util::abs( val ) );
}

//	\function	modulus	||	\date	2023/05/09 19:16
//	\brief	'fmod()' === '%' : returns the signed floating point remainder of a division (divident / divisor)
//	#WARNING: Innacurate, just use std::fmod
//template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
//constexpr T modulusFloat( const T divident,
//	const T divisor )
//{
//	return divident - util::truncate( divident / divisor ) * divisor;
//}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T modulus( const T divident,
	const T divisor ) noexcept
{
	if constexpr ( std::is_floating_point_v<T> )
	{
		return std::fmod( divident, divisor );
	}
	else
	{
		return divident % divisor;
	}
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr int ceil( const T val ) noexcept
{
	return val + (T)1 - util::modulus( val, (T)1 );
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr int floor( const T val ) noexcept
{
	return val - util::modulus( val, (T)1 );
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr int round( const T val ) noexcept
{
	return util::floor( val + (T)0.5 );
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T clamp( T number,
	T minimum,
	T maximum )
{
	if ( number < minimum )
	{
		return minimum;
	}
	else if ( number > maximum )
	{
		return maximum;
	}
	return number;
}

//	\function	numberToCoord	||	\date	2023/05/09 12:33
//	\brief	transformation of a scalar (1D) to a Cartesian coordinate
//		use this instead of mapRange when you don't want to scale the values of the range
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
int numberToCoord( T number,
	int amountPerGradation,
	int nGradations,
	bool is_x_axis )
{
	int amountLimit = amountPerGradation * nGradations / 2;
	int integer = static_cast<int>( number );
	integer = util::clamp( integer, -amountLimit, amountLimit );

	// we floor because we want to return a number in the form of an integer index
	int coord = util::floor( integer / amountPerGradation );
	coord = coord + ( nGradations / 2 ) + 1;

	// flip
	if ( !is_x_axis )
	{
		coord = nGradations + 1 - coord;
	}
	return coord;
}

template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T mapRange( const T val,
	const T inFirst,
	const T inLast,
	const T outFirst,
	const T outLast ) noexcept
{
	double slope = 1.0 * ( outLast - outFirst ) / ( inLast - inFirst );
	return outFirst + util::round( slope * ( val - inFirst ) );
}

template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr bool approximatelyEqual( const T x,
	const T y )
{
	return std::fabs( x - y ) <= std::numeric_limits<T>::epsilon();
}

// fast multiplication by 10
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T mult10( const T num )
{
	return ( num << 3 ) + ( num << 1 );
}

// fast division by 10
//https://stackoverflow.com/a/5558614/4743275
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T div10( const T dividend )
{
	long long int invDivisor = 0x1999999A;
	return static_cast<T>( ( invDivisor * dividend ) >> 32 );
}

//===================================================
//	\function	squareRoot	||	\date	2022/08/28 23:46
//	\brief	calculates the square root of a number
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T squareRoot( const T x ) noexcept
{
	const float epsilon = .00001;
	T guess = 1.0;
	int i = 0;

	if ( x < 0 )
	{
		std::cout << "Negative Argument. Error!\n";
		return -(T)1.0;
	}

	while ( util::abs( ( guess * guess ) / x - 1 ) >= epsilon )
	{
		guess = ( x / guess + guess ) / 2.0f;
		std::cout << "rep = "
			<< i
			<< ", guess = "
			<< guess
			<< "\n";
		++i;
	}

	return guess;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr void quadratic( const T a,
	const T b,
	const T c )
{
	T discriminant = b * b - 4 * a * c;	// diakrinousa

	if ( discriminant < 0 )
	{
		std::cout << "Imaginary roots.\n";
	}
	else if ( discriminant == 0 )
	{
		std::cout << "There is only one root: "
			<< -b / (2.0 * a)
			<< "\n";
	}
	else
	{
		std::cout << "1st root: "
					<< ( -b + util::squareRoot( discriminant ) ) / ( 2 * a )
					<< "\t 2nd root: "
					<< ( -b - util::squareRoot( discriminant ) ) / ( 2 * a )
					<< "\n";
	}
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T square( const T x )
{
	return x * x;
}

template<typename T>
std::string toHexString( const T i )
{
	std::ostringstream oss;
	oss << "0x"
		<< std::setfill('0')
		<< std::setw( sizeof( T ) * 2 )
		<< std::hex
		<< i;
	return oss.str();
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T normalizeTo01( const T val,
	const T min,
	const T max )
{
	return ( val - min ) / ( max - min );
}

// convert a range of numbers to any other range of numbers maintaining ratio
// protecting against the the case where the old range is 0 ( oldMin = oldMax )
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T convertRange( const T valueInRange,
	const T oldMin,
	const T oldMax,
	const T newMin,
	const T newMax )
{
	T oldRange = oldMax - oldMin;
	T newValueInRange;
	if ( oldRange == 0 )
	{
		newValueInRange = newMin;
	}
	else
	{
		T newRange = newMax - newMin;
		newValueInRange = ( ( ( valueInRange - oldMin ) * newRange ) / oldRange ) + newMin;
	}
	return newValueInRange;
}

//	\function	wrapAngle	||	/date	2020/07/15 5:10
//	\brief wrap angle (in radians) in the range -pi to pi
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T wrapAngle( const T angleRad )
{
	const T val = std::fmod( angleRad, (T)2.0 * (T)PI_D );
	return ( val > (T)PI_D ) ?
		( val - (T)2.0 * (T)PI_D ) :		// if > 2pi subtract 2pi from it
		val;
}

//	\function	toRadians	||	/date	2020/07/15 5:13
//	\brief convert angle in degrees to radians
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T toRadians( const T deg )
{
	return ( deg * PI ) / (T)180.0;
}

//	\function	interpolate	||	/date	2020/08/02 17:56
//	\brief linear interpolation from src to dst value using alpha provided, interpolates any two Types that support the operations +, -, *
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T interpolate( const T &src,
	const T &dst,
	const double alpha )
{
	return src + alpha * ( dst - src );
}
ALIAS_FUNCTION( interpolate, lerp );


/// <image url="C:/Users/nikla/pictures/gaussian function distribution.png" scale=".4" />
//	\function	gaussian1d	||	\date	2022/11/20 12:40
//	\brief	1d Gaussian distribution
//			x is distance from center of Kernel/curve
//			sigma is the standard deviation of the distribution
template<typename T>
constexpr T gaussian1d( const T x,
	const T sigma,
	const T mean = (T)0 ) noexcept
{
	const T ss = util::square( sigma );
	return ( (T)1 / sqrt( (T)2 * (T)PI_D * ss ) ) * exp( -util::square( x - mean ) / ((T)2 * ss) );
}

//	\function	gaussian2d	||	\date	2022/11/20 12:41
//	\brief	Isotropic (mean = 0) 2d Gaussian distribution
template<typename T>
constexpr T gaussian2d( const T x,
	const T y,
	const T sigma ) noexcept
{
	const T ss = util::square( sigma );
	return ( (T)1 / sqrt( (T)2 * (T)PI_D * ss ) ) * exp( -( ( util::square( x ) + util::square( y ) ) / ( (T)2 * util::square( ss ) ) ) );
}

//	\function	gaussian2d	||	\date	2022/11/20 12:41
//	\brief	Isotropic (mean = 0) 2d Gaussian distribution
//			as an image filter it can be used to smoothen/blur
// #TODO:
template<typename T>
constexpr T gaussianFilter( /*Image* img,*/
	const T x,
	const T y,
	const int kernelSize = 5 ) noexcept
{
	const T sigma = util::ceil( ( kernelSize - (T)1 ) / (T)6 );
	const T ss = util::square( sigma );
	for ( int i = -kernelSize; i < kernelSize; ++i )
	{
		for ( int j = -kernelSize; j < kernelSize; ++j )
		{
			( (T)1 / sqrt( (T)2 * (T)PI_D * ss ) ) * exp( -( ( util::square( x ) + util::square( y ) ) / ( (T)2 * util::square( ss ) ) ) );
		}
	}
}

//	\function	powerOf	||	/date	2021/10/25 19:40
//	\brief	calculate x^n, n > 1
template<typename T, typename J, typename = std::enable_if_t<std::is_arithmetic_v<T>>, typename = std::enable_if_t<std::is_arithmetic_v<J>>>
constexpr T powerOf( T x,
	J n )
{
	ASSERT( n >= 0, "powerOf doesn't handle negatives powers!" );
	if ( n == 0 )
	{
		return 1;
	}
	T xo = x;
	while ( --n > 0 )
	{
		x *= xo;
	}
	return x;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T randomInRange( const T start,
	const T end )
{
	return ( rand() * ( end - start + 1 ) ) + start;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr int signum( const T val )
{
	return val > (T) 0 ?
		1 :
		-1;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T nthMultipleOf( const T number,
	const int n )
{
	T multiple;
	for ( int i = 2; i <= n; ++i )
	{
		multiple = i * number;
	}
	return multiple;
}

//	\function	nextPowerOf2	||	\date	2022/04/06 12:52
//	\brief	find smallest power of 2 greater than or equal to n
constexpr int nextPowerOf2( int n );
//	\function	gcd	||	\date	2022/08/29 0:04
//	\brief	prints Greatest Common Divisor of numbers
//			Complexity: O(log_n(m))
int gcd( int divident, int divisor );
constexpr int factorialOf( int n ) noexcept;
//===================================================
//	\function	isPrime	||	\date	2022/08/28 23:45
//	\brief	trial division method
constexpr bool isPrime( const int number ) noexcept;

constexpr bool isPowerOfTwo( const std::size_t value ) noexcept;
float cosine( float x ) noexcept;
float sine( float x ) noexcept;

constexpr int linearSearch( const int *A, const int n, const int x );
constexpr int binarySearch( const int *A, int low, int high, const int x );

int toDecimal( const int hex );
int toHex( const int dec );

/// bitwise ops
//	\function	printDec2Bin	||	\date	2022/08/29 0:03
//	\brief	prints given datatype to binary
void printDec2Bin( const size_t size, void const *const ptr ) noexcept;
//===================================================
//	\function	printDecToBin	||	\date	2022/08/29 0:04
//	\brief	print binary conversion of supplied decimal number
void printDecToBin( const int num ) noexcept;
constexpr bool haveOppositeSigns( int x, int y ) noexcept;
//===================================================
//	\function	msbIndexOfDec	||	\date	2022/08/29 0:04
//	\brief	finds MSB index in decimal number (counting from 0)
constexpr int msbIndexOfDec( int num ) noexcept;
//===================================================
//	\function	countSetBits	||	\date	2022/08/29 0:02
//	\brief	count number of Ones in a given number
constexpr int countSetBits( int num ) noexcept;
//===================================================
//	\function	isPowerOf2	||	\date	2022/08/29 0:02
//	\brief	checks whether a number is a power of 2
constexpr int isPowerOf2( const int num ) noexcept;
constexpr void convertToBase( int number, const int base ) noexcept;
//===================================================
//	\function	swapBits	||	\date	2022/08/29 0:02
//	\brief	swap the bits at position i and j of given number iff they are different
constexpr int swapBits( int num, const int i, const int j ) noexcept;
constexpr int setNthBit( int num, const unsigned n ) noexcept;
constexpr int clearNthBit( int num, const unsigned n ) noexcept;
constexpr int toggleNthBit( int num, const unsigned n ) noexcept;
//===================================================
//	\function	isSet	||	\date	2022/08/29 0:02
//	\brief	check num's bitPos state and return the value (0 or 1)
constexpr int isSet( int num, const unsigned bitPos ) noexcept;
//	\function	isNumberSet	||	\date	2022/10/29 13:44
//	\brief	check if num has bit indicated by bitFlag set. bitFlag must be a power of 2.
bool isSetByNumber( int num, int bitFlag ) noexcept;
//===================================================
//	\function	changeNthBit	||	\date	2022/08/29 0:02
//	\brief	change nth bit to bitVal
constexpr int changeNthBit( int num, const unsigned n, const bool bitVal ) noexcept;

template<int b>
class ByteSet
{
	enum
	{
		bit7 = ( b & 0x80 ) ? 1 : 0,
		bit6 = ( b & 0x40 ) ? 1 : 0,
		bit5 = ( b & 0x20 ) ? 1 : 0,
		bit4 = ( b & 0x10 ) ? 1 : 0,
		bit3 = ( b & 0x08 ) ? 1 : 0,
		bit2 = ( b & 0x04 ) ? 1 : 0,
		bit1 = ( b & 0x02 ) ? 1 : 0,
		bit0 = ( b & 0x01 ) ? 1 : 0
	};
public:
	enum
	{
		nbits = bit0 + bit1 + bit2 + bit3 + bit4 + bit5 + bit6 + bit7
	};
};


}// namespace util