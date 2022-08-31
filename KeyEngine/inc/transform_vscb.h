#pragma once

#include <DirectXMath.h>
#include "constant_buffer.h"
#include "mesh.h"


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
	// #FIXME: try removing static
	// #FIXME: try putting cctors dtor etc. in VertexShaderConstantBuffer
	static inline std::unique_ptr<VertexShaderConstantBuffer<Transforms>> m_pVscb;
public:
	TransformVSCB( Graphics &gph, const unsigned slot );
	//===================================================
	//	\function	TransformVSCB
	//	\brief  cctor that behaves as mctor
	//	\date	2022/08/07 22:47
	//TransformVSCB( const TransformVSCB &rhs );
	//TransformVSCB& operator=( const TransformVSCB &rhs );
	//TransformVSCB( TransformVSCB &&rhs );
	//TransformVSCB& operator=( TransformVSCB &&rhs ) = delete;

	void bind( Graphics &gph ) cond_noex override;
	void setMesh( const Mesh &mesh ) noexcept override;
	std::unique_ptr<IBindableCloning> clone() const noexcept override;
	std::unique_ptr<IBindableCloning> clone() noexcept override;
protected:
	void update( Graphics &gph, const Transforms &tfs ) cond_noex;
	void bindCb( Graphics &gph ) cond_noex;
	Transforms getTransforms( Graphics &gph ) cond_noex;
};