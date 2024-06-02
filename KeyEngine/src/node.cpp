#include "node.h"
#include "mesh.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#ifndef FINAL_RELEASE
#	include "imgui_visitors.h"
#endif


// #TODO: 1. call the transformation function
// #TODO: 2. set m_bWorldTransformNeedsUpdate = true
// updating the parent's transform should not update the transform of the child
//		the child should be affected only during update() given the built transform of the parent

namespace dx = DirectX;

Node::Node( Node *pParent,
	const int imguiId,
	const std::string &name,
	const DirectX::XMMATRIX &localTransform,
	std::vector<Mesh*> meshes )
	:
	m_imguiId(imguiId),
	m_meshes{std::move( meshes )},
	m_name{name},
	m_pParent{pParent}
{
	dx::XMStoreFloat4x4( &m_localTransform, localTransform );
	dx::XMStoreFloat4x4( &m_worldTransform, dx::XMMatrixIdentity() );

	const auto &[scale, rot, pos] = util::decomposeAffineMatrix( m_worldTransform );

	m_scalePrev = m_scale = scale;
	m_rotPrev = m_rot = rot;
	m_posPrev = m_pos = pos;

	for ( const auto pMesh : m_meshes )
	{
		pMesh->setNode( *this );
	}
}

Node::~Node() noexcept
{
	m_children.clear();
	m_meshes.clear();
}

Node::Node( Node &&rhs ) noexcept
	:
	m_bWorldTransformNeedsUpdate{rhs.m_bWorldTransformNeedsUpdate},
	m_imguiId{rhs.m_imguiId},
	m_name{std::move( rhs.m_name )},
	m_scale{rhs.m_scale},
	m_rot{rhs.m_rot},
	m_pos{rhs.m_pos},
	m_scalePrev{rhs.m_scalePrev},
	m_rotPrev{rhs.m_rotPrev},
	m_posPrev{rhs.m_posPrev},
	m_localTransform{rhs.m_localTransform},
	m_worldTransform{rhs.m_worldTransform},
	m_pParent{rhs.m_pParent},
	m_children{std::move( rhs.m_children )},
	m_meshes{std::move( rhs.m_meshes )}
{
	for ( auto *mesh : rhs.m_meshes )
	{
		mesh = nullptr;
	}
	rhs.m_pParent = nullptr;
}

Node& Node::operator=( Node &&rhs ) noexcept
{
	Node tmp{std::move( rhs )};
	swap( *this, tmp );
	return *this;
}

void swap( Node &lhs,
	Node &rhs )
{
	lhs.m_bWorldTransformNeedsUpdate = rhs.m_bWorldTransformNeedsUpdate;
	lhs.m_imguiId = rhs.m_imguiId;
	std::swap(lhs.m_name, rhs. m_name );
	lhs.m_scale = rhs.m_scale;
	lhs.m_rot = rhs.m_rot;
	lhs.m_pos = rhs.m_pos;
	lhs.m_scalePrev = rhs.m_scalePrev;
	lhs.m_rotPrev = rhs.m_rotPrev;
	lhs.m_posPrev = rhs.m_posPrev;
	lhs.m_localTransform = rhs.m_localTransform;
	lhs.m_worldTransform = rhs.m_worldTransform;
	std::swap( lhs.m_pParent, rhs.m_pParent );
	std::swap(lhs.m_children, rhs. m_children );
	std::swap( lhs.m_meshes, rhs.m_meshes );
}

void Node::update( const float dt,
	const dx::XMMATRIX &parentBuiltTransform,
	const float renderFrameInterpolation,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	if ( m_bWorldTransformNeedsUpdate )
	{
		updateWorldTransform( dt, renderFrameInterpolation, bEnableSmoothMovement );
	}

	const auto builtTransform = dx::XMLoadFloat4x4( &m_localTransform ) * dx::XMLoadFloat4x4( &m_worldTransform ) * parentBuiltTransform;

	for ( const auto pMesh : m_meshes )
	{
		pMesh->update( dt, renderFrameInterpolation );
	}
	if ( m_bWorldTransformNeedsUpdate && !m_meshes.empty() )
	{
		setWorldTransform( builtTransform );
	}

	for ( const auto &pNode : m_children )
	{
		pNode->update( dt, builtTransform, renderFrameInterpolation, bEnableSmoothMovement );
	}
}

void Node::addChild( std::unique_ptr<Node> pChild ) cond_noex
{
	ASSERT( pChild, "Invalid Node!" );
	m_children.emplace_back( std::move( pChild ) );
}

void Node::setWorldTransform( const dx::XMFLOAT4X4 &worldTransform ) cond_noex
{
	m_worldTransform = worldTransform;
	m_bWorldTransformNeedsUpdate = false;	// #TODO: could be false here not sure
}

void Node::setWorldTransform( const dx::XMMATRIX &worldTransform ) cond_noex
{
	dx::XMStoreFloat4x4( &m_worldTransform, worldTransform );
	m_bWorldTransformNeedsUpdate = false;	// #TODO: could be false here not sure
}

void Node::setWorldTransform( const float scale,
	const DirectX::XMFLOAT3 &rotAnglesRadians,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	setScale( scale );
	setRotation( rotAnglesRadians );
	setTranslation( pos );
}

void Node::setWorldTransform( const float scale,
	const DirectX::XMFLOAT4 &rotQuat,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	const auto eulerAngles = util::quaternionToEulerAngles( rotQuat );

	setScale( scale );
	setRotation( eulerAngles );
	setTranslation( pos );
}

void Node::setWorldTransform( const float scale,
	const DirectX::XMVECTOR &rotQuat,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	dx::XMFLOAT4 rotQuat4{};
	dx::XMStoreFloat4( &rotQuat4, rotQuat );
	const auto eulerAngles = util::quaternionToEulerAngles( rotQuat4 );

	setScale( scale );
	setRotation( eulerAngles );
	setTranslation( pos );
}

void Node::setScale( const float scale ) cond_noex
{
	if ( m_scale.x != scale )
	{
		dx::XMVectorReplicate( scale );
		m_bWorldTransformNeedsUpdate = true;
	}
}

void Node::setScale( const DirectX::XMFLOAT3 &scale ) cond_noex
{
	using namespace util;
	if ( m_scale != scale )
	{
		m_scale = scale;
		m_bWorldTransformNeedsUpdate = true;
	}
}

void Node::scaleRel( const DirectX::XMFLOAT3 &scale ) cond_noex
{
	ASSERT( !(scale.x == 0 && scale.y == 0 && scale.z == 0), "No reason for relative scale!" );
	m_scale.x += scale.x;
	m_scale.y += scale.y;
	m_scale.z += scale.z;
	m_bWorldTransformNeedsUpdate = true;
}

void Node::setRotation( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex
{
	using namespace util;
	if ( m_rot != rotAnglesRadians )
	{
		m_rot = rotAnglesRadians;
		m_bWorldTransformNeedsUpdate = true;
	}
}

void Node::rotateRel( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex
{
	ASSERT( !(rotAnglesRadians.x == 0 && rotAnglesRadians.y == 0 && rotAnglesRadians.z == 0), "No reason for relative rotation!" );
	m_rot.x += rotAnglesRadians.x;
	m_rot.y += rotAnglesRadians.y;
	m_rot.z += rotAnglesRadians.z;
	m_bWorldTransformNeedsUpdate = true;
}

void Node::setTranslation( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	using namespace util;
	if ( m_pos != pos )
	{
		m_pos.x = pos.x;
		m_pos.y = pos.y;
		m_pos.z = pos.z;
		m_bWorldTransformNeedsUpdate = true;
	}
}

void Node::translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	ASSERT( !(pos.x == 0 && pos.y == 0 && pos.z == 0), "No reason for relative translation!" );
	m_pos.x += pos.x;
	m_pos.y += pos.y;
	m_pos.z += pos.z;
	m_bWorldTransformNeedsUpdate = true;
}

void Node::updateWorldTransform( const float dt,
	const float renderFrameInterpolation,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	dx::XMVECTOR scaleVec{};
	dx::XMVECTOR rotQuatVec{};
	dx::XMVECTOR posVec{};

	if ( bEnableSmoothMovement )
	{
		const float alpha = 3 * dt /** renderFrameInterpolation*/;

		const dx::XMVECTOR scaleVecPrev = dx::XMLoadFloat3( &m_scalePrev );
		const dx::XMVECTOR rotQuatVecPrev = util::pitchYawRollToQuaternion( m_rotPrev );
		const dx::XMVECTOR posVecPrev = dx::XMLoadFloat3( &m_posPrev );

		scaleVec = dx::XMLoadFloat3( &m_scale );
		rotQuatVec = util::pitchYawRollToQuaternion( m_rot );
		posVec = dx::XMLoadFloat3( &m_pos );

		scaleVec = dx::XMVectorLerp( scaleVecPrev, scaleVec, alpha );
		rotQuatVec = dx::XMQuaternionSlerp( rotQuatVecPrev, rotQuatVec, alpha );
		posVec = dx::XMVectorLerp( posVecPrev, posVec, alpha );

		m_scalePrev = m_scale;
		m_rotPrev = m_rot;
		m_posPrev = m_pos;
	}
	else
	{
		scaleVec = dx::XMLoadFloat3( &m_scale );
		rotQuatVec = util::pitchYawRollToQuaternion( m_rot );
		posVec = dx::XMLoadFloat3( &m_pos );
	}

	const dx::XMMATRIX scaleMat = dx::XMMatrixScalingFromVector( scaleVec );
	const dx::XMMATRIX rotMat = dx::XMMatrixRotationQuaternion( rotQuatVec );
	const dx::XMMATRIX posMat = dx::XMMatrixTranslationFromVector( posVec );

	dx::XMStoreFloat4x4( &m_worldTransform, scaleMat * rotMat * posMat );

	m_bWorldTransformNeedsUpdate = false;
}

DirectX::XMMATRIX Node::getWorldTransform() const noexcept
{
	return dx::XMLoadFloat4x4( &m_worldTransform );
}

DirectX::XMFLOAT4X4 Node::getWorldTransform4x4() const noexcept
{
	return m_worldTransform;
}

float Node::getScale() const noexcept
{
	return m_scale.x;
}

DirectX::XMFLOAT3 Node::getScale3() const noexcept
{
	return m_scale;
}

DirectX::XMFLOAT3 Node::getRotation() const noexcept
{
	return m_rot;
}

DirectX::XMVECTOR Node::getRotationQuat() const noexcept
{
	return util::pitchYawRollToQuaternion( m_rot );
}

DirectX::XMFLOAT3 Node::getPosition() const noexcept
{
	return m_pos;
}

void Node::accept( IImguiNodeVisitor &mv )
{
	const bool bImguiNodeOpen = mv.visit( *this );
	if ( bImguiNodeOpen )
	{
		for ( auto &node : m_children )
		{
			node->accept( mv );
		}
		mv.onVisit( *this );
	}
}

void Node::accept( IImGuiConstantBufferVisitor &ev )
{
	for ( auto &mesh : m_meshes )
	{
		mesh->accept( ev );
	}
}

int Node::getImguiId() const noexcept
{
	return m_imguiId;
}

bool Node::hasChildren() const noexcept
{
	return !m_children.empty();
}

const std::string& Node::getName() const noexcept
{
	return m_name;
}

bool Node::isRoot() const noexcept
{
	return m_pParent == nullptr;
}
