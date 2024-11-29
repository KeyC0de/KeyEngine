#pragma once

#include <string>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <sstream>
#include "linker.h"
#include "bindable.h"
#include "render_surface.h"
#include "render_target_view.h"
#include "depth_stencil_view.h"


class IBindable;

namespace ren
{

class IPass;

///=============================================================
/// \class	IBinder
/// \author	KeyC0de
/// \date	2022/09/29 20:48
/// \brief	BindableBinder has a dependency: their target gfx resource; they must acquire it from the Linker of the target pass
/// \brief	this link()ing will be done in Renderer::linkPassBinders
/// \brief	BindableBinder names are not required, but just in case, I currently store their names, in case the system is expanded in the future.
/// \brief	IBinder has a protected ctor; create child objects using the maker pattern
///=============================================================
class IBinder
{
	std::string m_name;
	std::string m_passName;
	std::string m_linkerName;
protected:
	IBinder( const std::string &name );
public:
	virtual ~IBinder() = default;

	void setPassAndLinkerNames( const std::string &passName, const std::string &linkerName );
	virtual void link( ILinker &linker ) = 0;
	/// \brief	assert validate after link()ing
	virtual void validateLinkage() const = 0;
	const std::string& getName() const noexcept;
	const std::string& getPassName() const noexcept;
	const std::string& getLinkerName() const noexcept;
};

template<class T>
class BindableBinder final
	: public IBinder
{
	static_assert( std::is_base_of_v<IBindable, T>, "BindableBinder target T must be IBindable." );

	std::shared_ptr<T> &m_target;	// target Linker resource Bindable
	bool m_bLinked = false;
public:
	BindableBinder( const std::string &name,
		std::shared_ptr<T> &target )
		:
		IBinder{name},
		m_target{target}
	{

	}

	static std::unique_ptr<IBinder> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<BindableBinder>( name, target );
	}

	void validateLinkage() const override
	{
		if ( !m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "binder " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( ILinker &linker ) override
	{
		auto bindable = std::dynamic_pointer_cast<T>( linker.getBindable() );
		if ( !bindable )
		{
			std::ostringstream oss;
			oss << "Linking binder [" << getName() << "] with linker [" << getPassName() << "." << getLinkerName() << "] " << " { " << typeid( T ).name() << " } does not match { " << typeid( *linker.getBindable().get() ).name() << " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_target = std::move( bindable );
		m_bLinked = true;
	}
};

///=============================================================
/// \class	ContainerBindableBinder
/// \author	KeyC0de
/// \date	2022/08/21 0:17
/// \brief	Used to make a BindableBinder 
///=============================================================
template<class T>
class ContainerBindableBinder final
	: public IBinder
{
	static_assert( std::is_array_v<T> || std::is_base_of_v<IBindable, T>, "ContainerBindableBinder target T must be IBindable." );

	std::vector<std::shared_ptr<IBindable>> &m_bindablesContainer;	// points to a Pass' bindables container
	size_t m_index;
	bool m_bLinked = false;
public:
	ContainerBindableBinder( const std::string &name,
		std::vector<std::shared_ptr<IBindable>> &bindablesContainer,
		const size_t index )
		:
		IBinder{name},
		m_bindablesContainer{bindablesContainer},
		m_index(index)
	{

	}

	void validateLinkage() const override
	{
		if ( !m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "BindableBinder " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( ILinker &linker ) override
	{
		auto bindable = std::dynamic_pointer_cast<T>( linker.getBindable() );
		if ( !bindable )
		{
			std::ostringstream oss;
			oss << "Linking binder [" << getName() << "] with linker [" << getPassName() << "." << getLinkerName() << "] " << " { " << typeid( T ).name() << " } does not match { " << typeid( *linker.getBindable().get() ).name() << " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_bindablesContainer[m_index] = std::move( bindable );
		m_bLinked = true;
	}
};

template<class T>
class RenderSurfaceBinder final
	: public IBinder
{
	static_assert( std::is_base_of_v<IRenderSurface, T>, "RenderSurfaceBinder target T is not IRenderSurface!" );

	std::shared_ptr<T> &m_target;
	bool m_bLinked = false;
public:
	RenderSurfaceBinder( const std::string &name,
		std::shared_ptr<T> &target )
		:
		IBinder{name},
		m_target{target}
	{

	}

	static std::unique_ptr<IBinder> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<RenderSurfaceBinder>( name, target );
	}

	void validateLinkage() const override
	{
		if ( !m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "BindableBinder " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( ILinker &linker ) override
	{
		auto buff = std::dynamic_pointer_cast<T>( linker.getRenderSurface() );
		if ( !buff )
		{
			std::ostringstream oss;
			oss << "Linking binder [" << getName() << "] with linker [" << getPassName() << "." << getLinkerName() << "] " << " { " << typeid( T ).name() << " } not compatible with { " << typeid( *linker.getRenderSurface().get() ).name() << " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_target = std::move( buff );
		m_bLinked = true;
	}
};


}//ren