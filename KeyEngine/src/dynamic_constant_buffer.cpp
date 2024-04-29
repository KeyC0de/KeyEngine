#include "dynamic_constant_buffer.h"
#include <algorithm>
#include <cctype>


namespace con
{

struct ExtraData final
{
	struct Struct final
		: public CBElement::IExtraData
	{
		std::vector<std::pair<std::string, CBElement>> m_layoutElements;
	};

	struct Array final
		: public CBElement::IExtraData
	{
		std::optional<CBElement> m_layoutElement;
		size_t m_elementSize = 0;
		size_t m_size = 0;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBElement
std::string CBElement::calcSignature() const cond_noex
{
	switch( m_type )
	{
#define X( el ) case el: return ElementProperties<el>::tag;
	CB_LEAF_TYPES
#undef X
	case Struct:
		return getSignatureForStruct();
	case Array:
		return calcSignatureForArray();
	default:
		ASSERT( false, "Bad type in signature generation" );
		return "???";
	}
}

bool CBElement::isValid() const noexcept
{
	return m_type != Empty;
}

std::pair<size_t, const CBElement*> CBElement::calculateArrayIndexingOffset( const size_t offset,
	const size_t index ) const cond_noex
{
	ASSERT( m_type == Array, "Attempted to index into a non-array type" );
	const auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
	ASSERT( index < data.m_size, "Indexing arithmetic!" );
	return {offset + data.m_elementSize * index, &*data.m_layoutElement};
}

CBElement& CBElement::operator[]( const std::string &key ) cond_noex
{
	ASSERT( m_type == Struct, "Attempted to key into a non-struct type" );
	for ( auto &layEl : static_cast<ExtraData::Struct&>( *m_pExtraData ).m_layoutElements )
	{
		if ( layEl.first == key )
		{
			return layEl.second;
		}
	}
	return getEmptyElement();
}

const CBElement& CBElement::operator[]( const std::string &key ) const cond_noex
{
	return const_cast<CBElement&>( *this )[key];
}

CBElement& CBElement::T() cond_noex
{
	ASSERT( m_type == Array, "Attempted to access inner CBElement type T of non-array type." );
	return *static_cast<ExtraData::Array&>( *m_pExtraData ).m_layoutElement;
}

const CBElement& CBElement::T() const cond_noex
{
	return const_cast<CBElement&>( *this ).T();
}

size_t CBElement::getOffsetBegin() const cond_noex
{
	return *m_offset;
}

size_t CBElement::getOffsetEnd() const cond_noex
{
	switch( m_type )
	{
#define X( el ) case el: \
		return *m_offset + ElementProperties<el>::hlslSize;
	CB_LEAF_TYPES
#undef X
	case Struct:
	{
		const auto &data = static_cast<ExtraData::Struct&>( *m_pExtraData );
		return advanceToBoundary( data.m_layoutElements.back().second.getOffsetEnd() );
	}
	case Array:
	{
		const auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
		return *m_offset + advanceToBoundary( data.m_layoutElement->getSizeInBytes() )
			* data.m_size;
	}
	default:
		ASSERT( false, "Tried to get offset of invalid element or at the wrong time!" );
		return 0u;
	}
}

size_t CBElement::getSizeInBytes() const cond_noex
{
	return getOffsetEnd() - getOffsetBegin();
}

CBElement& CBElement::add( const ElementType addedType,
	const std::string& name ) cond_noex
{
	ASSERT( m_type == Struct, "Attempted to add sub Element to non-Struct type" );
	ASSERT( validateMemberName( name ), "Invalid name for Struct inner Element type." );
	auto &structData = static_cast<ExtraData::Struct&>( *m_pExtraData );
	for ( auto &layEl : structData.m_layoutElements )
	{
		if ( layEl.first == name )
		{
			ASSERT( false, "Attempted to add element with duplicate name in Struct type." );
		}
	}
	structData.m_layoutElements.emplace_back( name, CBElement{addedType} );
	return *this;
}

CBElement& CBElement::set( const ElementType addedType,
	const size_t size ) cond_noex
{
	ASSERT( m_type == Array, "Set on non-array in layout" );
	ASSERT( size != 0u, "Element size is empty! Maybe it has not been committed yet." );
	auto &arrayData = static_cast<ExtraData::Array&>( *m_pExtraData );
	arrayData.m_layoutElement = {addedType};
	arrayData.m_size = size;
	return *this;
}

CBElement::CBElement( const ElementType typeIn ) cond_noex
	:
	m_type{typeIn}
{
	ASSERT( typeIn != Empty, "Element type is already filled!" );
	if ( typeIn == Struct )
	{
		m_pExtraData = std::make_unique<ExtraData::Struct>();
	}
	else if ( typeIn == Array )
	{
		m_pExtraData = std::make_unique<ExtraData::Array>();
	}
}

const size_t CBElement::commit( const size_t offsetIn ) cond_noex
{
	switch( m_type )
	{
#define X( el ) case el: \
		m_offset = advanceIfCrossesBoundary( offsetIn, \
			ElementProperties<el>::hlslSize ); \
			return *m_offset + ElementProperties<el>::hlslSize;
	CB_LEAF_TYPES
#undef X
	case Struct:
		return commitStruct( offsetIn );
	case Array:
		return commitArray( offsetIn );
	default:
		ASSERT( false, "Committing CBElement error. Bad layout." );
		return 0u;
	}
}

std::string CBElement::getSignatureForStruct() const cond_noex
{
	using namespace std::string_literals;
	auto sig = "St{"s;
	for ( const auto &el : static_cast<ExtraData::Struct&>( *m_pExtraData ).m_layoutElements )
	{
		sig += el.first + ":"s + el.second.calcSignature() + ";"s;
	}
	sig += "}"s;
	return sig;
}

std::string CBElement::calcSignatureForArray() const cond_noex
{
	using namespace std::string_literals;
	const auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
	return "Ar:"s
		+ std::to_string( data.m_size )
		+ "{"s
		+ data.m_layoutElement->calcSignature()
		+ "}"s;
}

const size_t CBElement::commitStruct( const size_t offsetIn )
{
	auto &data = static_cast<ExtraData::Struct&>( *m_pExtraData );
	ASSERT( data.m_layoutElements.size() != 0u, "Struct inner elements have not been committed yet!" );
	m_offset = advanceToBoundary( offsetIn );
	auto offsetNext = *m_offset;
	for ( auto &el : data.m_layoutElements )
	{
		offsetNext = el.second.commit( offsetNext );
	}
	return offsetNext;
}

const size_t CBElement::commitArray( const size_t offsetIn )
{
	auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
	ASSERT( data.m_size != 0u, "Array size is 0? WTF error!" );
	m_offset = advanceToBoundary( offsetIn );
	data.m_layoutElement->commit( *m_offset );
	data.m_elementSize = CBElement::advanceToBoundary( data.m_layoutElement->getSizeInBytes() );
	return getOffsetEnd();
}

bool CBElement::doesCrossBoundary( const size_t offset,
	const size_t size ) noexcept
{
	const size_t end = offset + size;
	const size_t pageStart = offset / 16u;
	const size_t pageEnd = end / 16u;
	ASSERT( pageEnd >= pageStart, "Wrong sizes!" );
	return ( pageStart != pageEnd && end % 16 != 0u ) || size > 16u;
}

const size_t CBElement::advanceIfCrossesBoundary( const size_t offset,
	const size_t size ) noexcept
{
	return doesCrossBoundary( offset, size ) ?
		advanceToBoundary( offset ) :
		offset;
}

const size_t CBElement::advanceToBoundary( const size_t offset ) noexcept
{
	return offset + ( 16u - offset % 16u ) % 16u;
}

bool CBElement::validateMemberName( const std::string &name ) noexcept
{
	// symbols can contain alphanumeric and underscore, must not start with digit
	return !name.empty()
		&& !std::isdigit( name.front() )
		&& std::all_of( name.begin(), name.end(),
			[]( char c )
			{
				return std::isalnum( c ) || c == '_';
			} );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBLayout
CBLayout::CBLayout( std::shared_ptr<CBElement> pRoot ) noexcept
	:
	m_pLayoutRoot{std::move( pRoot )}
{

}

size_t CBLayout::getSizeInBytes() const noexcept
{
	return m_pLayoutRoot->getSizeInBytes();
}

std::string CBLayout::calcSignature() const cond_noex
{
	return m_pLayoutRoot->calcSignature();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// RawLayout
RawLayout::RawLayout() noexcept
	:
	CBLayout{std::shared_ptr<CBElement>{new CBElement( Struct )}}
{

}

CBElement& RawLayout::operator[]( const std::string &key ) cond_noex
{
	return ( *m_pLayoutRoot )[key];
}

std::shared_ptr<CBElement> RawLayout::cookLayout() noexcept
{
	auto temp = std::move( m_pLayoutRoot );
	temp->commit( 0 );
	*this = RawLayout();
	return std::move( temp );
}

void RawLayout::clear() noexcept
{
	*this = RawLayout();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CookedLayout
CookedLayout::CookedLayout( std::shared_ptr<CBElement> pRoot ) noexcept
	:
	CBLayout(std::move( pRoot ))
{

}

std::shared_ptr<CBElement> CookedLayout::relinquishRoot() noexcept
{
	return std::move( m_pLayoutRoot );
}

std::shared_ptr<CBElement> CookedLayout::shareRootElement() const noexcept
{
	return m_pLayoutRoot;
}

const CBElement& CookedLayout::operator[]( const std::string &key ) const cond_noex
{
	return (*m_pLayoutRoot)[key];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBElementConstView
bool CBElementConstView::isValid() const noexcept
{
	return pLayout->isValid();
}

CBElementConstView CBElementConstView::operator[]( const std::string &key ) const cond_noex
{
	return {&(*pLayout)[key], m_p, m_arrayOffset};
}

CBElementConstView CBElementConstView::operator[]( size_t index ) const cond_noex
{
	const auto indexingData = pLayout->calculateArrayIndexingOffset( m_arrayOffset, index );
	return {indexingData.second, m_p, indexingData.first};
}

CBElementConstView::Ptr CBElementConstView::operator&() const cond_noex
{
	return Ptr{this};
}

CBElementConstView::CBElementConstView( const CBElement *pLayout,
	const char *pBytes,
	const size_t offset ) noexcept
	:
	m_arrayOffset(offset),
	pLayout(pLayout),
	m_p(pBytes)
{

}

CBElementConstView::Ptr::Ptr( const CBElementConstView *ref ) noexcept
	:
	p(ref)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBElementView
CBElementView::operator CBElementConstView() const noexcept
{
	return {m_pLayout, m_p, m_arrayOffset};
}

bool CBElementView::isValid() const noexcept
{
	return m_pLayout->isValid();
}

CBElementView CBElementView::operator[]( const std::string &key ) const cond_noex
{
	return {&(*m_pLayout)[key], m_p, m_arrayOffset};
}

CBElementView CBElementView::operator[]( const size_t index ) const cond_noex
{
	const auto indexingData = m_pLayout->calculateArrayIndexingOffset( m_arrayOffset, index );
	return {indexingData.second, m_p, indexingData.first};
}

CBElementView::Ptr CBElementView::operator&() const cond_noex
{
	return Ptr{const_cast<CBElementView*>( this )};
}

CBElementView::CBElementView( const CBElement *pLayout,
	char *pBytes,
	size_t offset ) noexcept
	:
	m_arrayOffset(offset),
	m_pLayout(pLayout),
	m_p(pBytes)
{

}

CBElementView::Ptr::Ptr( CBElementView *ref ) noexcept
	:
	p(ref)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LayoutMap
con::CookedLayout LayoutMap::fetch( con::RawLayout &&cbLayout ) cond_noex
{
	std::string sig = cbLayout.calcSignature();
	auto &map = getInstance().m_map;
	const auto it = map.find( sig );
	// identical layout already exists
	if ( it != map.end() )
	{
		// input layout is expected to be cleared after fetch
		cbLayout.clear();
		return {it->second};
	}
	// otherwise add layout root element to map
	auto result = map.insert( {std::move( sig ), cbLayout.cookLayout()} );
	// ASSERT( result.second, "Root layout is null!" );
	// return layout with additional reference to root
	return {result.first->second};
}

LayoutMap& LayoutMap::getInstance() noexcept
{
	static LayoutMap instance{};
	return instance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBuffer
CBuffer::CBuffer( RawLayout &&lay ) cond_noex
	:
CBuffer{LayoutMap::fetch( std::move( lay ) )}
{

}

CBuffer::CBuffer( const CookedLayout &lay ) cond_noex
	:
	m_pLayoutRoot(lay.shareRootElement()),
	m_buffer(m_pLayoutRoot->getOffsetEnd())
{

}

CBuffer::CBuffer( CookedLayout &&lay ) cond_noex
	:
	m_pLayoutRoot(lay.relinquishRoot()),
	m_buffer(m_pLayoutRoot->getOffsetEnd())
{

}

CBuffer::CBuffer( const CBuffer &rhs ) noexcept
	:
	m_pLayoutRoot(rhs.m_pLayoutRoot),
	m_buffer(rhs.m_buffer)
{

}

#pragma warning( disable : 4172 )
CBuffer& CBuffer::operator==( const CBuffer &rhs ) noexcept
{
	CBuffer tmp{rhs};
	return tmp;
}
#pragma warning( default : 4172 )

CBuffer::CBuffer( CBuffer &&rhs ) noexcept
	:
	m_pLayoutRoot(std::move( rhs.m_pLayoutRoot )),
	m_buffer(std::move( rhs.m_buffer ))
{
	rhs.m_pLayoutRoot.reset();
	rhs.m_buffer.clear();
}

CBuffer& CBuffer::operator=( CBuffer &&rhs ) noexcept
{
	CBuffer tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

CBuffer::~CBuffer() noexcept
{
	m_pLayoutRoot.reset();
	m_buffer.clear();
}


CBElementView CBuffer::operator[]( const std::string &key ) cond_noex
{
	return {&(*m_pLayoutRoot)[key], m_buffer.data(), 0u};
}

CBElementConstView CBuffer::operator[]( const std::string &key ) const cond_noex
{
	return const_cast<CBuffer&>( *this )[key];
}

const char* CBuffer::data() const noexcept
{
	return m_buffer.data();
}

const char* CBuffer::getRawBytes() const noexcept
{
	return m_buffer.data();
}

size_t CBuffer::getSizeInBytes() const noexcept
{
	return m_buffer.size();
}

const CBElement& CBuffer::getRootElement() const noexcept
{
	return *m_pLayoutRoot;
}

void CBuffer::copyFrom( const CBuffer &other ) cond_noex
{
	ASSERT( &getRootElement() == &other.getRootElement(), "Incompatible element layouts!" );
	std::copy( other.m_buffer.begin(), other.m_buffer.end(), m_buffer.begin() );
}

void CBuffer::moveFrom( CBuffer &other ) noexcept
{
	ASSERT( &getRootElement() == &other.getRootElement(), "Incompatible element layouts!" );
	std::move( other.m_buffer.begin(), other.m_buffer.end(), m_buffer.begin() );
}

std::shared_ptr<CBElement> CBuffer::shareLayoutRoot() const noexcept
{
	return m_pLayoutRoot;
}


}//namespace con