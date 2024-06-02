#include "mesh.h"
#include "node.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "material_loader.h"
#include "camera_manager.h"
#include "camera.h"
#include "settings_manager.h"
#include "utils.h"
#include "d3d_utils.h"


namespace mesh
{

static unsigned g_numMeshes = 0u;

}

namespace dx = DirectX;

Mesh::Mesh( Graphics &gfx,
	const MaterialLoader &mat,
	const aiMesh &aimesh,
	const float initialScale /*= 1.0f*/ )
{
	m_pVertexBuffer = mat.makeVertexBuffer( gfx, aimesh, initialScale );
	m_pIndexBuffer = mat.makeIndexBuffer( gfx, aimesh );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, 0u, *this );

	for ( auto &material : mat.getMaterial() )
	{
		addMaterial( std::move( material ) );
	}

	createAabb( aimesh );
	setMeshId();
}

Mesh::~Mesh() noexcept
{
	m_materials.clear();
	m_pTransformVscb.reset();
	m_pPrimitiveTopology.reset();
	m_pIndexBuffer.reset();
	m_pVertexBuffer.reset();
	m_pNode = nullptr;
	m_meshId = 0u;
	m_bRenderedThisFrame = false;
	m_distanceFromActiveCamera = -1.0f;
}

Mesh::Mesh( Mesh &&rhs ) noexcept
	:
	m_distanceFromActiveCamera{rhs.m_distanceFromActiveCamera},
	m_bRenderedThisFrame{rhs.m_bRenderedThisFrame},
	m_meshId{rhs.m_meshId},
	m_pNode{rhs.m_pNode},
	m_aabb{rhs.m_aabb},
	m_pVertexBuffer{std::move( rhs.m_pVertexBuffer )},		// TODO: ???
	m_pIndexBuffer{std::move( rhs.m_pIndexBuffer )},
	m_pPrimitiveTopology{std::move( rhs.m_pPrimitiveTopology )},
	m_pTransformVscb{std::move( rhs.m_pTransformVscb )},
	m_materials{std::move( rhs.m_materials )}
{
	rhs.m_pNode = nullptr;
	rhs.m_pVertexBuffer = nullptr;
	rhs.m_pIndexBuffer = nullptr;
	rhs.m_pPrimitiveTopology = nullptr;
}

void Mesh::setNode( Node &node )
{
	m_pNode = &node;
}

void Mesh::update( const float dt,
	const float renderFrameInterpolation ) cond_noex
{
	//ASSERT( m_pNode, "Critical: Node absent!" );	// #TODO: use only for Mesh Nodes
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

void Mesh::bind( Graphics &gfx ) const cond_noex
{
	m_pVertexBuffer->bind( gfx );
	m_pIndexBuffer->bind( gfx );
	m_pPrimitiveTopology->bind( gfx );
	m_pTransformVscb->bind( gfx );
}

void Mesh::addMaterial( Material material ) noexcept
{
	material.setMesh( *this );
	m_materials.emplace_back( std::move( material ) );
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

void Mesh::accept( IImGuiConstantBufferVisitor &ev )
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

float Mesh::getDistanceFromActiveCamera() const noexcept
{
	return m_distanceFromActiveCamera;
}

bool Mesh::isRenderedThisFrame() const noexcept
{
	return m_bRenderedThisFrame;
}

std::shared_ptr<VertexBuffer>& Mesh::getVertexBuffer()
{
	return m_pVertexBuffer;
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

const Node* Mesh::getNode() const noexcept
{
	return m_pNode;
}

std::string Mesh::getName() const noexcept
{
	using namespace std::string_literals;
	// even though the function is not virtual *this will take into account the "virtuality" of the class
	auto str = std::string{typeid( *this ).name()} + "#"s + std::to_string( m_meshId );
	// str = "class whatever#34" or "struct whatever#34"
	return util::trimStringFromStart( str, 6 );
}

unsigned Mesh::getMeshId() const noexcept
{
	return m_meshId;
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

void Mesh::setDistanceFromActiveCamera() noexcept
{
	const auto pos = m_pNode->getPosition();
	const auto &cameraPos = CameraManager::getInstance().getActiveCamera().getPosition();
	m_distanceFromActiveCamera = util::distance( pos, cameraPos );
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
	static auto &s = SettingsManager::getInstance();
	if ( !s.getSettings().bEnableFrustumCuling )
	{
		return false;
	}

	const std::vector<DirectX::XMFLOAT4> &frustumPlanes = CameraManager::getInstance().getActiveCamera().getFrustumPlanes();
	const int numPlanes = static_cast<const int>( frustumPlanes.size() );
	ASSERT( numPlanes == 6, "Invalid number of planes!" );

	const auto &minVertex = m_aabb.first;
	const auto &maxVertex = m_aabb.second;

	const auto pos = m_pNode->getPosition();

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