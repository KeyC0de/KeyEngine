#pragma once

#include <DirectXMath.h>
#include <memory>
#include <optional>
#include "effect.h"
#include "rendering_channel.h"


class IndexBuffer;
class VertexBuffer;
class Graphics;
class MaterialLoader;
class IBindable;
struct aiMesh;

namespace ren
{
	class Renderer;
}

namespace ver
{
	class VBuffer;
}

//=============================================================
//	\class	Mesh
//	\author	KeyC0de
//	\date	2022/09/12 23:26
//	\brief	imported Models will be handled via Mesh
//			stand-alone Meshes should not touch `m_worldTransform`
//			instead they should define their own transform vars (eg m_pos, m_rot, or m_scale) and override getTransform (for Cloning Bindables like TransformVSCB) that require the Mesh's transform to be bound to the pipeline
//=============================================================
class Mesh
{
	mutable DirectX::XMFLOAT4X4 m_worldTransform;
	mutable bool m_bCulledThisFrame = false;
	float m_distanceFromActiveCamera = 0.0f;
protected:
	unsigned m_meshId = 0u;
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> m_aabb;	// #TODO: Collision mesh needs to be recalculated if the object is rotated, scaled or animated
	std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	std::shared_ptr<VertexBuffer> m_pVertexBuffer;
	std::vector<Effect> m_effects;
public:
#pragma warning( disable : 26495 )
	//	\function	Mesh	||	\date	2022/11/06 14:46
	//	\brief	defctor to be called by subclasses - you must provide a m_meshId and an m_aabb yourself
	Mesh() = default;
#pragma warning( default : 26495 )
	Mesh( Graphics &gph, const MaterialLoader &mat, const aiMesh &aimesh, const float scale = 1.0f ) noexcept;
	virtual ~Mesh() noexcept = default;

	template<typename T, typename = std::enable_if_t<std::is_base_of_v<IBindable, T>>>
	std::optional<T*> findBindable() noexcept
	{
		for ( auto &effect : m_effects )
		{
			for ( auto &bindable : effect.bindables() )
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
		for ( const auto &effect : m_effects )
		{
			for ( const auto &bindable : effect.getBindables() )
			{
				if ( const auto *pB = dynamic_cast<T*>( bindable.get() ) )
				{
					return pB;
				}
			}
		}
		return std::nullopt;
	}

	//	\function	addEffect	||	\date	2021/10/26 23:58
	//	\brief	Effects are moved here
	void addEffect( Effect ef ) noexcept;
	//	\function	update	||	\date	2021/10/26 23:58
	//	\brief	does gameplay, transformation, physics
	void update( const float dt ) cond_noex;
	void render( const size_t channels = rch::all ) const noexcept;
	void setEffectEnabled( const size_t channels, const bool bEnabled ) noexcept;
	void bind( Graphics &gph ) const cond_noex;
	void accept( IEffectVisitor &ev );
	const unsigned getIndicesCount() const cond_noex;
	void connectEffectsToRenderer( ren::Renderer &r );
	//===================================================
	//	\function	setTransform	||	\date	2022/08/28 21:24
	//	\brief	sets the world transform matrix for the mesh
	void setTransform( const DirectX::XMMATRIX &worldTransform ) noexcept;
	DirectX::XMFLOAT4X4& transform();
	virtual const DirectX::XMMATRIX getTransform() const noexcept;
	DirectX::XMFLOAT3 calcPosition() const noexcept;
	DirectX::XMMATRIX calcPositionTr() const noexcept;
	DirectX::XMFLOAT4 calcRotationQuat() const noexcept;
	DirectX::XMMATRIX calcRotationTr() const noexcept;
	float calcScale() const noexcept;
	DirectX::XMMATRIX calcScaleTr() const noexcept;
	void setDistanceFromActiveCamera() noexcept;
	float getDistanceFromActiveCamera() const noexcept;
	void setCulled( const bool bCulled );
	bool isCulled() const noexcept;
	bool frustumCull( const std::vector<DirectX::XMFLOAT4> &frustumPlanes ) const noexcept;
protected:
	float calcDistanceFromActiveCamera( const DirectX::XMFLOAT3 &pos ) const noexcept;
	void setMeshId() noexcept;
	void createAabb( const ver::VBuffer &verts );
private:
	void createAabb( const aiMesh &aiMesh );
};