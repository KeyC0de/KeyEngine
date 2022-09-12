#include "frustum.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "rendering_channel.h"


namespace dx = DirectX;

Frustum::Frustum( Graphics &gph,
	const float width,
	const float height,
	const float nearZ,
	const float farZ )
{
	setupVertexBuffer( gph,
		width,
		height,
		nearZ,
		farZ );

	std::vector<unsigned> indices;
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 1 );
	indices.push_back( 2 );
	indices.push_back( 2 );
	indices.push_back( 3 );
	indices.push_back( 3 );
	indices.push_back( 0 );
	indices.push_back( 4 );
	indices.push_back( 5 );
	indices.push_back( 5 );
	indices.push_back( 6 );
	indices.push_back( 6 );
	indices.push_back( 7 );
	indices.push_back( 7 );
	indices.push_back( 4 );
	indices.push_back( 0 );
	indices.push_back( 4 );
	indices.push_back( 1 );
	indices.push_back( 5 );
	indices.push_back( 2 );
	indices.push_back( 6 );
	indices.push_back( 3 );
	indices.push_back( 7 );
	m_pIndexBuffer = IndexBuffer::fetch( gph,
		"$frustum",
		indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	// Draw the frustum with a dimmer color for the pixels that are occluded.
	// How? By leveraging the depth stencil modes
	// We draw the frustum two times
	// 1. normal lambertian
	// 2. DepthReversed mode and another color (dimmer) - only the occluded part of the frustum gets drawn
	{
		Effect lambert{rch::lambert, "lambertian", true};

		auto pVs = VertexShader::fetch( gph,
			"flat_vs.cso" );

		lambert.addBindable( InputLayout::fetch( gph,
			m_pVertexBuffer->getLayout(),
			*pVs ) );
		lambert.addBindable( std::move( pVs ) );
		lambert.addBindable( PixelShader::fetch( gph,
			"flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{0.6f, 0.2f, 0.2f};
			float padding = 0.0f;
		} colorPcb;
		lambert.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gph,
			colorPcb,
			0u ) );
		lambert.addBindable( std::make_shared<TransformVSCB>( gph,
			0u ) );
		lambert.addBindable( RasterizerState::fetch( gph,
			RasterizerState::FrontSided,
			RasterizerState::Solid ) );

		addEffect( std::move( lambert ) );
	}
	{
		Effect occluded{rch::lambert, "depthReversed", true};

		auto pvs = VertexShader::fetch( gph,
			"flat_vs.cso" );

		occluded.addBindable( InputLayout::fetch( gph,
			m_pVertexBuffer->getLayout(),
			*pvs ) );
		occluded.addBindable( std::move( pvs ) );
		occluded.addBindable( PixelShader::fetch( gph,
			"flat_ps.cso" ) );

		struct ColorPCB2
		{
			dx::XMFLOAT3 color{0.25f, 0.08f, 0.08f};
			float padding = 0.0f;
		} colorPcb;
		occluded.addBindable( PixelShaderConstantBuffer<ColorPCB2>::fetch( gph,
			colorPcb,
			0u ) );
		occluded.addBindable( std::make_shared<TransformVSCB>( gph,
			0u ) );
		occluded.addBindable( RasterizerState::fetch( gph,
			RasterizerState::FrontSided,
			RasterizerState::Solid ) );

		addEffect( std::move( occluded ) );
	}
}

void Frustum::setupVertexBuffer( Graphics &gph,
	const float width,
	const float height,
	const float nearZ,
	const float farZ )
{
	ver::VertexInputLayout vertexLayout;
	vertexLayout.add( ver::VertexInputLayout::Position3D );
	ver::VBuffer vb{std::move( vertexLayout )};
	{
		// A frustum requires 8 vertices.
		const float zRatio = farZ / nearZ;
		const float nearX = width / 2.0f;
		const float nearY = height / 2.0f;
		const float farX = nearX * zRatio;
		const float farY = nearY * zRatio;
		vb.emplaceVertex( dx::XMFLOAT3{-nearX, nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{nearX, nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{nearX, -nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{-nearX, -nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{-farX, farY, farZ} );
		vb.emplaceVertex( dx::XMFLOAT3{farX, farY, farZ} );
		vb.emplaceVertex( dx::XMFLOAT3{farX, -farY, farZ} );
		vb.emplaceVertex( dx::XMFLOAT3{-farX, -farY, farZ} );
	}
	// we don't share the frustum in the BindableMap because each will be unique
	m_pVertexBuffer = std::make_shared<VertexBuffer>( gph,
		vb );
}

void Frustum::setPosition( const DirectX::XMFLOAT3 &pos ) noexcept
{
	this->m_pos = pos;
}

void Frustum::setRotation( const DirectX::XMFLOAT3 &rot ) noexcept
{
	this->m_rot = rot;
}

const DirectX::XMMATRIX Frustum::getTransform() const noexcept
{
	return calcRotation() * calcPosition();
}

const DirectX::XMMATRIX Frustum::calcRotation() const noexcept
{
	return dx::XMMatrixRotationRollPitchYawFromVector( dx::XMLoadFloat3( &m_rot ) );
}

const DirectX::XMMATRIX Frustum::calcPosition() const noexcept
{
	return dx::XMMatrixTranslationFromVector( dx::XMLoadFloat3( &m_pos ) );
}