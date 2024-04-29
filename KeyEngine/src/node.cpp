#include "node.h"
#include "mesh.h"
#include "imgui_visitors.h"


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
	dx::XMStoreFloat4x4( &m_localTransform, localTransform );
	dx::XMStoreFloat4x4( &m_worldTransform, dx::XMMatrixIdentity() );
}

void Node::update( const float dt,
	const dx::XMMATRIX &parentWorldTransform ) const cond_noex
{
	const auto builtTr = dx::XMLoadFloat4x4( &m_localTransform ) * dx::XMLoadFloat4x4( &m_worldTransform ) *  parentWorldTransform;
	for ( const auto pMesh : m_meshes )
	{
		pMesh->setTransform( builtTr );
		pMesh->update( dt );
	}
	for ( const auto &pNode : m_children )
	{
		pNode->update( dt, builtTr );
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

void Node::setEffectEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	for ( const auto pMesh : m_meshes )
	{
		pMesh->setEffectEnabled( channels, bEnabled );
	}
	for ( const auto &pNode : m_children )
	{
		pNode->setEffectEnabled( channels, bEnabled );
	}
}

int Node::getImguiId() const noexcept
{
	return m_imguiId;
}

void Node::accept( IImguiNodeVisitor &mv )
{
	bool b = mv.visit( *this );
	if ( b )
	{
		for ( auto &node : m_children )
		{
			node->accept( mv );
		}
		mv.onVisit( *this );
	}
}

void Node::accept( IImGuiEffectVisitor &ev )
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

void Node::setWorldTransform( const dx::XMFLOAT4X4 &worldTransform ) noexcept
{
	m_worldTransform = worldTransform;
	for ( const auto pMesh : m_meshes )
	{
		pMesh->setTransform( worldTransform );
	}
}

void Node::setWorldTransform( const dx::XMMATRIX &worldTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform, worldTransform );
	for ( const auto pMesh : m_meshes )
	{
		pMesh->setTransform( worldTransform );
	}
}

DirectX::XMMATRIX Node::getWorldTransform() const noexcept
{
	return dx::XMLoadFloat4x4( &m_worldTransform );
}

DirectX::XMFLOAT4X4& Node::getWorldTransformAccess()
{
	return m_worldTransform;
}

void Node::setCulled( const bool bCulled )
{
	for ( const auto pMesh : m_meshes )
	{
		pMesh->setCulled( bCulled );
	}
	for ( const auto &pNode : m_children )
	{
		pNode->setCulled( bCulled );
	}
}

void Node::addChild( std::unique_ptr<Node> pChild ) cond_noex
{
	ASSERT( pChild, "Invalid Node!" );
	m_children.emplace_back( std::move( pChild ) );
}