#include "plane.h"
#include "geometry.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "transform_scale_vscb.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#include "imgui.h"
#include "rendering_channel.h"
#include "utils.h"
#include "blend_state.h"


namespace dx = DirectX;

Plane::Plane( Graphics &gph,
	const dx::XMFLOAT4 &color,
	const float scale )
	:
	m_colPcb{color}
{
	auto plane = Geometry::makePlane();
	plane.transform( dx::XMMatrixScaling( scale,
		scale,
		1.0f ) );

	const auto geometryTag = "$plane." + std::to_string( scale );

	m_pVertexBuffer = VertexBuffer::fetch( gph,
		geometryTag,
		plane.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gph,
		geometryTag,
		plane.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph );

	auto transformVscb = std::make_shared<TransformVSCB>( gph,
		0u );
	{
	// lambertian reflectance effect
		Effect lambertian{rch::lambert, "lambertian", true};
		lambertian.addBindable( transformVscb );

		lambertian.addBindable( Texture::fetch( gph,
			"assets/models/brick_wall/brick_wall_diffuse.jpg",
			0u ) );
		lambertian.addBindable( TextureSamplerState::fetch( gph,
			0u,
			TextureSamplerState::FilterMode::Anisotropic,
			TextureSamplerState::AddressMode::Wrap ) );

		auto pVs = VertexShader::fetch( gph,
			"plane_vs.cso" );
		lambertian.addBindable( InputLayout::fetch( gph,
			plane.m_vb.getLayout(),
			*pVs ) );
		lambertian.addBindable( std::move( pVs ) );

		lambertian.addBindable( PixelShader::fetch( gph,
			"plane_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "modelSpecularColor" );
		cbLayout.add<con::Float>( "modelSpecularGloss" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
		cb["modelSpecularGloss"] = 20.0f;
		lambertian.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		lambertian.addBindable( RasterizerState::fetch( gph,
			RasterizerState::FrontSided,
			RasterizerState::Solid ) );

		addEffect( std::move( lambertian ) );
	}
	{
	// shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", true};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( InputLayout::fetch( gph,
			plane.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
	{
	// blur outline mask effect
		Effect blurOutlineMask{rch::blurOutline, "blurOutlineMask", false};
		blurOutlineMask.addBindable( transformVscb );

		blurOutlineMask.addBindable( InputLayout::fetch( gph,
			plane.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineMask ) );
	}/*
	{
	// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", true};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gph,
			plane.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineMask ) );
	}
	{
	// blur outline draw effect
		Effect blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", false};
		blurOutlineDraw.addBindable( transformVscb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
		blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		blurOutlineDraw.addBindable( InputLayout::fetch( gph,
			plane.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineDraw ) );
	}*/
	{
	// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", true};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gph,
			0u,
			1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gph,
			plane.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineDraw ) );
	}
}

void Plane::setPosition( const dx::XMFLOAT3 &pos ) noexcept
{
	this->m_pos = pos;
}

void Plane::setRotation( const DirectX::XMFLOAT3 &rot ) noexcept
{
	this->m_rot = rot;
}

const dx::XMMATRIX Plane::getTransform() const noexcept
{
	return calcRotation() * calcPosition();
}

const dx::XMMATRIX Plane::calcPosition() const noexcept
{
	return dx::XMMatrixTranslationFromVector( dx::XMLoadFloat3( &m_pos ) );
}

const dx::XMMATRIX Plane::calcRotation() const noexcept
{
	return dx::XMMatrixRotationRollPitchYawFromVector( dx::XMLoadFloat3( &m_rot ) );
}

void Plane::displayImguiWidgets( Graphics &gph,
	const std::string &name ) noexcept
{
	if ( ImGui::Begin( name.c_str() ) )
	{
		ImGui::Text( "Position" );
		ImGui::SliderFloat( "X", &m_pos.x, -80.0f, 80.0f, "%.1f" );
		ImGui::SliderFloat( "Y", &m_pos.y, -80.0f, 80.0f, "%.1f" );
		ImGui::SliderFloat( "Z", &m_pos.z, -80.0f, 80.0f, "%.1f" );
		ImGui::Text( "Orientation" );
		ImGui::SliderAngle( "Pitch", &m_rot.x, -180.0f, 180.0f );
		ImGui::SliderAngle( "Yaw", &m_rot.y, -180.0f, 180.0f );
		ImGui::SliderAngle( "Roll", &m_rot.z, -180.0f, 180.0f );
		ImGui::Text( "Shading" );

		auto pBlendState = findBindable<BlendState>();
		if ( pBlendState )
		{
			float factor = (*pBlendState)->getBlendFactorRed();
			ImGui::SliderFloat( "Translucency", &factor, 0.0f, 1.0f );
			(*pBlendState)->fillBlendFactors( factor );
		}
	}
	ImGui::End();
}