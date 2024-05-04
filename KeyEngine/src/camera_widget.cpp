#include "camera_widget.h"
#include "geometry.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_shader.h"
#include "rendering_channel.h"


namespace dx = DirectX;

CameraWidget::CameraWidget( Graphics &gfx,
	const float initialScale/* = 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot/*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos/*= {0.0f, 0.0f, 0.0f}*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos}
{
	auto g = Geometry::makeCameraWidget();
	if ( initialScale != 1.0f )
	{
		g.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	// we don't have to duplicate the CameraWidget buffers - they're all the same
	m_pVertexBuffer = VertexBuffer::fetch( gfx, s_geometryTag, g.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, s_geometryTag, g.m_indices );

	createAabb( g.m_vb );
	setMeshId();

	{
		Effect wireframe{rch::wireframe, "wireframe", true};

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		wireframe.addBindable( InputLayout::fetch( gfx, m_pVertexBuffer->getLayout(), *pVs ) );
		wireframe.addBindable( std::move( pVs ) );

		wireframe.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{0.2f, 0.2f, 0.6f};
			float paddingPlaceholder = 0.0f;
		} colorPcb;

		wireframe.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gfx, colorPcb, 0u ) );
		wireframe.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

		addEffect( std::move( wireframe ) );
	}
}