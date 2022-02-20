#include "material_loader.h"
#include "graphics.h"
#include "effect.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vcb.h"
#include "transform_scale_vcb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler.h"
#include "rasterizer.h"
#include "constant_buffer_ex.h"
#include "rendering_channel.h"
#include "assertions_console.h"


namespace dx = DirectX;

MaterialLoader::MaterialLoader( Graphics& gph,
	const aiMaterial& aimaterial,
	const std::filesystem::path& modelPath ) cond_noex
	:
	m_modelPath{modelPath.string()}
{
	const auto rootPath = modelPath.parent_path().string() + "/";
	{
		aiString matName;
		aimaterial.Get( AI_MATKEY_NAME,
			matName );
		m_name = matName.C_Str();
	}
	{
	// lambertian effect
		Effect lambertian{rch::lambert, "lambertian", true};

		std::string shaderFileName = "phong";
		aiString textureFileName;

		m_vertexLayout.add( ver::VertexLayout::Position3D );
		m_vertexLayout.add( ver::VertexLayout::Normal );
		con::RawLayout cbLayout;
		bool bTexture = false;
		bool bSpecularTextureAlpha = false;
		{
		// does aiMaterial have a diffuse texture?
			bool bTextureAlphaChannel = false;
			if ( aimaterial.GetTexture( aiTextureType_DIFFUSE,
				0u,
				&textureFileName ) == aiReturn_SUCCESS )
			{
				bTexture = true;
				shaderFileName += "Dif";
				m_vertexLayout.add( ver::VertexLayout::Texture2D );
				auto tex = Texture::fetch( gph,
					rootPath + textureFileName.C_Str(),
					0u );
				if ( tex->hasAlpha() )
				{
					bTextureAlphaChannel = true;
					shaderFileName += "Alp";
				}
				lambertian.addBindable( std::move( tex ) );
			}
			else
			{
				cbLayout.add<con::Float3>( "materialColor" );
			}
			lambertian.addBindable( Rasterizer::fetch( gph,
				bTextureAlphaChannel ) );
		}
		{
		// how about specular?
			if ( aimaterial.GetTexture( aiTextureType_SPECULAR,
				0u,
				&textureFileName ) == aiReturn_SUCCESS )
			{
				bTexture = true;
				shaderFileName += "Spc";
				m_vertexLayout.add( ver::VertexLayout::Texture2D );
				auto tex = Texture::fetch( gph,
					rootPath + textureFileName.C_Str(),
					1u );
				bSpecularTextureAlpha = tex->hasAlpha();
				lambertian.addBindable( std::move( tex ) );
				// in our system of specular maps the alpha channel contains the gloss (specular power)
				cbLayout.add<con::Bool>( "bSpecularMap" );
				cbLayout.add<con::Bool>( "bSpecularMapAlpha" );
			}
			cbLayout.add<con::Float3>( "modelSpecularColor" );
			cbLayout.add<con::Float>( "modelSpecularGloss" );
		}
		{
		// normal texture?
			if ( aimaterial.GetTexture( aiTextureType_NORMALS,
				0,
				&textureFileName ) == aiReturn_SUCCESS )
			{
				bTexture = true;
				shaderFileName += "Nrm";
				m_vertexLayout.add( ver::VertexLayout::Texture2D );
				m_vertexLayout.add( ver::VertexLayout::Tangent );
				m_vertexLayout.add( ver::VertexLayout::Bitangent );
				lambertian.addBindable( Texture::fetch( gph,
					rootPath + textureFileName.C_Str(),
					2u ) );
				cbLayout.add<con::Bool>( "bNormalMap" );
				cbLayout.add<con::Float>( "normalMapStrength" );
			}
		}
		{
		// the rest of the Bindables:
			lambertian.addBindable( std::make_shared<TransformVCB>( gph,
				0u ) );
			auto pVs = VertexShader::fetch( gph,
				shaderFileName + "_vs.cso" );
			lambertian.addBindable( InputLayout::fetch( gph,
				m_vertexLayout,
				*pVs ) );
			lambertian.addBindable( std::move( pVs ) );
			lambertian.addBindable( PixelShader::fetch( gph,
				shaderFileName + "_ps.cso" ) );
			if ( bTexture )
			{
				lambertian.addBindable( TextureSampler::fetch( gph,
					0u,
					TextureSampler::FilterMode::Anisotropic,
					TextureSampler::AddressMode::Wrap ) );
			}

			// Assembling the Pixel Shader Constant Buffer
			con::Buffer pcb{std::move( cbLayout )};
			if ( auto cbElem = pcb["materialColor"]; cbElem.isValid() )
			{
				aiColor3D difCol = {0.45f, 0.45f, 0.85f};
				aimaterial.Get( AI_MATKEY_COLOR_DIFFUSE,
					difCol );
				cbElem = reinterpret_cast<dx::XMFLOAT3&>( difCol );
			}
			pcb["bSpecularMap"].setIfValid( true );
			pcb["bSpecularMapAlpha"].setIfValid( bSpecularTextureAlpha );
			if ( auto cbElem = pcb["modelSpecularColor"]; cbElem.isValid() )
			{
				aiColor3D specCol = {0.18f, 0.18f, 0.18f};
				aimaterial.Get( AI_MATKEY_COLOR_SPECULAR,
					specCol );
				cbElem = reinterpret_cast<dx::XMFLOAT3&>( specCol );
			}
			if ( auto cbElem = pcb["modelSpecularGloss"]; cbElem.isValid() )
			{
				float specGloss = 8.0f;
				aimaterial.Get( AI_MATKEY_SHININESS,
					specGloss );
				cbElem = specGloss;
			}
			pcb["bNormalMap"].setIfValid( true );
			pcb["normalMapStrength"].setIfValid( 1.0f );
			lambertian.addBindable( std::make_unique<PixelConstantBufferEx>( gph,
				0u,
				std::move( pcb ) ) );
		}
		m_effects.emplace_back( std::move( lambertian ) );
	}
	{
	// shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", true};

		shadowMap.addBindable( InputLayout::fetch( gph,
			m_vertexLayout,
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );
		shadowMap.addBindable( std::make_shared<TransformVCB>( gph,
			0u ) );

		m_effects.emplace_back( std::move( shadowMap ) );
	}
	{
	// blur outline mask effect
		Effect blurOutlineMask{rch::blurOutline, "blurOutlineMask", false};

		blurOutlineMask.addBindable( InputLayout::fetch( gph,
			m_vertexLayout,
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );
		blurOutlineMask.addBindable( std::make_shared<TransformVCB>( gph,
			0u ) );

		m_effects.emplace_back( std::move( blurOutlineMask ) );
	}
	{
	// blur outline draw effect
		Effect blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", false};
		{
			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "materialColor" );
			auto cb = con::Buffer{std::move( cbLayout )};
			cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
			blurOutlineDraw.addBindable( std::make_shared<PixelConstantBufferEx>( gph,
				0u,
				cb ) );
		}
		blurOutlineDraw.addBindable( InputLayout::fetch( gph,
			m_vertexLayout,
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );
		blurOutlineDraw.addBindable( std::make_shared<TransformVCB>( gph,
			0u ) );

		m_effects.emplace_back( std::move( blurOutlineDraw ) );
	}
	{
	// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", false};
		solidOutlineMask.addBindable( std::make_shared<TransformVCB>( gph,
			0u ) );

		solidOutlineMask.addBindable( InputLayout::fetch( gph,
			m_vertexLayout,
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		m_effects.emplace_back( std::move( solidOutlineMask ) );
	}
	{
	// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", false};

		auto transformScaledVcb = std::make_shared<TransformScaleVCB>( gph,
			0u,
			1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::Buffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT4{1.0f, 0.4f, 0.4f, 1.0f};
		solidOutlineDraw.addBindable( std::make_shared<PixelConstantBufferEx>( gph,
			0u,
			cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gph,
			m_vertexLayout,
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		m_effects.emplace_back( std::move( solidOutlineDraw ) );
	}
}

ver::Buffer MaterialLoader::extractVertexBuffer( const aiMesh& aimesh ) const noexcept
{
	return {m_vertexLayout, aimesh};
}

std::vector<unsigned> MaterialLoader::extractIndexBuffer( const aiMesh& aimesh ) const noexcept
{
	std::vector<unsigned> indices;
	indices.reserve( aimesh.mNumFaces * 3 );
	for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i )
	{
		const auto& face = aimesh.mFaces[i];
		ASSERT( face.mNumIndices == 3, "Non-triangle face detected!" );
		indices.push_back( face.mIndices[0] );
		indices.push_back( face.mIndices[1] );
		indices.push_back( face.mIndices[2] );
	}
	return indices;
}

std::shared_ptr<VertexBuffer> MaterialLoader::makeVertexBuffer( Graphics& gph,
	const aiMesh& aimesh,
	float scale ) const cond_noex
{
	auto vb = extractVertexBuffer( aimesh );
	if ( scale != 1.0f )
	{
		for ( size_t i = 0u; i < vb.getVertexCount(); ++i )
		{
			dx::XMFLOAT3& pos = vb[i].getMember<ver::VertexLayout::MemberType::Position3D>();
			pos.x *= scale;
			pos.y *= scale;
			pos.z *= scale;
		}
	}
	return VertexBuffer::fetch( gph,
		makeMeshTag( aimesh ),
		vb );
}

std::shared_ptr<IndexBuffer> MaterialLoader::makeIndexBuffer( Graphics& gph,
	const aiMesh& aimesh ) const cond_noex
{
	return IndexBuffer::fetch( gph,
		makeMeshTag( aimesh ),
		extractIndexBuffer( aimesh ) );
}

std::string MaterialLoader::makeMeshTag( const aiMesh& aimesh ) const noexcept
{
	return m_modelPath + "%" + aimesh.mName.C_Str();
}

std::vector<Effect> MaterialLoader::getEffects() const noexcept
{
	return m_effects;
}