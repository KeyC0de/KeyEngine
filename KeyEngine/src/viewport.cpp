#include "viewport.h"
#include "dxgi_info_queue.h"
#include "bindable_map.h"


Viewport::Viewport( Graphics &gph )
	:
	Viewport{gph, static_cast<const float>(gph.getClientWidth()), static_cast<const float>(gph.getClientHeight())}
{

}

Viewport::Viewport( Graphics &gph,
	const float width,
	const float height )
{
	m_viewport.Width = width;
	m_viewport.Height = height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
}

void Viewport::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->RSSetViewports( 1u, &m_viewport );
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

std::shared_ptr<Viewport> Viewport::fetch( Graphics &gph,
	const float width,
	const float height )
{
	using namespace std::string_literals;
	return BindableMap::fetch<Viewport>( gph, width, height );
}

std::string Viewport::calcUid( const float width,
	const float height )
{
	using namespace std::string_literals;
	return typeid( Viewport ).name() + "#"s + std::to_string( width ) + "x" + std::to_string( height );
}

std::string Viewport::getUid() const noexcept
{
	return calcUid( m_viewport.Width, m_viewport.Height );
}