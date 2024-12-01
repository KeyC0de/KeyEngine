#include "cube.h"
#include "graphics.h"
#include "node.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "geometry.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#include "blend_state.h"
#include "rendering_channel.h"
#include "utils.h"
#include "global_constants.h"


namespace dx = DirectX;

Cube::Cube( Graphics &gfx,
	const float initialScale /*= 1.0f*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= "assets/models/brick_wall/brick_wall_diffuse.jpg"*/ )
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

	auto cube = geometry::makeCubeIndependentFacesTextured();
	if ( initialScale != 1.0f )
	{
		cube.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	cube.setFlatShadedIndependentNormals();

	{
		using namespace std::string_literals;
		const auto geometryTag = s_geometryTag + "#"s + std::to_string( (int)initialScale );

		m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, cube.m_vb );
		m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, cube.m_indices );
		m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, g_modelVscbSlot, *this );
	}

	createAabb( cube.m_vb );
	setMeshId();

	if ( m_colorPscb.materialColor.w < 1.0f )
	{// transparent reflectance material
		Material transparent{rch::transparent, "transparent", true};

		transparent.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pVs = VertexShader::fetch( gfx, "cube_vs.cso" );
		transparent.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *pVs ) );
		transparent.addBindable( std::move( pVs ) );

		if ( diffuseTexturePath.empty() && ( m_colorPscb.materialColor.x != 1.0f || m_colorPscb.materialColor.y != 1.0f || m_colorPscb.materialColor.z != 1.0f ) )
		{
			transparent.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float4>( "cb_materialColor" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["cb_materialColor"] = m_colorPscb.materialColor;
			transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx,0u, cb ) );
		}
		else
		{
			transparent.addBindable( Texture::fetch( gfx, diffuseTexturePath,0u ) );

			transparent.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

			transparent.addBindable( PixelShader::fetch( gfx, "cube_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "cb_modelSpecularColor" );
			cbLayout.add<con::Float>( "cb_modelSpecularGloss" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["cb_modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
			cb["cb_modelSpecularGloss"] = 128.0f;
			transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
		}

		transparent.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		transparent.addBindable( std::make_shared<BlendState>( gfx, BlendState::Mode::Alpha, 0u, m_colorPscb.materialColor.w ) );

		addMaterial( std::move( transparent ) );
	}
	else
	{// opaque reflectance material
		Material opaque{rch::opaque, "opaque", true};

		opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pVs = VertexShader::fetch( gfx, "cube_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		if ( diffuseTexturePath.empty() && ( m_colorPscb.materialColor.x != 1.0f || m_colorPscb.materialColor.y != 1.0f || m_colorPscb.materialColor.z != 1.0f ) )
		{
			opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float4>( "cb_materialColor" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["cb_materialColor"] = m_colorPscb.materialColor;
			opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx,0u, cb ) );
		}
		else
		{
			opaque.addBindable( Texture::fetch( gfx, diffuseTexturePath, 0u ) );

			opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

			opaque.addBindable( PixelShader::fetch( gfx, "cube_ps.cso" ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "cb_modelSpecularColor" );
			cbLayout.add<con::Float>( "cb_modelSpecularGloss" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["cb_modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
			cb["cb_modelSpecularGloss"] = 128.0f;
			opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
		}

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		addMaterial( std::move( opaque ) );
	}
	{// shadow map material
		Material shadowMap{rch::shadow, "shadow", true};

		shadowMap.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		shadowMap.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( shadowMap ) );
	}
	{// blur outline mask material
		Material blurOutlineMask{rch::blurOutline, "blurOutlineMask", true};

		blurOutlineMask.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( blurOutlineMask ) );
	}
	{// blur outline draw material
		Material blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", true};

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "cb_materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["cb_materialColor"] = m_colorPscbOutline.materialColor;
		blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		blurOutlineDraw.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( blurOutlineDraw ) );
	}
	{// solid outline mask material
		Material solidOutlineMask{rch::solidOutline, "solidOutlineMask", true};

		solidOutlineMask.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( solidOutlineMask ) );
	}
	{// solid outline draw material
		Material solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", true};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "cb_materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["cb_materialColor"] = m_colorPscbOutline.materialColor;
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( solidOutlineDraw ) );
	}
}
