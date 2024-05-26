#include "material_loader.h"
#include "graphics.h"
#include "effect.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#include "rendering_channel.h"
#include "assertions_console.h"
#include "lighting_mode.h"


// #TODO: PBR Metallic Renderer (UE4 based)
namespace dx = DirectX;

MaterialLoader::MaterialLoader( Graphics &gfx,
	const aiMaterial &aimaterial,
	const std::filesystem::path &modelPath ) cond_noex
	:
	m_modelPath{modelPath.string()}
{
	const auto rootPath = modelPath.parent_path().string() + "/";
	{
		aiString matName;
		aimaterial.Get( AI_MATKEY_NAME, matName );
		m_name = matName.C_Str();
	}

	std::string shaderFileName;

	if constexpr ( lgh_mode::get() == lgh_mode::LightingMode::BlinnPhong )
	{
		// #TODO: add transparent effect
		{// opaque effect
			Effect opaque{rch::opaque, "opaque", true};

			opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

			shaderFileName = "phong_";
			aiString textureFileName;

			m_vertexLayout.add( ver::VertexInputLayout::Position3D );
			m_vertexLayout.add( ver::VertexInputLayout::Normal );
			con::RawLayout cbLayout;
			bool bTexture = false;
			bool bSpecularTextureAlpha = false;
			{// does aiMaterial have a diffuse/albedo texture?
				bool bTextureAlphaChannel = false;
				if ( aimaterial.GetTexture( aiTextureType_DIFFUSE, 0u, &textureFileName ) == aiReturn_SUCCESS )
				{
					bTexture = true;
					shaderFileName += "Dif";
					m_vertexLayout.add( ver::VertexInputLayout::Texture2D );
					auto tex = Texture::fetch( gfx, rootPath + textureFileName.C_Str(), 0u );
					if ( tex->hasAlpha() )
					{
						bTextureAlphaChannel = true;
						shaderFileName += "Alp";
					}
					opaque.addBindable( std::move( tex ) );
				}
				else
				{
					cbLayout.add<con::Float3>( "materialColor" );
				}

				RasterizerState::FaceMode faceMode = bTextureAlphaChannel ?
					RasterizerState::FaceMode::Both :
					RasterizerState::FaceMode::Front;
				opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, faceMode ) );
			}
			{// how about specular texture?
				if ( aimaterial.GetTexture( aiTextureType_SPECULAR, 0u, &textureFileName ) == aiReturn_SUCCESS )
				{
					bTexture = true;
					shaderFileName += "Spc";
					m_vertexLayout.add( ver::VertexInputLayout::Texture2D );
					auto tex = Texture::fetch( gfx, rootPath + textureFileName.C_Str(), 1u );
					bSpecularTextureAlpha = tex->hasAlpha();
					opaque.addBindable( std::move( tex ) );
					// in our system of specular maps the alpha channel contains the gloss (specular power)
					cbLayout.add<con::Bool>( "bSpecularMap" );
					cbLayout.add<con::Bool>( "bSpecularMapAlpha" );
				}
				cbLayout.add<con::Float3>( "modelSpecularColor" );
				cbLayout.add<con::Float>( "modelSpecularGloss" );
			}
			{// how about normal texture?
				if ( aimaterial.GetTexture( aiTextureType_NORMALS, 0, &textureFileName ) == aiReturn_SUCCESS )
				{
					bTexture = true;
					shaderFileName += "Nrm";
					m_vertexLayout.add( ver::VertexInputLayout::Texture2D );
					m_vertexLayout.add( ver::VertexInputLayout::Tangent );
					m_vertexLayout.add( ver::VertexInputLayout::Bitangent );
					opaque.addBindable( Texture::fetch( gfx, rootPath + textureFileName.C_Str(), 2u ) );
					cbLayout.add<con::Bool>( "bNormalMap" );
					cbLayout.add<con::Float>( "normalMapStrength" );
				}
			}
			{// the rest of the Bindables:
				opaque.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );
				auto pVs = VertexShader::fetch( gfx, shaderFileName + "_vs.cso" );
				opaque.addBindable( InputLayout::fetch( gfx, m_vertexLayout, *pVs ) );
				opaque.addBindable( std::move( pVs ) );
				opaque.addBindable( PixelShader::fetch( gfx, shaderFileName + "_ps.cso" ) );
				if ( bTexture )
				{
					opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );
				}

				// Assembling the Pixel Shader Constant Buffer
				con::CBuffer pscb{std::move( cbLayout )};
				if ( auto cbElem = pscb["materialColor"]; cbElem.isValid() )
				{
					aiColor3D difCol = {0.45f, 0.45f, 0.85f};
					aimaterial.Get( AI_MATKEY_COLOR_DIFFUSE, difCol );
					cbElem = reinterpret_cast<dx::XMFLOAT3&>( difCol );
				}
				pscb["bSpecularMap"].setIfValid( true );
				pscb["bSpecularMapAlpha"].setIfValid( bSpecularTextureAlpha );
				if ( auto cbElem = pscb["modelSpecularColor"]; cbElem.isValid() )
				{
					aiColor3D specCol = {0.18f, 0.18f, 0.18f};
					aimaterial.Get( AI_MATKEY_COLOR_SPECULAR, specCol );
					cbElem = reinterpret_cast<dx::XMFLOAT3&>( specCol );
				}
				if ( auto cbElem = pscb["modelSpecularGloss"]; cbElem.isValid() )
				{
					float specGloss = 8.0f;
					aimaterial.Get( AI_MATKEY_SHININESS, specGloss );
					cbElem = specGloss;
				}
				pscb["bNormalMap"].setIfValid( true );
				pscb["normalMapStrength"].setIfValid( 1.0f );
				opaque.addBindable( std::make_unique<PixelShaderConstantBufferEx>( gfx, 0u, std::move( pscb ) ) );
			}
			m_effects.emplace_back( std::move( opaque ) );
		}
		{// shadow map effect
			Effect shadowMap{rch::shadow, "shadowMap", true};

			shadowMap.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

			shadowMap.addBindable( InputLayout::fetch( gfx, m_vertexLayout, *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );
			shadowMap.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

			m_effects.emplace_back( std::move( shadowMap ) );
		}
		{// blur outline mask effect
			Effect blurOutlineMask{rch::blurOutline, "blurOutlineMask", false};

			blurOutlineMask.addBindable( InputLayout::fetch( gfx, m_vertexLayout, *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );
			blurOutlineMask.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

			m_effects.emplace_back( std::move( blurOutlineMask ) );
		}
		{// blur outline draw effect
			Effect blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", false};
			{
				con::RawLayout cbLayout;
				cbLayout.add<con::Float3>( "materialColor" );
				auto cb = con::CBuffer{std::move( cbLayout )};
				cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
				blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
			}
			blurOutlineDraw.addBindable( InputLayout::fetch( gfx, m_vertexLayout, *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );
			blurOutlineDraw.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

			m_effects.emplace_back( std::move( blurOutlineDraw ) );
		}
		{// solid outline mask effect
			Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", false};
			solidOutlineMask.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

			solidOutlineMask.addBindable( InputLayout::fetch( gfx, m_vertexLayout, *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

			m_effects.emplace_back( std::move( solidOutlineMask ) );
		}
		{// solid outline draw effect
			Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", false};

			auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f );
			solidOutlineDraw.addBindable( transformScaledVcb );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "materialColor" );
			auto cb = con::CBuffer{std::move( cbLayout )};
			cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
			solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

			solidOutlineDraw.addBindable( InputLayout::fetch( gfx, m_vertexLayout, *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

			m_effects.emplace_back( std::move( solidOutlineDraw ) );
		}
	}//BlinnPhong
	if constexpr ( lgh_mode::get() == lgh_mode::LightingMode::PBR_UE )
	{
		// #TODO:
	}
}

std::shared_ptr<VertexBuffer> MaterialLoader::makeVertexBuffer( Graphics &gfx,
	const aiMesh &aimesh,
	float scale ) const cond_noex
{
	auto vb = makeVertexBuffer_impl( aimesh );
	if ( scale != 1.0f )
	{
		for ( size_t i = 0u; i < vb.getVertexCount(); ++i )
		{
			dx::XMFLOAT3 &pos = vb[i].getElement<ver::VertexInputLayout::ILEementType::Position3D>();
			pos.x *= scale;
			pos.y *= scale;
			pos.z *= scale;
		}
	}
	return VertexBuffer::fetch( gfx, calcMeshTag( aimesh ), vb );
}

std::shared_ptr<IndexBuffer> MaterialLoader::makeIndexBuffer( Graphics &gfx,
	const aiMesh &aimesh ) const cond_noex
{
	return IndexBuffer::fetch( gfx, calcMeshTag( aimesh ), makeIndexBuffer_impl( aimesh ) );
}

ver::VBuffer MaterialLoader::makeVertexBuffer_impl( const aiMesh &aimesh ) const noexcept
{
	return {m_vertexLayout, aimesh};
}

std::vector<unsigned> MaterialLoader::makeIndexBuffer_impl( const aiMesh &aimesh ) const noexcept
{
	std::vector<unsigned> indices;
	indices.reserve( aimesh.mNumFaces * 3 );
	for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i )
	{
		const auto &face = aimesh.mFaces[i];
		ASSERT( face.mNumIndices == 3, "Non-triangle face detected!" );
		indices.push_back( face.mIndices[0] );
		indices.push_back( face.mIndices[1] );
		indices.push_back( face.mIndices[2] );
	}
	return indices;
}

std::string MaterialLoader::calcMeshTag( const aiMesh &aimesh ) const noexcept
{
	return m_modelPath + "%" + aimesh.mName.C_Str();
}

std::vector<Effect> MaterialLoader::getEffects() const noexcept
{
	return m_effects;
}