#include "node.h"
#include "drawable.h"
#include "model_visitor.h"


namespace dx = DirectX;

Node::Node( int id,
	const std::string& name,
	std::vector<Drawable*> pDrawables,
	const dx::XMMATRIX& localTransform ) cond_noex
	:
	m_id(id),
	m_pDrawables{std::move( pDrawables )},
	m_name{name}
{
	dx::XMStoreFloat4x4( &m_localTransform,
		localTransform );
	dx::XMStoreFloat4x4( &m_worldTransform,
		dx::XMMatrixIdentity() );
}

void Node::update( float dt,
	const dx::XMMATRIX& worldTransform ) const cond_noex
{
	const auto built = dx::XMLoadFloat4x4( &m_worldTransform )
		* dx::XMLoadFloat4x4( &m_localTransform ) * worldTransform;
	for ( const auto pm : m_pDrawables )
	{
		pm->setTransform( built );
		pm->update( dt );
	}
	for ( const auto& pn : m_children )
	{
		pn->update( dt,
			built );
	}
}

void Node::render( size_t channels ) const cond_noex
{
	for ( const auto pm : m_pDrawables )
	{
		pm->render( channels );
	}
	for ( const auto& pn : m_children )
	{
		pn->render( channels );
	}
}

void Node::addChild( std::unique_ptr<Node> pChild ) cond_noex
{
	ASSERT( pChild, "Node is null!" );
	m_children.push_back( std::move( pChild ) );
}

void Node::setTransform( const dx::XMMATRIX& worldTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform,
		worldTransform );
}

const dx::XMFLOAT4X4& Node::getWorldTransform() const noexcept
{
	return m_worldTransform;
}

int Node::getId() const noexcept
{
	return m_id;
}

void Node::accept( IModelVisitor& mv )
{
	bool b = mv.visit( *this );
	if ( b )
	{
		for ( auto& node : m_children )
		{
			node->accept( mv );
		}
		mv.onNodeLeave( *this );
	}
}

void Node::accept( IEffectVisitor& ev )
{
	for ( auto& mesh : m_pDrawables )
	{
		mesh->accept( ev );
	}
}

bool Node::hasChildren() const noexcept
{
	return !m_children.empty();
}

const std::string& Node::getName() const noexcept
{
	return m_name;
}