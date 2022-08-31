#include "mesh.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "vertex_buffer.h"
#include "material_loader.h"


namespace dx = DirectX;

Mesh::Mesh( Graphics &gph,
	const MaterialLoader &mat,
	const aiMesh &aimesh,
	const float scale ) noexcept
{
	m_pVertexBuffer = mat.makeVertexBuffer( gph,
		aimesh,
		scale );
	m_pIndexBuffer = mat.makeIndexBuffer( gph,
		aimesh );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph );

	for ( auto &effect : mat.effects() )
	{
		addEffect( std::move( effect ) );
	}
}

//Mesh::~Mesh() noexcept
//{
//	pass_;
//}

void Mesh::update( const float dt ) cond_noex
{
	pass_;
}

void Mesh::render( const size_t channels ) const noexcept
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( const auto &effect : m_effects )
	{
		effect.render( *this,
			channels );
	}
}

void Mesh::addEffect( Effect effect ) noexcept
{
	effect.setMesh( *this );
	m_effects.emplace_back( std::move( effect ) );
}

void Mesh::bind( Graphics &gph ) const cond_noex
{
	m_pPrimitiveTopology->bind( gph );
	m_pIndexBuffer->bind( gph );
	m_pVertexBuffer->bind( gph );
}

void Mesh::accept( IEffectVisitor &ev )
{
	for ( auto &effect : m_effects )
	{
		effect.accept( ev );
	}
}

const unsigned Mesh::getIndicesCount() const cond_noex
{
	return m_pIndexBuffer->getIndexCount();
}

void Mesh::connectEffectsToRenderer( ren::Renderer &r )
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( auto &effect : m_effects )
	{
		effect.connectPass( r );
	}
}

void Mesh::setTransform( const dx::XMMATRIX &worldTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform,
		worldTransform );
}

const DirectX::XMMATRIX Mesh::getTransform() const noexcept
{
	return DirectX::XMLoadFloat4x4( &m_worldTransform );
}

void Mesh::setDistanceFromActiveCamera( const int dist ) noexcept
{
	m_distanceFromActiveCamera = dist;
}

const int Mesh::getDistanceFromActiveCamera() const noexcept
{
	return m_distanceFromActiveCamera;
}