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
#include "file_utils.h"


namespace dx = DirectX;

Model::Model( Graphics &gfx,
	const std::string &path,
	const float initialScale /*= 1*/,
	const DirectX::XMFLOAT3 &initialRot /*= {0, 0, 0}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0, 0, 0}*/ )
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
		materials.emplace_back( gfx, *paiScene->mMaterials[i], path, MaterialLoader::LightingModel::BlinnPhong );
	}

	// create N meshes for N materials in the model file
	for ( size_t i = 0; i < paiScene->mNumMeshes; ++i )
	{
		const auto &aiMesh = *paiScene->mMeshes[i];

		m_meshes.emplace_back( std::make_unique<Mesh>( gfx, materials[aiMesh.mMaterialIndex], aiMesh, initialScale ) );
	}

	const int imguiNodeId = 0;
	m_pRoot = parseModelNodeGraph( *paiScene->mRootNode, imguiNodeId, initialScale );

	setTransform( 1, initialRot, initialPos );
}

void Model::update( const float dt,
	const float renderFrameInterpolation ) const cond_noex
{
	m_pRoot->update( dt, dx::XMMatrixIdentity(), renderFrameInterpolation );
}

void Model::render( const size_t channels ) const cond_noex
{
	m_pRoot->render( channels );
}

void Model::setEffectEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	m_pRoot->setEffectEnabled( channels, bEnabled );
}

void Model::displayImguiWidgets( Graphics &gfx ) noexcept
{
#ifndef FINAL_RELEASE
	m_imguiVisitor.displayImguiWidgets( *this );
#endif
}

void Model::accept( IImguiNodeVisitor &v )
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

void Model::setTransform( const DirectX::XMMATRIX &tr ) cond_noex
{
	m_pRoot->setWorldTransform( tr );
}

void Model::setTransform( const float scale,
	const DirectX::XMFLOAT3 &rotAngles,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	const dx::XMVECTOR rotQuat = util::pitchYawRollToQuaternion( dx::XMLoadFloat3( &rotAngles ) );

	dx::XMMATRIX worldTransform = dx::XMMatrixAffineTransformation( dx::XMVectorReplicate( scale ), util::s_XMZero, rotQuat, dx::XMLoadFloat3( &pos ) );

	m_pRoot->setWorldTransform( worldTransform );
}

void Model::setTransform( const float scale,
	const DirectX::XMFLOAT4 &rotQuat,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	dx::XMMATRIX worldTransform = dx::XMMatrixAffineTransformation( dx::XMVectorReplicate( scale ), util::s_XMZero, dx::XMLoadFloat4( &rotQuat ), dx::XMLoadFloat3( &pos ) );

	m_pRoot->setWorldTransform( worldTransform );
}

void Model::setTransform( const float scale,
	const DirectX::XMVECTOR &rotQuat,
	const DirectX::XMFLOAT3 &pos ) cond_noex
{
	dx::XMMATRIX worldTransform = dx::XMMatrixAffineTransformation( dx::XMVectorReplicate( scale ), util::s_XMZero, rotQuat, dx::XMLoadFloat3( &pos ) );

	m_pRoot->setWorldTransform( worldTransform );
}

void Model::setScale( const DirectX::XMFLOAT3 &scale ) cond_noex
{
	const auto &currentTransform = m_pRoot->getWorldTransform();
	dx::XMVECTOR currentScale{}, currentRotQuat{}, currentPos{};
	const bool ret = dx::XMMatrixDecompose( &currentScale, &currentRotQuat, &currentPos, currentTransform );
	ASSERT( ret, "Matrix decomposition failed!" );

	const dx::XMMATRIX scaleMat = dx::XMMatrixScaling( scale.x, scale.y, scale.z );
	const dx::XMMATRIX rotMat = dx::XMMatrixRotationQuaternion( currentRotQuat );
	const dx::XMMATRIX posMat = dx::XMMatrixTranslationFromVector( currentPos );
	m_pRoot->setWorldTransform( scaleMat * rotMat * posMat );
}

void Model::setRotation( const DirectX::XMFLOAT3 &rot ) cond_noex
{
	const auto &currentTransform = m_pRoot->getWorldTransform();
	dx::XMVECTOR currentScale{}, currentRotQuat{}, currentPos{};
	const bool ret = dx::XMMatrixDecompose( &currentScale, &currentRotQuat, &currentPos, currentTransform );
	ASSERT( ret, "Matrix decomposition failed!" );

	const dx::XMMATRIX scaleMat = dx::XMMatrixScalingFromVector( currentScale );
	const dx::XMMATRIX rotMat = dx::XMMatrixRotationRollPitchYaw( rot.x, rot.y, rot.z );
	const dx::XMMATRIX posMat = dx::XMMatrixTranslationFromVector( currentPos );
	m_pRoot->setWorldTransform( scaleMat * rotMat * posMat );
}

void Model::rotateRel( const DirectX::XMFLOAT3 &rot ) cond_noex
{
	const auto &currentTransform = m_pRoot->getWorldTransform();
	dx::XMFLOAT4X4 currentTransform4x4{};
	dx::XMStoreFloat4x4( &currentTransform4x4, currentTransform );
	const dx::XMFLOAT3 pitchYawRoll = util::extractRotation( currentTransform4x4 );
	const float pitch = std::clamp( pitchYawRoll.x + rot.x, 0.995f * -util::PI / 2.0f, 0.995f * util::PI / 2.0f );
	const float yaw = util::wrapAngle( pitchYawRoll.y + rot.y );
	const float roll = std::clamp( pitchYawRoll.z + rot.z, 0.995f * -util::PI / 2.0f, 0.995f * util::PI / 2.0f );
	setRotation( dx::XMFLOAT3{pitch, yaw, roll} );
}

void Model::setPosition( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	auto &worldTransform = m_pRoot->getWorldTransformAccess();
	worldTransform._41 = pos.x;
	worldTransform._42 = pos.y;
	worldTransform._43 = pos.z;
}

void Model::translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex
{
	auto &worldTransform = m_pRoot->getWorldTransformAccess();
	worldTransform._41 += pos.x;
	worldTransform._42 += pos.y;
	worldTransform._43 += pos.z;
}

DirectX::XMMATRIX Model::getTransform() const noexcept
{
	return m_pRoot->getWorldTransform();
}

float Model::getScale() const noexcept
{
	return m_meshes[0]->getScale();
}

DirectX::XMFLOAT3 Model::getRotation() const noexcept
{
	return m_meshes[0]->getRotation();
}

DirectX::XMFLOAT3 Model::getPosition() const noexcept
{
	return m_meshes[0]->getPosition();
}

float Model::getDistanceFromActiveCamera() const noexcept
{
	return m_meshes[0]->getDistanceFromActiveCamera();	// #TODO:
}

void Model::setCulled( const bool bCulled )
{
	m_pRoot->setCulled( bCulled );
}

bool Model::isCulled() const noexcept
{
	bool bCulled = true;
	for ( const auto &pMesh : m_meshes )
	{
		bCulled = bCulled && pMesh->isCulled();
	}
	return bCulled;	// if even 1 mesh is not culled then the model is not-culled
}

std::unique_ptr<Node> Model::parseModelNodeGraph( const aiNode &ainode,
	int imguiNodeId,
	const float scale ) noexcept
{
	namespace dx = DirectX;
	// Assimp is row major
	const auto transform = util::scaleTranslation( dx::XMLoadFloat4x4( reinterpret_cast<const dx::XMFLOAT4X4*>( &ainode.mTransformation ) ), scale );

	std::vector<Mesh*> pMeshes;
	pMeshes.reserve( ainode.mNumMeshes );
	for ( size_t i = 0; i < ainode.mNumMeshes; ++i )
	{
		const unsigned int meshId = ainode.mMeshes[i];
		pMeshes.push_back( m_meshes.at( meshId ).get() );
	}

	auto pNode = std::make_unique<Node>( imguiNodeId, ainode.mName.C_Str(), transform, std::move( pMeshes ) );

	// create children Nodes and attach them to the Model's hierarchical tree structure
	++imguiNodeId;
	for ( size_t i = 0; i < ainode.mNumChildren; ++i )
	{
		pNode->addChild( std::move( parseModelNodeGraph( *ainode.mChildren[i], imguiNodeId, scale ) ) );
	}

	return pNode;
}