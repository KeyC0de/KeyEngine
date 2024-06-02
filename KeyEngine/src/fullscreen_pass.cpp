#include "fullscreen_pass.h"
#include "graphics.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "primitive_topology.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "rasterizer_state.h"
#include "depth_stencil_state.h"
#include "texture_sampler_state.h"


namespace ren
{

namespace dx = DirectX;

IFullscreenPass::IFullscreenPass( Graphics &gfx,
	const std::string &name ) cond_noex
	:
	IBindablePass{name}
{
	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	// setup fullscreen geometry
	ver::VertexInputLayout vil;
	vil.add( ver::VertexInputLayout::Position2D );
	//ver::VertexInputLayout::Texture2D is setup in the vertex shader
	ver::VBuffer vb{vil};
	vb.emplaceVertex( dx::XMFLOAT2{-1, 1} );
	vb.emplaceVertex( dx::XMFLOAT2{1, 1} );
	vb.emplaceVertex( dx::XMFLOAT2{-1, -1} );
	vb.emplaceVertex( dx::XMFLOAT2{1, -1} );
	addPassBindable( VertexBuffer::fetch( gfx, s_fullscreenTag, vb ) );

	std::vector<unsigned> indices{0, 1, 2, 1, 3, 2};
	addPassBindable( IndexBuffer::fetch( gfx, s_fullscreenTag, indices ) );

	auto vs = VertexShader::fetch( gfx, "fullscreen_quad_vs.cso" );
	addPassBindable( InputLayout::fetch( gfx, vil, *vs ) );
	addPassBindable( std::move( vs ) );
	addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );
}

void IFullscreenPass::run( Graphics &gfx ) const cond_noex
{
	bind( gfx );
	gfx.drawIndexed( 6u );
}


}//ren