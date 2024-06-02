#include "sphere.h"
#include "graphics.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "constant_buffer_ex.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "geometry.h"
#include "rendering_channel.h"


namespace dx = DirectX;

Sphere::Sphere( Graphics &gfx,
	const float initialScale /*= 1.0f*/,	// radius
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}*/ )
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

	auto sphere = Geometry::makeSphereTesselated();
	if ( initialScale != 1.0f )
	{
		sphere.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	{
		using namespace std::string_literals;
		const auto geometryTag = s_geometryTag + "#"s + std::to_string( (int)initialScale );

		m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, sphere.m_vb );
		m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, sphere.m_indices );
		m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, 0u, *this );
	}

	createAabb( sphere.m_vb );
	setMeshId();

	{
		Material opaque{rch::opaque, "opaque", true};

		opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pvs = VertexShader::fetch( gfx, "flat_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, sphere.m_vb.getLayout(), *pvs ) );
		opaque.addBindable( std::move( pvs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = m_colorPscb.materialColor;
		opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx,0u, cb ) );

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		addMaterial( std::move( opaque ) );
	}
}
