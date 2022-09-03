#include "camera_widget.h"
#include "dynamic_vertex_buffer.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "rendering_channel.h"


namespace dx = DirectX;

CameraWidget::CameraWidget( Graphics &gph )
{
	ver::VertexInputLayout vertexLayout;
	vertexLayout.add( ver::VertexInputLayout::Position3D );

	ver::Buffer vb{std::move( vertexLayout )};
	{
		const float x = 4.0f / 3.0f * 0.75f;
		const float y = 1.0f * 0.75f;
		const float z = -2.0f;
		const float xHalf = x * 0.5f;
		const float tSpace = y * 0.2f;
		vb.emplaceVertex( dx::XMFLOAT3{-x, y, 0.0f} );
		vb.emplaceVertex( dx::XMFLOAT3{x, y, 0.0f} );
		vb.emplaceVertex( dx::XMFLOAT3{x, -y, 0.0f} );
		vb.emplaceVertex( dx::XMFLOAT3{-x, -y, 0.0f} );
		vb.emplaceVertex( dx::XMFLOAT3{0.0f, 0.0f, z} );
		vb.emplaceVertex( dx::XMFLOAT3{-xHalf, y + tSpace, 0.0f} );
		vb.emplaceVertex( dx::XMFLOAT3{xHalf, y + tSpace, 0.0f} );
		vb.emplaceVertex( dx::XMFLOAT3{0.0f, y + tSpace + xHalf, 0.0f} );
	}

	std::vector<unsigned> indices;
	{
		indices.push_back( 0 );
		indices.push_back( 1 );
		indices.push_back( 1 );
		indices.push_back( 2 );
		indices.push_back( 2 );
		indices.push_back( 3 );
		indices.push_back( 3 );
		indices.push_back( 0 );
		indices.push_back( 0 );
		indices.push_back( 4 );
		indices.push_back( 1 );
		indices.push_back( 4 );
		indices.push_back( 2 );
		indices.push_back( 4 );
		indices.push_back( 3 );
		indices.push_back( 4 );
		indices.push_back( 5 );
		indices.push_back( 6 );
		indices.push_back( 6 );
		indices.push_back( 7 );
		indices.push_back( 7 );
		indices.push_back( 5 );
	}

	// we don't have to duplicate the CameraWidget buffers - they're all the same
	const auto tag = "$cam";
	m_pVertexBuffer = VertexBuffer::fetch( gph,
		tag,
		vb );
	m_pIndexBuffer = IndexBuffer::fetch( gph,
		tag,
		indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	{
		Effect lambertian{rch::lambert, "lambertian", true};

		auto pVs = VertexShader::fetch( gph,
			"flat_vs.cso" );
		lambertian.addBindable( InputLayout::fetch( gph,
			m_pVertexBuffer->getLayout(),
			*pVs ) );
		lambertian.addBindable( std::move( pVs ) );

		lambertian.addBindable( PixelShader::fetch( gph,
			"flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{0.2f, 0.2f, 0.6f};
			float paddingPlaceholder = 0.0f;
		} colorPcb;

		lambertian.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gph,
			colorPcb,
			0u ) );
		lambertian.addBindable( std::make_shared<TransformVSCB>( gph,
			0u ) );
		lambertian.addBindable( RasterizerState::fetch( gph,
			RasterizerState::FrontSided,
			RasterizerState::Solid ) );

		addEffect( std::move( lambertian ) );
	}
}

void CameraWidget::setPosition( const DirectX::XMFLOAT3 &pos )
{
	m_pos = pos;
}

void CameraWidget::setRotation( const DirectX::XMFLOAT3 &rot )
{
	m_rot = rot;
}

const DirectX::XMMATRIX CameraWidget::getTransform() const noexcept
{
	return dx::XMMatrixRotationRollPitchYawFromVector( dx::XMLoadFloat3( &m_rot ) ) *
		dx::XMMatrixTranslationFromVector( dx::XMLoadFloat3( &m_pos ) );
}

const DirectX::XMMATRIX CameraWidget::calcPosition() const noexcept
{
	return dx::XMMatrixTranslationFromVector( dx::XMLoadFloat3( &m_pos ) );
}

const DirectX::XMMATRIX CameraWidget::calcRotation() const noexcept
{
	return dx::XMMatrixRotationRollPitchYawFromVector( dx::XMLoadFloat3( &m_rot ) );
}