#pragma once

#include <DirectXMath.h>
#include <memory>
#include <optional>
#include "effect.h"


class Graphics;
class IEffectVisitor;
class MaterialLoader;
class IndexBuffer;
class VertexBuffer;
class PrimitiveTopology;
class InputLayout;
struct aiMesh;

namespace ren
{
	class Renderer;
}

class Drawable
{
protected:
	std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	std::shared_ptr<VertexBuffer> m_pVertexBuffer;
	std::shared_ptr<PrimitiveTopology> m_pPrimitiveTopology;
	std::vector<Effect> m_effects;
	mutable DirectX::XMFLOAT4X4 m_worldTransform;
	int m_distanceFromActiveCamera = 0;
public:
	Drawable() = default;
	Drawable( Graphics &gph, const MaterialLoader &mat, const aiMesh &aimesh, float scale = 1.0f ) noexcept;
	Drawable( const Drawable &rhs ) = delete;
	virtual ~Drawable() noexcept;

	template<class T>
	std::optional<T*> getBindable() noexcept
	{
		for ( auto &effect : m_effects )
		{
			for ( auto &bindable : effect.accessBindables() )
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
	//	\brief  physics/transform stuff
	//	\date	2021/10/26 23:58
	virtual void update( float dt ) cond_noex;
	void render( size_t channels ) const noexcept;
	void bind( Graphics &gph ) const cond_noex;
	void accept( IEffectVisitor &ev );
	const unsigned getIndicesCount() const cond_noex;
	void connectEffectsToRenderer( ren::Renderer &r );
	void setTransform( const DirectX::XMMATRIX &worldTransform ) noexcept;
	//===================================================
	//	\function	getTransform
	//	\brief  returns the world transform matrix
	//	\date	2022/08/20 23:56
	virtual const DirectX::XMMATRIX getTransform() const noexcept;
	void setDistanceFromActiveCamera( int dist ) noexcept;
	const int getDistanceFromActiveCamera() const noexcept;
};