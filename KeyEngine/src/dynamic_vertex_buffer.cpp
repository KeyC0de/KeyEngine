#include "dynamic_vertex_buffer.h"


namespace ver
{

const VertexInputLayout::ILElement& VertexInputLayout::getElementByIndex( const size_t i ) const cond_noex
{
	return m_vertexLayoutElements[i];
}

VertexInputLayout& VertexInputLayout::add( const ILEementType type ) cond_noex
{
	if ( !hasType( type ) )
	{
		m_vertexLayoutElements.emplace_back( type, getSizeInBytes() );
	}
	return *this;
}

bool VertexInputLayout::hasType( const ILEementType& type ) const noexcept
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

size_t VertexInputLayout::getSizeInBytes() const cond_noex
{
	return m_vertexLayoutElements.empty() ?
		0u :
		m_vertexLayoutElements.back().getOffsetAfterThis();
}

size_t VertexInputLayout::getElementCount() const noexcept
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
		tag += e.getTag();
	}
	return tag;
}

namespace lookups
{

template<VertexInputLayout::ILEementType type>
struct CPUSizeLookup final
{
	static constexpr auto exec() noexcept
	{
		return sizeof( VertexInputLayout::ElementProperties<type>::CPUType );
	}
};

template<VertexInputLayout::ILEementType type>
struct D3dDescLookup final
{
	static constexpr D3D11_INPUT_ELEMENT_DESC exec( const size_t offset ) noexcept
	{
		return {VertexInputLayout::ElementProperties<type>::hlslSemantic, 0u, VertexInputLayout::ElementProperties<type>::dxgiFormat, 0u, (unsigned)offset, D3D11_INPUT_PER_VERTEX_DATA, 0u};
	}
};

template<VertexInputLayout::ILEementType type>
struct TagLookup final
{
	static constexpr auto exec() noexcept
	{
		return VertexInputLayout::ElementProperties<type>::tag;
	}
};

template<VertexInputLayout::ILEementType type>
struct AiMeshElementLookup final
{
	static constexpr void exec( VBuffer *pBuf,
		const aiMesh &aimesh ) cond_noex
	{
		for ( auto end = aimesh.mNumVertices, i = 0u; i < end; ++i )
		{
			(*pBuf)[i].getElement<type>() = VertexInputLayout::ElementProperties<type>::extract( aimesh, i );
		}
	}
};

}

// VertexInputLayout::ILElement
VertexInputLayout::ILElement::ILElement( const ILEementType type,
	const size_t offset )
	:
	m_type(type),
	m_offset(offset)
{

}

constexpr size_t VertexInputLayout::ILElement::getElementTypeSize( const ILEementType type ) cond_noex
{
	return bridge<lookups::CPUSizeLookup>( type );
}

size_t VertexInputLayout::ILElement::getOffset() const
{
	return m_offset;
}

size_t VertexInputLayout::ILElement::getOffsetAfterThis() const cond_noex
{
	return m_offset + getElementTypeSize( m_type );
}

const VertexInputLayout::ILEementType VertexInputLayout::ILElement::getType() const noexcept
{
	return m_type;
}

const D3D11_INPUT_ELEMENT_DESC VertexInputLayout::ILElement::getD3dDesc() const cond_noex
{
	return bridge<lookups::D3dDescLookup>( m_type, getOffset() );
}

const char* ver::VertexInputLayout::ILElement::getTag() const noexcept
{
	return bridge<lookups::TagLookup>( m_type );
}


VBElementView::VBElementView( char *pData,
	const VertexInputLayout &layout ) cond_noex
	:
	m_p(pData),
	m_layout(layout)
{
	ASSERT( pData != nullptr, "Input data bytes pointer is null!" );
}

VBElementConstView::VBElementConstView( const VBElementView &v ) cond_noex
	:
	m_vertexView(v)
{

}

VBuffer::VBuffer( VertexInputLayout layout,
	const size_t vertexCount ) cond_noex
	:
	m_vertexLayout(std::move( layout ))
{
	resize( vertexCount );
}

void VBuffer::resize( const size_t newVertexCount ) cond_noex
{
	const auto vertexCount = getVertexCount();
	if ( vertexCount < newVertexCount )
	{
		m_data.resize( getSizeInBytes() + m_vertexLayout.getSizeInBytes() * ( newVertexCount - vertexCount ) );
	}
}

const char* VBuffer::getRawBytes() const cond_noex
{
	return m_data.data();
}


VBuffer::VBuffer( VertexInputLayout vertLayout,
	const aiMesh &aimesh )
	:
	m_vertexLayout(std::move( vertLayout ))
{
	resize( aimesh.mNumVertices );
	for ( size_t i = 0, end = m_vertexLayout.getElementCount(); i < end; ++i )
	{
		VertexInputLayout::bridge<lookups::AiMeshElementLookup>( m_vertexLayout.getElementByIndex( i ).getType(), this, aimesh );
	}
}

const VertexInputLayout& VBuffer::getLayout() const noexcept
{
	return m_vertexLayout;
}

size_t VBuffer::getVertexCount() const cond_noex
{
	return getSizeInBytes() / m_vertexLayout.getSizeInBytes();
}

size_t VBuffer::getSizeInBytes() const cond_noex
{
	return m_data.size();
}

VBElementView VBuffer::back() cond_noex
{
	ASSERT( !m_data.empty(), "VertexBuffer is empty!" );
	return VBElementView{m_data.data() + getSizeInBytes() - m_vertexLayout.getSizeInBytes(), m_vertexLayout};
}

VBElementConstView VBuffer::back() const cond_noex
{
	return const_cast<VBuffer*>( this )->back();
}

VBElementView VBuffer::front() cond_noex
{
	ASSERT( !m_data.empty(), "VertexBuffer is empty!" );
	return VBElementView{m_data.data(), m_vertexLayout};
}

VBElementConstView VBuffer::front() const cond_noex
{
	return const_cast<VBuffer*>( this )->front();
}

VBElementView VBuffer::operator[]( const size_t i ) cond_noex
{
	ASSERT( i < getVertexCount(), "Requested index falls out of bounds!" );
	return VBElementView{m_data.data() + m_vertexLayout.getSizeInBytes() * i, m_vertexLayout};
}

VBElementConstView VBuffer::operator[]( const size_t i ) const cond_noex
{
	return const_cast<VBuffer&>( *this )[i];
}


}//namespace ver