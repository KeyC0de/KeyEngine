#pragma once

#include <DirectXMath.h>
#include <memory>
#include <optional>
#include "material.h"
#include "rendering_channel.h"
#include "transform_vscb.h"


class VertexBuffer;
class IndexBuffer;
class PrimitiveTopology;
class Graphics;
class MaterialLoader;
class IBindable;
class Node;
struct aiMesh;

namespace ren
{
class Renderer;
}

namespace ver
{
class VBuffer;
}

class Mesh
{
	float m_distanceFromActiveCamera = 0.0f;
	mutable bool m_bRenderedThisFrame = false;
protected:
	unsigned m_meshId = 0u;
	Node *m_pNode = nullptr;
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> m_aabb{{0, 0, 0},{0, 0, 0}};	// #TODO: Collision mesh needs to be recalculated if the object is rotated, scaled or animated
	std::shared_ptr<VertexBuffer> m_pVertexBuffer;
	std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	std::shared_ptr<PrimitiveTopology> m_pPrimitiveTopology;
	std::unique_ptr<TransformVSCB> m_pTransformVscb;
	std::vector<Material> m_materials;

	struct ColorPSCB
	{
		DirectX::XMFLOAT4 materialColor = {1.0f, 1.0f, 1.0f, 1.0f};
	} m_colorPscb;

	struct ColorPSCB2
	{
		DirectX::XMFLOAT4 materialColor = {0.8f, 0.2f, 0.15f, 1.0f};
	} m_colorPscbOutline;

	struct ColorPSCB3
	{
		DirectX::XMFLOAT4 materialColor = {0.1f, 0.8f, 0.05f, 1.0f};
	} m_colorPscbWireframe;

	struct ColorPSCB4
	{
		DirectX::XMFLOAT4 materialColor = {0.3f, 0.2f, 0.15f, 1.0f};
	} m_colorPscbDepthReversed;
public:
#pragma warning( disable : 26495 )
	/// \brief	defctor to be called by subclasses
	Mesh() = default;
#pragma warning( default : 26495 )
	/// \brief	ctor for imported models, creates bounding box & meshId
	Mesh( Graphics &gfx, const MaterialLoader &mat, const aiMesh &aimesh, const float initialScale = 1.0f );
	virtual ~Mesh() noexcept;
	Mesh( const Mesh &rhs ) = delete;
	Mesh& operator=( const Mesh &rhs ) = delete;
	Mesh( Mesh &&rhs ) noexcept;
	Mesh& operator=( Mesh &&rhs ) noexcept = delete;

	void setNode( Node &node );
	void update( const float dt, const float lerpBetweenFrames ) cond_noex;
	void render( const size_t channels = rch::all ) const noexcept;
	void bind( Graphics &gfx ) const cond_noex;

	template<typename T, typename = std::enable_if_t<std::is_base_of_v<IBindable, T>>>
	std::optional<T*> findBindable() noexcept
	{
		for ( auto &material : m_materials )
		{
			for ( auto &bindable : material.getBindables() )
			{
				if ( auto *pB = dynamic_cast<T*>( bindable.get() ) )
				{
					return pB;
				}
			}
		}
		return std::nullopt;
	}

	template<typename T, typename = std::enable_if_t<std::is_base_of_v<IBindable, T>>>
	std::optional<const T*> findBindable() const noexcept
	{
		for ( const auto &material : m_materials )
		{
			for ( const auto &bindable : material.getBindables() )
			{
				if ( const auto *pB = dynamic_cast<T*>( bindable.get() ) )
				{
					return pB;
				}
			}
		}
		return std::nullopt;
	}

	void addMaterial( Material material ) noexcept;
	void setMaterialEnabled( const size_t channels, const bool bEnabled ) noexcept;
	void accept( IImGuiConstantBufferVisitor &ev );
	unsigned getIndicesCount() const cond_noex;
	void connectMaterialsToRenderer( ren::Renderer &r );
	float getDistanceFromActiveCamera() const noexcept;
	bool isRenderedThisFrame() const noexcept;
	std::shared_ptr<VertexBuffer>& getVertexBuffer();
	void createAabb( const ver::VBuffer &verts );
	const Node* getNode() const noexcept;
	std::string getName() const noexcept;
	unsigned getMeshId() const noexcept;
protected:
	float getDistanceFromActiveCamera( const DirectX::XMFLOAT3 &pos ) const noexcept;
	void setMeshId() noexcept;
private:
	void setDistanceFromActiveCamera() noexcept;
	void createAabb( const aiMesh &aiMesh );
	/// \brief	returns true if the Mesh is culled this frame by the active camera and false otherwise
	bool isFrustumCulled() const noexcept;
};
