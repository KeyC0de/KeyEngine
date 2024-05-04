#include "font_pass.h"
#include "primitive_topology.h"
#include "utils.h"
#include "settings_manager.h"

#pragma comment( lib, "DirectXTK.lib" )


// #TODO: extend this into UiPass ...
namespace dx = DirectX;

namespace ren
{

FontPass::FontPass( Graphics &gfx,
	const std::string &name,
	const std::string &fpsFontNameNoExtension )
	:
	IBindablePass{name}
{
	addPassBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	m_pSpriteBatch = std::make_unique<dx::SpriteBatch>( getDeviceContext( gfx ) );
	using namespace std::string_literals;
	const auto fpsFontRelativePath = L"assets/fonts/"s + util::s2ws( fpsFontNameNoExtension ) + L".spritefont"s;
	m_pFpsSpriteFont = std::make_unique<dx::SpriteFont>( getDevice( gfx ), fpsFontRelativePath.c_str() );
}

void FontPass::run( Graphics &gfx ) const cond_noex
{
	bind( gfx );
#if defined _DEBUG && !defined NDEBUG
	const auto &g_setMan = SettingsManager::getInstance();
	if ( g_setMan.getSettings().bFpsCounting )
	{
		const_cast<FontPass*>( this )->updateAndRenderFpsTimer( gfx );
	}
#endif
}

void FontPass::reset() cond_noex
{
	pass_;
}

void FontPass::updateAndRenderFpsTimer( Graphics &gfx )
{
	static int fpsDisplayFrameCount = 0;
	static std::wstring fpsText;

	++fpsDisplayFrameCount;

	auto &fpsTimer = gfx.getFpsTimer();

	// if more than 1000ms have passed do an fps count
	if ( fpsTimer.getDurationFromStart() > 1000 )
	{
		fpsText = std::to_wstring( fpsDisplayFrameCount );
#if defined _DEBUG && !defined NDEBUG
		OutputDebugStringW( fpsText.data() );
#endif
		fpsTimer.restart();
		fpsDisplayFrameCount = 0;
	}

	m_pSpriteBatch->Begin();
	m_pFpsSpriteFont->DrawString(m_pSpriteBatch.get(), fpsText.c_str(), dx::XMFLOAT2{0.0f, 0.0f}, dx::Colors::Green, 0.0f, dx::XMFLOAT2{0.0f, 0.0f}, dx::XMFLOAT2{1.0f, 1.0f} );
	m_pSpriteBatch->End();
}


}//namespace ren