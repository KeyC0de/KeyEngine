#include "model.h"
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "node.h"
#include "mesh.h"
#include "material_loader.h"
#include "math_utils.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#include "model_visitor.h"


namespace dx = DirectX;

Model::Model( Graphics &gph,
	const std::string &path,
	const float scale )
{
	Assimp::Importer importer;
	const auto paiScene = importer.ReadFile( path.c_str(),
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals | aiProcess_CalcTangentSpace );

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

	// create meshes
	for ( size_t i = 0; i < paiScene->mNumMeshes; ++i )
	{
		const auto &aiMesh = *paiScene->mMeshes[i];
		m_meshes.emplace_back( std::make_unique<Mesh>( gph,
			materials[aiMesh.mMaterialIndex],
			aiMesh,
			scale ) );
	}

	int imguiNodeId = 0;
	m_pRoot = parseModelNodeGraph( *paiScene->mRootNode,
		imguiNodeId,
		scale );
}

void Model::update( const float dt ) const cond_noex
{
	m_pRoot->update( dt,
		dx::XMMatrixIdentity() );
}

void Model::render( const size_t channels ) const cond_noex
{
	m_pRoot->render( channels );
}

void Model::setEffectEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	m_pRoot->setEffectEnabled( channels,
		bEnabled );
}

void Model::accept( IModelVisitor &v )
{
	m_pRoot->accept( v );
}

void Model::connectEffectsToRenderer( ren::Renderer &r )
{
	ASSERT( !m_meshes.empty(), "Model has no meshes to render!" );
	for ( auto &pMesh : m_meshes )
	{
		pMesh->connectEffectsToRenderer( r );
	}
}

std::unique_ptr<Node> Model::parseModelNodeGraph( const aiNode &ainode,
	int imguiNodeId,
	const float scale ) noexcept
{
	namespace dx = DirectX;
	// Assimp is row major
	const auto transform = util::scaleTranslation( dx::XMLoadFloat4x4( reinterpret_cast<const dx::XMFLOAT4X4*>( &ainode.mTransformation ) ),
		scale );

	std::vector<Mesh*> pMeshes;
	pMeshes.reserve( ainode.mNumMeshes );
	for ( size_t i = 0; i < ainode.mNumMeshes; ++i )
	{
		const unsigned int meshId = ainode.mMeshes[i];
		pMeshes.push_back( m_meshes.at( meshId ).get() );
	}

	// create root Node
	auto pNode = std::make_unique<Node>( imguiNodeId,
		ainode.mName.C_Str(),
		transform,
		std::move( pMeshes ) );

	// create children Nodes and attach them to the Model's hierarchical tree structure
	++imguiNodeId;
	for ( size_t i = 0; i < ainode.mNumChildren; ++i )
	{
		pNode->addChild( std::move( parseModelNodeGraph( *ainode.mChildren[i],
			imguiNodeId,
			scale ) ) );
	}

	return pNode;
}

void Model::setRootTransform( const DirectX::XMMATRIX &tr ) cond_noex
{
	m_pRoot->setWorldTransform( tr );
}

void Model::setTransform( const DirectX::XMFLOAT4 &rot,
	const DirectX::XMFLOAT4 &pos,
	const float scale ) cond_noex
{
	dx::XMMATRIX worldTransform = dx::XMMatrixAffineTransformation( dx::XMVectorReplicate( scale ),
		util::s_XMZero,
		dx::XMLoadFloat4( &rot ),
		dx::XMLoadFloat4( &pos ) );

	m_pRoot->setWorldTransform( worldTransform );
}

void Model::setRotation( const DirectX::XMFLOAT3 &rot ) cond_noex
{
	dx::XMMATRIX worldTransform = dx::XMMatrixRotationRollPitchYaw( rot.x,
		rot.y,
		rot.z );

	m_pRoot->setWorldTransform( worldTransform );
}

void Model::rotateRel( const DirectX::XMFLOAT3 &rotIn ) cond_noex
{
	auto &worldTransform = m_pRoot->getWorldTransform();
	const dx::XMFLOAT3 pitchYawRoll = util::extractEulerAngles( worldTransform );
	const float pitch = std::clamp( pitchYawRoll.x + rotIn.x,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const float yaw = util::wrapAngle( pitchYawRoll.y + rotIn.y );
	const float roll = std::clamp( pitchYawRoll.z + rotIn.z,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const dx::XMFLOAT3 angles{pitch, yaw, roll};
	setRotation( angles );
}

void Model::rotateRel( const float pitchIn,
	const float yawIn,
	const float rollIn ) cond_noex
{
	auto &worldTransform = m_pRoot->getWorldTransform();
	const dx::XMFLOAT3 pitchYawRoll = util::extractEulerAngles( worldTransform );
	const float pitchOut = std::clamp( pitchYawRoll.x + pitchIn,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const float yawOut = util::wrapAngle( pitchYawRoll.y + yawIn );
	const float rollOut = std::clamp( pitchYawRoll.z + rollIn,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const dx::XMFLOAT3 angles{pitchOut, yawOut, rollOut};
	setRotation( angles );
}

void Model::setPosition( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	auto &worldTransform = m_pRoot->worldTransform();
	worldTransform._41 = pos.x;
	worldTransform._42 = pos.y;
	worldTransform._43 = pos.z;
}

void Model::translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	auto &worldTransform = m_pRoot->worldTransform();
	worldTransform._41 += pos.x;
	worldTransform._42 += pos.y;
	worldTransform._43 += pos.z;
}

void Model::setScale( const DirectX::XMFLOAT3 scaleVec ) cond_noex
{
	dx::XMMATRIX scaleMat = dx::XMMatrixScaling( scaleVec.x,
		scaleVec.y,
		scaleVec.z );

	m_pRoot->setWorldTransform( scaleMat );
}

void Model::setScale( const float scale ) cond_noex
{
	dx::XMMATRIX scaleMat = dx::XMMatrixScaling( scale,
		scale,
		scale );

	m_pRoot->setWorldTransform( scaleMat );
}

const DirectX::XMMATRIX Model::getTransform() const noexcept
{
	return m_meshes[0]->getTransform();
}

DirectX::XMFLOAT3 Model::calcPosition() const noexcept
{
	return m_meshes[0]->calcPosition();
}

DirectX::XMMATRIX Model::calcPositionTr() const noexcept
{
	return m_meshes[0]->calcPositionTr();
}

DirectX::XMFLOAT4 Model::calcRotationQuat() const noexcept
{
	return m_meshes[0]->calcRotationQuat();
}

DirectX::XMMATRIX Model::calcRotationTr() const noexcept
{
	return m_meshes[0]->calcRotationTr();
}

float Model::calcScale() const noexcept
{
	return m_meshes[0]->calcScale();
}

DirectX::XMMATRIX Model::calcScaleTr() const noexcept
{
	return m_meshes[0]->calcScaleTr();
}

const float Model::getDistanceFromActiveCamera() const noexcept
{
	return m_meshes[0]->getDistanceFromActiveCamera();
}