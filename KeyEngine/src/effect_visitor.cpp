#include "effect_visitor.h"
#include <string>
#include <DirectXMath.h>
#include "imgui.h"
#include "dynamic_constant_buffer.h"
#include "effect.h"
#include "utils.h"


namespace dx = DirectX;

IEffectVisitor::~IEffectVisitor()
{
	pass_;
}

void IEffectVisitor::setEffect( Effect *ef )
{
	m_pEffect = ef;
	++m_effectId;
	onSetEffect();
}

bool IEffectVisitor::visit( con::Buffer &cb )
{
	++m_cbId;
	return onVisit( cb );
}

void IEffectVisitor::onSetEffect()
{
	pass_;
}


bool EVShowcase::onVisit( con::Buffer &cb )
{
	float bDirty = false;
	const auto dirtyCheck = [&bDirty]( bool bChanged )
	{
		bDirty = bDirty || bChanged;
	};

	auto tagImGuiWidget = [imguiNodeName = std::string{},
		str = '#' + std::to_string( m_cbId )]
		( const char *label ) mutable
	{
		imguiNodeName = label + str;
		return imguiNodeName.c_str();
	};

	// query the Drawable's Constant Buffer for available CB Elements
	// and display ImGui controls for those available
	if ( const auto &el = cb["scale"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Scale" ),
			&el, 1.0f, 2.0f, "%.3f", 3.5f ) );
	}

	if ( const auto &el = cb["offset"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "offset" ),
			&el, 0.0f, 1.0f, "%.3f", 2.5f ) );
	}

	if ( const auto &el = cb["materialColor"]; el.isValid() )
	{
		dirtyCheck( ImGui::ColorPicker3( tagImGuiWidget( "Diff. Color" ),
			reinterpret_cast<float*>( &static_cast<dx::XMFLOAT3&>( el ) ) ) );
	}

	if ( const auto &el = cb["modelSpecularColor"]; el.isValid() )
	{
		dirtyCheck( ImGui::ColorPicker3( tagImGuiWidget( "Spec. Color" ),
			reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>( el ) ) ) );
	}

	if ( const auto &el = cb["modelSpecularGloss"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Gloss" ),
			&el, 1.0f, 100.0f, "%.1f", 1.5f ) );
	}

	if ( const auto &el = cb["useSpecularMap"]; el.isValid() )
	{
		dirtyCheck( ImGui::Checkbox( tagImGuiWidget( "Enable Specular Map" ),
			&el ) );
	}

	if ( const auto &el = cb["useNormalMap"]; el.isValid() )
	{
		dirtyCheck( ImGui::Checkbox( tagImGuiWidget( "Enable Normal Map" ),
			&el ) );
	}

	if ( const auto &el = cb["normalMapWeight"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Normal Map Weight" ),
			&el, 0.0f, 2.0f ) );
	}

	return bDirty;
}

void EVShowcase::onSetEffect()
{
	ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f},
		util::capitalizeFirstLetter( m_pEffect->getTargetPassName() ).c_str() );

	bool bActive = m_pEffect->isEnabled();
	using namespace std::string_literals;
	ImGui::Checkbox( ( "Effect Active#"s + std::to_string( m_effectId ) ).c_str(),
		&bActive );
	m_pEffect->setEnabled( bActive );
}