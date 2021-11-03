#pragma once

#include <cstddef>
#include <cinttypes>
#include "assertions_console.h"


namespace util
{

//  check whether the address is aligned to `alignment` boundary
bool isAligned( const volatile void* p, std::size_t alignment ) noexcept;
bool isAligned( std::uintptr_t pi, std::size_t alignment ) noexcept;

constexpr int is4ByteAligned( intptr_t *addr );

/// align pointer forward with given alignment
template<typename T>
T* alignForward( T* p,
	std::size_t alignment ) noexcept
{
	if ( alignment == 0 )
	{
		return p;
	}
	const std::uintptr_t ip = reinterpret_cast<std::uintptr_t>( p );
	if ( ip % alignment == 0 )
	{
		return p;
	}
	return reinterpret_cast<T*>( ( ip + ( alignment - 1 ) ) & ~( alignment - 1 ) );
	// or: (ip + alignment - 1) / alignment * alignment;
}

std::uintptr_t alignForward( std::uintptr_t ip, std::size_t alignment ) noexcept;

// calculates alignment in bits supposedly
std::size_t calcAlignedSize( std::size_t size, std::size_t alignment );

// calculate padding bytes needed to align address p forward given the alignment
const std::size_t getForwardPadding( const std::size_t p, const std::size_t alignment );

const std::size_t getForwardPaddingWithHeader( const std::size_t p,
	const std::size_t alignment, const std::size_t headerSize );

template<typename T>
T* alignPtr( const T *ptr,
	const std::size_t alignment )
{
	const std::uintptr_t uintPtr = reinterpret_cast<std::uintptr_t>( ptr );
	const std::uintptr_t alignedUintPtr = ( uintPtr + ( alignment - 1 ) ) & ~( alignment - 1 );
	T* alignedPtr = reinterpret_cast<T*>( alignedUintPtr );
	ASSERT( isAligned( alignedPtr, alignment ), "Not aligned!" );
	return alignedPtr;
}

// TODO: doesn't work properly
void* alignedMalloc( std::size_t nBytes, std::size_t alignment );
void alignedFree( void *p ) noexcept;

// INTEL:
//void* _mm_malloc(int size, int align)
//void _mm_free(void *p)


}//namespace util