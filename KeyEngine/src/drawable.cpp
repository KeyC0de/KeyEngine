#include "drawable.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "vertex_buffer.h"
#include "material_loader.h"


namespace dx = DirectX;

Drawable::Drawable( Graphics &gph,
	const MaterialLoader &mat,
	const aiMesh &aimesh,
	float scale ) noexcept
{
	m_pVertexBuffer = mat.makeVertexBuffer( gph,
		aimesh,
		scale );
	m_pIndexBuffer = mat.makeIndexBuffer( gph,
		aimesh );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph );

	for ( const auto &ef : mat.getEffects() )
	{
		addEffect( std::move( ef ) );
	}
}

Drawable::~Drawable()
{
	pass_;
}

void Drawable::update( float dt ) cond_noex
{
	pass_;
}

void Drawable::render( size_t channels ) const noexcept
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( const auto &ef : m_effects )
	{
		ef.render( *this,
			channels );
	}
}

void Drawable::addEffect( Effect ef ) noexcept
{
	ef.setParentDrawable( *this );
	m_effects.push_back( std::move( ef ) );
}

void Drawable::bind( Graphics &gph ) const cond_noex
{
	m_pPrimitiveTopology->bind( gph );
	m_pIndexBuffer->bind( gph );
	m_pVertexBuffer->bind( gph );
}

void Drawable::accept( IEffectVisitor &ev )
{
	for ( auto &ef : m_effects )
	{
		ef.accept( ev );
	}
}

unsigned Drawable::getIndicesCount() const cond_noex
{
	return m_pIndexBuffer->getIndexCount();
}

void Drawable::connectEffectsToRenderer( ren::Renderer &r )
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( auto &ef : m_effects )
	{
		ef.connectPass( r );
	}
}

void Drawable::setTransform( const dx::XMMATRIX &worldTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform,
		worldTransform );
}

DirectX::XMMATRIX Drawable::getTransform() const noexcept
{
	return DirectX::XMLoadFloat4x4( &m_worldTransform );
}

void Drawable::setDistanceFromActiveCamera( int dist ) noexcept
{
	m_distanceFromActiveCamera = dist;
}

int Drawable::getDistanceFromActiveCamera() const noexcept
{
	return m_distanceFromActiveCamera;
}