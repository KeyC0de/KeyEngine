#pragma once

#include <memory>
#include "graphics_friend.h"
#include "assertions_console.h"


class Drawable;
class IEffectVisitor;
class Graphics;

class IBindable
	: public GraphicsFriend
{
public:
	virtual ~IBindable() noexcept
	{

	}

	// bind resource to a certain stage of the graphics pipeline
	virtual void bind( Graphics &gph ) cond_noex = 0;

	virtual void setParentDrawable( const Drawable &drawable ) noexcept
	{
		pass_;
	}

	virtual void accept( IEffectVisitor &ev )
	{
		pass_;
	}

	virtual std::string getUid() const noexcept
	{
		ASSERT( false, "Base Bindable has no UID." );
		return "";
	}
};

class IBindableCloning
	: public IBindable
{
public:
	virtual std::unique_ptr<IBindableCloning> clone() const noexcept = 0;
};