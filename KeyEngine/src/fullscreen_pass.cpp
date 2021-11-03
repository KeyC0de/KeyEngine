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
	addPassSharedBindable( VertexBuffer::fetch( gph,
		"$fullscreen",
		vb ) );
	
	std::vector<unsigned> indices{0, 1, 2, 1, 3, 2};
	addPassSharedBindable( IndexBuffer::fetch( gph,
		"$fullscreen",
		indices ) );

	auto vs = VertexShader::fetch( gph,
		"fullscreen_quad_vs.cso" );
	addPassSharedBindable( InputLayout::fetch( gph,
		vertexLayout,
		*vs ) );
	addPassSharedBindable( std::move( vs ) );
	addPassSharedBindable( PrimitiveTopology::fetch( gph ) );
	addPassSharedBindable( Rasterizer::fetch( gph,
		false ) );
	addPassSharedBindable( PixelShaderNull::fetch( gph ) );
}

void FullscreenPass::run( Graphics& gph ) const cond_noex
{
	bindPassShared( gph );
	gph.drawIndexed( 6u );
}


}//ren