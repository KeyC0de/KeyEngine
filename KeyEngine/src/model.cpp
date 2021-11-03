#include "model.h"
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "node.h"
#include "drawable.h"
#include "material_loader.h"
#include "d3d_utils.h"
#include "assertions_console.h"


namespace dx = DirectX;

Model::Model( Graphics& gph,
	const std::string& path,
	const float scale )
{
	Assimp::Importer importer;
	const auto paiScene = importer.ReadFile( path.c_str(),
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
		| aiProcess_ConvertToLeftHanded	| aiProcess_GenNormals
		| aiProcess_CalcTangentSpace );

	ASSERT( paiScene, "aiScene is null!" );

	// create materials
	std::vector<MaterialLoader> materials;
	materials.reserve( paiScene->mNumMaterials );
	for ( size_t i = 0; i < paiScene->mNumMaterials; ++i )
	{
		materials.emplace_back( gph,
			*paiScene->mMaterials[i],
			path );
	}

	// create drawables
	for ( size_t i = 0; i < paiScene->mNumMeshes; ++i )
	{
		const auto& aiMesh = *paiScene->mMeshes[i];
		m_drawables.push_back( std::make_unique<Drawable>( gph,
			materials[aiMesh.mMaterialIndex],
			aiMesh,
			scale ) );
	}

	int nodeId = 0;
	m_pRoot = createNodeHierarchy( nodeId,
		*paiScene->mRootNode,
		scale );
}

Model::~Model() noexcept
{

}

void Model::update( float dt ) const cond_noex
{
	m_pRoot->update( dt,
		dx::XMMatrixIdentity() );
}

void Model::render( size_t channels ) const cond_noex
{
	m_pRoot->render( channels );
}

void Model::setTransform( const DirectX::XMMATRIX& tr ) noexcept
{
	m_pRoot->setTransform( tr );
}

void Model::accept( IModelVisitor& v )
{
	m_pRoot->accept( v );
}

void Model::connectEffectsToRenderer( ren::Renderer& r )
{
	ASSERT( !m_drawables.empty(), "Model has no drawables to render!" );
	for ( auto& pDrawable : m_drawables )
	{
		pDrawable->connectEffectsToRenderer( r );
	}
}

std::unique_ptr<Node> Model::createNodeHierarchy( int& nodeId,
	const aiNode& ainode,
	float scale ) noexcept
{
	namespace dx = DirectX;
	const auto transform = util::scaleTranslation( dx::XMMatrixTranspose(
		dx::XMLoadFloat4x4( reinterpret_cast<const dx::XMFLOAT4X4*>(
			&ainode.mTransformation ) ) ),
		scale );

	std::vector<Drawable*> pDrawables;
	pDrawables.reserve( ainode.mNumMeshes );
	for ( size_t i = 0; i < ainode.mNumMeshes; ++i )
	{
		const auto meshIdx = ainode.mMeshes[i];
		pDrawables.push_back( m_drawables.at( meshIdx ).get() );
	}

	auto pNode = std::make_unique<Node>( nodeId,
		ainode.mName.C_Str(),
		std::move( pDrawables ),
		transform );
	++nodeId;
	for ( size_t i = 0; i < ainode.mNumChildren; ++i )
	{
		pNode->addChild( createNodeHierarchy( nodeId,
			*ainode.mChildren[i],
			scale ) );
	}

	return pNode;
}