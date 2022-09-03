#include "node.h"
#include "mesh.h"
#include "model_visitor.h"
#include "effect_visitor.h"


namespace dx = DirectX;

Node::Node( const int imguiId,
	const std::string &name,
	const dx::XMMATRIX &localTransform,
	std::vector<Mesh*> pmeshes ) cond_noex
	:
	m_imguiId(imguiId),
	m_meshes{std::move( pmeshes )},
	m_name{name}
{
	dx::XMStoreFloat4x4( &m_localTransform,
		localTransform );
	dx::XMStoreFloat4x4( &m_worldTransform,
		dx::XMMatrixIdentity() );
}

void Node::update( const float dt,
	const dx::XMMATRIX &parentWorldTransform ) const cond_noex
{
	const auto built = dx::XMLoadFloat4x4( &m_localTransform ) * dx::XMLoadFloat4x4( &m_worldTransform ) * parentWorldTransform;
	for ( const auto pMesh : m_meshes )
	{
		pMesh->setTransform( built );
		pMesh->update( dt );
		pMesh->setDistanceFromActiveCamera();
	}
	for ( const auto &pNode : m_children )
	{
		pNode->update( dt,
			built );
	}
}

void Node::render( const size_t channels ) const cond_noex
{
	for ( const auto pMesh : m_meshes )
	{
		pMesh->render( channels );
	}
	for ( const auto &pNode : m_children )
	{
		pNode->render( channels );
	}
}

const int Node::getImguiId() const noexcept
{
	return m_imguiId;
}

void Node::accept( IModelVisitor &mv )
{
	bool b = mv.visit( *this );
	if ( b )
	{
		for ( auto &node : m_children )
		{
			node->accept( mv );
		}
		mv.onVisited( *this );
	}
}

void Node::accept( IEffectVisitor &ev )
{
	for ( auto &mesh : m_meshes )
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

void Node::setWorldTransform( const dx::XMMATRIX &worldTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform,
		worldTransform );
}

const DirectX::XMFLOAT4X4& Node::getWorldTransform() const noexcept
{
	return m_worldTransform;
}

DirectX::XMFLOAT4X4& Node::worldTransform()
{
	return m_worldTransform;
}

void Node::addChild( std::unique_ptr<Node> pChild ) cond_noex
{
	ASSERT( pChild, "Node is null!" );
	m_children.emplace_back( std::move( pChild ) );
}