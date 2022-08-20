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

	//===================================================
	//	\function	bind
	//	\brief  bind graphics resource to a certain stage of the graphics pipeline
	//	\date	2022/08/21 0:00
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
	virtual std::unique_ptr<IBindableCloning> clone() noexcept = 0;
};