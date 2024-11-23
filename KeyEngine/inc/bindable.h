#pragma once

#include <memory>
#include "graphics_friend.h"
#include "assertions_console.h"


class Mesh;
class IImGuiConstantBufferVisitor;
class Graphics;

///=============================================================
/// \class	IBindable
/// \author	KeyC0de
/// \date	2022/08/30 19:27
/// \brief	a class encapsulating a GPU - Graphics Pipeline - resource
///=============================================================
class IBindable
	: public GraphicsFriend
{
public:
	virtual ~IBindable() noexcept
	{

	}

	/// \brief	bind graphics resource to a certain stage of the graphics pipeline
	virtual void bind( Graphics &gfx ) cond_noex = 0;

	virtual void setMesh( const Mesh &mesh ) noexcept
	{
		pass_;
	}

	virtual void accept( IImGuiConstantBufferVisitor &ev )
	{
		pass_;
	}

	virtual std::string getUid() const noexcept
	{
		ASSERT( false, "Base Bindable has no UID." );
		return "";
	}

	virtual void setDebugObjectName( const char* name ) noexcept
	{
		pass_;
	}
};

class IBindableCloning
	: public IBindable
{
public:
	virtual std::unique_ptr<IBindableCloning> clone() const noexcept = 0;
	virtual std::unique_ptr<IBindableCloning> clone() noexcept = 0;
};