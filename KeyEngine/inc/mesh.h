#pragma once

#include <DirectXMath.h>
#include <memory>
#include <optional>
#include "effect.h"


class IndexBuffer;
class VertexBuffer;
class PrimitiveTopology;
class Graphics;
class MaterialLoader;
struct aiMesh;

namespace ren
{
	class Renderer;
}

//=============================================================
//	\class	
//
//	\author	KeyC0de
//	\date	2022/09/12 23:26
//
//	\brief	imported Models will be handled via Mesh
//			stand-alone Meshes should not touch `m_worldTransform`
//			instead they should define their own transform vars (eg m_pos, m_rot, or m_scale) and override getTransform (for Cloning Bindables like TransformVSCB) that require the Mesh's transform to be bound to the pipeline
//=============================================================
class Mesh
{
	float m_distanceFromActiveCamera = 0.0f;
	mutable DirectX::XMFLOAT4X4 m_worldTransform;
protected:
	std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	std::shared_ptr<VertexBuffer> m_pVertexBuffer;
	std::shared_ptr<PrimitiveTopology> m_pPrimitiveTopology;
	std::vector<Effect> m_effects;
public:
#pragma warning( disable : 26495 )
	Mesh() = default;
#pragma warning( default : 26495 )
	Mesh( Graphics &gph, const MaterialLoader &mat, const aiMesh &aimesh, const float scale = 1.0f ) noexcept;
	virtual ~Mesh() noexcept = default;

	template<class T>
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
	//===================================================
	//	\function	addEffect
	//	\brief  Effects are moved here
	void addEffect( Effect ef ) noexcept;
	//===================================================
	//	\function	update
	//	\brief  does gameplay, transformation, physics
	//	\date	2021/10/26 23:58
	void update( const float dt ) cond_noex;
	void render( const size_t channels ) const noexcept;
	void bind( Graphics &gph ) const cond_noex;
	void accept( IEffectVisitor &ev );
	const unsigned getIndicesCount() const cond_noex;
	void connectEffectsToRenderer( ren::Renderer &r );
	//===================================================
	//	\function	setTransform
	//	\brief  sets the world transform matrix for the mesh
	//	\date	2022/08/28 21:24
	void setTransform( const DirectX::XMMATRIX &worldTransform ) noexcept;
	DirectX::XMFLOAT4X4& transform();
	virtual const DirectX::XMMATRIX getTransform() const noexcept;
	DirectX::XMFLOAT3 calcPosition() const noexcept;
	DirectX::XMMATRIX calcPositionTr() const noexcept;
	DirectX::XMFLOAT4 calcRotationQuat() const noexcept;
	DirectX::XMMATRIX calcRotationTr() const noexcept;
	float calcScale() const noexcept;
	DirectX::XMMATRIX calcScaleTr() const noexcept;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void setDistanceFromActiveCamera() noexcept;
	const float getDistanceFromActiveCamera() const noexcept;
protected:
	float calcDistanceFromActiveCamera( const DirectX::XMFLOAT3 &pos ) const noexcept;
};