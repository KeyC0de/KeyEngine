#include "ui_pass.h"
#include "primitive_topology.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"
#include "utils.h"
#include "settings_manager.h"

#pragma comment( lib, "DirectXTK.lib" )


namespace dx = DirectX;

namespace ren
{

bool UIPass::ComponentComparator::operator()( const std::unique_ptr<ui::Component> pComponent1,
	const std::unique_ptr<ui::Component> pComponent2 )
{
	return pComponent1->getId() < pComponent2->getId();
}


UIPass::UIPass( Graphics &gfx,
	const std::string &name )
	:
	IBindablePass{name},
	m_pBlendState{BlendState::fetch( gfx, BlendState::Mode::NoBlend, 0u )}
{
	//addPassBindable( m_pBlendState );

	//addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	m_pRasterizerState = RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Back );
	//addPassBindable( m_pRasterizerState );

	//addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );
	
	m_pDepthStencil = std::make_shared<DepthStencilState>( gfx, DepthStencilState::Mode::DepthOffStencilOff );

	//addPassBindable( m_pDepthStencil );

	m_pSpriteBatch = std::make_unique<dx::SpriteBatch>( getDeviceContext( gfx ) );
	using namespace std::string_literals;
	const auto fontPath = L"assets/fonts/"s + m_fontFilenameNoExtension + L".spritefont"s;
	m_pFpsSpriteFont = std::make_unique<dx::SpriteFont>( getDevice( gfx ), fontPath.c_str() );

	//addPassBindable( std::make_shared<PixelShader>( gfx, "flat2d_ps.cso" ) );

	m_pTexture1 = std::make_shared<Texture>( gfx, "assets/textures/ui/red_vignette.png"s, 0u );
	m_pTexture2 = std::make_shared<Texture>( gfx, "assets/textures/ui/health_icon.png"s, 1u );
	//addPassBindable( m_pTexture );

	//addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Point, TextureSamplerState::AddressMode::Clamp ) );
}

void UIPass::run( Graphics &gfx ) const cond_noex
{
	// By default SpriteBatch uses premultiplied alpha blending, no depth buffer, counter clockwise culling, and linear filtering with clamp texture addressing. You can change this by passing custom state objects to SpriteBatch::Begin. Pass null for any parameters that should use their default value.
	// SpriteBatch makes use of the following states: BlendState, Constant buffer (Vertex Shader stage, slot 0), DepthStencilState, Index buffer, Input layout, Pixel shader, Primitive topology, RasterizerState, SamplerState (Pixel Shader stage, slot 0), Shader resources (Pixel Shader stage, slot 0), Vertex buffer (slot 0), Vertex shader
	// The SpriteBatch class assumes you've already set the Render Target view, Depth Stencil view, and Viewport. It will also read the first viewport set on the device unless you've explicitly called SetViewport.
	// Be sure that if you set any of the following shaders prior to using SpriteBatch that you clear them: Geometry Shader, Hull Shader, Domain Shader, Compute Shader.

	bind( gfx );

	auto nonConstThis = const_cast<UIPass*>( this );
	nonConstThis->drawTexture1( gfx, (gfx.getClientWidth() >> 1) - (m_pTexture1->getWidth() >> 1), (gfx.getClientHeight() >> 1) + (m_pTexture1->getHeight() >> 1), m_pTexture1->getWidth(), m_pTexture1->getHeight() );
	nonConstThis->drawTexture2( gfx, 10, gfx.getClientHeight() - 10 - m_pTexture2->getHeight(), m_pTexture2->getWidth(), m_pTexture2->getHeight() );
	nonConstThis->drawText( gfx, "KeyEngine - All Rights Reserved", dx::XMFLOAT2(gfx.getClientWidth() - 320, gfx.getClientHeight() - 100), dx::Colors::White, dx::XMFLOAT2{.8f, .8f} );

#if defined _DEBUG && !defined NDEBUG
	if ( SettingsManager::getInstance().getSettings().bFpsCounting )
	{
		nonConstThis->updateAndRenderFpsTimer( gfx );
	}
#endif
}

void UIPass::reset() cond_noex
{
	pass_;
}

void UIPass::drawText( Graphics &gfx,
	const std::string &text,
	const DirectX::XMFLOAT2 &pos,
	const DirectX::XMVECTORF32 &color /*= DirectX::Colors::White*/,
	const DirectX::XMFLOAT2 &scale /*= DirectX::XMFLOAT2{1.0f, 1.0f}*/ )
{
	m_pSpriteBatch->Begin();
	m_pFpsSpriteFont->DrawString(m_pSpriteBatch.get(), text.c_str(), pos, color, 0.0f, dx::XMFLOAT2{0.0f, 0.0f}, scale );
	m_pSpriteBatch->End();
}

// #TODO: make this extensible
void UIPass::drawTexture1( Graphics &gfx,
	const int x,
	const int y,
	const int width,
	const int height )
{
	RECT rect{x, y, x + width, y - height};

	m_pSpriteBatch->Begin( DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, nullptr, m_pRasterizerState->getD3dRasterizerState().Get() );
	m_pSpriteBatch->Draw( m_pTexture1->getD3dSrv().Get(), rect );
	m_pSpriteBatch->End();
}

void UIPass::drawTexture2( Graphics &gfx,
	const int x,
	const int y,
	const int width,
	const int height )
{
	RECT rect{x, y, x + width, y - height};

	m_pSpriteBatch->Begin( DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, nullptr, m_pRasterizerState->getD3dRasterizerState().Get() );
	m_pSpriteBatch->Draw( m_pTexture2->getD3dSrv().Get(), rect );
	m_pSpriteBatch->End();
}

void UIPass::updateAndRenderFpsTimer( Graphics &gfx )
{
	static int fpsDisplayFrameCount = 0;
	static std::string fpsText;

	++fpsDisplayFrameCount;

	auto &fpsTimer = gfx.getFpsTimer();

	// if more than 1000ms have passed count fps
	if ( fpsTimer.getDurationFromStart() > 1000 )
	{
		fpsText = std::to_string( fpsDisplayFrameCount );
#if defined _DEBUG && !defined NDEBUG
		OutputDebugStringA( fpsText.data() );
#endif
		fpsTimer.restart();
		fpsDisplayFrameCount = 0;
	}

	drawText( gfx, fpsText, dx::XMFLOAT2{0.0f, 0.0f}, dx::Colors::Green );
}


}//namespace ren