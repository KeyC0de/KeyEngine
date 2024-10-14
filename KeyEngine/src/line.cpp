#include "line.h"
#include "graphics.h"
#include "node.h"
#include "geometry.h"
#include "dynamic_constant_buffer.h"
#include "index_buffer.h"
#include "vertex_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "rasterizer_state.h"
#include "transform_vscb.h"
#include "constant_buffer_ex.h"
#include "blend_state.h"
#include "rendering_channel.h"
#include "utils.h"


namespace dx = DirectX;

Line::Line( Graphics &gfx,
	const float lengthScale /*= 1.0f*/,
	const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath /*= DirectX::XMFLOAT4{1.0f, 0.4f, 0.4f, 1.0f}*/ )
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

	auto line = Geometry::makeLine();
	if ( lengthScale != 1.0f )
	{
		line.transform( dx::XMMatrixScaling( lengthScale, lengthScale, lengthScale ) );
	}

	{
		using namespace std::string_literals;
		const auto geometryTag = s_geometryTag + "#"s + std::to_string( lengthScale );

		m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, line.m_vb );
		m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, line.m_indices );
		m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
		m_pTransformVscb = std::make_unique<TransformVSCB>( gfx, 0u, *this );
	}

	using Type = ver::VertexInputLayout::ILEementType;
	m_aabb = std::make_pair( line.m_vb[0].getElement<Type::Position3D>(), line.m_vb[1].getElement<Type::Position3D>() );
	setMeshId();

	if ( m_colorPscb.materialColor.w < 1.0f )
	{// transparent reflectance material
		Material transparent{rch::transparent, "transparent", true};

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		transparent.addBindable( InputLayout::fetch( gfx, line.m_vb.getLayout(), *pVs ) );
		transparent.addBindable( std::move( pVs ) );

		transparent.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = m_colorPscb.materialColor;
		transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		transparent.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		transparent.addBindable( std::make_shared<BlendState>( gfx, BlendState::Mode::Alpha, 0u, m_colorPscb.materialColor.w ) );

		addMaterial( std::move( transparent ) );
	}
	else
	{// opaque reflectance material
		Material opaque{rch::opaque, "opaque", true};

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, line.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = m_colorPscb.materialColor;
		opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		addMaterial( std::move( opaque ) );
	}
	{// shadow map material
		Material shadowMap{rch::shadow, "shadow", false};

		shadowMap.addBindable( InputLayout::fetch( gfx, line.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addMaterial( std::move( shadowMap ) );
	}
}
