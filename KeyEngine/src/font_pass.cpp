#include "font_pass.h"
#include "primitive_topology.h"
#include "utils.h"
#include "settings_manager.h"

#pragma comment( lib, "DirectXTK.lib" )


namespace dx = DirectX;

namespace ren
{

FontPass::FontPass( Graphics &gph,
	const std::string &name,
	const std::string &fpsFontNameNoExtension )
	:
	IBindablePass{name}
{
	addPassBindable( PrimitiveTopology::fetch( gph, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	m_pSpriteBatch = std::make_unique<dx::SpriteBatch>( getDeviceContext( gph ) );
	using namespace std::string_literals;
	const auto fpsFontRelativePath = L"assets/fonts/"s + util::s2ws( fpsFontNameNoExtension ) + L".spritefont"s;
	m_pFpsSpriteFont = std::make_unique<dx::SpriteFont>( getDevice( gph ), fpsFontRelativePath.c_str() );

	m_fpsTimer.start();
}

void FontPass::run( Graphics &gph ) const cond_noex
{
	bind( gph );
#if defined _DEBUG && !defined NDEBUG
	const auto &g_setMan = SettingsManager::getInstance();
	if ( g_setMan.getSettings().bFpsCounting )
	{
		const_cast<FontPass*>( this )->updateAndRenderFpsTimer();
	}
#endif
}

void FontPass::reset() cond_noex
{
	pass_;
}

void FontPass::updateAndRenderFpsTimer()
{
	static int fpsDisplayFrameCount = 0;
	static std::wstring fpsText;

	++fpsDisplayFrameCount;

	// if more than 1000ms have passed do an fps count
	if ( m_fpsTimer.getDurationFromStart() > 1000 )
	{
		fpsText = std::to_wstring( fpsDisplayFrameCount );
#if defined _DEBUG && !defined NDEBUG
		OutputDebugStringW( fpsText.data() );
#endif
		m_fpsTimer.restart();
		fpsDisplayFrameCount = 0;
	}

	m_pSpriteBatch->Begin();
	m_pFpsSpriteFont->DrawString(m_pSpriteBatch.get(), fpsText.c_str(), dx::XMFLOAT2{0.0f, 0.0f}, dx::Colors::Green, 0.0f, dx::XMFLOAT2{0.0f, 0.0f}, dx::XMFLOAT2{1.0f, 1.0f} );
	m_pSpriteBatch->End();
}


}//namespace ren