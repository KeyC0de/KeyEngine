#pragma once

#include <memory>
#include <filesystem>
#include <string>
#include <DirectXMath.h>
#include "rendering_channel.h"
#ifndef FINAL_RELEASE
#	include "imgui_visitors.h"
#endif


class Graphics;
class Mesh;
class ModelWindow;
struct aiMesh;
struct aiMaterial;
struct aiNode;

namespace ren
{
	class Renderer;
}

class Model
{
	int m_nNodes = 0;
	int m_nMeshNodes = 0;
	std::string m_name;
	std::unique_ptr<Node> m_pRoot;
	std::vector<std::unique_ptr<Mesh>> m_meshes;	// m_meshes[0] is the Mesh attached to m_pRoot Node
#ifndef FINAL_RELEASE
	ImguiPerModelNodeVisitor m_imguiVisitor;
#endif
public:
	//	\function	Model	||	\date	2021/05/27 23:38
	//	\brief	Model ctor for imported meshes
	//			initialRot is in degrees - it will be converted and stored as radians
	Model( Graphics &gfx, const std::string &path, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRotDeg = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f} );
	//	\function	Model	||	\date	2024/05/27 23:39
	//	\brief	ctor for single-Mesh/Node primitives
	//			initialRot is in degrees - it will be converted and stored as radians
	Model( std::unique_ptr<Mesh> pMesh, Graphics &gfx, const DirectX::XMFLOAT3 &initialRotDeg, const DirectX::XMFLOAT3 &initialPos );
	~Model() noexcept;
	Model( const Model &rhs ) = delete;
	Model& operator=( const Model &rhs ) = delete;
	Model( Model &&rhs ) noexcept;
	Model& operator=( Model &&rhs ) noexcept = delete;

	void update( const float dt, const float renderFrameInterpolation, const bool bEnableSmoothMovemenzzt = false ) cond_noex;
	void render( const size_t channels = rch::all ) const cond_noex;
	void setMaterialEnabled( const size_t channels, const bool bEnabled ) noexcept;
	void displayImguiWidgets( Graphics &gfx ) noexcept;
	void accept( IImguiNodeVisitor &v );
	void connectMaterialsToRenderer( ren::Renderer &r );
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//	\function	setTransform	||	\date	2022/09/03 20:25
	//	\brief	All transformation-related-functions simply call the root Node's corresponding function and propagate the operation down the Node hierarchy; the world transform itself is updated only once-per-tick during update
	void setTransform( const DirectX::XMFLOAT4X4 &tr ) cond_noex;
	void setTransform( const DirectX::XMMATRIX &tr ) cond_noex;
	void setTransform( const float scale, const DirectX::XMFLOAT3 &rotAnglesRadians, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setTransform( const float scale, const DirectX::XMFLOAT4 &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setTransform( const float scale, const DirectX::XMVECTOR &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setScale( const float scale ) cond_noex;
	void setScale( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void scaleRel( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void setRotation( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex;
	void rotateRel( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex;
	void setTranslation( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex;
	DirectX::XMMATRIX getTransform() const noexcept;
	DirectX::XMFLOAT4X4 getTransform4x4() const noexcept;
	float getScale() const noexcept;
	DirectX::XMFLOAT3 getScale3() const noexcept;
	DirectX::XMFLOAT3 getRotation() const noexcept;
	DirectX::XMVECTOR getRotationQuat() const noexcept;
	DirectX::XMFLOAT3 getPosition() const noexcept;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//	\function	getDistanceFromActiveCamera	||	\date	2022/09/03 13:27
	//	\brief	return the distance between the currently active camera and the Mesh at specified index
	//			if index == 0 -> it's the root Node's Mesh
	float getDistanceFromActiveCamera( const int index = 0 ) const noexcept;
	const std::string& getName() const noexcept;
	const Mesh* const getMesh( const int index = 0 ) const noexcept;
	Mesh* const getMesh( const int index = 0 );
private:
	std::unique_ptr<Node> parseModelNodeGraph( Node *pParent, const aiNode &node, int imguiNodeId, const float initialScale ) cond_noex;
};