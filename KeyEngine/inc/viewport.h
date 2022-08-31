#pragma once

#include "bindable.h"
#include <typeinfo>


class Viewport
	: public IBindable
{
	D3D11_VIEWPORT m_viewport{};
public:
	Viewport( Graphics &gph );
	Viewport( Graphics &gph, const float width, const float height );

	void bind( Graphics &gph ) cond_noex override;
	D3D11_VIEWPORT getViewport() const noexcept;
	const float getWidth() const noexcept;
	const float getHeight() const noexcept;
	static std::shared_ptr<Viewport> fetch( Graphics &gph, const float width, const float height );
	static std::string calcUid( const float width, const float height );
	const std::string getUid() const noexcept override;
};