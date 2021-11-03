#pragma once

#include <DirectXMath.h>
#include "constant_buffer.h"
#include "drawable.h"


class TransformVCB
	: public IBindableCloning
{
private:
	const Drawable* m_pDrawable = nullptr;
protected:
	struct Transforms
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldView;
		DirectX::XMMATRIX worldViewProjection;
	};

	static inline std::unique_ptr<VertexConstantBuffer<Transforms>> m_pVcb;
public:
	TransformVCB( Graphics& gph, unsigned slot );

	void bind( Graphics& gph ) cond_noex override;
	void setParentDrawable( const Drawable& parent ) noexcept override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
protected:
	void updateVcb( Graphics& gph, const Transforms& tfs ) cond_noex;
	Transforms getTransforms( Graphics& gph ) cond_noex;
};