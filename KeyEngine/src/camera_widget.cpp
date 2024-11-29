#include "camera_widget.h"
#include "graphics.h"
#include "geometry.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "constant_buffer.h"
#include "vertex_shader.h"
#include "rendering_channel.h"


namespace dx = DirectX;

CameraWidget::CameraWidget( Graphics &gfx,
	const float initialScale/* = 1.0f*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{0.2f, 0.2f, 0.6f, 1.0f}*/ )
{
	std::string diffuseTexturePath;
	if ( std::holds_alternative<DirectX::XMFLOAT4>( colorOrTexturePath ) )
	{
		m_colorPscb.materialColor = std::get<DirectX::XMFLOAT4>( colorOrTexturePath );
	}
	else
	{
		diffuseTexturePath = std::get<std::string>( colorOrTexturePath );
	}

	auto g = geometry::makeCameraWidget();
	if ( initialScale != 1.0f )
	{
		g.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	{
		// we don't have to duplicate the CameraWidget buffers - they're all the same
		m_pVertexBuffer = VertexBuffer::fetch( gfx, s_geometryTag, g.m_vb );
		m_pIndexBuffer = IndexBuffer::fetch( gfx, s_geometryTag, g.m_indices );
		m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
		m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, 0u, *this );
	}

	createAabb( g.m_vb );
	setMeshId();

	{
		Material wireframe{rch::wireframe, "wireframe", true};

		wireframe.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		wireframe.addBindable( InputLayout::fetch( gfx, m_pVertexBuffer->getLayout(), *pVs ) );
		wireframe.addBindable( std::move( pVs ) );

		wireframe.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		wireframe.addBindable( PixelShaderConstantBuffer<ColorPSCB3>::fetch( gfx, m_colorPscbWireframe, 0u ) );

		addMaterial( std::move( wireframe ) );
	}
}
