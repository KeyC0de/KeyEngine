#include "model.h"
#include "node.h"
#include "graphics.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "mesh.h"
#include "material_loader.h"
#include "math_utils.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#include "file_utils.h"


// #TODO: Model LOD automatic switching
namespace dx = DirectX;

Model::Model( Graphics &gfx,
	const std::string &path,
	const float initialScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRotDeg /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0.0f, 0.0f, 0.0f}*/ )
#ifndef FINAL_RELEASE
	:
	m_imguiVisitor{util::getFilename( path )}
#endif
{
	Assimp::Importer importer;
	const auto paiScene = importer.ReadFile( path.c_str(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals | aiProcess_CalcTangentSpace );
	//aiAnimation** mAnimations		// The array of animations.
	//aiCamera** mCameras			// The array of cameras.
	//unsigned int mFlags			// Any combination of the AI_SCENE_FLAGS_XXX flags.
	//aiLight** mLights				// The array of light sources.
	//aiMaterial** mMaterials		// The array of materials.
	//aiMesh** mMeshes				// The array of meshes.
	//unsigned int mNumAnimations	// The number of animations in the scene.
	//unsigned int mNumCameras		// The number of cameras in the scene.
	//unsigned int mNumLights		// The number of light sources in the scene.
	//unsigned int mNumMaterials	// The number of materials in the scene.
	//unsigned int mNumMeshes		// The number of meshes in the scene.
	//unsigned int mNumTextures		// The number of textures embedded into the file.
	//void* mPrivate 				// Internal data, do not touch.
	//aiNode* mRootNode				// The root node of the hierarchy.
	//aiTexture** mTextures			// The array of embedded textures. - only useful for model formals that contain embedded textures

	ASSERT( paiScene, "aiScene is null!" );

	// create materials
	std::vector<MaterialLoader> materials;
	materials.reserve( paiScene->mNumMaterials );
	for ( size_t i = 0; i < paiScene->mNumMaterials; ++i )
	{
		materials.emplace_back( gfx, *paiScene->mMaterials[i], path );
	}

	// create N meshes for N materials in the model file
	for ( size_t i = 0; i < paiScene->mNumMeshes; ++i )
	{
		const auto &aiMesh = *paiScene->mMeshes[i];

		m_meshes.emplace_back( std::make_unique<Mesh>( gfx, materials[aiMesh.mMaterialIndex], aiMesh, initialScale ) );
	}

	const int imguiNodeId = 0;
	m_pRoot = parseModelNodeGraph( nullptr, *paiScene->mRootNode, imguiNodeId, initialScale );

	setTransform( 1, util::toRadians3( initialRotDeg ), initialPos );
}

Model::Model( std::unique_ptr<Mesh> pMesh,
	Graphics &gfx,
	const DirectX::XMFLOAT3 &initialRotDeg,
	const DirectX::XMFLOAT3 &initialPos )
	:
	m_nNodes{1},
	m_nMeshNodes{1}
{
	const auto &meshName = pMesh->getName();
	const auto &meshId = pMesh->getMeshId();
#ifndef FINAL_RELEASE
	m_imguiVisitor = ImguiPerModelNodeVisitor{meshName};
#endif

	m_meshes.emplace_back( std::move( pMesh ) );

	std::vector<Mesh*> pMeshes;
	pMeshes.reserve( 1 );
	pMeshes.push_back( m_meshes.at( 0 ).get() );

	const auto localNodeTransform = util::scaleTranslation( dx::XMMatrixIdentity(), 1.0f );
	using namespace std::string_literals;
	const auto &nodeName = "Node#"s + std::to_string( 0 ) + "#"s + meshName;
	m_pRoot = std::make_unique<Node>( nullptr, 0, nodeName, localNodeTransform, std::move( pMeshes ) );

	setTransform( 1.0f, util::toRadians3( initialRotDeg ), initialPos );

	m_name = "Model#"s + meshName;
}

Model::~Model() noexcept
{
	m_meshes.clear();
}

Model::Model( Model &&rhs ) noexcept
	:
	m_nNodes{rhs.m_nNodes},
	m_nMeshNodes{rhs.m_nMeshNodes},
	m_name{std::move( rhs.m_name )},
	m_pRoot{std::move( rhs.m_pRoot )},
	m_meshes{std::move( rhs.m_meshes )}
#ifndef FINAL_RELEASE
	, m_imguiVisitor{rhs.m_imguiVisitor}
#endif
{
	rhs.m_pRoot = nullptr;
}

void Model::update( const float dt,
	const float renderFrameInterpolation,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{
	m_pRoot->update( dt, dx::XMMatrixIdentity(), renderFrameInterpolation, bEnableSmoothMovement );
}

void Model::render( const size_t channels /*= rch::all*/ ) const cond_noex
{
	for ( const auto &pMesh : m_meshes )
	{
		pMesh->render( channels );
	}
}

void Model::setMaterialEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	for ( const auto &pMesh : m_meshes )
	{
		pMesh->setMaterialEnabled( channels, bEnabled );
	}
}

void Model::displayImguiWidgets( Graphics &gfx ) noexcept
{
#ifndef FINAL_RELEASE
	if ( m_nNodes > 6 )
	{
		return;
	}
	m_imguiVisitor.displayImguiWidgets( *this );
#endif
}

void Model::accept( IImguiNodeVisitor &v )
{
	m_pRoot->accept( v );
}

void Model::connectMaterialsToRenderer( ren::Renderer &r )
{
	ASSERT( !m_meshes.empty(), "Model has no meshes to render!" );
	for ( auto &pMesh : m_meshes )
	{
		pMesh->connectMaterialsToRenderer( r );
	}
}

void Model::setTransform( const DirectX::XMMATRIX &tr ) cond_noex
{
	m_pRoot->setWorldTransform( tr );
}

void Model::setTransform( const float scale,
	const DirectX::XMFLOAT3 &rotAnglesRadians,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	m_pRoot->setTransform( scale, rotAnglesRadians, pos );
}

void Model::setTransform( const float scale,
	const DirectX::XMFLOAT4 &rotQuat,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	m_pRoot->setTransform( scale, rotQuat, pos );
}

void Model::setTransform( const float scale,
	const DirectX::XMVECTOR &rotQuat,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	m_pRoot->setTransform( scale, rotQuat, pos );
}

void Model::setScale( const float scale ) cond_noex
{
	m_pRoot->setScale( scale );
}

void Model::setScale( const DirectX::XMFLOAT3 &scale ) cond_noex
{
	m_pRoot->setScale( scale );
}

void Model::scaleRel( const DirectX::XMFLOAT3 &scale )
{
	m_pRoot->setScale( scale );
}

void Model::setRotation( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex
{
	m_pRoot->setRotation( rotAnglesRadians );
}

void Model::rotateRel( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex
{
	m_pRoot->rotateRel( rotAnglesRadians );
}

void Model::setTranslation( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	m_pRoot->setTranslation( pos );
}

void Model::translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	m_pRoot->translateRel( pos );
}

DirectX::XMMATRIX Model::getTransform() const noexcept
{
	return m_pRoot->getWorldTransform();
}

DirectX::XMFLOAT4X4 Model::getTransform4x4() const noexcept
{
	return m_pRoot->getWorldTransform4x4();
}

float Model::getScale() const noexcept
{
	return m_pRoot->getScale();
}

DirectX::XMFLOAT3 Model::getScale3() const noexcept
{
	return m_pRoot->getScale3();
}

DirectX::XMFLOAT3 Model::getRotation() const noexcept
{
	return m_pRoot->getRotation();
}

DirectX::XMVECTOR Model::getRotationQuat() const noexcept
{
	return m_pRoot->getRotationQuat();
}

DirectX::XMFLOAT3 Model::getPosition() const noexcept
{
	return m_pRoot->getPosition();
}

float Model::getDistanceFromActiveCamera( const int index /*= 0*/ ) const noexcept
{
	return ( index > -1 && index < m_meshes.size() ) ? m_meshes[index]->getDistanceFromActiveCamera() : -1;
}

const std::string& Model::getName() const noexcept
{
	return m_name;
}

const Mesh* const Model::getMesh( const int index /*= 0*/ ) const noexcept
{
	return ( index > -1 && index < m_meshes.size() ) ? m_meshes[index].get() : nullptr;
}

Mesh* const Model::getMesh( const int index /*= 0*/ )
{
	return ( index > -1 && index < m_meshes.size() ) ? m_meshes[index].get() : nullptr;
}

std::unique_ptr<Node> Model::parseModelNodeGraph( Node *pParent,
	const aiNode &ainode,
	int imguiNodeId,
	const float initialScale ) cond_noex
{
	namespace dx = DirectX;
	// Assimp is row major
	const auto localNodeTransform = util::scaleTranslation( dx::XMLoadFloat4x4( reinterpret_cast<const dx::XMFLOAT4X4*>( &ainode.mTransformation ) ), initialScale );

	std::vector<Mesh*> pMeshes;
	pMeshes.reserve( ainode.mNumMeshes );
	for ( size_t i = 0; i < ainode.mNumMeshes; ++i )
	{
		const unsigned int meshId = ainode.mMeshes[i];
		pMeshes.push_back( m_meshes.at( meshId ).get() );
		++m_nMeshNodes;
		m_name = "Model#" + m_meshes.at( meshId )->getName();
	}

	// Only child Nodes have stuff attached to them (Meshes, Cameras, Lights, Helpers, Pivots etc.)
	// Not all imported-file/fbx `Node`s are Mesh Nodes!
	//ASSERT( pMeshes.size() <= 1, "In KeyEngine, we attach up to 1 single Mesh to each MeshNode for the time being! The root Node has no Meshes or anything else attached to it." );
	ASSERT( pMeshes.size() <= 2, "test" );

	auto pNode = std::make_unique<Node>( pParent, imguiNodeId, ainode.mName.C_Str(), localNodeTransform, std::move( pMeshes ) );

	++m_nNodes;

	// create children Nodes and attach them to the Model's hierarchical tree structure
	++imguiNodeId;
	for ( size_t i = 0; i < ainode.mNumChildren; ++i )
	{
		pNode->addChild( std::move( parseModelNodeGraph( pNode.get(), *ainode.mChildren[i], imguiNodeId, initialScale ) ) );
	}

	return pNode;
}
