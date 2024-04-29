#include "sphere.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "geometry.h"
#include "rendering_channel.h"


namespace dx = DirectX;

Sphere::Sphere( Graphics &gph,
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
	m_pVertexBuffer = VertexBuffer::fetch( gph, geometryTag, sphere.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gph, geometryTag, sphere.m_indices );

	createAabb( sphere.m_vb );
	setMeshId();

	{
		Effect lambertian{rch::lambert, "lambertian", true};

		auto pvs = VertexShader::fetch( gph, "flat_vs.cso" );
		lambertian.addBindable( InputLayout::fetch( gph, sphere.m_vb.getLayout(), *pvs ) );
		lambertian.addBindable( std::move( pvs ) );

		lambertian.addBindable( PixelShader::fetch( gph, "flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{1.0f, 1.0f, 1.0f};
			float padding = 0.0f;
		} colorCb;
		lambertian.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gph, colorCb, 0u ) );

		lambertian.addBindable( std::make_shared<TransformVSCB>( gph, 0u ) );

		lambertian.addBindable( RasterizerState::fetch( gph, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		addEffect( std::move( lambertian ) );
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