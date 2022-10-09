#include "renderer.h"
#include <string>
#include "renderer_exception.h"
#include "producer.h"
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
#include "imgui.h"
#include "math_utils.h"
#include "assertions_console.h"
#include "wireframe_pass.h"
#include "blur_pass.h"


namespace ren
{

Renderer::Renderer( Graphics &gph )
	:
	m_globalColorBuffer{gph.renderTarget()},
	m_globalDepthStencil{gph.depthStencil()}
{
	addGlobalConsumer( RenderSurfaceConsumer<IRenderTargetView>::make( "backColorbuffer",
		m_globalColorBuffer ) );

	addGlobalProducer( RenderSurfaceProducer<IRenderTargetView>::make( "backColorbuffer",
		m_globalColorBuffer ) );
	addGlobalProducer( RenderSurfaceProducer<IDepthStencilView>::make( "backDepthBuffer",
		m_globalDepthStencil ) );

	// #TODO: Add active flag as argument in the ctor of all Passes
	{
		auto pass = std::make_unique<RenderSurfaceClearPass>( "clearRt" );
		pass->setupConsumerTarget( "buffer",
			"$",
			"backColorbuffer" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<RenderSurfaceClearPass>( "clearDs" );
		pass->setupConsumerTarget( "buffer",
			"$",
			"backDepthBuffer" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
}

Renderer::~Renderer() noexcept
{

}

void Renderer::addGlobalProducer( std::unique_ptr<IProducer> pProducer )
{
	m_globalProducers.emplace_back( std::move( pProducer ) );
}

void Renderer::addGlobalConsumer( std::unique_ptr<IConsumer> pConsumer )
{
	m_globalConsumers.emplace_back( std::move( pConsumer ) );
}

void Renderer::run( Graphics &gph ) cond_noex
{
	ASSERT( m_bValidatedPasses, "Renderer is not validated!" );
	for ( auto &pass : m_passes )
	{
		if ( pass->isActive() )
		{
			pass->run( gph );
		}
	}
}

void Renderer::reset() noexcept
{
	ASSERT( m_bValidatedPasses, "Renderer is not validated!" );
	for ( auto &pass : m_passes )
	{
		pass->reset();
	}
}

void Renderer::addPass( std::unique_ptr<IPass> pPass )
{
	ASSERT( !m_bValidatedPasses, "Renderer is already validated!" );
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

void Renderer::setupGlobalConsumerTarget( const std::string &globalConsumerName,
	const std::string &passName,
	const std::string &producerName )
{
	const auto passFinder = [&globalConsumerName]( const std::unique_ptr<IConsumer>& consumer )
	{
		return consumer->getName() == globalConsumerName;
	};
	const auto consumer = std::find_if( m_globalConsumers.begin(),
		m_globalConsumers.end(),
		passFinder );
	if ( consumer == m_globalConsumers.end() )
	{
		THROW_RENDERER_EXCEPTION( "Global consumer " + globalConsumerName + " does not exist!" );
	}
	(*consumer)->setPassAndProducerNames( passName,
		producerName );
}

void Renderer::validateConsumersLinkage()
{
	ASSERT( !m_bValidatedPasses, "Renderer is already validated!" );
	for ( const auto &pass : m_passes )
	{
		pass->validate();
	}
	m_bValidatedPasses = true;
}

void Renderer::linkPassConsumers( IPass &pass )
{
	for ( auto &consumer : pass.getConsumers() )
	{
		const auto &consumerPassName = consumer->getPassName();
		if ( consumerPassName.empty() )
		{
			std::ostringstream oss;
			oss << "In pass named ["
				<< pass.getName()
				<< "] consumer named ["
				<< consumer->getName()
				<< "] has no target producer set.";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}

		// check whether consumer is global
		if ( consumerPassName == "$" )
		{
			bool bLinked = false;
			for ( auto &globalProducer : m_globalProducers )
			{
				if ( globalProducer->getName() == consumer->getProducerName() )
				{
					consumer->link( *globalProducer );
					bLinked = true;
					break;
				}
			}
			if ( !bLinked )
			{
				std::ostringstream oss;
				oss << "Producer named ["
					<< consumer->getProducerName()
					<< "] not a global";
				THROW_RENDERER_EXCEPTION( oss.str() );
			}
		}
		else
		{// find producer from within existing passes
			bool bLinked = false;
			for ( auto &pass : m_passes )
			{
				if ( pass->getName() == consumerPassName )
				{
					auto &producer = pass->producer( consumer->getProducerName() );
					consumer->link( producer );
					bLinked = true;
					break;
				}
			}
			if ( !bLinked )
			{
				std::ostringstream oss;
				oss << "Pass named ["
					<< consumerPassName
					<< "] not found";
				THROW_RENDERER_EXCEPTION( oss.str() );
			}
		}
	}
}

void Renderer::linkGlobalConsumers()
{
	this->validateConsumersLinkage();

	for ( auto &consumer : m_globalConsumers )
	{
		const auto &consumerPassname = consumer->getPassName();
		for ( auto &pass : m_passes )
		{
			if ( pass->getName() == consumerPassname )
			{
				auto &producer = pass->producer( consumer->getProducerName() );
				consumer->link( producer );
				break;
			}
		}
	}
}

IPass& ren::Renderer::getPass( const std::string &name )
{
	const auto finder = std::find_if( m_passes.begin(),
		m_passes.end(),
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
	const int radius,
	const float sigma,
	const KernelType kernelType )
	:
	Renderer{gph},
	m_radius(radius),
	m_sigma(sigma),
	m_kernelType{kernelType}
{
	{
		auto pass = std::make_unique<ShadowPass>( gph,
			"shadowMap" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<LambertianPass>( gph,
			"lambertian" );
		pass->setupConsumerTarget( "renderTarget",
			"clearRt",
			"buffer" );
		pass->setupConsumerTarget( "depthStencil",
			"clearDs",
			"buffer" );
		pass->setupConsumerTarget( "offscreenShadowCubemapIn",
			"shadowMap",
			"offscreenShadowCubemapOut" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}/*
	{
		auto pass = std::make_unique<SkyPass>( gph,
			"sky" );
		pass->setupConsumerTarget( "renderTarget",
			"lambertian",
			"renderTarget" );
		pass->setupConsumerTarget( "depthStencil",
			"lambertian",
			"depthStencil" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<BlurOutlineMaskPass>( gph,
			"blurOutlineMask" );
		pass->setupConsumerTarget( "depthStencil",
			"sky",
			"depthStencil" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<BlurOutlineDrawPass>( gph,
			"blurOutlineDraw",
			s_fullscreenRezReductFactor );
		linkPassConsumers( *pass );
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
			m_blurKernel = std::make_shared<PixelShaderConstantBufferEx>( gph,
				0u,
				cb );
			setKernelGauss( m_radius,
				m_sigma );
			addGlobalProducer( BindableProducer<PixelShaderConstantBufferEx>::make( "blurKernel",
				m_blurKernel ) );
		}
		{
			con::RawLayout layout;
			layout.add<con::Bool>( "bHorizontal" );

			con::CBuffer cb{std::move( layout )};
			m_blurDirection = std::make_shared<PixelShaderConstantBufferEx>( gph,
				1u,
				cb );
			addGlobalProducer( BindableProducer<PixelShaderConstantBufferEx>::make( "blurDirection",
				m_blurDirection ) );
		}
	}
	{
		auto pass = std::make_unique<HorizontalBlurPass>( gph,
			"horizontalBlur",
			s_fullscreenRezReductFactor );
		pass->setupConsumerTarget( "offscreenBlurOutlineIn",
			"blurOutlineDraw",
			"offscreenBlurOutlineOut" );
		pass->setupConsumerTarget( "blurKernel",
			"$",
			"blurKernel" );
		pass->setupConsumerTarget( "blurDirection",
			"$",
			"blurDirection" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<VerticalBlurPass>( gph,
			"verticalBlur" );
		pass->setupConsumerTarget( "renderTarget",
			"sky",
			"renderTarget" );
		pass->setupConsumerTarget( "depthStencil",
			"blurOutlineMask",
			"depthStencil" );
		pass->setupConsumerTarget( "offscreenBlurOutlineIn",
			"horizontalBlur",
			"offscreenBlurOutlineOut" );
		pass->setupConsumerTarget( "blurKernel",
			"$",
			"blurKernel" );
		pass->setupConsumerTarget( "blurDirection",
			"$",
			"blurDirection" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}*/
	{
		auto pass = std::make_unique<SolidOutlineMaskPass>( gph,
			"solidOutlineMask" );
		pass->setupConsumerTarget( "renderTarget",
			"lambertian",
			"renderTarget" );
		pass->setupConsumerTarget( "depthStencil",
			"lambertian",
			"depthStencil" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<SolidOutlineDrawPass>( gph,
			"solidOutlineDraw" );
		pass->setupConsumerTarget( "renderTarget",
			"solidOutlineMask",
			"renderTarget" );
		pass->setupConsumerTarget( "depthStencil",
			"solidOutlineMask",
			"depthStencil" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}

	//{
		//auto pass = std::make_unique<FullscreenPass>( gph,
			//"fullscreen" );
		//pass->setupConsumerTarget( "renderTarget",
			//"solidOutlineDraw",
			//"renderTarget" );
		//pass->setupConsumerTarget( "depthStencil",
			//"solidOutlineDraw",
			//"depthStencil" );
		//pass->setupConsumerTarget( "offscreenFullscreenBlurIn",
			//"blur",
			//"offscreenFullscreenBlurOut" );
		//linkPassConsumers( *pass );
		//addPass( std::move( pass ) );
	//}
	//{
		//auto pass = std::make_unique<BlurPass>( gph,
			//"blur",
			//s_fullscreenRezReductFactor );
		//linkPassConsumers( *pass );
		//addPass( std::move( pass ) );
	//}
	{
		auto pass = std::make_unique<DepthReversedPass>( gph,
			"depthReversed" );
		pass->setupConsumerTarget( "renderTarget",
			"solidOutlineDraw",
			"renderTarget" );
		pass->setupConsumerTarget( "depthStencil",
			"solidOutlineDraw",
			"depthStencil" );
		//pass->setupConsumerTarget( "offscreenFullscreenBlurIn",
			//"blur",
			//"offscreenFullscreenBlurOut" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}
	{
		auto pass = std::make_unique<WireframePass>( gph,
			"wireframe" );
		pass->setupConsumerTarget( "renderTarget",
			"depthReversed",
			"renderTarget" );
		pass->setupConsumerTarget( "depthStencil",
			"depthReversed",
			"depthStencil" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}

	setupGlobalConsumerTarget( "backColorbuffer",
		"wireframe",
		"renderTarget" );
	Renderer::linkGlobalConsumers();
}

void Renderer3d::showImGuiWindows( Graphics &gph )
{
	showShadowDumpImguiWindow( gph );
	showGaussianBlurImguiWindow( gph );
	//dynamic_cast<SkyPass&>( getPass( "sky" ) ).displayImguiWidgets();
}

void Renderer3d::showGaussianBlurImguiWindow( Graphics &gph )
{
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

		bool bRadiusChanged = ImGui::SliderInt( "Radius",
			&m_radius,
			0,
			s_maxRadius );

		bool bSigmaChanged = false;
		if ( m_kernelType == Gauss )
		{
			bSigmaChanged = ImGui::SliderFloat( "Sigma",
				&m_sigma,
				0.1f,
				s_maxSigma );
		}

		if ( bRadiusChanged || bSigmaChanged || bFilterChanged )
		{
			if ( m_kernelType == Gauss )
			{
				setKernelGauss( m_radius,
					m_sigma );
			}
			else if ( m_kernelType == Box )
			{
				setKernelBox( m_radius );
			}
		}
	}
	ImGui::End();
}

void ren::Renderer3d::setActiveCamera( Camera &cam )
{
	dynamic_cast<LambertianPass&>( getPass( "lambertian" ) ).setActiveCamera( cam );
	//dynamic_cast<SkyPass&>( getPass( "sky" ) ).setActiveCamera( cam );
}

void ren::Renderer3d::setShadowCamera( Camera &cam )
{
	dynamic_cast<ShadowPass&>( getPass( "shadowMap" ) ).setShadowCamera( cam );
}

void ren::Renderer3d::showShadowDumpImguiWindow( Graphics &gph )
{
	if ( ImGui::Begin( "Shadow" ) )
	{
		if ( ImGui::Button( "Dump Cubemap" ) )
		{
			dumpShadowMap( gph,
				"dumps/shadow_" );
		}
	}
	ImGui::End();
}

void Renderer3d::dumpShadowMap( Graphics &gph,
	const std::string &path )
{
	dynamic_cast<ShadowPass&>( getPass( "shadowMap" ) ).dumpShadowMap( gph,
		path );
}

void Renderer3d::setKernelGauss( int radius,
	float sigma ) cond_noex
{
	ASSERT( radius <= s_maxRadius, "Blur Kernel radius is over the max!" );
	ASSERT( sigma <= s_maxSigma, "Blur Kernel sigma is over the max!" );

	auto cb = m_blurKernel->getBuffer();
	const int nTaps = radius * 2 + 1;
	cb["nTaps"] = nTaps;
	float sum = 0.0f;
	for ( int i = 0; i < nTaps; ++i )
	{
		const auto x = float( i - radius );
		const auto coef = util::gaussianDistr( x,
			sigma,
			0.0f );
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

void Renderer3d::setKernelBox( int radius ) cond_noex
{
	ASSERT( radius <= s_maxRadius, "Blur Kernel radius is over the max!" );

	auto cb = m_blurKernel->getBuffer();
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
	Renderer{gph}
{
	{
		auto pass = std::make_unique<Pass2D>( gph,
			"pass2d" );
		pass->setupConsumerTarget( "renderTarget",
			"clearRt",
			"buffer" );
		pass->setupConsumerTarget( "depthStencil",
			"clearDs",
			"buffer" );
		linkPassConsumers( *pass );
		addPass( std::move( pass ) );
	}

	setupGlobalConsumerTarget( "backColorbuffer",
		"pass2d",
		"renderTarget" );
	Renderer::linkGlobalConsumers();
}


}//ren