#include "mesh.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "vertex_buffer.h"
#include "material_loader.h"
#include "d3d_utils.h"
#include "camera_manager.h"
#include "camera.h"
#include "d3d_utils.h"


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

void Mesh::render( const size_t channels /* = rch::all*/ ) const noexcept
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( const auto &effect : m_effects )
	{
		effect.render( *this,
			channels );
	}
}

void Mesh::setEffectEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	ASSERT( !m_effects.empty(), "No Effects to submit to the Renderer!" );
	for ( auto &effect : m_effects )
	{
		effect.setEnabled( channels,
			bEnabled );
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

DirectX::XMFLOAT4X4& Mesh::transform()
{
	return m_worldTransform;
}

const DirectX::XMMATRIX Mesh::getTransform() const noexcept
{
	return DirectX::XMLoadFloat4x4( &m_worldTransform );
}

DirectX::XMFLOAT3 Mesh::calcPosition() const noexcept
{
	return util::extractTranslation( m_worldTransform );
}

DirectX::XMMATRIX Mesh::calcPositionTr() const noexcept
{
	auto pos = calcPosition();
	return dx::XMMatrixTranslationFromVector( dx::XMLoadFloat3( &pos ) );
}

DirectX::XMFLOAT4 Mesh::calcRotationQuat() const noexcept
{
	//return dx::XMQuaternionRotationMatrix( dx::XMLoadFloat4x4( &m_worldTransform ) );
	dx::XMVECTOR rotQuatVec;
	bool res = dx::XMMatrixDecompose( nullptr,
		&rotQuatVec,
		nullptr,
		dx::XMLoadFloat4x4( &m_worldTransform ) );
	ASSERT( res, "XMMatrixDecompose did not succeed!" );

	dx::XMFLOAT4 rotQuat;
	dx::XMStoreFloat4( &rotQuat,
		rotQuatVec );
	return rotQuat;
}

DirectX::XMMATRIX Mesh::calcRotationTr() const noexcept
{
	const auto quat = calcRotationQuat();
	return dx::XMMatrixRotationRollPitchYawFromVector( dx::XMLoadFloat4( &quat ) );
}

float Mesh::calcScale() const noexcept
{
	dx::XMVECTOR scaleVec;
	bool res = dx::XMMatrixDecompose( &scaleVec,
		nullptr,
		nullptr,
		dx::XMLoadFloat4x4( &m_worldTransform ) );
	ASSERT( res, "XMMatrixDecompose did not succeed!" );

	float scale;
	dx::XMStoreFloat( &scale,
		scaleVec );
	return scale;
}

DirectX::XMMATRIX Mesh::calcScaleTr() const noexcept
{
	const float scale = calcScale();
	return dx::XMMatrixScalingFromVector( dx::XMVectorReplicate( scale ) );
}

void Mesh::setDistanceFromActiveCamera() noexcept
{
	auto pos = calcPosition();
	auto &cameraPos = CameraManager::instance().activeCamera().getPosition();
	m_distanceFromActiveCamera = util::distance( pos,
		cameraPos );
}

const float Mesh::getDistanceFromActiveCamera() const noexcept
{
	return m_distanceFromActiveCamera;
}

float Mesh::calcDistanceFromActiveCamera( const DirectX::XMFLOAT3 &pos ) const noexcept
{
	auto &cameraPos = CameraManager::instance().activeCamera().getPosition();
	return util::distance( pos,
		cameraPos );
}