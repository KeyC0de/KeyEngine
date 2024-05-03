#pragma once

#include <DirectXMath.h>
#include "constant_buffer.h"
#include "mesh.h"
#include "dynamic_constant_buffer.h"


class TransformVSCB
	: public IBindableCloning
{
	const Mesh *m_pMesh = nullptr;
protected:
	struct Transforms
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldView;
		DirectX::XMMATRIX worldViewProjection;
	};
private:
	std::unique_ptr<VertexShaderConstantBuffer<Transforms>> m_pVscb;
public:
	TransformVSCB( Graphics &gph, const unsigned slot );
	TransformVSCB( const TransformVSCB &rhs );
	TransformVSCB& operator=( const TransformVSCB &rhs );
	TransformVSCB( TransformVSCB &&rhs ) noexcept;
	TransformVSCB& operator=( TransformVSCB &&rhs ) noexcept;

	void bind( Graphics &gph ) cond_noex override;
	void setMesh( const Mesh &mesh ) noexcept override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
	std::unique_ptr<IBindableCloning> clone() noexcept override;
protected:
	void update( Graphics &gph, const Transforms &tfs ) cond_noex;
	void bindCb( Graphics &gph ) cond_noex;
	Transforms getTransforms( Graphics &gph ) cond_noex;
};


class TransformScaleVSCB
	: public TransformVSCB
{
	con::CBuffer m_cbScale;
public:
	TransformScaleVSCB( Graphics &gph, const unsigned slot, const float scale );
	TransformScaleVSCB( const TransformScaleVSCB &rhs );
	TransformScaleVSCB& operator=( const TransformScaleVSCB &rhs );
	TransformScaleVSCB( TransformScaleVSCB &&rhs ) noexcept;
	TransformScaleVSCB& operator=( TransformScaleVSCB &&rhs ) noexcept;

	void accept( IImGuiEffectVisitor &ev ) override;
	void bind( Graphics &gph ) cond_noex override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
	std::unique_ptr<IBindableCloning> clone() noexcept override;
private:
	static con::RawLayout calcCbLayout();
};