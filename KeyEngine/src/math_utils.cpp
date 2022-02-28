#include <type_traits>
#include <iostream>
#include <sstream>
#include "math_utils.h"
#include "winner.h"


namespace util
{

float frand() noexcept
{
	return static_cast<float>( rand() / ( RAND_MAX + 1.0f ) );
}

int gcd( int m,
	int n ) noexcept
{
	int r = 0;
	int temp = 0;
	if ( n > m )
	{
		temp = n;
		n = m;
		m = temp;
	}

	while ( ( r = m % n ) != 0 )
	{
		m = n;
		n = r;
	}
	return n;
}


constexpr float squareRoot( float x ) noexcept
{
	const float epsilon = .00001;
	float guess = 1.0;
	int i = 0;

	if ( x < 0 )
	{
		std::cout << "Negative Argument. Error!\n";
		return -1.0;
	}

	while ( abs( ( guess * guess ) / x - 1 ) >= epsilon )
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

constexpr int factorialOf( int n ) noexcept
{
	int fact = 1;
	for ( ; n > 0; --n ) // initialize factorial to 1
	{
		fact *= n;
	}
	return fact;
}

int isPrime( int number ) noexcept
{
	int n = (int)pow( number, (double)1 / 2.0 );

	for ( int i = 2; i <= n; ++i )
	{
		if ( number % i == 0 )
		{
			return 0;
		}
	}

	return 1;
}

constexpr bool isPowerOfTwo( const std::size_t value ) noexcept
{
	return value > 0 && ( value & ( value - 1 ) ) == 0;
}

float cosine( float x ) noexcept
{
	if ( fmod( x, 90 ) == 0 && fmod( x, 180 ) != 0 )
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
	if ( fmod( x, 90 ) == 0 && fmod( x, 180 ) != 0 )
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

constexpr int linearSearch( int* A,
	int n,
	int x )
{
	for ( int i = 0; i < n; ++i )
	{
		if ( A[i] == x )
		{
			return i;
		}
	}
	return -1;
}

constexpr int binarySearch( int* A,
	int low,
	int high,
	int x )
{
	int med = -1;
	while ( low <= high )
	{
		med = ( low + high ) >> 1;
		if ( A[med] == x )
		{
			return med;
		}
		else if ( A[med] < x )
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

int toDecimal( int hex )
{
	int dec;
	std::stringstream ss;
	ss << hex;
	ss >> std::dec
		>> dec;
	return dec;
}

int toHex( int dec )
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
void printDec2Bin( size_t const size,
	void const * const ptr )
{
	unsigned char* p = (unsigned char*)ptr;
	unsigned char byte;

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


bool oppositeSigns( int x,
	int y )
{
	return ( ( x ^ y ) < 0 );
}

void dec2bin( int num )
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

#pragma warning( push, 0 )
int msbIndexOfDec( int num )
{
	int i = 0;

	while ( num >>= 1 != 0 )
	{
		++i;
	}

	return i;
}
#pragma warning( pop )

int countSetBits( int num )
{
	int count = 0;
	while ( num != 0 )
	{
		num &= ( num - 1 );
		count++;
	}
	return count;
}

int isPowerOf2( int num )
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

void convertToBase( int number,
	int base ) noexcept
{
	int convertedDigits[64];
	int i = 0;
	do
	{
		convertedDigits[i] = number % base;
		++i;
		number /= base;
	} while ( number != 0 );
}

int swapBits( int num,
	int i,
	int j )
{
	int lo = ( ( num >> i ) & 1 );
	int hi = ( ( num >> j ) & 1 );
	if ( lo ^ hi )
	{
		num ^= ( ( 1 << i ) | ( 1 << j ) ); // XOR with 1 to toggle bits
	}
	return num;
}

int setNthBit( int num,
	unsigned n )
{
	num |= 1 << n;
	return num;
}

int clearNthBit( int num,
	unsigned n )
{
	num &= ~( 1 << n );
	return num;
}

int toggleNthBit( int num,
	unsigned n )
{
	num ^= 1 << n;
	return num;
}

int returnNthBit( int num,
	unsigned n )
{
	return ( num >> n ) & 1;
}

#pragma warning( push, 0 )
int changeNthBit( int num,
	unsigned n,
	bool bitVal )
{
	num ^= ( -bitVal ^ num ) & ( 1 << n );
	return num;
}
#pragma warning( pop )


}// namespace util