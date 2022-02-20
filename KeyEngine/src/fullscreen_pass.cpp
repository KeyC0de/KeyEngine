#include "fullscreen_pass.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "primitive_topology.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "rasterizer.h"
#include "pixel_shader_null.h"


namespace ren
{

namespace dx = DirectX;

FullscreenPass::FullscreenPass( Graphics& gph,
	const std::string& name ) cond_noex
	:
	IBindablePass{name}
{
	// setup fullscreen geometry
	ver::VertexLayout vertexLayout;
	vertexLayout.add( ver::VertexLayout::Position2D );
	//ver::VertexLayout::Texture2D is setup in the vertex shader
	ver::Buffer vb{vertexLayout};
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
		vertexLayout,
		*vs ) );
	addPassBindable( std::move( vs ) );
	addPassBindable( PrimitiveTopology::fetch( gph ) );
	addPassBindable( Rasterizer::fetch( gph,
		false ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
}

void FullscreenPass::run( Graphics& gph ) const cond_noex
{
	bindPass( gph );
	gph.drawIndexed( 6u );
}


}//ren