#include "plane.h"
#include "geometry.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "blend_state.h"
#include "constant_buffer_ex.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif
#include "rendering_channel.h"
#include "utils.h"


namespace dx = DirectX;

Plane::Plane( Graphics &gfx,
	const int length /*= 2*/,
	const int width /*= 2*/,
	const float initialScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT4 &color /*= {1.0f, 1.0f, 1.0f, 1.0f}*/,
	const std::optional<std::string> &diffuseTexturePath /*= "assets/models/brick_wall/brick_wall_diffuse.jpg"*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos},
	m_colPcb{color}
{
	auto plane = Geometry::makePlanarGridTextured( length, width );
	if ( initialScale != 1.0f )
	{
		plane.transform( dx::XMMatrixScaling( initialScale, initialScale, 1.0f ) );
	}

	const auto geometryTag = "$plane." + std::to_string( initialScale * length ) + std::to_string( initialScale * width );

	m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, plane.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, plane.m_indices );

	createAabb( plane.m_vb );
	setMeshId();

	auto transformVscb = std::make_shared<TransformVSCB>( gfx, 0u );
	if ( color.w < 1.0f )
	{// transparent reflectance effect
		Effect transparent{rch::transparent, "transparent", true};
		transparent.addBindable( transformVscb );

		auto pVs = VertexShader::fetch( gfx, "plane_vs.cso" );
		transparent.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *pVs ) );
		transparent.addBindable( std::move( pVs ) );

		if ( diffuseTexturePath.has_value() )
		{
			transparent.addBindable( Texture::fetch( gfx, *diffuseTexturePath, 0u ) );

			transparent.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

			transparent.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "modelSpecularColor" );
			cbLayout.add<con::Float>( "modelSpecularGloss" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
			cb["modelSpecularGloss"] = 20.0f;
			transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
		}
		else
		{
			transparent.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

			transparent.addBindable( std::make_shared<PixelShaderConstantBuffer<ColorPSCB>>( gfx, m_colPcb, 0u ) );
		}

		transparent.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		transparent.addBindable( std::make_shared<BlendState>( gfx, BlendState::Mode::Alpha, 0u, color.w ) );

		addEffect( std::move( transparent ) );
	}
	else
	{// opaque reflectance effect
		Effect opaque{rch::opaque, "opaque", true};
		opaque.addBindable( transformVscb );

		opaque.addBindable( Texture::fetch( gfx, *diffuseTexturePath, 0u ) );

		opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

		auto pVs = VertexShader::fetch( gfx, "plane_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "modelSpecularColor" );
		cbLayout.add<con::Float>( "modelSpecularGloss" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
		cb["modelSpecularGloss"] = 20.0f;
		opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		addEffect( std::move( opaque ) );
	}
	{// shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", true};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
	{// blur outline mask effect
		Effect blurOutlineMask{rch::blurOutline, "blurOutlineMask", false};
		blurOutlineMask.addBindable( transformVscb );

		blurOutlineMask.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineMask ) );
	}
	{// blur outline draw effect
		Effect blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", false};
		blurOutlineDraw.addBindable( transformVscb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		blurOutlineDraw.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineDraw ) );
	}
	{// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", true};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineMask ) );
	}
	{// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", true};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineDraw ) );
	}
}

void Plane::displayImguiWidgets( Graphics &gfx,
	const std::string &name ) noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( name.c_str() ) )
	{
		bool bDirtyRot = false;
		const auto dirtyCheckRot = []( const bool bChanged, bool &bDirtyRot )
		{
			bDirtyRot = bDirtyRot || bChanged;
		};

		ImGui::Text( "Orientation" );
		dx::XMFLOAT3 rot = getRotation();
		dirtyCheckRot( ImGui::SliderAngle( "Pitch", &rot.x, 0.995f * -90.0f, 0.995f * 90.0f ), bDirtyRot );
		dirtyCheckRot( ImGui::SliderAngle( "Yaw", &rot.y, -180.0f, 180.0f ), bDirtyRot );
		dirtyCheckRot( ImGui::SliderAngle( "Roll", &rot.z, -180.0f, 180.0f ), bDirtyRot );
		if ( bDirtyRot )
		{
			setRotation( rot );
		}

		bool bDirtyPos = false;
		const auto dirtyCheckPos = []( const bool bChanged, bool &bDirtyPos )
		{
			bDirtyPos = bDirtyPos || bChanged;
		};

		ImGui::Text( "Position" );
		dx::XMFLOAT3 pos = getPosition();
		dirtyCheckPos( ImGui::SliderFloat( "X", &pos.x, -80.0f, 80.0f, "%.1f" ), bDirtyPos );
		dirtyCheckPos( ImGui::SliderFloat( "Y", &pos.y, -80.0f, 80.0f, "%.1f" ), bDirtyPos );
		dirtyCheckPos( ImGui::SliderFloat( "Z", &pos.z, -80.0f, 80.0f, "%.1f" ), bDirtyPos );
		if ( bDirtyPos )
		{
			setPosition( pos );
		}

		auto pBlendState = findBindable<BlendState>();
		if ( pBlendState )
		{
			bool bDirtyAlpha = false;
			const auto dirtyCheckAlpha = []( const bool bChanged, bool &bDirtyAlpha )
			{
				bDirtyAlpha = bDirtyAlpha || bChanged;
			};

			ImGui::Text( "Blending" );
			float factor = (*pBlendState)->getBlendFactorAlpha();
			dirtyCheckAlpha( ImGui::SliderFloat( "Transparency", &factor, 0.0f, 1.0f ), bDirtyAlpha);

			if ( bDirtyAlpha )
			{
				(*pBlendState)->fillBlendFactors( factor );
			}
		}
	}
	ImGui::End();
#endif
}