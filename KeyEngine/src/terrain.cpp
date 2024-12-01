#include "terrain.h"
#include "graphics.h"
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
#include "constant_buffer.h"
#include "constant_buffer_ex.h"
#include "rendering_channel.h"
#include "utils.h"
#include "math_utils.h"
#include "d3d_utils.h"
#include "global_constants.h"


namespace dx = DirectX;

Terrain::Terrain( Graphics &gfx,
	const float initialScale /*= 1.0f*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= "assets/models/brick_wall/brick_wall_diffuse.jpg"*/,
	const std::string &heightMapfilename /*= ""*/,
	const int length /*= 100*/,
	const int width /*= 100*/,
	const int normalizeAmount /*= 4*/,
	const int terrainAreaUnitMultiplier /*= 10*/ )
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

	const int lengthVerts = util::ceil( length ) * 2;
	const int widthVerts = util::ceil( width ) * 2;

	auto planarGrid = heightMapfilename.empty() ?
		geometry::makePlanarGridTextured( length, width, lengthVerts, widthVerts ) :
		geometry::makePlanarGridTexturedFromHeighmap( heightMapfilename, normalizeAmount, terrainAreaUnitMultiplier, length, width, lengthVerts, widthVerts );

	if ( initialScale != 1.0f )
	{
		planarGrid.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	{
		using namespace std::string_literals;
		const auto geometryTag = s_geometryTag + "#len"s + std::to_string( length ) + "#wid"s + std::to_string( width ) + "#scale"s + std::to_string( (int)initialScale );

		m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, planarGrid.m_vb );
		m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, planarGrid.m_indices );
		m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, g_modelVscbSlot, *this );
	}

	createAabb( planarGrid.m_vb );
	setMeshId();

	{// opaque reflectance material
		Material opaque{rch::opaque, "opaque", true};

		opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		auto pVs = VertexShader::fetch( gfx, "plane_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		if ( diffuseTexturePath.empty() && ( m_colorPscb.materialColor.x != 1.0f || m_colorPscb.materialColor.y != 1.0f || m_colorPscb.materialColor.z != 1.0f ) )
		{
			opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

			opaque.addBindable( std::make_shared<PixelShaderConstantBuffer<ColorPSCB>>( gfx, m_colorPscb, 0u ) );
		}
		else
		{
			auto lightenTexture = [] ( Bitmap::Texel col ) -> dx::XMVECTOR
				{
					const auto v = Bitmap::colorToVector( col );
					const auto lighter = dx::XMVectorReplicate( 1.2f );
					return dx::XMVectorMultiply( v, lighter );
				};

			opaque.addBindable( Texture::fetch( gfx, diffuseTexturePath, 0u, lightenTexture ) );

			opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

			con::RawLayout cbLayout;
			cbLayout.add<con::Float3>( "cb_modelSpecularColor" );
			cbLayout.add<con::Float>( "cb_modelSpecularGloss" );
			auto cb = con::CBuffer( std::move( cbLayout ) );
			cb["cb_modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
			cb["cb_modelSpecularGloss"] = 32.0f;

			opaque.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

			opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );
		}

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		addMaterial( std::move( opaque ) );
	}
	{//shadow map material
		Material shadowMap{rch::shadow, "shadow", false};

		shadowMap.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		shadowMap.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( shadowMap ) );
	}
	{// solid outline mask material
		Material solidOutlineMask{rch::solidOutline, "solidOutlineMask", false};

		solidOutlineMask.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( solidOutlineMask ) );
	}
	{// solid outline draw material
		Material solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", false};

		solidOutlineDraw.addBindable( std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "cb_materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["cb_materialColor"] = m_colorPscbOutline.materialColor;
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( solidOutlineDraw ) );
	}
	{// wireframe material
		Material wireframe{rch::wireframe, "wireframe", true};

		wireframe.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		wireframe.addBindable( InputLayout::fetch( gfx, m_pVertexBuffer->getLayout(), *pVs ) );
		wireframe.addBindable( std::move( pVs ) );

		wireframe.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		wireframe.addBindable( PixelShaderConstantBuffer<ColorPSCB3>::fetch( gfx, m_colorPscbWireframe, 0u ) );

		addMaterial( std::move( wireframe ) );
	}
}

void Terrain::transformVerticesPosition( ver::VBuffer &vb,
	const DirectX::XMMATRIX &matrix ) noexcept
{
	const size_t numVertices = vb.getVertexCount();
	for ( size_t i = 0; i < numVertices; ++i )
	{
		auto &pos = vb[0].getElement<ver::VertexInputLayout::ILEementType::Position3D>();
		dx::XMStoreFloat3( &pos, dx::XMVector3Transform( dx::XMLoadFloat3( &pos ), matrix ) );
	}
}
