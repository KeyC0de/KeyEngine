#include "sky_pass.h"
#include <string>
#include "binder.h"
#include "linker.h"
#include "primitive_topology.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "render_target_view.h"
#include "depth_stencil_view.h"
#include "depth_stencil_state.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "geometry.h"
#include "cube_texture.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "input_layout.h"
#include "triangle_mesh.h"
#include "settings_manager.h"
#include "global_constants.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif


namespace ren
{

SkyPass::SkyPass( Graphics &gfx,
	const std::string &name,
	const bool useSphere )
	:
	IBindablePass{name},
	m_skyVscb{VertexShaderConstantBuffer<SkyTransform>{gfx, g_modelVscbSlot}},
	m_bUseSphere{useSphere}
{
	addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	const auto skyboxFilepath = std::string{"assets/textures/skybox/"} + SettingsManager::getInstance().getSettings().sSkyboxFileName;
	addBindable( std::make_shared<CubeTexture>( gfx, skyboxFilepath, 0u ) );
	addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Wrap ) );
	addBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthReadOnlyEquals1StencilOff ) );
	addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );
	addBindable( PixelShader::fetch( gfx, "sky_ps.cso" ) );
	{
		auto vs = VertexShader::fetch( gfx, "sky_vs.cso" );
		{// cube
			TriangleMesh cube = geometry::makeCube();
			m_pCubeVb = VertexBuffer::fetch( gfx, s_cubeGeometryTag, cube.m_vb );
			m_pCubeIb = IndexBuffer::fetch( gfx, s_cubeGeometryTag, cube.m_indices );
			m_nCubeIndices = (unsigned)cube.m_indices.size();
			addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *vs ) );
		}
		{// sphere
			TriangleMesh sphere = geometry::makeSphereTesselated();
			m_pSphereVb = VertexBuffer::fetch( gfx, s_sphereGeometryTag, sphere.m_vb );
			m_pSphereIb = IndexBuffer::fetch( gfx, s_sphereGeometryTag, sphere.m_indices );
			m_nSphereIndices = (unsigned)sphere.m_indices.size();
			addBindable( InputLayout::fetch( gfx, sphere.m_vb.getLayout(), *vs ) );
		}
		addBindable( std::move( vs ) );
	}

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}

void SkyPass::run( Graphics &gfx ) const cond_noex
{
	const_cast<SkyPass*>( this )->bindSkyVSCB( gfx );

	unsigned nIndices;
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

void SkyPass::bindSkyVSCB( Graphics &gfx ) cond_noex
{
	m_skyVscb.update( gfx, getTransform( gfx ) );
	m_skyVscb.bind( gfx );
}

SkyPass::SkyTransform SkyPass::getTransform( Graphics &gfx ) cond_noex
{
	return {DirectX::XMMatrixTranspose( gfx.getViewMatrix() * gfx.getProjectionMatrix() )};
}

}//namespace ren