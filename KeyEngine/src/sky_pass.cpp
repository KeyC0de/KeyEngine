#include "sky_pass.h"
#include <string>
#include "primitive_topology.h"
#include "binder.h"
#include "linker.h"
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
#include "sky_vscb.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "assertions_console.h"
#ifndef FINAL_RELEASE
#	include "imgui.h"
#endif
#include "triangle_mesh.h"


namespace ren
{

SkyPass::SkyPass( Graphics &gph,
	const std::string &name,
	const bool useSphere )
	:
	IBindablePass{name},
	m_bUseSphere{useSphere}
{
	addPassBindable( PrimitiveTopology::fetch( gph, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	addPassBindable( std::make_shared<CubeTexture>( gph, "assets/textures/skybox/space", 0u ) );
	addPassBindable( TextureSamplerState::fetch( gph, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Wrap ) );
	addPassBindable( DepthStencilState::fetch( gph, DepthStencilState::Mode::DepthEquals1 ) );
	addPassBindable( RasterizerState::fetch( gph, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );
	addPassBindable( PrimitiveTopology::fetch( gph, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
	addPassBindable( std::make_shared<SkyVSCB>( gph ) );
	addPassBindable( PixelShader::fetch( gph, "sky_ps.cso" ) );
	{
		auto vs = VertexShader::fetch( gph, "sky_vs.cso" );
		{// cube
			TriangleMesh cube = Geometry::makeCube();
			m_pCubeVb = VertexBuffer::fetch( gph, s_cubeGeometryTag, cube.m_vb );
			m_pCubeIb = IndexBuffer::fetch( gph, s_cubeGeometryTag, cube.m_indices );
			m_nCubeIndices = (unsigned)cube.m_indices.size();
			addPassBindable( InputLayout::fetch( gph, cube.m_vb.getLayout(), *vs ) );
		}
		{// sphere
			TriangleMesh sphere = Geometry::makeSphereTesselated();
			m_pSphereVb = VertexBuffer::fetch( gph, s_sphereGeometryTag, sphere.m_vb );
			m_pSphereIb = IndexBuffer::fetch( gph, s_sphereGeometryTag, sphere.m_indices );
			m_nSphereIndices = (unsigned)sphere.m_indices.size();
			addPassBindable( InputLayout::fetch( gph, sphere.m_vb.getLayout(), *vs ) );
		}
		addPassBindable( std::move( vs ) );
	}

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
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
	m_pActiveCamera->makeActive( gph, false );
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

void SkyPass::displayImguiWidgets() noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( "Skybox" ) )
	{
		ImGui::Checkbox( "Use Sphere", &m_bUseSphere );
	}
	ImGui::End();
#endif
}


}//ren