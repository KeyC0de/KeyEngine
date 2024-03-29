#include "pass_2d.h"
#include "graphics.h"
#include "binder.h"
#include "linker.h"
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
#include "graphics_mode.h"
#include "assertions_console.h"
#include "key_random.h"
#include "math_utils.h"


namespace ren
{

Pass2D::Pass2D( Graphics &gph,
	const std::string &name )
	:
	IBindablePass{name}
{
	ASSERT( gph_mode::get() == gph_mode::_2D, "Not in d2d mode!" );

	addPassBindable( PrimitiveTopology::fetch( gph ) );

	addPassBindable( std::make_shared<DepthStencilState>( gph,
		DepthStencilState::DepthOffStencilOff ) );

	auto pVs = VertexShader::fetch( gph,
		"flat2d_vs.cso" );

	ver::VertexInputLayout vl;
	using Type = ver::VertexInputLayout::ILEementType;
	vl.add( Type::Position2D );
	vl.add( Type::Texture2D );

	ver::VBuffer vb{std::move( vl ), 4u};
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

	addPassBindable( std::make_shared<PixelShader>( gph,
		"flat2d_ps.cso" ) );

	addPassBindable( std::make_shared<Texture>( gph,
		gph.getClientWidth(),
		gph.getClientHeight(),
		0u ) );

#ifdef D2D_INTEROP
	gph.create2dInteroperability();
#endif

	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::FrontSided,
		RasterizerState::Solid ) );

	addPassBindable( std::make_shared<TextureSamplerState>( gph,
		0u,
		TextureSamplerState::FilterMode::Point,
		TextureSamplerState::AddressMode::Clamp ) );

	//m_sprite = std::make_unique<Sprite>( LR"(assets\textures\ba2_icon.png)",
	//	gph );
	//m_player = std::make_unique<SpriteSheet>( LR"(assets\textures\player.png)",
	//	gph,
	//	32,
	//	32 );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
}

/*
void update()
{
	ySpeed += .1f * util::signum( ySpeed );
	y += ySpeed;
	if ( y > g_height - 120 - 1 || y < -100 )
	{
		ySpeed = fabs( ySpeed ) * math::signum( ySpeed ) * -1.0f ;
	}

	m_currentSpriteIndex = ( m_currentSpriteIndex + 1 ) % ( m_spriteOrder.size() );
}
*/

void Pass2D::run( Graphics &gph ) const cond_noex
{
/*	for ( int i = 0; i < 10; ++i )
	{
		gph.drawCircle( m_randomness.rand() % 800,
			m_randomness.rand() % 600,
			m_randomness.rand() % 100,
			D2D1::ColorF{m_randomness.getRandomFloat(), m_randomness.getRandomFloat(), m_randomness.getRandomFloat(), m_randomness.getRandomFloat() / 1.75 + 0.25f},
			static_cast<int>( ceilf( m_randomness.rand() % 8 ) ) );
	}

	// text
	gph.createTextFormat( L"Zrnic" );
	const auto rect = D2D1_RECT_F{100, 100 + y, 200, 120 + y};
	gph.drawText( L"KeyC0de",
		rect,
		D2D1::ColorF{0, 111 / 256.f, 0} );

	// image
	const auto ba2Rect = D2D1_RECT_F{0, 0, m_sprite->getWidth(), m_sprite->getWidth()};
	m_sprite->render( gph,
		ba2Rect,
		ba2Rect,
		.6f );

	m_player->render( gph,
		m_spriteOrder[m_currentSpriteIndex],
		300,
		100 );
*/
	bind( gph );
	gph.drawIndexed( 6u );
}

void Pass2D::reset() cond_noex
{
	pass_;
}


}// namespace ren