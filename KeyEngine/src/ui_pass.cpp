#include "ui_pass.h"
#include "graphics.h"
#include "ui_component.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"
#include <variant>
#include "rasterizer_state.h"
#include "texture.h"
#include "utils.h"
#include "settings_manager.h"

#pragma comment( lib, "DirectXTK.lib" )


namespace dx = DirectX;

namespace gui
{

UIPass::UIPass( Graphics &gfx )
{
	recreate( gfx );
}

UIPass::~UIPass() noexcept
{
}

void UIPass::recreate( Graphics &gfx )
{
	m_pRasterizerState = RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front );

	m_pSpriteBatch = std::make_unique<dx::SpriteBatch>( getDeviceContext( gfx ) );
	using namespace std::string_literals;
	const auto fontPath = L"assets/fonts/"s + util::s2ws( SettingsManager::getInstance().getSettings().sFontName ) + L".spritefont"s;
	m_pSpriteFont = std::make_unique<dx::SpriteFont>( getDevice( gfx ), fontPath.c_str() );

	// create root Component
	m_pRoot = std::make_unique<Component>( gfx, "root", nullptr, 0, 0, gfx.getClientWidth(), gfx.getClientHeight() );

	std::variant<Component*, std::string> pRoot;
	pRoot = m_pRoot.get();

	// create the rest of the components
	{
		std::vector<std::pair<std::string, std::string>> compStateTexts;
		compStateTexts.emplace_back( "default"s, "KeyEngine - All Rights Reserved"s );
		Component::create_component( gfx, "logo", pRoot, gfx.getClientWidth() - 320, gfx.getClientHeight() - 100, 60, 6, false, compStateTexts, {}, dx::Colors::White, dx::XMFLOAT2{.8f, .8f}/*, "With a tooltip too!"*/ );
		// #TODO: come up with automatic sizing of text-only components given the length of the text size-to-content
	}
	if ( SettingsManager::getInstance().getSettings().bFpsCounting )
	{
		std::vector<std::pair<std::string, std::string>> compStateTexts;
		compStateTexts.emplace_back( "default_fps_counter"s, "fps_counter"s );
		Component::create_component( gfx, "fps_counter", pRoot, 0, 0, 32, 8, false, compStateTexts, {}, dx::Colors::Green );
	}
	/*
	{
		std::vector<std::pair<std::string, std::string>> compStateImages;
		const std::string filepath = "assets/textures/ui/red_vignette.png"s;
		compStateImages.emplace_back( "default"s, filepath );
		int bitmapWidth;
		int bitmapHeight;
		{
			const auto bitmap = Bitmap::loadFromFile( filepath );
			bitmapWidth = bitmap.getWidth();
			bitmapHeight = bitmap.getHeight();
		}
		Component::create_component( gfx, "vignette", pRoot, (gfx.getClientWidth() >> 1) - (bitmapWidth >> 1), (gfx.getClientHeight() >> 1) - (bitmapHeight >> 1), bitmapWidth, bitmapHeight, false, {}, compStateImages );
	}
	*/
	{
		std::vector<std::pair<std::string, std::string>> compStateImages;
		const std::string filepath = "assets/textures/ui/health_icon.png"s;
		compStateImages.emplace_back( "default"s, filepath );
		int bitmapWidth;
		int bitmapHeight;
		{
			const auto bitmap = Bitmap::loadFromFile( filepath );
			bitmapWidth = bitmap.getWidth();
			bitmapHeight = bitmap.getHeight();
		}
		Component::create_component( gfx, "health_icon", pRoot, 10, gfx.getClientHeight() - 10 - bitmapHeight * 1.7, bitmapWidth, bitmapHeight, false, {}, compStateImages );
	}
}

void UIPass::update( const float dt,
	const std::pair<int, int> inputXandY,
	const float lerpBetweenFrames ) cond_noex
{
	m_pRoot->update( dt, {inputXandY.first, inputXandY.second}, lerpBetweenFrames );
}

void UIPass::render( Graphics &gfx ) const cond_noex
{
	m_pRoot->render( gfx, m_pSpriteBatch.get(), m_pSpriteFont.get(), m_pRasterizerState.get() );
}

const std::unique_ptr<Component>& UIPass::getRoot() const noexcept
{
	return m_pRoot;
}

std::unique_ptr<Component>& UIPass::getRoot()
{
	return m_pRoot;
}


}//namespace gui