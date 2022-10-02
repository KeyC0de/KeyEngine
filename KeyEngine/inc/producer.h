#pragma once

#include <string>
#include <memory>
#include "renderer_exception.h"


class IBindable;
class IRenderSurface;

namespace ren
{

class IProducer
{
	std::string m_name;
protected:
	IProducer( const std::string &name );
public:
	virtual ~IProducer() noexcept = default;

	const std::string& getName() const noexcept;
	virtual std::shared_ptr<IBindable> getBindable();
	virtual std::shared_ptr<IRenderSurface> getRenderSurface();
};

//=============================================================
//	\class	BindableProducer
//
//	\author	KeyC0de
//	\date	2022/10/02 21:08
//
//	\brief	This T will mostly be RTV of an offscreen surface
//=============================================================
template<class T>
class BindableProducer final
	: public IProducer
{
	static_assert( std::is_base_of_v<IBindable, T>, "BindableProducer target T is not IBindable!" );

	std::shared_ptr<T> &m_target;
public:
	BindableProducer( const std::string &name,
		std::shared_ptr<T>& target )
		:
		IProducer{name},
		m_target{target}
	{

	}

	static std::unique_ptr<BindableProducer> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<BindableProducer>( name,
			target );
	}

	std::shared_ptr<IBindable> getBindable() override
	{
		return m_target;
	}
};

template<class T>
class RenderSurfaceProducer final
	: public IProducer
{
	static_assert( std::is_base_of_v<IRenderSurface, T>, "RenderSurfaceProducer target T is not IRenderSurface!" );

	std::shared_ptr<T> &m_target;
	bool m_bLinked = false;
public:
	RenderSurfaceProducer( const std::string &name,
		std::shared_ptr<T> &target )
		:
		IProducer{name},
		m_target{target}
	{

	}

	static std::unique_ptr<RenderSurfaceProducer> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<RenderSurfaceProducer>( name,
			target );
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