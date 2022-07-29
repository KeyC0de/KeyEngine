#include "triangle_mesh.h"


TriangleMesh::TriangleMesh( const ver::Buffer &vertices,
	const std::vector<unsigned>& indices )
	:
	m_vb{vertices},
	m_indices(indices)
{
	ASSERT( m_vb.getVertexCount() > 2, "Insufficient vertices!" );
	ASSERT( m_indices.size() % 3 == 0, "indices not a multiple of 3!" );
}

void TriangleMesh::transform( const DirectX::XMMATRIX &matrix )
{
	using Elements = ver::VertexLayout::MemberType;
	for ( int i = 0; i < m_vb.getVertexCount(); ++i )
	{
		auto &pos = m_vb[i].getMember<Elements::Position3D>();
		DirectX::XMStoreFloat3( &pos,
			DirectX::XMVector3Transform( DirectX::XMLoadFloat3( &pos ), matrix ) );
	}
}

void TriangleMesh::setNormalsIndependent() cond_noex
{
	using namespace DirectX;
	using MemberType = ver::VertexLayout::MemberType;

	for ( size_t i = 0; i < m_indices.size(); i += 3 )
	{
		auto v0 = m_vb[m_indices[i]];
		auto v1 = m_vb[m_indices[i + 1]];
		auto v2 = m_vb[m_indices[i + 2]];
		const auto p0 = XMLoadFloat3( &v0.getMember<MemberType::Position3D>() );
		const auto p1 = XMLoadFloat3( &v1.getMember<MemberType::Position3D>() );
		const auto p2 = XMLoadFloat3( &v2.getMember<MemberType::Position3D>() );

		const auto n = XMVector3Normalize( XMVector3Cross( ( p1 - p0 ),
			( p2 - p0 ) ) );
		
		XMStoreFloat3( &v0.getMember<MemberType::Normal>(),
			n );
		XMStoreFloat3( &v1.getMember<MemberType::Normal>(),
			n );
		XMStoreFloat3( &v2.getMember<MemberType::Normal>(),
			n );
	}
}