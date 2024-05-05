#pragma once

#include <memory>
#include <filesystem>
#include "node.h"
#include "graphics.h"
#include "rendering_channel.h"
#ifndef FINAL_RELEASE
#	include "imgui_visitors.h"
#endif


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
	std::unique_ptr<Node> m_pRoot;
	std::vector<std::unique_ptr<Mesh>> m_meshes;
#ifndef FINAL_RELEASE
	ImguiNodeVisitorShowcase m_imguiVisitor;
#endif
public:
	Model( Graphics &gfx, const std::string &path, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0, 0, 0}, const DirectX::XMFLOAT3 &initialPos = {0, 0, 0} );

	void update( const float dt, const float renderFrameInterpolation ) const cond_noex;
	void render( const size_t channels = rch::all ) const cond_noex;
	void setEffectEnabled( const size_t channels, const bool bEnabled ) noexcept;
	void displayImguiWidgets( Graphics &gfx ) noexcept;
	void accept( IImguiNodeVisitor &v );
	void connectEffectsToRenderer( ren::Renderer &r );
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//	\function	setTransform	||	\date	2022/09/03 20:25
	//	\brief	Sets the root Node's Worldtransform
	//			That transform propagates during Node::update to all children nodes and to the Meshes themselves.
	void setTransform( const DirectX::XMMATRIX &tr ) cond_noex;
	void setTransform( const float scale, const DirectX::XMFLOAT3 &rotAngles, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setTransform( const float scale, const DirectX::XMFLOAT4 &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setTransform( const float scale, const DirectX::XMVECTOR &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setScale( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void setRotation( const DirectX::XMFLOAT3 &rot ) cond_noex;
	void rotateRel( const DirectX::XMFLOAT3 &rot ) cond_noex;
	void setPosition( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex;
	DirectX::XMMATRIX getTransform() const noexcept;
	float getScale() const noexcept;
	DirectX::XMFLOAT3 getRotation() const noexcept;
	DirectX::XMFLOAT3 getPosition() const noexcept;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//	\function	getDistanceFromActiveCamera	||	\date	2022/09/03 13:27
	//	\brief	return the distance from the currently active camera for the primary Mesh
	float getDistanceFromActiveCamera() const noexcept;
private:
	std::unique_ptr<Node> parseModelNodeGraph( const aiNode &node, int imguiNodeId, const float scale ) noexcept;
};