#include <type_traits>
#include <iostream>
#include <sstream>
#include "math_utils.h"
#include "winner.h"
#include "assertions_console.h"


namespace util
{

constexpr int nextPowerOf2( int n )
{
	if ( n < 0 )
	{
		return - 1;
	}
	unsigned count = 0;

	// first n in the below condition is for the case where n is 0
	if ( n && !( n & ( n - 1 ) ) )
	{
		return n;
	}

	while ( n != 0 )
	{
		n >>= 1;
		count += 1;
	}

	return 1 << count;
}

int gcd( int divident,
	int divisor )
{
	int r = 0;
	int temp = 0;
	if ( divisor > divident )
	{
		temp = divisor;
		divisor = divident;
		divident = temp;
	}

	while ( ( r = divident % divisor ) != 0 )
	{
		divident = divisor;
		divisor = r;
	}
	return divisor;
}

constexpr int factorialOf( int n ) noexcept
{
	int fact = 1;
	for ( ; n > 0; --n ) // initialize factorial to 1
	{
		fact *= n;
	}
	return fact;
}

constexpr bool isPrime( const int number ) noexcept
{
	int n = (int)util::powerOf( number, (double)1 / 2.0 );

	for ( int i = 2; i <= n; ++i )
	{
		if ( number % i == 0 )
		{
			return false;
		}
	}

	return true;
}

constexpr bool isPowerOfTwo( const std::size_t value ) noexcept
{
	return value > 0 && ( value & ( value - 1 ) ) == 0;
}

float cosine( float x ) noexcept
{
	if ( std::fmod( x, 90.0f ) == 0.0f && std::fmod( x, 180.0f ) != 0.0f )
	{
		return 0;
	}
	x = x * 3.14159f / 180;	// must be in radians
	int sign = -1;
	int terms = 16;
	float result = 1.0f;
	for ( int i = 2; i <= terms; i += 2 )
	{
		result += sign * powerOf( x, i ) / factorialOf( i );
		sign *= -1;
	}
	return result;
}

float sine( float x ) noexcept
{
	if ( std::fmod( x, 90.0f ) == 0.0f && std::fmod( x, 180.0f ) != 0.0f )
	{
		return 1;
	}
	x = x * 3.14159f / 180;	// must be in radians
	int sign = -1;
	int terms = 15;
	float result = x;
	for ( int i = 3; i <= terms; i += 2 )
	{
		result += sign * powerOf( x, i ) / factorialOf( i );
		sign *= -1;
	}
	return result;
}

constexpr int linearSearch( const int *A,
	const int n,
	const int val )
{
	ASSERT( n > 0 && A != nullptr, "Input raw array is invalid!" );
	for ( int i = 0; i < n; ++i )
	{
		if ( A[i] == val )
		{
			return i;
		}
	}
	return -1;
}

constexpr int binarySearch( const int *A,
	int low,
	int high,
	const int val )
{
	ASSERT( A != nullptr, "Input raw array is invalid!" );
	int med = -1;
	while ( low <= high )
	{
		med = ( low + high ) >> 1;
		if ( A[med] == val )
		{
			return med;
		}
		else if ( A[med] < val )
		{
			low = med + 1;
		}
		else
		{
			high = med - 1;
		}
	}
	return -1; // not present
}

int toDecimal( const int hex )
{
	int dec;
	std::stringstream ss;
	ss << hex;
	ss >> std::dec
		>> dec;
	return dec;
}

int toHex( const int dec )
{
	int hex;
	std::stringstream ss;
	ss << dec;
	ss >> std::hex
		>> hex;
	return hex;
}

// bitting
#pragma warning( push, 0 )
void printDec2Bin( const size_t size,
	void const * const ptr ) noexcept
{
	unsigned char *p = (unsigned char*)ptr;
	unsigned char byte = 0;

	for ( int i = size - 1; i >= 0; i-- )
	{
		for ( int j = 7; j >= 0; j-- )
		{
			byte = ( p[i] >> j ) & 1;
			std::cout << byte;
		}
	}

	std::cout << '\n';
}
#pragma warning( pop )

void printDecToBin( const int num ) noexcept
{
	int i = 0;
	for ( i = 31; i >= 0; i-- )
	{
		if ( ( num & ( 1 << i ) ) != 0 )
		{
			std::cout << "1" << '\n';
		}
		else
		{
			std::cout << "0" << '\n';
		}
	}
}

constexpr bool haveOppositeSigns( const int x,
	const int y ) noexcept
{
	return ( ( x ^ y ) < 0 );
}

#pragma warning( push, 0 )
constexpr int msbIndexOfDec( int num ) noexcept
{
	int i = 0;

	while ( num >>= 1 != 0 )
	{
		++i;
	}

	return i;
}
#pragma warning( pop )

constexpr int countSetBits( int num ) noexcept
{
	int count = 0;
	while ( num != 0 )
	{
		num &= ( num - 1 );
		count++;
	}
	return count;
}

constexpr int isPowerOf2( const int num ) noexcept
{
	// if a number is a power of 2 then it has only one 1 in its binary representation
	if ( ( num & ( num - 1 ) ) == 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

constexpr void convertToBase( int number,
	const int base ) noexcept
{
	int convertedDigits[64]{0};
	int i = 0;
	do
	{
		convertedDigits[i] = number % base;
		++i;
		number /= base;
	} while ( number != 0 );
}

constexpr int swapBits( int num,
	const int i,
	const int j ) noexcept
{
	int lo = ( ( num >> i ) & 1 );
	int hi = ( ( num >> j ) & 1 );
	if ( lo ^ hi )
	{
		num ^= ( ( 1 << i ) | ( 1 << j ) ); // XOR with 1 to toggle bits
	}
	return num;
}

constexpr int setNthBit( int num,
	const unsigned n ) noexcept
{
	num |= 1 << n;
	return num;
}

constexpr int clearNthBit( int num,
	const unsigned n ) noexcept
{
	num &= ~( 1 << n );
	return num;
}

constexpr int toggleNthBit( int num,
	const unsigned n ) noexcept
{
	num ^= 1 << n;
	return num;
}

constexpr int isSet( int num,
	const unsigned n ) noexcept
{
	return ( num >> n ) & 1;
}

bool isSetByNumber( int num,
	int value ) noexcept
{
	if ( !isPowerOf2( value ) )
	{
		return false;
	}

	unsigned pos = 0;
	while ( value % 2 == 0 )
	{
		value = value / 2;
		++pos;
	}
	return ( num >> pos ) & 1;
}

#pragma warning( push, 0 )
constexpr int changeNthBit( int num,
	const unsigned n,
	const bool bitVal ) noexcept
{
	num ^= ( -bitVal ^ num ) & ( 1 << n );
	return num;
}
#pragma warning( pop )


}// namespace util