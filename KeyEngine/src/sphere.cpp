#include "sphere.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "geometry.h"
#include "rendering_channel.h"


namespace dx = DirectX;

Sphere::Sphere( Graphics &gph,
	const float radius )
{
	auto model = Geometry::makeTesselatedSphere();
	model.transform( dx::XMMatrixScaling( radius,
		radius,
		radius ) );
	const auto geometryTag = "$sphere." + std::to_string( radius );
	m_pVertexBuffer = VertexBuffer::fetch( gph,
		geometryTag,
		model.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gph,
		geometryTag,
		model.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph );

	{
		Effect lambertian{rch::lambert, "lambertian", true};

		auto pvs = VertexShader::fetch( gph,
			"flat_vs.cso" );
		lambertian.addBindable( InputLayout::fetch( gph,
			model.m_vb.getLayout(),
			*pvs ) );
		lambertian.addBindable( std::move( pvs ) );

		lambertian.addBindable( PixelShader::fetch( gph,
			"flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{1.0f, 1.0f, 1.0f};
			float padding;
		} colorCb;
		lambertian.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gph,
			colorCb,
			0u ) );

		lambertian.addBindable( std::make_shared<TransformVSCB>( gph,
			0u ) );

		lambertian.addBindable( RasterizerState::fetch( gph,
			RasterizerState::FrontSided,
			RasterizerState::Solid ) );

		addEffect( std::move( lambertian ) );
	}
}

void Sphere::setPosition( const dx::XMFLOAT3 &pos ) noexcept
{
	this->m_pos = pos;
}

const dx::XMMATRIX Sphere::getTransform() const noexcept
{
	return getPosition();
}

const DirectX::XMMATRIX Sphere::getPosition() const noexcept
{
	return dx::XMMatrixTranslation( m_pos.x,
		m_pos.y,
		m_pos.z );
}
