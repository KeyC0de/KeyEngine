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
	const std::string &diffuseTexturePath /*= "assets/models/brick_wall/brick_wall_diffuse.jpg"*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos},
	m_colorPscb{color}
{
	auto plane = Geometry::makePlanarGridTextured( length, width );
	if ( initialScale != 1.0f )
	{
		plane.transform( dx::XMMatrixScaling( initialScale, initialScale, 1.0f ) );
	}

	const auto geometryTag = "$plane." + std::to_string( initialScale * length ) + std::to_string( initialScale * width );

	m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, plane.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, plane.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	createAabb( plane.m_vb );
	setMeshId();

	auto transformVscb = std::make_shared<TransformVSCB>( gfx, 0u );
	if ( color.w < 1.0f )
	{// transparent reflectance material
		Material transparent{rch::transparent, "transparent", true};
		transparent.addBindable( transformVscb );

		transparent.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pVs = VertexShader::fetch( gfx, "plane_vs.cso" );
		transparent.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *pVs ) );
		transparent.addBindable( std::move( pVs ) );

		if ( diffuseTexturePath.empty() && ( color.x != 1.0f || color.y != 1.0f || color.z != 1.0f ) )
		{
			transparent.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

			transparent.addBindable( std::make_shared<PixelShaderConstantBuffer<ColorPSCB>>( gfx, m_colorPscb, 0u ) );
		}
		else
		{
			transparent.addBindable( Texture::fetch( gfx, diffuseTexturePath, 0u ) );

			transparent.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

			transparent.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "modelSpecularColor" );
			cbLayout.add<con::Float>( "modelSpecularGloss" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
			cb["modelSpecularGloss"] = 128.0f;
			transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
		}

		transparent.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		transparent.addBindable( std::make_shared<BlendState>( gfx, BlendState::Mode::Alpha, 0u, color.w ) );

		addMaterial( std::move( transparent ) );
	}
	else
	{// opaque reflectance material
		Material opaque{rch::opaque, "opaque", true};
		opaque.addBindable( transformVscb );

		opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pVs = VertexShader::fetch( gfx, "plane_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

		if ( diffuseTexturePath.empty() && ( color.x != 1.0f || color.y != 1.0f || color.z != 1.0f ) )
		{
			opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

			opaque.addBindable( std::make_shared<PixelShaderConstantBuffer<ColorPSCB>>( gfx, m_colorPscb, 0u ) );
		}
		else
		{
			opaque.addBindable( Texture::fetch( gfx, diffuseTexturePath, 0u ) );

			opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

			opaque.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "modelSpecularColor" );
			cbLayout.add<con::Float>( "modelSpecularGloss" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
			cb["modelSpecularGloss"] = 128.0f;
			opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
		}

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		addMaterial( std::move( opaque ) );
	}
	{// shadow map material
		Material shadowMap{rch::shadow, "shadowMap", true};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		shadowMap.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( shadowMap ) );
	}
	{// blur outline mask material
		Material blurOutlineMask{rch::blurOutline, "blurOutlineMask", false};
		blurOutlineMask.addBindable( transformVscb );

		blurOutlineMask.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( blurOutlineMask ) );
	}
	{// blur outline draw material
		Material blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", false};
		blurOutlineDraw.addBindable( transformVscb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		blurOutlineDraw.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( blurOutlineDraw ) );
	}
	{// solid outline mask material
		Material solidOutlineMask{rch::solidOutline, "solidOutlineMask", true};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( solidOutlineMask ) );
	}
	{// solid outline draw material
		Material solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", true};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gfx, plane.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( solidOutlineDraw ) );
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