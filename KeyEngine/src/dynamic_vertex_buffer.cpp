#define VB_IMPL_SOURCE

#include "dynamic_vertex_buffer.h"


namespace ver
{

const VertexLayout::VertexMember& VertexLayout::fetchByIndex( size_t i ) const cond_noex
{
	return m_vertexMembers[i];
}

VertexLayout& VertexLayout::add( MemberType type ) cond_noex
{
	if ( !hasType( type ) )
	{
		m_vertexMembers.emplace_back( type,
			getSizeInBytes() );
	}
	return *this;
}

bool VertexLayout::hasType( MemberType type ) const noexcept
{
	for ( auto &e : m_vertexMembers )
	{
		if ( e.getType() == type )
		{
			return true;
		}
	}
	return false;
}

size_t VertexLayout::getSizeInBytes() const cond_noex
{
	return m_vertexMembers.empty() ?
		0u :
		m_vertexMembers.back().getOffsetAfterThisInLayout();
}

size_t VertexLayout::getMemberCount() const noexcept
{
	return m_vertexMembers.size();
}

std::vector<D3D11_INPUT_ELEMENT_DESC>
	VertexLayout::getD3DInputElementDescs() const cond_noex
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
	desc.reserve( getMemberCount() );
	for ( const auto &e : m_vertexMembers )
	{
		desc.push_back( e.getDESC() );
	}
	return desc;
}

std::string VertexLayout::getSignature() const cond_noex
{
	std::string tag;
	for ( const auto &e : m_vertexMembers )
	{
		tag += e.getTag();
	}
	return tag;
}

// VertexLayout::VertexMember
VertexLayout::VertexMember::VertexMember( MemberType type,
	size_t offset )
	:
	m_type(type),
	m_offset(offset)
{

}

size_t VertexLayout::VertexMember::getOffsetAfterThisInLayout() const cond_noex
{
	return m_offset + getTypeSize( m_type );
}

size_t VertexLayout::VertexMember::getOffsetInLayout() const
{
	return m_offset;	// in Bytes
}

VertexLayout::MemberType VertexLayout::VertexMember::getType() const noexcept
{
	return m_type;
}

template<VertexLayout::MemberType type>
struct CPUSizeLookup final
{
	static constexpr auto exec() noexcept
	{
		return sizeof( VertexLayout::MemberProperties<type>::CPUType );
	}
};

constexpr size_t VertexLayout::VertexMember::getTypeSize( MemberType type ) cond_noex
{
	return bridge<CPUSizeLookup>( type );
}

template<VertexLayout::MemberType type>
struct TagLookup final
{
	static constexpr auto exec() noexcept
	{
		return VertexLayout::MemberProperties<type>::tag;
	}
};

const char *ver::VertexLayout::VertexMember::getTag() const noexcept
{
	return bridge<TagLookup>( m_type );
}

template<VertexLayout::MemberType type>
struct DescLookup final
{
	static constexpr D3D11_INPUT_ELEMENT_DESC exec( size_t offset ) noexcept
	{
		return {VertexLayout::MemberProperties<type>::hlslSemantic,
			0u,
			VertexLayout::MemberProperties<type>::dxgiFormat,
			0u,
			(unsigned)offset,
			D3D11_INPUT_PER_VERTEX_DATA,
			0u};
	}
};

D3D11_INPUT_ELEMENT_DESC VertexLayout::VertexMember::getDESC() const cond_noex
{
	return bridge<DescLookup>( m_type,
		getOffsetInLayout() );
}


VertexView::VertexView( char *pData,
	const VertexLayout &layout ) cond_noex
	:
	m_p(pData),
	m_layout(layout)
{
	ASSERT( pData != nullptr, "Input data bytes pointer is null!" );
}

ConstVertexView::ConstVertexView( const VertexView &v ) cond_noex
	:
	m_vertex(v)
{

}

Buffer::Buffer( VertexLayout layout,
	size_t size ) cond_noex
	:
	m_vertexLayout(std::move( layout ))
{
	resize( size );
}

void Buffer::resize( size_t newSize ) cond_noex
{
	const auto size = getVertexCount();
	if ( size < newSize )
	{
		m_v.resize( m_v.size() + m_vertexLayout.getSizeInBytes() * ( newSize - size ) );
	}
}

const char* Buffer::getRawBytes() const cond_noex
{
	return m_v.data();
}


template<VertexLayout::MemberType type>
struct AiMeshMemberLookup final
{
	static constexpr void exec( Buffer *pBuf,
		const aiMesh &aimesh ) cond_noex
	{
		for ( auto end = aimesh.mNumVertices, i = 0u; i < end; ++i )
		{
			(*pBuf)[i].getMember<type>()
				= VertexLayout::MemberProperties<type>::extract( aimesh,
					i );
		}
	}
};

Buffer::Buffer( VertexLayout vertLayout,
	const aiMesh &aimesh )
	:
	m_vertexLayout(std::move( vertLayout ))
{
	resize( aimesh.mNumVertices );
	for ( size_t i = 0, end = m_vertexLayout.getMemberCount();
		i < end;
		++i )
	{
		VertexLayout::bridge<AiMeshMemberLookup>(
				m_vertexLayout.fetchByIndex( i ).getType(),
			this,
			aimesh );
	}
}

const VertexLayout& Buffer::getLayout() const noexcept
{
	return m_vertexLayout;
}

size_t Buffer::getVertexCount() const cond_noex
{
	return m_v.size() / m_vertexLayout.getSizeInBytes();
}

size_t Buffer::getSizeInBytes() const cond_noex
{
	return m_v.size();
}

VertexView Buffer::back() cond_noex
{
	ASSERT( !m_v.empty(), "VertexBuffer is empty!" );
	return VertexView{m_v.data() + m_v.size() - m_vertexLayout.getSizeInBytes(),
		m_vertexLayout};
}

VertexView Buffer::front() cond_noex
{
	ASSERT( !m_v.empty(), "VertexBuffer is empty!" );
	return VertexView{m_v.data(), m_vertexLayout};
}

VertexView Buffer::operator[]( size_t i ) cond_noex
{
	ASSERT( i < getVertexCount(), "Requested index falls out of bounds!" );
	return VertexView{m_v.data() + m_vertexLayout.getSizeInBytes() * i,	m_vertexLayout};
}

ConstVertexView Buffer::back() const cond_noex
{
	return const_cast<Buffer*>( this )->back();
}

ConstVertexView Buffer::front() const cond_noex
{
	return const_cast<Buffer*>( this )->front();
}

ConstVertexView Buffer::operator[]( size_t i ) const cond_noex
{
	return const_cast<Buffer&>( *this )[i];
}


}//ver