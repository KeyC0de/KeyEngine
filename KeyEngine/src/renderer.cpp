#include "renderer.h"
#include "graphics_mode.h"
#include "constant_buffer_ex.h"
#include "renderer_exception.h"
#include "linker.h"
#include "render_queue_pass.h"
#include "render_surface_clear_pass.h"
#include "shadow_pass.h"
#include "lambertian_pass.h"
#include "sky_pass.h"
#include "blur_outline_mask_pass.h"
#include "blur_outline_draw_pass.h"
#include "horizontal_blur_pass.h"
#include "vertical_blur_pass.h"
#include "solid_outline_mask_pass.h"
#include "solid_outline_draw_pass.h"
#include "depth_reversed_pass.h"
#include "pass_2d.h"
#include "render_target.h"
#include "dynamic_constant_buffer.h"
#ifndef FINAL_RELEASE
#	include "imgui.h"
#endif
#include "math_utils.h"
#include "assertions_console.h"
#include "wireframe_pass.h"
#include "negative_pass.h"
#include "blur_pass.h"
#include "pass_through.h"
#include "font_pass.h"


namespace ren
{

Renderer::Renderer( Graphics &gph,
	bool drawToOffscreen )
	:
	m_pRtv{drawToOffscreen ? gph.getRenderTargetOffscreen( 0u, RenderTargetViewMode::DefaultRT )->shareRenderTarget() : gph.getRenderTargetFromBackBuffer()},
	m_pDsv{drawToOffscreen ? gph.getDepthBufferOffscreen( 0u, DepthStencilViewMode::DefaultDS )->shareDepthBuffer() : gph.getDepthBufferFromBackBuffer()}
{
	addGlobalBinder( RenderSurfaceBinder<IRenderTargetView>::make( "backColorbuffer", m_pRtv ) );

	addGlobalLinker( RenderSurfaceLinker<IRenderTargetView>::make( "backColorbuffer", m_pRtv ) );
	addGlobalLinker( RenderSurfaceLinker<IDepthStencilView>::make( "backDepthBuffer", m_pDsv ) );

	// #TODO: Add active flag as argument in the ctor of all Passes
	{
		auto pass = std::make_unique<RenderSurfaceClearPass>( "clearRt" );
		pass->setupBinderTarget( "buffer", "$", "backColorbuffer" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<RenderSurfaceClearPass>( "clearDs" );
		pass->setupBinderTarget( "buffer", "$", "backDepthBuffer" );
		addPass( std::move( pass ) );
	}
}

Renderer::~Renderer() noexcept
{

}

void Renderer::addGlobalLinker( std::unique_ptr<ILinker> pLinker )
{
	m_globalLinkers.emplace_back( std::move( pLinker ) );
}

void Renderer::addGlobalBinder( std::unique_ptr<IBinder> pBinder )
{
	m_globalBinders.emplace_back( std::move( pBinder ) );
}

void Renderer::run( Graphics &gph ) cond_noex
{
	ASSERT( m_bValidatedPasses, "Renderer is not validated!" );
	// Run the offscreen passes
	for ( auto &pass : m_passes )
	{
		if ( pass->isActive() )
		{
			pass->run( gph );
#if defined _DEBUG && !defined NDEBUG
			const auto *renderQueuePass = dynamic_cast<RenderQueuePass*>( pass.get() );
			if ( renderQueuePass != nullptr )
			{
				using namespace std::string_literals;
				KeyConsole &console = KeyConsole::getInstance();
				console.print( pass->getName() + " "s + std::to_string( renderQueuePass->getJobCount() ) + "\n"s );
			}
#endif
		}
	}

	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		offscreenToBackBufferSwap( gph );
		m_pFinalPostProcessPass->run( gph );
	}

	// UI rendering continues...
	m_pFontPass->run( gph );
}

void Renderer::reset() noexcept
{
	ASSERT( m_bValidatedPasses, "Renderer is not validated!" );
	for ( auto &pass : m_passes )
	{
		pass->reset();
	}
}

void Renderer::offscreenToBackBufferSwap( Graphics &gph )
{
	gph.getRenderTargetFromBackBuffer()->bindRenderSurface( gph );
	gph.getRenderTargetOffscreen( 0u, RenderTargetViewMode::DefaultRT )->bind( gph );
}

void Renderer::addPass( std::unique_ptr<IPass> pPass )
{
	ASSERT( !m_bValidatedPasses, "Renderer is already validated!" );

	linkPassBinders( *pPass );

	// validate name uniqueness
	for ( const auto &pa : m_passes )
	{
		if ( pa->getName() == pPass->getName() )
		{
			THROW_RENDERER_EXCEPTION( "Pass name already exists: " + pPass->getName() );
		}
	}
	m_passes.emplace_back( std::move( pPass ) );
}

void Renderer::setupGlobalBinderTarget( const std::string &globalBinderName,
	const std::string &passName,
	const std::string &linkerName )
{
	const auto passFinder = [&globalBinderName]( const std::unique_ptr<IBinder>& binder )
	{
		return binder->getName() == globalBinderName;
	};
	const auto binder = std::find_if( m_globalBinders.begin(), m_globalBinders.end(), passFinder );
	if ( binder == m_globalBinders.end() )
	{
		THROW_RENDERER_EXCEPTION( "Global binder " + globalBinderName + " does not exist!" );
	}
	(*binder)->setPassAndLinkerNames( passName, linkerName );
}

void Renderer::validateBindersLinkage()
{
	ASSERT( !m_bValidatedPasses, "Renderer is already validated!" );
	for ( const auto &pass : m_passes )
	{
		pass->validate();
	}
	m_bValidatedPasses = true;
}

void Renderer::linkPassBinders( IPass &pass )
{
	for ( auto &binder : pass.getBinders() )
	{
		const auto &binderPassName = binder->getPassName();
		if ( binderPassName.empty() )
		{
			std::ostringstream oss;
			oss << "In pass named [" << pass.getName() << "] binder named [" << binder->getName() << "] has no target linker set.";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}

		// check whether binder is global
		if ( binderPassName == "$" )
		{
			bool bLinked = false;
			for ( auto &globalLinker : m_globalLinkers )
			{
				if ( globalLinker->getName() == binder->getLinkerName() )
				{
					binder->link( *globalLinker );
					bLinked = true;
					break;
				}
			}
			if ( !bLinked )
			{
				std::ostringstream oss;
				oss << "Linker named [" << binder->getLinkerName() << "] not a global";
				THROW_RENDERER_EXCEPTION( oss.str() );
			}
		}
		else
		{// find linker from within existing passes
			bool bLinked = false;
			for ( auto &pass : m_passes )
			{
				if ( pass->getName() == binderPassName )
				{
					auto &linker = pass->getLinker( binder->getLinkerName() );
					binder->link( linker );
					bLinked = true;
					break;
				}
			}
			if ( !bLinked )
			{
				std::ostringstream oss;
				oss << "Pass named [" << binderPassName << "] not found";
				THROW_RENDERER_EXCEPTION( oss.str() );
			}
		}
	}
}

void Renderer::linkGlobalBinders()
{
	validateBindersLinkage();

	for ( auto &binder : m_globalBinders )
	{
		const auto &binderPassName = binder->getPassName();
		for ( auto &pass : m_passes )
		{
			if ( pass->getName() == binderPassName )
			{
				auto &linker = pass->getLinker( binder->getLinkerName() );
				binder->link( linker );
				break;
			}
		}
	}
}

IPass& ren::Renderer::getPass( const std::string &name )
{
	const auto finder = std::find_if( m_passes.begin(), m_passes.end(),
		[&name]( auto &pass )
		{
			return pass->getName() == name;
		} );
	if ( finder == m_passes.end() )
	{
		THROW_RENDERER_EXCEPTION( "Pass '" + name + "' not found in Renderer!" );
	}
	return **finder;
}

RenderQueuePass& Renderer::getRenderQueuePass( const std::string &name )
{
	try
	{
		for ( const auto &pass : m_passes )
		{
			if ( pass->getName() == name )
			{
				return dynamic_cast<RenderQueuePass&>( *pass );
			}
		}
	}
	catch( std::bad_cast &ex )
	{
		(void)ex;
		THROW_RENDERER_EXCEPTION( "Renderer::getRenderQueuePass pass '" + name + "' was not a RenderQueuePass" );
	}
	THROW_RENDERER_EXCEPTION( "Renderer::getRenderQueuePass pass '" + name + "' was not found" );
}


Renderer3d::Renderer3d( Graphics &gph,
	bool drawToOffscreen,
	const int radius,
	const float sigma,
	const KernelType kernelType )
	:
	Renderer{gph, drawToOffscreen},
	m_radius(radius),
	m_sigma(sigma),
	m_kernelType{kernelType}
{
	{
		auto pass = std::make_unique<ShadowPass>( gph, "shadowMap" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<LambertianPass>( gph, "lambertian" );
		pass->setupBinderTarget( "renderTarget", "clearRt", "buffer" );
		pass->setupBinderTarget( "depthStencil", "clearDs", "buffer" );
		pass->setupBinderTarget( "offscreenShadowCubemapIn", "shadowMap", "offscreenShadowCubemapOut" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<SkyPass>( gph, "sky", true );
		pass->setupBinderTarget( "renderTarget", "lambertian", "renderTarget" );
		pass->setupBinderTarget( "depthStencil", "lambertian", "depthStencil" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<BlurOutlineMaskPass>( gph, "blurOutlineMask" );
		pass->setupBinderTarget( "depthStencil", "sky", "depthStencil" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<BlurOutlineDrawPass>( gph, "blurOutlineDraw", s_fullscreenRezReductFactor );
		addPass( std::move( pass ) );
	}
	{
		// setup blur global PCBuffers
		{
			con::RawLayout layout;
			layout.add<con::Integer>( "nTaps" );
			layout.add<con::Array>( "coefficients" );
			layout["coefficients"].set<con::Float>( s_maxRadius * 2 + 1 );

			con::CBuffer cb{std::move( layout )};
			m_blurKernel = std::make_shared<PixelShaderConstantBufferEx>( gph, 0u, cb );
			setKernelGauss( m_radius, m_sigma );
			addGlobalLinker( BindableLinker<PixelShaderConstantBufferEx>::make( "blurKernel", m_blurKernel ) );
		}
		{
			con::RawLayout layout;
			layout.add<con::Bool>( "bHorizontal" );

			con::CBuffer cb{std::move( layout )};
			m_blurDirection = std::make_shared<PixelShaderConstantBufferEx>( gph, 1u, cb );
			addGlobalLinker( BindableLinker<PixelShaderConstantBufferEx>::make( "blurDirection", m_blurDirection ) );
		}
	}
	{
		auto pass = std::make_unique<HorizontalBlurPass>( gph, "horizontalBlur", s_fullscreenRezReductFactor );
		pass->setupBinderTarget( "offscreenBlurOutlineIn", "blurOutlineDraw", "offscreenBlurOutlineOut" );
		pass->setupBinderTarget( "blurKernel", "$", "blurKernel" );
		pass->setupBinderTarget( "blurDirection", "$", "blurDirection" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<VerticalBlurPass>( gph, "verticalBlur" );
		pass->setupBinderTarget( "renderTarget", "sky", "renderTarget" );
		pass->setupBinderTarget( "depthStencil", "blurOutlineMask", "depthStencil" );
		pass->setupBinderTarget( "offscreenBlurOutlineIn", "horizontalBlur", "offscreenBlurOutlineOut" );
		pass->setupBinderTarget( "blurKernel", "$", "blurKernel" );
		pass->setupBinderTarget( "blurDirection", "$", "blurDirection" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<SolidOutlineMaskPass>( gph, "solidOutlineMask" );
		pass->setupBinderTarget( "renderTarget", "verticalBlur", "renderTarget" );
		pass->setupBinderTarget( "depthStencil", "verticalBlur", "depthStencil" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<SolidOutlineDrawPass>( gph, "solidOutlineDraw" );
		pass->setupBinderTarget( "renderTarget", "solidOutlineMask", "renderTarget" );
		pass->setupBinderTarget( "depthStencil", "solidOutlineMask", "depthStencil" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<DepthReversedPass>( gph, "depthReversed" );
		pass->setupBinderTarget( "renderTarget", "solidOutlineDraw", "renderTarget" );
		pass->setupBinderTarget( "depthStencil", "solidOutlineDraw", "depthStencil" );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<WireframePass>( gph, "wireframe" );
		pass->setupBinderTarget( "renderTarget", "depthReversed", "renderTarget" );
		pass->setupBinderTarget( "depthStencil", "depthReversed", "depthStencil" );
		addPass( std::move( pass ) );
	}

	setupGlobalBinderTarget( "backColorbuffer", "wireframe", "renderTarget" );
	Renderer::linkGlobalBinders();

	{
		//m_pFinalPostProcessPass = std::make_unique<ren::BlurPass>( gph, "blur" );
		//m_pFinalPostProcessPass = std::make_unique<ren::NegativePass>( gph, "negative" );
		m_pFinalPostProcessPass = std::make_unique<ren::PassThrough>( gph, "passthrough" );
	}

	m_pFontPass = std::make_unique<ren::FontPass>( gph, "fpsText", "myComicSansMSSpriteFont" );
}

void Renderer3d::displayImguiWidgets( Graphics &gph ) noexcept
{
#ifndef FINAL_RELEASE
	showShadowDumpImguiWindow( gph );
	showGaussianBlurImguiWindow( gph );
	dynamic_cast<SkyPass&>( getPass( "sky" ) ).displayImguiWidgets();
#endif
}

void Renderer3d::showShadowDumpImguiWindow( Graphics &gph ) noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( "Shadow" ) )
	{
		if ( ImGui::Button( "Dump Cubemap" ) )
		{
			dumpShadowMap( gph, "dumps/shadow_" );
		}
	}
	ImGui::End();
#endif
}

void Renderer3d::showGaussianBlurImguiWindow( Graphics &gph ) noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( "Blur Kernel" ) )
	{
		bool bFilterChanged = false;
		{
			const char *blurKernelMethodNames[] = {"Gauss", "Box"};
			static const char *selectedMethod = blurKernelMethodNames[0];
			if ( ImGui::BeginCombo( "Method", selectedMethod ) )
			{
				for ( int i = 0; i < std::size( blurKernelMethodNames ); ++i )
				{
					const bool bSelected = selectedMethod == blurKernelMethodNames[i];
					if ( ImGui::Selectable( blurKernelMethodNames[i], bSelected ) )
					{
						bFilterChanged = true;
						selectedMethod = blurKernelMethodNames[i];
						if ( selectedMethod == blurKernelMethodNames[0] )
						{
							m_kernelType = Gauss;
						}
						else if ( selectedMethod == blurKernelMethodNames[1] )
						{
							m_kernelType = Box;
						}
					}
					if ( bSelected )
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}

		bool bRadiusChanged = ImGui::SliderInt( "Radius", &m_radius, 0, s_maxRadius );

		bool bSigmaChanged = false;
		if ( m_kernelType == Gauss )
		{
			bSigmaChanged = ImGui::SliderFloat( "Sigma", &m_sigma, 0.1f, s_maxSigma );
		}

		if ( bRadiusChanged || bSigmaChanged || bFilterChanged )
		{
			if ( m_kernelType == Gauss )
			{
				setKernelGauss( m_radius, m_sigma );
			}
			else if ( m_kernelType == Box )
			{
				setKernelBox( m_radius );
			}
		}
	}
	ImGui::End();
#endif
}

void ren::Renderer3d::setActiveCamera( const Camera &cam )
{
	dynamic_cast<LambertianPass&>( getPass( "lambertian" ) ).setActiveCamera( cam );
	dynamic_cast<SkyPass&>( getPass( "sky" ) ).setActiveCamera( cam );
}

void ren::Renderer3d::setShadowCamera( const Camera &cam,
	const bool bEnable )
{
	dynamic_cast<ShadowPass&>( getPass( "shadowMap" ) ).setShadowCamera( cam, bEnable );
}

void Renderer3d::dumpShadowMap( Graphics &gph,
	const std::string &path )
{
	dynamic_cast<ShadowPass&>( getPass( "shadowMap" ) ).dumpShadowMap( gph, path );
}

void Renderer3d::setKernelGauss( const int radius,
	const float sigma ) cond_noex
{
	ASSERT( radius <= s_maxRadius, "Blur Kernel radius is over the max!" );
	ASSERT( sigma <= s_maxSigma, "Blur Kernel sigma is over the max!" );

	auto cb = m_blurKernel->getBufferCopy();
	const int nTaps = radius * 2 + 1;
	cb["nTaps"] = nTaps;
	float sum = 0.0f;
	for ( int i = 0; i < nTaps; ++i )
	{
		const auto x = float( i - radius );
		const auto coef = util::gaussian1d( x, sigma );
		sum += coef;
		cb["coefficients"][i] = coef;
	}
	// div by the weighted average
	for ( int i = 0; i < nTaps; ++i )
	{
		cb["coefficients"][i] = (float)cb["coefficients"][i] / sum;
	}
	m_blurKernel->setBuffer( cb );
}

void Renderer3d::setKernelBox( const int radius ) cond_noex
{
	ASSERT( radius <= s_maxRadius, "Blur Kernel radius is over the max!" );

	auto cb = m_blurKernel->getBufferCopy();
	const int nTaps = radius * 2 + 1;
	cb["nTaps"] = nTaps;
	const float c = 1.0f / nTaps;
	for ( int i = 0; i < nTaps; ++i )
	{
		cb["coefficients"][i] = c;
	}
	m_blurKernel->setBuffer( cb );
}


Renderer2d::Renderer2d( Graphics &gph )
	:
	Renderer{gph, false}
{
	{
		auto pass = std::make_unique<Pass2D>( gph, "pass2d" );
		pass->setupBinderTarget( "renderTarget", "clearRt", "buffer" );
		pass->setupBinderTarget( "depthStencil", "clearDs", "buffer" );
		addPass( std::move( pass ) );
	}

	setupGlobalBinderTarget( "backColorbuffer", "pass2d", "renderTarget" );
	Renderer::linkGlobalBinders();
}


}//ren