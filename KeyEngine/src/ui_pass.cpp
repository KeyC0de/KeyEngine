#include "ui_pass.h"
#include "primitive_topology.h"
#include "rasterizer_state.h"
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
	return pComponent1->getHierarchyId() < pComponent2->getHierarchyId();
}


UIPass::UIPass( Graphics &gfx,
	const std::string &name,
	const std::string &fpsFontNameNoExtension )
	:
	IBindablePass{name},
	m_pBlendState{BlendState::fetch( gfx, BlendState::Mode::NoBlend, 0u )}
{
	addPassBindable( m_pBlendState );

	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	//addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

	m_pSpriteBatch = std::make_unique<dx::SpriteBatch>( getDeviceContext( gfx ) );
	using namespace std::string_literals;
	const auto fpsFontRelativePath = L"assets/fonts/"s + util::s2ws( fpsFontNameNoExtension ) + L".spritefont"s;
	m_pFpsSpriteFont = std::make_unique<dx::SpriteFont>( getDevice( gfx ), fpsFontRelativePath.c_str() );

	addPassBindable( std::make_shared<PixelShader>( gfx, "flat2d_ps.cso" ) );

	m_pTexture = std::make_shared<Texture>( gfx, "assets/textures/ui/first_person_crosshair.png", 0u );
	addPassBindable( m_pTexture );

	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Point, TextureSamplerState::AddressMode::Clamp ) );
}

void UIPass::run( Graphics &gfx ) const cond_noex
{
	bind( gfx );

	auto nonConstThis = const_cast<UIPass*>( this );
	nonConstThis->drawText( gfx, "KeyEngine - All Rights Reserved", dx::XMFLOAT2(gfx.getClientWidth() - 320, gfx.getClientHeight() - 100), dx::Colors::White, dx::XMFLOAT2{.8f, .8f} );
	nonConstThis->drawTexture( gfx, (gfx.getClientWidth() / 2) - 20, (gfx.getClientHeight() / 2) + 20, 40, 40 );
	//nonConstThis->drawTexture( gfx, "health_icon.png", 30, (gfx.getClientHeight() / 2) + 60, 20, 20 );

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
	const DirectX::XMVECTORF32 color /*= DirectX::Colors::White*/,
	const DirectX::XMFLOAT2 &scale /*= DirectX::XMFLOAT2{1.0f, 1.0f}*/ )
{
	m_pSpriteBatch->Begin();
	m_pFpsSpriteFont->DrawString(m_pSpriteBatch.get(), text.c_str(), pos, color, 0.0f, dx::XMFLOAT2{0.0f, 0.0f}, scale );
	m_pSpriteBatch->End();
}

// #FIXME: doesn't work
void UIPass::drawTexture( Graphics &gfx,
	const int x,
	const int y,
	const int width,
	const int height )
{
	using namespace std::string_literals;
	
	RECT rect{x, y, x + width, y - height};

	m_pSpriteBatch->Begin( DirectX::SpriteSortMode::SpriteSortMode_Deferred, m_pBlendState->getD3dBlendState().Get() );
	m_pSpriteBatch->Draw( m_pTexture->getD3dSrv().Get(), rect );
	m_pSpriteBatch->End();
}

void UIPass::updateAndRenderFpsTimer( Graphics &gfx )
{
	static int fpsDisplayFrameCount = 0;
	static std::string fpsText;

	++fpsDisplayFrameCount;

	auto &fpsTimer = gfx.getFpsTimer();

	// if more than 1000ms have passed do an fps count
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