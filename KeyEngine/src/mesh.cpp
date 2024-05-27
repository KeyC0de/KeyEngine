#include "mesh.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "material_loader.h"
#include "d3d_utils.h"
#include "camera_manager.h"
#include "camera.h"
#include "d3d_utils.h"


// #TODO: Model LOD automatic switching
namespace mesh
{

static unsigned g_numMeshes = 0u;

}

namespace dx = DirectX;

Mesh::Mesh( Graphics &gfx,
	const MaterialLoader &mat,
	const aiMesh &aimesh,
	const float initialScale /*= 1.0f*/ ) noexcept
{
	m_pVertexBuffer = mat.makeVertexBuffer( gfx, aimesh, initialScale );
	m_pIndexBuffer = mat.makeIndexBuffer( gfx, aimesh );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	for ( auto &material : mat.getMaterial() )
	{
		addMaterial( std::move( material ) );
	}

	createAabb( aimesh );
	setMeshId();
}

Mesh::Mesh( const DirectX::XMFLOAT3 &initialScale,
	const DirectX::XMFLOAT3 &initialRot /*= {0, 0, 0}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0, 0, 0}*/ ) noexcept
{
	const dx::XMMATRIX scaleMat = dx::XMMatrixScaling( initialScale.x, initialScale.y, initialScale.z );
	const dx::XMMATRIX rotMat = dx::XMMatrixRotationRollPitchYaw( initialRot.x, initialRot.y, initialRot.z );
	const dx::XMMATRIX posMat = dx::XMMatrixTranslation( initialPos.x, initialPos.y, initialPos.z );
	dx::XMStoreFloat4x4( &m_worldTransform, scaleMat * rotMat * posMat );
}

Mesh::Mesh( const DirectX::XMMATRIX &initialTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform, initialTransform );
}

//Mesh::~Mesh() noexcept
//{
//	pass_;
//}

void Mesh::update( const float dt,
	const float renderFrameInterpolation ) cond_noex
{
	setDistanceFromActiveCamera();
}

void Mesh::render( const size_t channels /* = rch::all*/ ) const noexcept
{
	ASSERT( !m_materials.empty(), "No Materials to submit to the Renderer!" );
	ASSERT( m_meshId != 0, "Mesh not initialized properly!" );

	m_bRenderedThisFrame = !isFrustumCulled();
	if ( m_bRenderedThisFrame )
	{
		for ( const auto &material : m_materials )
		{
			material.render( *this, channels );
		}
	}
}

void Mesh::setMaterialEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	ASSERT( !m_materials.empty(), "No Materials to submit to the Renderer!" );
	for ( auto &material : m_materials )
	{
		material.setEnabled( channels, bEnabled );
	}
}

void Mesh::addMaterial( Material material ) noexcept
{
	material.setMesh( *this );
	m_materials.emplace_back( std::move( material ) );
}

void Mesh::bind( Graphics &gfx ) const cond_noex
{
	m_pVertexBuffer->bind( gfx );
	m_pIndexBuffer->bind( gfx );
	m_pPrimitiveTopology->bind( gfx );
}

void Mesh::accept( IImGuiMaterialVisitor &ev )
{
	for ( auto &material : m_materials )
	{
		material.accept( ev );
	}
}

unsigned Mesh::getIndicesCount() const cond_noex
{
	return m_pIndexBuffer->getIndexCount();
}

void Mesh::connectMaterialsToRenderer( ren::Renderer &r )
{
	ASSERT( !m_materials.empty(), "No Materials to submit to the Renderer!" );
	for ( auto &material : m_materials )
	{
		material.connectPass( r );
	}
}

void Mesh::setTransform( const dx::XMMATRIX &worldTransform ) noexcept
{
	dx::XMStoreFloat4x4( &m_worldTransform, worldTransform );
}

void Mesh::setTransform( const dx::XMFLOAT4X4 &worldTransform ) noexcept
{
	m_worldTransform = worldTransform;
}

void Mesh::setScale( const DirectX::XMFLOAT3 &scale ) cond_noex
{
	const auto &currentTransform = dx::XMLoadFloat4x4( &m_worldTransform );
	dx::XMVECTOR currentScale{}, currentRotQuat{}, currentPos{};
	const bool ret = dx::XMMatrixDecompose( &currentScale, &currentRotQuat, &currentPos, currentTransform );
	ASSERT( ret, "Matrix decomposition failed!" );

	const dx::XMMATRIX scaleMat = dx::XMMatrixScaling( scale.x, scale.y, scale.z );
	const dx::XMMATRIX rotMat = dx::XMMatrixRotationQuaternion( currentRotQuat );
	const dx::XMMATRIX posMat = dx::XMMatrixTranslationFromVector( currentPos );
	dx::XMStoreFloat4x4( &m_worldTransform, scaleMat * rotMat * posMat );
}

void Mesh::setRotation( const DirectX::XMFLOAT3 &rot ) cond_noex
{
	const auto &currentTransform = dx::XMLoadFloat4x4( &m_worldTransform );
	dx::XMVECTOR currentScale{}, currentRotQuat{}, currentPos{};
	const bool ret = dx::XMMatrixDecompose( &currentScale, &currentRotQuat, &currentPos, currentTransform );
	ASSERT( ret, "Matrix decomposition failed!" );

	const dx::XMMATRIX scaleMat = dx::XMMatrixScalingFromVector( currentScale );
	const dx::XMMATRIX rotMat = dx::XMMatrixRotationRollPitchYaw( rot.x, rot.y, rot.z );
	const dx::XMMATRIX posMat = dx::XMMatrixTranslationFromVector( currentPos );
	dx::XMStoreFloat4x4( &m_worldTransform, scaleMat * rotMat * posMat );
}

void Mesh::setPosition( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	m_worldTransform._41 = pos.x;
	m_worldTransform._42 = pos.y;
	m_worldTransform._43 = pos.z;
}

DirectX::XMMATRIX Mesh::getTransform() const noexcept
{
	return dx::XMLoadFloat4x4( &m_worldTransform );
}

float Mesh::getScale() const noexcept
{
	dx::XMVECTOR currentScale{}, currentRotQuat{}, currentPos{};
	const bool ret = dx::XMMatrixDecompose( &currentScale, &currentRotQuat, &currentPos, dx::XMLoadFloat4x4( &m_worldTransform ) );
	ASSERT( ret, "Matrix decomposition failed!" );

	float scale;
	dx::XMStoreFloat( &scale, currentScale );
	return scale;
}

DirectX::XMFLOAT3 Mesh::getRotation() const noexcept
{
	return util::extractRotation( m_worldTransform );
}

DirectX::XMFLOAT3 Mesh::getPosition() const noexcept
{
	return util::extractTranslation( m_worldTransform );
}

void Mesh::setDistanceFromActiveCamera() noexcept
{
	const auto pos = getPosition();
	const auto &cameraPos = CameraManager::getInstance().getActiveCamera().getPosition();
	m_distanceFromActiveCamera = util::distance( pos, cameraPos );
}

float Mesh::getDistanceFromActiveCamera() const noexcept
{
	return m_distanceFromActiveCamera;
}

bool Mesh::isRenderedThisFrame() const noexcept
{
	return m_bRenderedThisFrame;
}

void Mesh::createAabb( const ver::VBuffer &verts )
{
	dx::XMFLOAT3 minVertex{FLT_MAX, FLT_MAX, FLT_MAX};
	dx::XMFLOAT3 maxVertex{-FLT_MAX, -FLT_MAX, -FLT_MAX};

	const auto n = verts.getVertexCount();
	using Type = ver::VertexInputLayout::ILEementType;
	for ( int i = 0; i < n; ++i )
	{
		const auto vertex = verts[i].getElement<Type::Position3D>();

		minVertex.x = std::min( minVertex.x, vertex.x );
		minVertex.y = std::min( minVertex.y, vertex.y );
		minVertex.z = std::min( minVertex.z, vertex.z );

		maxVertex.x = std::max( maxVertex.x, vertex.x );
		maxVertex.y = std::max( maxVertex.y, vertex.y );
		maxVertex.z = std::max( maxVertex.z, vertex.z );
	}

	m_aabb = std::make_pair( minVertex, maxVertex );
}

float Mesh::getDistanceFromActiveCamera( const DirectX::XMFLOAT3 &pos ) const noexcept
{
	const auto &cameraPos = CameraManager::getInstance().getActiveCamera().getPosition();
	return util::distance( pos, cameraPos );
}

void Mesh::setMeshId() noexcept
{
	++mesh::g_numMeshes;
	m_meshId = mesh::g_numMeshes;
}

void Mesh::createAabb( const aiMesh &aiMesh )
{
	dx::XMFLOAT3 minVertex{FLT_MAX, FLT_MAX, FLT_MAX};
	dx::XMFLOAT3 maxVertex{-FLT_MAX, -FLT_MAX, -FLT_MAX};

	unsigned int n = aiMesh.mNumVertices;
	for ( unsigned int i = 0; i < n; ++i )
	{
		// the minVertex and maxVertex will most likely not be actual vertices in the model, but vertices that use the smallest and largest x, y, z coords
		// from the model to make sure ALL vertices are covered by the bounding volume

		// get the smallest vertex
		minVertex.x = std::min(minVertex.x, aiMesh.mVertices[i].x);
		minVertex.y = std::min(minVertex.y, aiMesh.mVertices[i].y);
		minVertex.z = std::min(minVertex.z, aiMesh.mVertices[i].z);

		// get the largest vertex
		maxVertex.x = std::max(maxVertex.x, aiMesh.mVertices[i].x);
		maxVertex.y = std::max(maxVertex.y, aiMesh.mVertices[i].y);
		maxVertex.z = std::max(maxVertex.z, aiMesh.mVertices[i].z);
	}

	m_aabb = std::make_pair( minVertex, maxVertex );
}

bool Mesh::isFrustumCulled() const noexcept
{
	const std::vector<DirectX::XMFLOAT4> &frustumPlanes = CameraManager::getInstance().getActiveCamera().getFrustumPlanes();
	const int numPlanes = static_cast<const int>( frustumPlanes.size() );
	ASSERT( numPlanes == 6, "Invalid number of planes!" );

	const auto &minVertex = m_aabb.first;
	const auto &maxVertex = m_aabb.second;

	const auto pos = getPosition();

	for ( int i = 0; i < numPlanes; ++i )
	{
		// AABB vertex furthest away from the direction the plane normal is facing
		dx::XMFLOAT3 axisVert{};

		// along the ...
		// ... x-axis
		if ( frustumPlanes[i].x < 0.0f )
		{
			axisVert.x = minVertex.x + pos.x;
		}
		else
		{
			axisVert.x = maxVertex.x + pos.x;
		}

		// ... y-axis
		if ( frustumPlanes[i].y < 0.0f )
		{
			axisVert.y = minVertex.y + pos.y;
		}
		else
		{
			axisVert.y = maxVertex.y + pos.y;
		}

		// ... z-axis
		if ( frustumPlanes[i].z < 0.0f )
		{
			axisVert.z = minVertex.z + pos.z;
		}
		else
		{
			axisVert.z = maxVertex.z + pos.z;
		}

		const dx::XMVECTOR planeNormal{dx::XMVectorSet( frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z, 0.0f )};
		const float planeConstant = frustumPlanes[i].w;

		// get the signed distance from the AABB vertex that's furthest down the frustum planes normal, and if the signed distance is negative
		//	then the entire bounding box is behind the frustum plane, which means that it should be culled
		if ( dx::XMVectorGetX( dx::XMVector3Dot( planeNormal, dx::XMLoadFloat3( &axisVert ) ) ) + planeConstant < 0.0f )
		{
			return true;
		}
	}

	return false;
}