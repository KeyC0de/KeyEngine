#include "triangle_mesh.h"


namespace dx = DirectX;

TriangleMesh::TriangleMesh( const ver::VBuffer &vertices,
	const std::vector<unsigned> &indices )
	:
	m_vb{vertices},
	m_indices(indices)
{
	ASSERT( m_vb.getVertexCount() > 2, "Insufficient vertices!" );
	ASSERT( m_indices.size() % 3 == 0, "indices not a multiple of 3!" );
}

void TriangleMesh::transform( const dx::XMMATRIX &matrix )
{
	using ElementType = ver::VertexInputLayout::ILEementType;
	for ( int i = 0; i < m_vb.getVertexCount(); ++i )
	{
		auto &pos = m_vb[i].element<ElementType::Position3D>();
		dx::XMStoreFloat3( &pos,
			dx::XMVector3Transform( dx::XMLoadFloat3( &pos ), matrix ) );
	}
}

void TriangleMesh::setFlatShadedIndependentNormals() cond_noex
{
	using namespace DirectX;
	using ILEementType = ver::VertexInputLayout::ILEementType;

	for ( size_t i = 0; i < m_indices.size(); i += 3 )
	{
		auto v0 = m_vb[m_indices[i]];
		auto v1 = m_vb[m_indices[i + 1]];
		auto v2 = m_vb[m_indices[i + 2]];
		const auto p0 = XMLoadFloat3( &v0.element<ILEementType::Position3D>() );
		const auto p1 = XMLoadFloat3( &v1.element<ILEementType::Position3D>() );
		const auto p2 = XMLoadFloat3( &v2.element<ILEementType::Position3D>() );

		const auto n = XMVector3Normalize( XMVector3Cross( p1 - p0,
			p2 - p0 ) );

		XMStoreFloat3( &v0.element<ILEementType::Normal>(),
			n );
		XMStoreFloat3( &v1.element<ILEementType::Normal>(),
			n );
		XMStoreFloat3( &v2.element<ILEementType::Normal>(),
			n );
	}
}