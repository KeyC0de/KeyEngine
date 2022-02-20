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
#include "texture_sampler.h"
#include "rasterizer.h"
#include "os_utils.h"
#include "texture.h"


namespace ren
{

Pass2D::Pass2D( Graphics& gph,
	const std::string& name )
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

	ver::VertexLayout vl;
	using Type = ver::VertexLayout::MemberType;
	vl.add( Type::Position2D );
	vl.add( Type::Texture2D );

	ver::Buffer vb{std::move( vl ), 4u};
	vb[0].getMember<Type::Position2D>() = {-1.0f, 1.0f};
	vb[1].getMember<Type::Position2D>() = {1.0f, 1.0f};
	vb[2].getMember<Type::Position2D>() = {1.0f, -1.0f};
	vb[3].getMember<Type::Position2D>() = {-1.0f, -1.0f};
	vb[0].getMember<Type::Texture2D>() = {0.0f, 0.0f};
	vb[1].getMember<Type::Texture2D>() = {1.0f, 0.0f};
	vb[2].getMember<Type::Texture2D>() = {1.0f, 1.0f};
	vb[3].getMember<Type::Texture2D>() = {0.0f, 1.0f};

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

	addPassBindable( Rasterizer::fetch( gph,
		false ) );

	addPassBindable( std::make_shared<TextureSampler>( gph,
		0u,
		TextureSampler::FilterMode::Point,
		TextureSampler::AddressMode::Clamp ) );
}

void Pass2D::run( Graphics& gph ) const cond_noex
{
	bindPass( gph );
	gph.drawIndexed( 6u );
}

void Pass2D::reset() cond_noex
{
	pass_;
}


}// ren