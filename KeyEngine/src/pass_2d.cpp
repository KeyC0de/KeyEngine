#include "pass_2d.h"
#include "consumer.h"
#include "producer.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "blend_state.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "dynamic_vertex_buffer.h"
#include "primitive_topology.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "os_utils.h"
#include "texture.h"


namespace ren
{

Pass2D::Pass2D( Graphics &gph,
	const std::string &name )
	:
	IBindablePass{name}
{
	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );

	addPassBindable( PrimitiveTopology::fetch( gph ) );

	addPassBindable( std::make_shared<DepthStencilState>( gph,
		DepthStencilState::DepthOffStencilOff ) );

	addPassBindable( std::make_shared<BlendState>( gph,
		BlendState::NoBlend,
		0 ) );

	auto pVs = VertexShader::fetch( gph,
		"flat2d_vs.cso" );

	ver::VertexInputLayout vl;
	using Type = ver::VertexInputLayout::VertexInputLayoutElementType;
	vl.add( Type::Position2D );
	vl.add( Type::Texture2D );

	ver::Buffer vb{std::move( vl ), 4u};
	vb[0].element<Type::Position2D>() = {-1.0f, 1.0f};
	vb[1].element<Type::Position2D>() = {1.0f, 1.0f};
	vb[2].element<Type::Position2D>() = {1.0f, -1.0f};
	vb[3].element<Type::Position2D>() = {-1.0f, -1.0f};
	vb[0].element<Type::Texture2D>() = {0.0f, 0.0f};
	vb[1].element<Type::Texture2D>() = {1.0f, 0.0f};
	vb[2].element<Type::Texture2D>() = {1.0f, 1.0f};
	vb[3].element<Type::Texture2D>() = {0.0f, 1.0f};

	addPassBindable( InputLayout::fetch( gph,
		vb.getLayout(),
		*pVs ) );

	addPassBindable( std::make_shared<VertexBuffer>( gph,
		vb ) );

	addPassBindable( std::move( pVs ) );

	std::vector<unsigned> indices = {0, 1, 2, 2, 3, 0};
	addPassBindable( std::make_shared<IndexBuffer>( gph,
		indices ) );

	addPassBindable( std::make_shared<Texture>( gph,
		gph.getClientWidth(),
		gph.getClientHeight(),
		0u ) );

	addPassBindable( std::make_shared<PixelShader>( gph,
		"flat2d_ps.cso" ) );

	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::FrontSided,
		RasterizerState::Solid ) );

	addPassBindable( std::make_shared<TextureSamplerState>( gph,
		0u,
		TextureSamplerState::FilterMode::Point,
		TextureSamplerState::AddressMode::Clamp ) );
}

void Pass2D::run( Graphics &gph ) const cond_noex
{
	bindPass( gph );
	gph.drawIndexed( 6u );
}

void Pass2D::reset() cond_noex
{
	pass_;
}


}// ren