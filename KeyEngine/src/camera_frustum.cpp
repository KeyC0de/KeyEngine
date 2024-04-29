#include "camera_frustum.h"
#include "geometry.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "vertex_shader.h"
#include "transform_vscb.h"
#include "pixel_shader.h"
#include "rendering_channel.h"


namespace dx = DirectX;

CameraFrustum::CameraFrustum( Graphics &gph,
	const float width,
	const float height,
	const float nearZ,
	const float farZ,
	const float initialScale/* = 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot/*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos/*= {0.0f, 0.0f, 0.0f}*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos}
{
	auto g = Geometry::makeCameraFrustum( width, height, nearZ, farZ );

	m_pVertexBuffer = std::make_shared<VertexBuffer>( gph, g.m_vb );	// we don't share the frustum in the BindableMap because each will be unique
	m_pIndexBuffer = IndexBuffer::fetch( gph, s_geometryTag, g.m_indices );

	createAabb( g.m_vb );
	setMeshId();

	// Draw the frustum with a dimmer color for the pixels that are occluded.
	// How? By leveraging the depth stencil modes
	// We draw the frustum two times
	// 1. normal lambertian
	// 2. DepthReversed mode and another color (dimmer) - only the occluded part of the frustum gets drawn
	{
		Effect front{rch::wireframe, "wireframe", true};

		auto pVs = VertexShader::fetch( gph, "flat_vs.cso" );

		front.addBindable( InputLayout::fetch( gph, m_pVertexBuffer->getLayout(), *pVs ) );
		front.addBindable( std::move( pVs ) );
		front.addBindable( PixelShader::fetch( gph, "flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{0.6f, 0.2f, 0.2f};
			float padding = 0.0f;
		} colorPcb;
		front.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gph, colorPcb, 0u ) );
		front.addBindable( std::make_shared<TransformVSCB>( gph, 0u ) );

		addEffect( std::move( front ) );
	}
	{
		Effect occluded{rch::wireframe, "depthReversed", true};

		auto pvs = VertexShader::fetch( gph, "flat_vs.cso" );

		occluded.addBindable( InputLayout::fetch( gph, m_pVertexBuffer->getLayout(), *pvs ) );
		occluded.addBindable( std::move( pvs ) );
		occluded.addBindable( PixelShader::fetch( gph, "flat_ps.cso" ) );

		struct ColorPCB2
		{
			dx::XMFLOAT3 color{0.15f, 0.08f, 0.08f};
			float padding = 0.0f;
		} colorPcb;
		occluded.addBindable( PixelShaderConstantBuffer<ColorPCB2>::fetch( gph, colorPcb, 0u ) );
		occluded.addBindable( std::make_shared<TransformVSCB>( gph, 0u ) );

		addEffect( std::move( occluded ) );
	}
}

std::shared_ptr<VertexBuffer>& CameraFrustum::getVertexBuffer()
{
	return m_pVertexBuffer;
}