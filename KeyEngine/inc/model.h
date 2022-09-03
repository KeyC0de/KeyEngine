#pragma once

#include <memory>
#include <filesystem>
#include "node.h"
#include "graphics.h"


class Mesh;
class ModelWindow;
struct aiMesh;
struct aiMaterial;
struct aiNode;
class IModelVisitor;

namespace ren
{
	class Renderer;
}

class Model
{
	std::unique_ptr<Node> m_pRoot;
	std::vector<std::unique_ptr<Mesh>> m_meshes;
public:
	Model( Graphics &gph, const std::string &path, const float scale = 1.0f );

	void update( const float dt ) const cond_noex;
	void render( const size_t channels ) const cond_noex;
	void accept( IModelVisitor &v );
	void connectEffectsToRenderer( ren::Renderer &r );
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//===================================================
	//	\function	setRootTransform
	//	\brief  Sets the root Node's Worldtransform
	//			That transform propagates during Node::update to all children nodes and to the Meshes themselves.
	//	\date	2022/09/03 20:25
	void setRootTransform( const DirectX::XMMATRIX &tr ) cond_noex;
	void setTransform( const DirectX::XMFLOAT4 &rot, const DirectX::XMFLOAT4 &pos, const float scale ) cond_noex;
	void setRotation( const DirectX::XMFLOAT3 &rot ) cond_noex;
	void rotateRel( const DirectX::XMFLOAT3 &rot ) cond_noex;
	void rotateRel( const float pitch, const float yaw, const float roll ) cond_noex;
	void setPosition( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setScale( const DirectX::XMFLOAT3 scaleVec ) cond_noex;
	void setScale( const float scale ) cond_noex;
	const DirectX::XMMATRIX getTransform() const noexcept;
	DirectX::XMFLOAT3 calcPosition() const noexcept;
	DirectX::XMMATRIX calcPositionTr() const noexcept;
	DirectX::XMFLOAT4 calcRotationQuat() const noexcept;
	DirectX::XMMATRIX calcRotationTr() const noexcept;
	float calcScale() const noexcept;
	DirectX::XMMATRIX calcScaleTr() const noexcept;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//===================================================
	//	\function	getDistanceFromActiveCamera
	//	\brief  return the distance from the currently active camera for the primary Mesh
	//	\date	2022/09/03 13:27
	const float getDistanceFromActiveCamera() const noexcept;
private:
	std::unique_ptr<Node> parseModelNodeGraph( const aiNode &node, int imguiNodeId, const float scale ) noexcept;
};