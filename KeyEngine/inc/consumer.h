#pragma once

#include <string>
#include <memory>
#include <typeinfo>
#include <sstream>
#include <type_traits>
#include "producer.h"
#include "render_surface.h"
#include "bindable.h"
#include "render_target.h"
#include "depth_stencil_view.h"


class IBindable;

namespace ren
{

class IPass;

//=============================================================
//	\class	IConsumer
//
//	\author	KeyC0de
//	\date	2022/09/29 20:48
//
//	\brief	Consumers link to Producers, so they require their names.
//			The opposite is not strictly required.
//			But I store the names of Consumers in case the system is expanded in the future, or may be needed for misc purposes.s
//=============================================================
class IConsumer
{
	std::string m_name;
	std::string m_passName;
	std::string m_producerName;
protected:
	IConsumer( const std::string &name );
public:
	virtual ~IConsumer() = default;

	void setPassAndProducerNames( const std::string &passName, const std::string &producerName );
	virtual void link( IProducer &producer ) = 0;
	//===================================================
	//	\function	validateLinkage
	//	\brief  assert validate after link()ing
	//	\date	2021/06/26 23:57
	virtual void validateLinkage() const = 0;
	const std::string& getName() const noexcept;
	const std::string& getPassName() const noexcept;
	const std::string& getProducerName() const noexcept;
};


template<class T>
class BindableConsumer final
	: public IConsumer
{
	static_assert( std::is_base_of_v<IBindable, T>, "BindableConsumer target T must be IBindable." );

	std::shared_ptr<T> &m_target;	// target Producer resource Bindable
	bool m_bLinked = false;
public:
	BindableConsumer( const std::string &name,
		std::shared_ptr<T> &target )
		:
		IConsumer{name},
		m_target{target}
	{

	}

	static std::unique_ptr<IConsumer> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<BindableConsumer>( name,
			target );
	}

	void validateLinkage() const override
	{
		if ( !m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "consumer " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( IProducer &producer ) override
	{
		auto bindable = std::dynamic_pointer_cast<T>( producer.getBindable() );
		if ( !bindable )
		{
			std::ostringstream oss;
			oss << "Linking consumer ["
				<< getName()
				<< "] with producer ["
				<< getPassName()
				<< "."
				<< getProducerName()
				<< "] "
				<< " { "
				<< typeid( T ).name()
				<< " } does not match { "
				<< typeid( *producer.getBindable().get() ).name()
				<< " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_target = std::move( bindable );
		m_bLinked = true;
	}
};

//=============================================================
//	\class	ContainerBindableConsumer
//
//	\author	KeyC0de
//	\date	2022/08/21 0:17
//
//	\brief	Container Bindable is either an RTV, DSV or an array type Constant Buffer
//=============================================================
template<class T>
class ContainerBindableConsumer final
	: public IConsumer
{
	//static_assert( std::is_array_v<T> || std::is_base_of_v<IBindable, T>, "ContainerBindableConsumer target T must be IBindable." );

	std::vector<std::shared_ptr<IBindable>> &m_container;
	size_t m_index;
	bool m_bLinked = false;
public:
	ContainerBindableConsumer( const std::string &name,
		std::vector<std::shared_ptr<IBindable>> &container,
		const size_t index )
		:
		IConsumer{name},
		m_container{container},
		m_index(index)
	{

	}

	void validateLinkage() const override
	{
		if ( !m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "Consumer " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( IProducer &producer ) override
	{
		auto bindable = std::dynamic_pointer_cast<T>( producer.getBindable() );
		if ( !bindable )
		{
			std::ostringstream oss;
			oss << "Linking consumer ["
				<< getName()
				<< "] with producer ["
				<< getPassName()
				<< "."
				<< getProducerName()
				<< "] "
				<< " { "
				<< typeid( T ).name()
				<< " } does not match { "
				<< typeid( *producer.getBindable().get() ).name()
				<< " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_container[m_index] = std::move( bindable );
		m_bLinked = true;
	}
};


template<class T>
class RenderSurfaceConsumer final
	: public IConsumer
{
	static_assert( std::is_base_of_v<IRenderSurface, T>, "RenderSurfaceConsumer target T is not IRenderSurface!" );

	std::shared_ptr<T> &m_target;
	bool m_bLinked = false;
public:
	RenderSurfaceConsumer( const std::string &name,
		std::shared_ptr<T> &target )
		:
		IConsumer{name},
		m_target{target}
	{

	}

	static std::unique_ptr<IConsumer> make( const std::string &name,
		std::shared_ptr<T> &target )
	{
		return std::make_unique<RenderSurfaceConsumer>( name,
			target );
	}

	void validateLinkage() const override
	{
		if ( !m_bLinked )
		{
			THROW_RENDERER_EXCEPTION( "Consumer " + getName() + " unliked! Target hasn't been set!" );
		}
	}

	void link( IProducer &producer ) override
	{
		auto buff = std::dynamic_pointer_cast<T>( producer.getRenderSurface() );
		if ( !buff )
		{
			std::ostringstream oss;
			oss << "Linking consumer ["
				<< getName()
				<< "] with producer ["
				<< getPassName()
				<< "."
				<< getProducerName()
				<< "] "
				<< " { "
				<< typeid( T ).name()
				<< " } not compatible with { "
				<< typeid( *producer.getRenderSurface().get() ).name()
				<< " }";
			THROW_RENDERER_EXCEPTION( oss.str() );
		}
		m_target = std::move( buff );
		m_bLinked = true;
	}
};


}//ren