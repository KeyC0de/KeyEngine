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

SkyPass::SkyPass( Graphics &gfx,
	const std::string &name,
	const bool useSphere )
	:
	IBindablePass{name},
	m_bUseSphere{useSphere}
{
	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	addPassBindable( std::make_shared<CubeTexture>( gfx, "assets/textures/skybox/space", 0u ) );
	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Wrap ) );
	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthEquals1 ) );
	addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );
	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
	addPassBindable( std::make_shared<SkyVSCB>( gfx ) );
	addPassBindable( PixelShader::fetch( gfx, "sky_ps.cso" ) );
	{
		auto vs = VertexShader::fetch( gfx, "sky_vs.cso" );
		{// cube
			TriangleMesh cube = Geometry::makeCube();
			m_pCubeVb = VertexBuffer::fetch( gfx, s_cubeGeometryTag, cube.m_vb );
			m_pCubeIb = IndexBuffer::fetch( gfx, s_cubeGeometryTag, cube.m_indices );
			m_nCubeIndices = (unsigned)cube.m_indices.size();
			addPassBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *vs ) );
		}
		{// sphere
			TriangleMesh sphere = Geometry::makeSphereTesselated();
			m_pSphereVb = VertexBuffer::fetch( gfx, s_sphereGeometryTag, sphere.m_vb );
			m_pSphereIb = IndexBuffer::fetch( gfx, s_sphereGeometryTag, sphere.m_indices );
			m_nSphereIndices = (unsigned)sphere.m_indices.size();
			addPassBindable( InputLayout::fetch( gfx, sphere.m_vb.getLayout(), *vs ) );
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

void SkyPass::run( Graphics &gfx ) const cond_noex
{
	// no need to inherit from RenderQueuePass to add a Job for a single "special" object
	ASSERT( m_pActiveCamera, "SkyPass - Main camera not specified (null)!" );
	unsigned nIndices;
	m_pActiveCamera->makeActive( gfx, false );
	if ( m_bUseSphere )
	{
		m_pSphereVb->bind( gfx );
		m_pSphereIb->bind( gfx );
		nIndices = m_nSphereIndices;
	}
	else
	{
		m_pCubeVb->bind( gfx );
		m_pCubeIb->bind( gfx );
		nIndices = m_nCubeIndices;
	}
	bind( gfx );
	gfx.drawIndexed( nIndices );
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