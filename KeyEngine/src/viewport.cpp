#include "viewport.h"
#include "dxgi_info_queue.h"
#include "bindable_map.h"
#include "assertions_console.h"


Viewport::Viewport( Graphics& gph )
	:
	Viewport{gph, static_cast<float>(gph.getClientWidth()),
		static_cast<float>(gph.getClientHeight())}
{

}

Viewport::Viewport( Graphics& gph,
	float width,
	float height )
{
	m_viewport.Width = width;
	m_viewport.Height = height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
}

void Viewport::bind( Graphics& gph ) cond_noex
{
	getContext( gph )->RSSetViewports( 1u,
		&m_viewport );
	DXGI_GET_QUEUE_INFO( gph );
}

D3D11_VIEWPORT Viewport::getViewport() const noexcept
{
	return m_viewport;
}

float Viewport::getWidth() const noexcept
{
	return m_viewport.Width;
}

float Viewport::getHeight() const noexcept
{
	return m_viewport.Height;
}

std::shared_ptr<Viewport> Viewport::fetch( Graphics& gph,
	float width,
	float height )
{
	using namespace std::string_literals;
	return BindableMap::fetch<Viewport>( gph,
		width,
		height );
}

std::string Viewport::generateUID( float width,
	float height )
{
	using namespace std::string_literals;
	return typeid( Viewport ).name() + "#"s + std::to_string( width ) + "x"
		+ std::to_string( height );
}

std::string Viewport::getUID() const noexcept
{
	return generateUID( m_viewport.Width,
		m_viewport.Height );
}