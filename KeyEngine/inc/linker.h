#pragma once

#include <string>
#include <memory>
#include "renderer_exception.h"


class IBindable;
class IRenderSurface;

namespace ren
{

class ILinker
{
	std::string m_name;
protected:
	ILinker( const std::string &name );
public:
	virtual ~ILinker() noexcept = default;

	const std::string& getName() const noexcept;
	virtual std::shared_ptr<IBindable> getBindable();
	virtual std::shared_ptr<IRenderSurface> getRenderSurface();
};

//=============================================================
//	\class	BindableLinker
//	\author	KeyC0de
//	\date	2022/10/02 21:08
//	\brief	This T will mostly be RTV of an offscreen surface, but it can also be a PSCB
//=============================================================
template<class T>
class BindableLinker final
	: public ILinker
{
	static_assert( std::is_base_of_v<IBindable, T>, "BindableLinker target T is not IBindable!" );

	std::shared_ptr<T> &m_target;
public:
	BindableLinker( const std::string &name,
		std::shared_ptr<T>& target )
		:
		ILinker{name},
		m_target{target}
	{

	}

	static std::unique_ptr<BindableLinker> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<BindableLinker>( name, target );
	}

	std::shared_ptr<IBindable> getBindable() override
	{
		return m_target;
	}
};

//=============================================================
//	\class	RenderSurfaceLinker
//	\author	KeyC0de
//	\date	2022/10/07 21:30
//	\brief	T should mostly be the main Render Surface color texture/buffer or depth-stencil texture/buffer
//=============================================================
template<class T>
class RenderSurfaceLinker final
	: public ILinker
{
	static_assert( std::is_base_of_v<IRenderSurface, T>, "RenderSurfaceLinker target T is not IRenderSurface!" );

	std::shared_ptr<T> &m_target;
	bool m_bLinked = false;
public:
	RenderSurfaceLinker( const std::string &name,
		std::shared_ptr<T> &target )
		:
		ILinker{name},
		m_target{target}
	{

	}

	static std::unique_ptr<RenderSurfaceLinker> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<RenderSurfaceLinker>( name, target );
	}

	std::shared_ptr<IRenderSurface> getRenderSurface() override
	{
		if ( m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "Repeated attempt made to bind " + getName() + "!" )
		}
		m_bLinked = true;
		return m_target;
	}
};


}