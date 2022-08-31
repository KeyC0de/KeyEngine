#define VB_IMPL_SOURCE

#include "dynamic_vertex_buffer.h"


namespace ver
{

const VertexInputLayout::VertexInputLayoutElement& VertexInputLayout::getElementByIndex( const size_t i ) const cond_noex
{
	return m_vertexLayoutElements[i];
}

VertexInputLayout& VertexInputLayout::add( const VertexInputLayoutElementType type ) cond_noex
{
	if ( !hasType( type ) )
	{
		m_vertexLayoutElements.emplace_back( type,
			getSizeInBytes() );
	}
	return *this;
}

bool VertexInputLayout::hasType( const VertexInputLayoutElementType& type ) const noexcept
{
	for ( auto &e : m_vertexLayoutElements )
	{
		if ( e.getType() == type )
		{
			return true;
		}
	}
	return false;
}

const size_t VertexInputLayout::getSizeInBytes() const cond_noex
{
	return m_vertexLayoutElements.empty() ?
		0u :
		m_vertexLayoutElements.back().getByteOffsetAfterThisInLayout();
}

const size_t VertexInputLayout::getElementCount() const noexcept
{
	return m_vertexLayoutElements.size();
}

std::vector<D3D11_INPUT_ELEMENT_DESC> VertexInputLayout::getD3DInputElementDescs() const cond_noex
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
	desc.reserve( getElementCount() );
	for ( const auto &e : m_vertexLayoutElements )
	{
		desc.push_back( e.getD3dDesc() );
	}
	return desc;
}

std::string VertexInputLayout::calcSignature() const cond_noex
{
	std::string tag;
	for ( const auto &e : m_vertexLayoutElements )
	{
		tag += e.calcTag();
	}
	return tag;
}

// VertexInputLayout::VertexInputLayoutElement
VertexInputLayout::VertexInputLayoutElement::VertexInputLayoutElement( const VertexInputLayoutElementType type,
	const size_t offset )
	:
	m_type(type),
	m_offset(offset)
{

}

const size_t VertexInputLayout::VertexInputLayoutElement::getByteOffsetAfterThisInLayout() const cond_noex
{
	return m_offset + getTypeSize( m_type );
}

const size_t VertexInputLayout::VertexInputLayoutElement::getByteOffsetInLayout() const
{
	return m_offset;
}

const VertexInputLayout::VertexInputLayoutElementType VertexInputLayout::VertexInputLayoutElement::getType() const noexcept
{
	return m_type;
}

template<VertexInputLayout::VertexInputLayoutElementType type>
struct CPUSizeLookup final
{
	static constexpr auto exec() noexcept
	{
		return sizeof( VertexInputLayout::VertexInputLayoutElementProperties<type>::CPUType );
	}
};

constexpr size_t VertexInputLayout::VertexInputLayoutElement::getTypeSize( const VertexInputLayoutElementType type ) cond_noex
{
	return bridge<CPUSizeLookup>( type );
}

template<VertexInputLayout::VertexInputLayoutElementType type>
struct TagLookup final
{
	static constexpr auto exec() noexcept
	{
		return VertexInputLayout::VertexInputLayoutElementProperties<type>::tag;
	}
};

const char* ver::VertexInputLayout::VertexInputLayoutElement::calcTag() const noexcept
{
	return bridge<TagLookup>( m_type );
}

template<VertexInputLayout::VertexInputLayoutElementType type>
struct DescLookup final
{
	static constexpr D3D11_INPUT_ELEMENT_DESC exec( const size_t offset ) noexcept
	{
		return {VertexInputLayout::VertexInputLayoutElementProperties<type>::hlslSemantic,
			0u,
			VertexInputLayout::VertexInputLayoutElementProperties<type>::dxgiFormat,
			0u,
			(unsigned)offset,
			D3D11_INPUT_PER_VERTEX_DATA,
			0u};
	}
};

const D3D11_INPUT_ELEMENT_DESC VertexInputLayout::VertexInputLayoutElement::getD3dDesc() const cond_noex
{
	return bridge<DescLookup>( m_type,
		getByteOffsetInLayout() );
}


VertexView::VertexView( char *pData,
	const VertexInputLayout &layout ) cond_noex
	:
	m_p(pData),
	m_layout(layout)
{
	ASSERT( pData != nullptr, "Input data bytes pointer is null!" );
}

ConstVertexView::ConstVertexView( const VertexView &v ) cond_noex
	:
	m_vertexView(v)
{

}

Buffer::Buffer( VertexInputLayout layout,
	const size_t size ) cond_noex
	:
	m_vertexLayout(std::move( layout ))
{
	resize( size );
}

void Buffer::resize( const size_t newSize ) cond_noex
{
	const auto size = getVertexCount();
	if ( size < newSize )
	{
		m_data.resize( getSizeInBytes() + m_vertexLayout.getSizeInBytes() * ( newSize - size ) );
	}
}

const char* Buffer::getRawBytes() const cond_noex
{
	return m_data.data();
}


template<VertexInputLayout::VertexInputLayoutElementType type>
struct AiMeshElementLookup final
{
	static constexpr void exec( Buffer *pBuf,
		const aiMesh &aimesh ) cond_noex
	{
		for ( auto end = aimesh.mNumVertices, i = 0u; i < end; ++i )
		{
			(*pBuf)[i].element<type>() = VertexInputLayout::VertexInputLayoutElementProperties<type>::extract( aimesh,
				i );
		}
	}
};

Buffer::Buffer( VertexInputLayout vertLayout,
	const aiMesh &aimesh )
	:
	m_vertexLayout(std::move( vertLayout ))
{
	resize( aimesh.mNumVertices );
	for ( size_t i = 0, end = m_vertexLayout.getElementCount(); i < end; ++i )
	{
		VertexInputLayout::bridge<AiMeshElementLookup>( m_vertexLayout.getElementByIndex( i ).getType(),
			this,
			aimesh );
	}
}

const VertexInputLayout& Buffer::getLayout() const noexcept
{
	return m_vertexLayout;
}

const size_t Buffer::getVertexCount() const cond_noex
{
	return getSizeInBytes() / m_vertexLayout.getSizeInBytes();
}

const size_t Buffer::getSizeInBytes() const cond_noex
{
	return m_data.size();
}

VertexView Buffer::back() cond_noex
{
	ASSERT( !m_data.empty(), "VertexBuffer is empty!" );
	return VertexView{m_data.data() + getSizeInBytes() - m_vertexLayout.getSizeInBytes(), m_vertexLayout};
}

ConstVertexView Buffer::back() const cond_noex
{
	return const_cast<Buffer*>( this )->back();
}

VertexView Buffer::front() cond_noex
{
	ASSERT( !m_data.empty(), "VertexBuffer is empty!" );
	return VertexView{m_data.data(), m_vertexLayout};
}

ConstVertexView Buffer::front() const cond_noex
{
	return const_cast<Buffer*>( this )->front();
}

VertexView Buffer::operator[]( const size_t i ) cond_noex
{
	ASSERT( i < getVertexCount(), "Requested index falls out of bounds!" );
	return VertexView{m_data.data() + m_vertexLayout.getSizeInBytes() * i,	m_vertexLayout};
}

ConstVertexView Buffer::operator[]( const size_t i ) const cond_noex
{
	return const_cast<Buffer&>( *this )[i];
}


}//ver