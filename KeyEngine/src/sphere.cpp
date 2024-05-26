#include "sphere.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "geometry.h"
#include "rendering_channel.h"


namespace dx = DirectX;

Sphere::Sphere( Graphics &gfx,
	const float radius /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0.0f, 0.0f, 0.0f}*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos},
	m_radius{radius}
{
	auto sphere = Geometry::makeSphereTesselated();
	if ( radius != 1.0f )
	{
		sphere.transform( dx::XMMatrixScaling( radius, radius, radius ) );
	}

	const auto geometryTag = s_geometryTag + std::to_string( radius );
	m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, sphere.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, sphere.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	createAabb( sphere.m_vb );
	setMeshId();

	{
		Effect opaque{rch::opaque, "opaque", true};

		opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pvs = VertexShader::fetch( gfx, "flat_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, sphere.m_vb.getLayout(), *pvs ) );
		opaque.addBindable( std::move( pvs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		struct ColorPSCB
		{
			dx::XMFLOAT3 color{1.0f, 1.0f, 1.0f};
			float padding = 0.0f;
		} colorCb;
		opaque.addBindable( PixelShaderConstantBuffer<ColorPSCB>::fetch( gfx, colorCb, 0u ) );

		opaque.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		addEffect( std::move( opaque ) );
	}
}

void Sphere::setRadius( const float radius ) noexcept
{
	setScale( {radius, radius, radius} );
	m_radius = radius;
}

float Sphere::getRadius() const noexcept
{
	return m_radius;
}