#pragma once

#include "bindable.h"
#include <typeinfo>


class Viewport
	: public IBindable
{
	D3D11_VIEWPORT m_viewport{};
public:
	Viewport( Graphics& gph );
	Viewport( Graphics& gph, float width, float height );

	void bind( Graphics& gph ) cond_noex override;
	D3D11_VIEWPORT getViewport() const noexcept;
	float getWidth() const noexcept;
	float getHeight() const noexcept;
	static std::shared_ptr<Viewport> fetch( Graphics& gph, float width, float height );
	static std::string generateUID( float width, float height );
	std::string getUID() const noexcept override;
};