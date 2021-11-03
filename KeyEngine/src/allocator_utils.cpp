#include "allocator_utils.h"
#include "utils.h"


namespace util
{

bool isAligned( const volatile void* p,
	std::size_t alignment ) noexcept
{
	return ( reinterpret_cast<std::uintptr_t>( p ) % alignment ) == 0;
}

bool isAligned( std::uintptr_t pi,
	std::size_t alignment ) noexcept
{
	return ( pi % alignment ) == 0;
}

#pragma warning( push, 0 )
constexpr int is4ByteAligned( intptr_t *addr )
{
	if ( ( (intptr_t)addr & 0x3 ) == 0 )
	{ // changing int* to int
		return 1;
	}
	return 0;
}
#pragma warning( pop )

std::uintptr_t alignForward( std::uintptr_t ip,
	std::size_t alignment ) noexcept
{
	if ( alignment == 0 )
	{
		return ip;
	}
	if ( ip % alignment == 0 )
	{
		return ip;
	}
	return ( ip + ( alignment - 1 ) ) & ~( alignment - 1 );
	// or: (ip + alignment - 1) / alignment * alignment;
}

// calculates alignment in bits supposedly
std::size_t calcAlignedSize( std::size_t size,
	std::size_t alignment )
{
	return size + ( size % ( alignment / 8 ) );
}

// calculate padding bytes needed to align address p forward given the alignment
const std::size_t getForwardPadding( const std::size_t p,
	const std::size_t alignment )
{
	const std::size_t mult = ( p / alignment ) + 1;
	const std::size_t alignedAddress = mult * alignment;
	const std::size_t padding = alignedAddress - p;
	return padding;
}

const std::size_t getForwardPaddingWithHeader( const std::size_t p,
	const std::size_t alignment,
	const std::size_t headerSize )
{
	std::size_t padding = getForwardPadding( p,
		alignment );
	std::size_t neededSpace = headerSize;

	if ( padding < neededSpace )
	{
		// Header does not fit - Calculate next aligned address that header fits
		neededSpace -= padding;

		// How many alignments I need to fit the header        
		if ( neededSpace % alignment > 0 )
		{
			padding += alignment * ( 1 + ( neededSpace / alignment ) );
		}
		else
		{
			padding += alignment * ( neededSpace / alignment );
		}
	}
	return padding;
}

void* alignedMalloc( std::size_t nBytes,
	std::size_t alignment )
{
	// allocate `nBytes` + `nBytesForAlignment` required given requested `alignment` value
	// store malloced address in `pMem`
	// compute aligned address `pAlignedMem` by adding the `bytesForAdjustment` to malloced `pMem` address
	ASSERT( false, "Never should have come here!" );
	void* pMem = nullptr;
	std::size_t nBytesForAlignment = alignment - 1 + sizeof(void*);
	if ( ( pMem = static_cast<void*>( ::operator new( nBytes + nBytesForAlignment ) ) ) == nullptr )
	{
		return nullptr;
	}
	// round-up / align address forward
	std::size_t bytesForAdjustment = ( nBytesForAlignment ) & ~( alignment - 1 );
	void** pAlignedMem = reinterpret_cast<void**>( reinterpret_cast<std::size_t>( pMem )
		+ bytesForAdjustment );
	pAlignedMem[-1] = pMem;
	return pAlignedMem;
}

void alignedFree( void *p ) noexcept
{
	ASSERT( false, "Never should have come here!" );
	::operator delete( static_cast<void**>( p )[-1] );
}


}//namespace util