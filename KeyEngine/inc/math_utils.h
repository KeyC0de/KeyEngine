#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include "assertions_console.h"


namespace util
{

constexpr float PI = 3.14159265f;
constexpr double PI_D = 3.1415926535897932;

//===================================================
//	\function	nextPowerOf2
//	\brief  find smallest power of 2 greater than or equal to n
//	\date	2022/04/06 12:52
int nextPowerOf2( int n );

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T abs( const T val )
{
	return ( val >= 0 ) ?
		val :
		-val;
}

template<typename T>
constexpr int truncate( const T val )
{
	return static_cast<int>( val );
}

template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T modulusFloat( const T divident,
	const T divisor )
{
	return divident - truncate( divident / divisor ) * divisor;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T modulus( const T divident,
	const T divisor ) noexcept
{
	if constexpr ( std::is_floating_point_v<T> )
	{
		return modulusFloat( divident,
			abs( divisor ) );
	}
	else
	{
		return divident % divisor;
	}
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T ceil( const T val ) noexcept
{
	return val + (T)1 - modulus( val,
		(T)1 );
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T floor( const T val ) noexcept
{
	return val - modulus( val,
		(T)1 );
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T round( const T val ) noexcept
{
	return floor( val + (T)0.5 );
}

template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T mapRange( const T val,
	const T inFirst,
	const T inLast,
	const T outFirst,
	const T outLast ) noexcept
{
	double slope = 1.0 * ( outLast - outFirst ) / ( inLast - inFirst );
	return outFirst + round( slope * ( val - inFirst ) );
}

template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool approximatelyEqual( T x,
	T y )
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

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr void quadratic( const T a,
	const T b,
	const T c )
{
	T diakr = b * b - 4 * a * c;

	if ( diakr < 0 )
	{
		std::cout << "Imaginary roots.\n";
	}
	else if ( diakr == 0 )
	{
		std::cout << "There is only one root: "
			<< -b / (2.0 * a)
			<< "\n";
	}
	else
	{
		std::cout << "1st root: "
					<< ( -b + squareRoot( diakr ) ) / ( 2 * a )
					<< "\t 2nd root: "
					<< ( -b - squareRoot( diakr ) ) / ( 2 * a )
					<< "\n";
	}
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T square( const T x )
{
	return x * x;
}

template<typename T>
std::string toHexString( T i )
{
	std::ostringstream oss;
	oss << "0x"
		<< std::setfill ('0')
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
T convertRange( T valueInRange,
	T oldMin,
	T oldMax,
	T newMin,
	T newMax )
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

//===================================================
//	\function	wrapAngle
//	\brief wrap angle (in radians) in the range -pi to pi
//	\date 2020/07/15 5:10
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T wrapAngle( const T angle )
{
	const T val = fmod( angle, (T)2.0 * (T)PI_D );
	return ( val > (T)PI_D ) ?
		( val - (T)2.0 * (T)PI_D ) :		// if > 2pi subtract 2pi from it
		val;
}

//===================================================
//	\function	toRadians
//	\brief convert angle in degrees to radians
//	\date 2020/07/15 5:13
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T toRadians( const T deg )
{
	return ( deg * PI ) / (T)180.0;
}

//===================================================
//	\function	interpolate
//	\brief linear interpolation from src to dst value using alpha provided
//			interpolates any two Types that support the operations +, -, *
//	\date 2020/08/02 17:56
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T interpolate( const T &src,
	const T &dst,
	const float alpha )
{
	return src + alpha * ( dst - src );
}

/// <image url="C:/Users/nikla/pictures/gaussian function distribution.png" scale=".4" />
// x is distance from center of Kernel/curve
template<typename T>
constexpr T gaussianDistr( const T x,
	const T sigma,
	const T mean ) noexcept
{
	const T ss = square( sigma );
	return ( (T)1 / sqrt( (T)2 * (T)PI_D * ss ) ) * exp( -square( x - mean ) / ((T)2 * ss) );
}

//===================================================
//	\function	powerOf
//	\brief  calculate x^n, n > 1
//				doesn't take into account negative powers for now
//	\date	2021/10/25 19:40
template<typename T, typename J, typename = std::enable_if_t<std::is_arithmetic_v<T>>,
	typename = std::enable_if_t<std::is_arithmetic_v<J>>>
constexpr T powerOf( T x,
	J n )
{
	if ( n == 0 )
	{
		return 1;
	}
	if ( n < 0 )
	{
		return -1;
	}
	T xo = x;
	while ( --n > 0 )
	{
		x *= xo;
	}
	return x;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T randomInRange( T start,
	T end )
{
	return ( rand() * ( end - start + 1 ) ) + start;
}

// produce random floating point numbers
float frand() noexcept;
// O(log_n(m))
int gcd( int divident, int divisor ) noexcept;
constexpr int factorialOf( int n ) noexcept;
// trial division method, returns 1 for prime, 0 for not prime
int isPrime( int number ) noexcept;
// Function that calculates the square root of a number
constexpr float squareRoot( float x ) noexcept;
constexpr bool isPowerOfTwo( const std::size_t value ) noexcept;
float cosine( float x ) noexcept;
float sine( float x ) noexcept;

constexpr int linearSearch( int *A, int n, int x );
constexpr int binarySearch( int *A, int low, int high, int x );

int toDecimal( int hex );
int toHex( int dec );

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
int signum( T val )
{
	return val > (T)0 ?
		1 :
		-1;
}

void convertToBase( int number, int base ) noexcept;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T nthMultipleOf( T number, int n )
{
	T multiple;
	for ( int i = 2; i <= n; ++i )
	{
		multiple = i * number;
	}
	return multiple;
}

// bitwise ops
// Prints given datatype to binary
void printDec2Bin( size_t const size, void const *const ptr );
bool oppositeSigns( int x, int y );
// print binary conversion of supplied decimal number
void dec2bin( int num );
// finds MSB index in decimal number (counting from 0)
int msbIndexOfDec( int num );
//  count number of Ones in a given number
int countSetBits( int num );
// checks whether a number is a power of 2
int isPowerOf2( int num );
// Swap the bits at position i and j of given number iff they are different
int swapBits( int num, int i, int j );
int setNthBit( int num, unsigned n );
int clearNthBit( int num, unsigned n );
int toggleNthBit( int num, unsigned n );
// check nth bit's state 0 or 1
int returnNthBit( int num, unsigned n );
// change nth bit to bitVal
int changeNthBit( int num, unsigned n, bool bitVal );

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