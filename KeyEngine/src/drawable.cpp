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

	for ( auto &effect : mat.getEffects() )
	{
		addEffect( std::move( effect ) );
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
	for ( const auto &effect : m_effects )
	{
		effect.render( *this,
			channels );
	}
}

void Drawable::addEffect( Effect effect ) noexcept
{
	effect.setParentDrawable( *this );
	m_effects.emplace_back( std::move( effect ) );
}

void Drawable::bind( Graphics &gph ) const cond_noex
{
	m_pPrimitiveTopology->bind( gph );
	m_pIndexBuffer->bind( gph );
	m_pVertexBuffer->bind( gph );
}

void Drawable::accept( IEffectVisitor &ev )
{
	for ( auto &effect : m_effects )
	{
		effect.accept( ev );
	}
}

const unsigned Drawable::getIndicesCount() const cond_noex
{
	return m_pIndexBuffer->getIndexCount();
}

void Drawable::connectEffectsToRenderer( ren::Renderer &r )
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( auto &effect : m_effects )
	{
		effect.connectPass( r );
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

const int Drawable::getDistanceFromActiveCamera() const noexcept
{
	return m_distanceFromActiveCamera;
}