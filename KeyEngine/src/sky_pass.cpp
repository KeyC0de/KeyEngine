#include "sky_pass.h"
#include <string>
#include "consumer.h"
#include "producer.h"
#include "camera.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "render_target.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "geometry.h"
#include "cube_texture.h"
#include "skybox_vscb.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "assertions_console.h"
#include "imgui.h"
#include "triangle_mesh.h"


namespace ren
{

SkyPass::SkyPass( Graphics &gph,
	const std::string &name )
	:
	IBindablePass{name}
{
	addConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addConsumer( RenderSurfaceConsumer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
	addPassBindable( std::make_shared<CubeTexture>( gph,
		"assets/textures/skybox/space",
		0u ) );
	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Wrap ) );
	addPassBindable( DepthStencilState::fetch( gph,
		DepthStencilState::Mode::DepthEquals1 ) );
	addPassBindable( RasterizerState::fetch( gph,
		RasterizerState::TwoSided,
		RasterizerState::Solid ) );
	addPassBindable( PrimitiveTopology::fetch( gph ) );
	addPassBindable( std::make_shared<SkyboxVSCB>( gph ) );
	addPassBindable( PixelShader::fetch( gph,
		"skybox_ps.cso" ) );
	{
		auto vs = VertexShader::fetch( gph,
			"skybox_vs.cso" );
		{// cube
			TriangleMesh cube = Geometry::makeCube();
			m_pCubeVb = VertexBuffer::fetch( gph,
				s_cubeGeometryTag,
				cube.m_vb );
			m_pCubeIb = IndexBuffer::fetch( gph,
				s_cubeGeometryTag,
				cube.m_indices );
			m_nCubeIndices = (unsigned)cube.m_indices.size();
			addPassBindable( InputLayout::fetch( gph,
				cube.m_vb.getLayout(),
				*vs ) );
		}
		{// sphere
			TriangleMesh sphere = Geometry::makeTesselatedSphere();
			m_pSphereVb = VertexBuffer::fetch( gph,
				s_sphereGeometryTag,
				sphere.m_vb );
			m_pSphereIb = IndexBuffer::fetch( gph,
				s_sphereGeometryTag,
				sphere.m_indices );
			m_nSphereIndices = (unsigned)sphere.m_indices.size();
		}
		addPassBindable( std::move( vs ) );
	}
	addProducer( RenderSurfaceProducer<IRenderTargetView>::make( "renderTarget",
		m_pRtv ) );
	addProducer( RenderSurfaceProducer<IDepthStencilView>::make( "depthStencil",
		m_pDsv ) );
}

void SkyPass::setActiveCamera( const Camera &cam ) noexcept
{
	m_pActiveCamera = &cam;
}

void SkyPass::run( Graphics &gph ) const cond_noex
{
	// no need to inherit from RenderQueuePass to add a Job for a single "special" object
	ASSERT( m_pActiveCamera, "SkyPass - Main camera not specified (null)!" );
	unsigned nIndices;
	m_pActiveCamera->makeActive( gph,
		false );
	if ( m_bUseSphere )
	{
		m_pSphereVb->bind( gph );
		m_pSphereIb->bind( gph );
		nIndices = m_nSphereIndices;
	}
	else
	{
		m_pCubeVb->bind( gph );
		m_pCubeIb->bind( gph );
		nIndices = m_nCubeIndices;
	}
	bind( gph );
	gph.drawIndexed( nIndices );
}

void SkyPass::reset() cond_noex
{
	pass_;
}

void SkyPass::displayImguiWidgets()
{
	if ( ImGui::Begin( "Skybox" ) )
	{
		ImGui::Checkbox( "Use Sphere",
			&m_bUseSphere );
	}
	ImGui::End();
}


}//ren