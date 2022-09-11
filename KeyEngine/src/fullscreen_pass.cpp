#include "fullscreen_pass.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "primitive_topology.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "rasterizer_state.h"
#include "pixel_shader_null.h"


namespace ren
{

namespace dx = DirectX;

FullscreenPass::FullscreenPass( Graphics &gph,
	const std::string &name ) cond_noex
	:
	IBindablePass{name}
{
	// setup fullscreen geometry
	ver::VertexInputLayout vil;
	vil.add( ver::VertexInputLayout::Position2D );
	//ver::VertexInputLayout::Texture2D is setup in the vertex shader
	ver::VBuffer vb{vil};
	vb.emplaceVertex( dx::XMFLOAT2{-1, 1} );
	vb.emplaceVertex( dx::XMFLOAT2{1, 1} );
	vb.emplaceVertex( dx::XMFLOAT2{-1, -1} );
	vb.emplaceVertex( dx::XMFLOAT2{1, -1} );
	addPassBindable( VertexBuffer::fetch( gph,
		"$fullscreen",
		vb ) );

	std::vector<unsigned> indices{0, 1, 2, 1, 3, 2};
	addPassBindable( IndexBuffer::fetch( gph,
		"$fullscreen",
		indices ) );

	auto vs = VertexShader::fetch( gph,
		"fullscreen_quad_vs.cso" );
	addPassBindable( InputLayout::fetch( gph,
		vil,
		*vs ) );
	addPassBindable( std::move( vs ) );
	addPassBindable( PrimitiveTopology::fetch( gph ) );
	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::FrontSided,
		RasterizerState::Solid ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
}

void FullscreenPass::run( Graphics &gph ) const cond_noex
{
	bind( gph );
	gph.drawIndexed( 6u );
}


}//ren