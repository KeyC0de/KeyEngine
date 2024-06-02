#pragma once

#include <DirectXMath.h>
#include "bindable.h"
#include "dynamic_constant_buffer.h"


template<typename CB>
class VertexShaderConstantBuffer;
class Graphics;
class IImGuiConstantBufferVisitor;
class Mesh;

class TransformVSCB
	: public IBindableCloning
{
protected:
	const Mesh *m_pMesh = nullptr;
	struct Transforms
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldView;
		DirectX::XMMATRIX worldViewProjection;
	};
private:
	std::unique_ptr<VertexShaderConstantBuffer<Transforms>> m_pVscb;
public:
	TransformVSCB( Graphics &gfx, const unsigned slot, const Mesh &mesh );
	TransformVSCB( Graphics &gfx, const unsigned slot );
	TransformVSCB( const TransformVSCB &rhs );
	TransformVSCB& operator=( const TransformVSCB &rhs );
	TransformVSCB( TransformVSCB &&rhs ) noexcept;
	TransformVSCB& operator=( TransformVSCB &&rhs ) noexcept;

	void bind( Graphics &gfx ) cond_noex override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
	std::unique_ptr<IBindableCloning> clone() noexcept override;
protected:
	void update( Graphics &gfx, const Transforms &tfs ) cond_noex;
	void bindCb( Graphics &gfx ) cond_noex;
	Transforms getTransforms( Graphics &gfx ) cond_noex;
};


class TransformScaleVSCB
	: public TransformVSCB
{
	con::CBuffer m_cbScale;
public:
	TransformScaleVSCB( Graphics &gfx, const unsigned slot, const float scale );
	TransformScaleVSCB( const TransformScaleVSCB &rhs );
	TransformScaleVSCB& operator=( const TransformScaleVSCB &rhs );
	TransformScaleVSCB( TransformScaleVSCB &&rhs ) noexcept;
	TransformScaleVSCB& operator=( TransformScaleVSCB &&rhs ) noexcept;

	void accept( IImGuiConstantBufferVisitor &ev ) override;
	void bind( Graphics &gfx ) cond_noex override;
	void setMesh( const Mesh &mesh ) noexcept override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
	std::unique_ptr<IBindableCloning> clone() noexcept override;
private:
	static con::RawLayout calcCbLayout();
};
