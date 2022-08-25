#define CB_IMPL_SOURCE
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
		std::vector<std::pair<std::string, CBElement>> layoutElements;
	};
	struct Array final
		: public CBElement::IExtraData
	{
		std::optional<CBElement> layoutElement;
		size_t element_size;
		size_t size;
	};
};

std::string CBElement::getSignature() const cond_noex
{
	switch( m_type )
	{
	#define X(el) case el: return ElementProperties<el>::tag;
	CB_LEAF_TYPES
	#undef X
	case Struct:
		return getSignatureForStruct();
	case Array:
		return getSignatureForArray();
	default:
		ASSERT( false, "Bad type in signature generation" );
		return "???";
	}
}

bool CBElement::isValid() const noexcept
{
	return m_type != Empty;
}

std::pair<size_t, const CBElement*> CBElement::calculateArrayIndexingOffset( size_t offset,
	size_t index ) const cond_noex
{
	ASSERT( m_type == Array, "Attempted to index into a non-array type" );
	const auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
	ASSERT( index < data.size, "Indexing arithmetic!" );
	return {offset + data.element_size * index,
		&*data.layoutElement};
}

CBElement &CBElement::operator[]( const std::string &key ) cond_noex
{
	ASSERT( m_type == Struct, "Attempted to key into a non-struct type" );
	for ( auto &layEl : static_cast<ExtraData::Struct&>( *m_pExtraData ).layoutElements )
	{
		if ( layEl.first == key )
		{
			return layEl.second;
		}
	}
	return getEmptyElement();
}

const CBElement &CBElement::operator[]( const std::string &key ) const cond_noex
{
	return const_cast<CBElement&>( *this )[key];
}

CBElement &CBElement::T() cond_noex
{
	ASSERT( m_type == Array, "Attempted to access inner CBElement type T of non-array type." );
	return *static_cast<ExtraData::Array&>( *m_pExtraData ).layoutElement;
}

const CBElement &CBElement::T() const cond_noex
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
	#define X( el ) case el:\
		return *m_offset + ElementProperties<el>::hlslSize;
	CB_LEAF_TYPES
	#undef X
	case Struct:
	{
		const auto &data = static_cast<ExtraData::Struct&>( *m_pExtraData );
		return advanceToBoundary( data.layoutElements.back().second.getOffsetEnd() );
	}
	case Array:
	{
		const auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
		return *m_offset + advanceToBoundary( data.layoutElement->getSizeInBytes() )
			* data.size;
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

CBElement &CBElement::add( ElementType addedType,
	std::string name ) cond_noex
{
	ASSERT( m_type == Struct, "Attempted to add sub Element to non-Struct type" );
	ASSERT( validateMemberName( name ), "Invalid name for Struct inner Element type." );
	auto &structData = static_cast<ExtraData::Struct&>( *m_pExtraData );
	for ( auto &layEl : structData.layoutElements )
	{
		if ( layEl.first == name )
		{
			ASSERT( false, "Attempted to add element with duplicate name in Struct type." );
		}
	}
	structData.layoutElements.emplace_back( std::move( name ),
		CBElement{addedType} );
	return *this;
}

CBElement &CBElement::set( ElementType addedType,
	size_t size ) cond_noex
{
	ASSERT( m_type == Array, "Set on non-array in layout" );
	ASSERT( size != 0u, "Element size is empty! Maybe it has not been committed yet." );
	auto &arrayData = static_cast<ExtraData::Array&>( *m_pExtraData );
	arrayData.layoutElement = {addedType};
	arrayData.size = size;
	return *this;
}

CBElement::CBElement( ElementType typeIn ) cond_noex
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

size_t CBElement::commit( size_t offsetIn ) cond_noex
{
	switch( m_type )
	{
	#define X( el ) case el:\
		m_offset = advanceIfCrossesBoundary( offsetIn,\
			ElementProperties<el>::hlslSize );\
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
	for ( const auto &el : static_cast<ExtraData::Struct&>( *m_pExtraData )
		.layoutElements )
	{
		sig += el.first + ":"s + el.second.getSignature() + ";"s;
	}
	sig += "}"s;
	return sig;
}

std::string CBElement::getSignatureForArray() const cond_noex
{
	using namespace std::string_literals;
	const auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
	return "Ar:"s + std::to_string( data.size ) + "{"s
		+ data.layoutElement->getSignature() + "}"s;
}

size_t CBElement::commitStruct( size_t offsetIn )
{
	auto &data = static_cast<ExtraData::Struct&>( *m_pExtraData );
	ASSERT( data.layoutElements.size() != 0u, "Struct inner elements have not been committed yet!" );
	m_offset = advanceToBoundary( offsetIn );
	auto offsetNext = *m_offset;
	for ( auto &el : data.layoutElements )
	{
		offsetNext = el.second.commit( offsetNext );
	}
	return offsetNext;
}

size_t CBElement::commitArray( size_t offsetIn )
{
	auto &data = static_cast<ExtraData::Array&>( *m_pExtraData );
	ASSERT( data.size != 0u, "Array size is 0? WTF error!" );
	m_offset = advanceToBoundary( offsetIn );
	data.layoutElement->commit( *m_offset );
	data.element_size = CBElement::advanceToBoundary( data.layoutElement->getSizeInBytes() );
	return getOffsetEnd();
}

bool CBElement::doesCrossBoundary( size_t offset,
	size_t size ) noexcept
{
	const size_t end = offset + size;
	const size_t pageStart = offset / 16u;
	const size_t pageEnd = end / 16u;
	ASSERT( pageEnd >= pageStart, "Wrong sizes!" );
	return ( pageStart != pageEnd && end % 16 != 0u ) || size > 16u;
}

size_t CBElement::advanceIfCrossesBoundary( size_t offset,
	size_t size ) noexcept
{
	return doesCrossBoundary( offset, size ) ?
		advanceToBoundary( offset ) :
		offset;
}

size_t CBElement::advanceToBoundary( size_t offset ) noexcept
{
	return offset + ( 16u - offset % 16u ) % 16u;
}

bool CBElement::validateMemberName( const std::string &name ) noexcept
{
	// symbols can contain alphanumeric and underscore, must not start with digit
	return !name.empty() && !std::isdigit( name.front() ) && std::all_of( name.begin(),
			name.end(),
			[]( char c )
			{
				return std::isalnum( c ) || c == '_';
			} );
}


CBLayout::CBLayout( std::shared_ptr<CBElement> pRoot ) noexcept
	:
	m_pLayoutRoot{std::move( pRoot )}
{

}

size_t CBLayout::getSizeInBytes() const noexcept
{
	return m_pLayoutRoot->getSizeInBytes();
}

std::string CBLayout::getSignature() const cond_noex
{
	return m_pLayoutRoot->getSignature();
}


RawLayout::RawLayout() noexcept
	:
	CBLayout{std::shared_ptr<CBElement>{new CBElement( Struct )}}
{

}

CBElement &RawLayout::operator[]( const std::string &key ) cond_noex
{
	return ( *m_pLayoutRoot )[key];
}

std::shared_ptr<CBElement> RawLayout::commitLayout() noexcept
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


CookedLayout::CookedLayout( std::shared_ptr<CBElement> pRoot ) noexcept
	:
	CBLayout(std::move( pRoot ))
{

}

std::shared_ptr<CBElement> CookedLayout::relinquishRoot() const noexcept
{
	return std::move( m_pLayoutRoot );
}

std::shared_ptr<CBElement> CookedLayout::shareRootElement() const noexcept
{
	return m_pLayoutRoot;
}

const CBElement &CookedLayout::operator[]( const std::string &key ) const cond_noex
{
	return (*m_pLayoutRoot)[key];
}


bool ConstElementView::isValid() const noexcept
{
	return pLayout->isValid();
}

ConstElementView ConstElementView::operator[]( const std::string &key ) const cond_noex
{
	return {&(*pLayout)[key], m_p, m_arrayOffset};
}

ConstElementView ConstElementView::operator[]( size_t index ) const cond_noex
{
	const auto indexingData = pLayout->calculateArrayIndexingOffset( m_arrayOffset,
		index );
	return {indexingData.second, m_p, indexingData.first};
}

ConstElementView::Ptr ConstElementView::operator&() const cond_noex
{
	return Ptr{this};
}

ConstElementView::ConstElementView( const CBElement *pLayout,
	const char *pBytes,
	size_t offset ) noexcept
	:
	m_arrayOffset(offset),
	pLayout(pLayout),
	m_p(pBytes)
{

}

ConstElementView::Ptr::Ptr( const ConstElementView *ref ) noexcept
	:
	p(ref)
{

}


ElementView::operator ConstElementView() const noexcept
{
	return {m_pLayout, m_p, m_arrayOffset};
}

bool ElementView::isValid() const noexcept
{
	return m_pLayout->isValid();
}

ElementView ElementView::operator[]( const std::string &key ) const cond_noex
{
	return {&(*m_pLayout)[key], m_p, m_arrayOffset};
}

ElementView ElementView::operator[]( size_t index ) const cond_noex
{
	const auto indexingData = m_pLayout->calculateArrayIndexingOffset( m_arrayOffset,
		index );
	return {indexingData.second, m_p, indexingData.first};
}

ElementView::Ptr ElementView::operator&() const cond_noex
{
	return Ptr{const_cast<ElementView*>( this )};
}

ElementView::ElementView( const CBElement *pLayout,
	char *pBytes,
	size_t offset ) noexcept
	:
	m_arrayOffset(offset),
	m_pLayout(pLayout),
	m_p(pBytes)
{

}

ElementView::Ptr::Ptr( ElementView *ref ) noexcept
	:
	p(ref)
{

}


Buffer::Buffer( RawLayout &&lay ) cond_noex
	:
Buffer{LayoutMap::fetch( std::move( lay ) )}
{

}

Buffer::Buffer( const CookedLayout &lay ) cond_noex
	:
	m_pLayoutRoot(lay.shareRootElement()),
	m_buffer(m_pLayoutRoot->getOffsetEnd())
{

}

Buffer::Buffer( CookedLayout &&lay ) cond_noex
	:
	m_pLayoutRoot(lay.relinquishRoot()),
	m_buffer(m_pLayoutRoot->getOffsetEnd())
{

}

Buffer::Buffer( const Buffer &rhs ) noexcept
	:
	m_pLayoutRoot(rhs.m_pLayoutRoot),
	m_buffer(rhs.m_buffer)
{

}

#pragma warning( disable : 4172 )
Buffer &Buffer::operator==( const Buffer &rhs ) noexcept
{
	Buffer tmp{rhs};
	return tmp;
}
#pragma warning( default : 4172 )

Buffer::Buffer( Buffer &&rhs ) noexcept
	:
	m_pLayoutRoot(std::move( rhs.m_pLayoutRoot )),
	m_buffer(std::move( rhs.m_buffer ))
{
	rhs.m_pLayoutRoot.reset();
	rhs.m_buffer.clear();
}

Buffer &Buffer::operator=( Buffer &&rhs ) noexcept
{
	std::swap( m_pLayoutRoot, rhs.m_pLayoutRoot );
	std::swap( m_buffer, rhs.m_buffer );
	rhs.m_pLayoutRoot.reset();
	rhs.m_buffer.clear();
	return *this;
}

Buffer::~Buffer() noexcept
{
	m_pLayoutRoot.reset();
	m_buffer.clear();
}


ElementView Buffer::operator[]( const std::string &key ) cond_noex
{
	return {&(*m_pLayoutRoot)[key], m_buffer.data(), 0u};
}

ConstElementView Buffer::operator[]( const std::string &key ) const cond_noex
{
	return const_cast<Buffer&>( *this )[key];
}

const char *Buffer::getRawBytes() const noexcept
{
	return m_buffer.data();
}

size_t Buffer::getSizeInBytes() const noexcept
{
	return m_buffer.size();
}

const CBElement &Buffer::getRootLayoutElement() const noexcept
{
	return *m_pLayoutRoot;
}

void Buffer::copyFrom( const Buffer &other ) cond_noex
{
	ASSERT( &getRootLayoutElement() == &other.getRootLayoutElement(), "Incompatible element layouts!" );
	std::copy( other.m_buffer.begin(),
		other.m_buffer.end(),
		m_buffer.begin() );
}

std::shared_ptr<CBElement> Buffer::shareLayoutRoot() const noexcept
{
	return m_pLayoutRoot;
}


// LayoutMap
con::CookedLayout LayoutMap::fetch( con::RawLayout &&cbLayout ) cond_noex
{
	auto sig = cbLayout.getSignature();
	auto &map = getInstance().m_map;
	const auto i = map.find( sig );
	// identical layout already exists
	if ( i != map.end() )
	{
		// input layout is expected to be cleared after fetch
		// so just throw away the layout tree
		cbLayout.clear();
		return {i->second};
	}
	// otherwise add layout root element to map
	auto result = map.insert( {std::move( sig ), cbLayout.commitLayout()} );
	// ASSERT( result.second, "Root layout is null!" );
	// return layout with additional reference to root
	return {result.first->second};
}

LayoutMap& LayoutMap::getInstance() noexcept
{
	static LayoutMap instance{};
	return instance;
}


}//con