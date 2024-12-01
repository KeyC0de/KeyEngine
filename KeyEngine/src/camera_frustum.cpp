#include "camera_frustum.h"
#include "graphics.h"
#include "geometry.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "constant_buffer.h"
#include "rendering_channel.h"
#include "global_constants.h"


namespace dx = DirectX;

CameraFrustum::CameraFrustum( Graphics &gfx,
	const float initialScale/* = 1.0f*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{0.6f, 0.2f, 0.2f, 1.0f}*/,
	const float width /*= 1.0f*/,
	const float height /*= 1.0f*/,
	const float nearZ /*= 0.5f*/,
	const float farZ /*= 200.0f*/,
	const float fovDegrees /*= 90.0f*/ )
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

	auto g = geometry::makeCameraFrustum( width, height, nearZ, farZ, dx::XMConvertToRadians(fovDegrees) );
	if ( initialScale != 1.0f )
	{
		g.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	m_pVertexBuffer = std::make_shared<VertexBuffer>( gfx, g.m_vb );	// we don't share the frustum in the BindableRegistry because each one will be unique
	m_pIndexBuffer = IndexBuffer::fetch( gfx, s_geometryTag, g.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, g_modelVscbSlot, *this );

	createAabb( g.m_vb );
	setMeshId();

	// Draw the frustum with a dimmer color for the pixels that are occluded.
	// How? By leveraging the depth stencil modes
	// We draw the frustum two times
	// 1. Opaque - normal mode
	// 2. DepthReversed - mode & with another color (dimmer) - only the occluded part of the frustum gets drawn
	{
		Material front{rch::wireframe, "wireframe", true};

		front.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );

		front.addBindable( InputLayout::fetch( gfx, m_pVertexBuffer->getLayout(), *pVs ) );
		front.addBindable( std::move( pVs ) );
		front.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		front.addBindable( PixelShaderConstantBuffer<ColorPSCB3>::fetch( gfx, m_colorPscbWireframe, g_modelPscbSlot ) );

		addMaterial( std::move( front ) );
	}
	{
		Material occluded{rch::wireframe, "depthReversed", true};

		occluded.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );

		auto pvs = VertexShader::fetch( gfx, "flat_vs.cso" );

		occluded.addBindable( InputLayout::fetch( gfx, m_pVertexBuffer->getLayout(), *pvs ) );
		occluded.addBindable( std::move( pvs ) );
		occluded.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		occluded.addBindable( PixelShaderConstantBuffer<ColorPSCB4>::fetch( gfx, m_colorPscbDepthReversed, g_modelPscbSlot ) );

		addMaterial( std::move( occluded ) );
	}
}
