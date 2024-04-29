#pragma once

#include <string>
#include <memory>
#include <typeinfo>
#include <sstream>
#include <type_traits>
#include "linker.h"
#include "render_surface.h"
#include "bindable.h"
#include "render_target.h"
#include "depth_stencil_view.h"


class IBindable;

namespace ren
{

class IPass;

//=============================================================
//	\class	IBinder
//	\author	KeyC0de
//	\date	2022/09/29 20:48
//	\brief	Binders contain the graphics resources and those resources are bound to the pipeline of a next pass
//			Linkers must link the resources of a previous pass to the next pass
//			This linking is done using string names. So Binders try to find at runtime their Linker names to make the link. The opposite is not strictly required.
//			But just in case, I currently store the names of Binders, in case the system is expanded in the future, or may be needed for misc purposes.
//=============================================================
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
	//	\function	validateLinkage	||	\date	2021/06/26 23:57
	//	\brief  assert validate after link()ing
	virtual void validateLinkage() const = 0;
	const std::string& getName() const noexcept;
	const std::string& getPassName() const noexcept;
	const std::string& getLinkerName() const noexcept;
};

template<class T>
class Binder final
	: public IBinder
{
	static_assert( std::is_base_of_v<IBindable, T>, "Binder target T must be IBindable." );

	std::shared_ptr<T> &m_target;	// target Linker resource Bindable
	bool m_bLinked = false;
public:
	Binder( const std::string &name,
		std::shared_ptr<T> &target )
		:
		IBinder{name},
		m_target{target}
	{

	}

	static std::unique_ptr<IBinder> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<Binder>( name, target );
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
			oss << "Linking binder ["
				<< getName()
				<< "] with linker ["
				<< getPassName()
				<< "."
				<< getLinkerName()
				<< "] "
				<< " { "
				<< typeid( T ).name()
				<< " } does not match { "
				<< typeid( *linker.getBindable().get() ).name()
				<< " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_target = std::move( bindable );
		m_bLinked = true;
	}
};

//=============================================================
//	\class	ContainerBindableBinder
//	\author	KeyC0de
//	\date	2022/08/21 0:17
//	\brief	Container Bindable is either an RTV, DSV or an array type Constant Buffer
//=============================================================
template<class T>
class ContainerBindableBinder final
	: public IBinder
{
	static_assert( std::is_array_v<T> || std::is_base_of_v<IBindable, T>, "ContainerBindableBinder target T must be IBindable." );

	std::vector<std::shared_ptr<IBindable>> &m_bindablesContainer;
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
			THROW_RENDERER_EXCEPTION( "Binder " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( ILinker &linker ) override
	{
		auto bindable = std::dynamic_pointer_cast<T>( linker.getBindable() );
		if ( !bindable )
		{
			std::ostringstream oss;
			oss << "Linking binder ["
				<< getName()
				<< "] with linker ["
				<< getPassName()
				<< "."
				<< getLinkerName()
				<< "] "
				<< " { "
				<< typeid( T ).name()
				<< " } does not match { "
				<< typeid( *linker.getBindable().get() ).name()
				<< " }";
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
			THROW_RENDERER_EXCEPTION( "Binder " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( ILinker &linker ) override
	{
		auto buff = std::dynamic_pointer_cast<T>( linker.getRenderSurface() );
		if ( !buff )
		{
			std::ostringstream oss;
			oss << "Linking binder ["
				<< getName()
				<< "] with linker ["
				<< getPassName()
				<< "."
				<< getLinkerName()
				<< "] "
				<< " { "
				<< typeid( T ).name()
				<< " } not compatible with { "
				<< typeid( *linker.getRenderSurface().get() ).name()
				<< " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_target = std::move( buff );
		m_bLinked = true;
	}
};


}//ren