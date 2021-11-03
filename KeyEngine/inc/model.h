#pragma once

#include <memory>
#include <filesystem>
#include "graphics.h"


class Node;
class Drawable;
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
	std::vector<std::unique_ptr<Drawable>> m_drawables;
public:
	Model( Graphics& gph, const std::string& path, float scale = 1.0f );
	~Model() noexcept;

	void update( float dt ) const cond_noex;
	void render( size_t channels ) const cond_noex;
	void setTransform( const DirectX::XMMATRIX& tr ) noexcept;
	void accept( class IModelVisitor& v );
	void connectEffectsToRenderer( ren::Renderer& r );
private:
	// parse the Scene Graph
	std::unique_ptr<Node> createNodeHierarchy( int& nodeId, const aiNode& node,
		float scale ) noexcept;
};