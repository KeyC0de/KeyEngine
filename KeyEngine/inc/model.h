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
	// #TODO: add setPosition, setRotation, getPosition, getRotation functions, setPositionRelative, setRotationRelative
	// use XMMatrixDecompose
	void setTransform( const DirectX::XMMATRIX &tr ) noexcept;
	void accept( IModelVisitor &v );
	void connectEffectsToRenderer( ren::Renderer &r );
private:
	//===================================================
	//	\function	createNodeGraph
	//	\brief  parse the Model's scene graph
	//	\date	9 May 2022 8:30
	std::unique_ptr<Node> createNodeGraph( const aiNode &node, int imguiNodeId, const float scale ) noexcept;
};