#include "pass.h"
#include "binder.h"


namespace ren
{

IPass::IPass( const std::string &name,
	const bool bActive /*= true*/ ) noexcept
	:
	m_name{name},
	m_bActive{bActive}
{

}

IPass::~IPass() noexcept
{
	pass_;
}

const std::string& IPass::getName() const noexcept
{
	return m_name;
}

void IPass::validate()
{
	for ( auto &binder : m_binders )
	{
		binder->validateLinkage();
	}
}

void IPass::setActive( const bool bActive ) noexcept
{
	m_bActive = bActive;
}

const bool IPass::isActive() const noexcept
{
	return m_bActive;
}

void IPass::recreateRtvsAndDsvs( Graphics &gfx )
{
	pass_;
}

const std::vector<std::unique_ptr<IBinder>>& IPass::getBinders() const
{
	return m_binders;
}

const std::vector<std::unique_ptr<ILinker>>& IPass::getLinkers() const
{
	return m_linkers;
}

IBinder& IPass::getBinder( const std::string &name ) const
{
	for ( auto &binder : m_binders )
	{
		if ( binder->getName() == name )
		{
			return *binder;
		}
	}

	std::ostringstream oss;
	oss << "BindableBinder named [" << name << "] not found in pass:" << getName();
	THROW_RENDERER_EXCEPTION( oss.str() );
}

ILinker& IPass::getLinker( const std::string &name ) const
{
	for ( auto &linker : m_linkers )
	{
		if ( linker->getName() == name )
		{
			return *linker;
		}
	}

	std::ostringstream oss;
	oss << "Linker named [" << name << "] not found in pass: " << getName();
	THROW_RENDERER_EXCEPTION( oss.str() );
}

void IPass::addBinder( std::unique_ptr<IBinder> pBinder )
{
	// verify there are no name collisions with other binders
	for ( auto &binder : m_binders )
	{
		if ( binder->getName() == pBinder->getName() )
		{
			THROW_RENDERER_EXCEPTION( "BindableBinder name " + pBinder->getName() + " collides with existing." );
		}
	}
	m_binders.emplace_back( std::move( pBinder ) );
}

void IPass::addLinker( std::unique_ptr<ILinker> pLinker )
{
	// verify there are no name collisions with other linkers
	for ( auto &linker : m_linkers )
	{
		if ( linker->getName() == pLinker->getName() )
		{
			THROW_RENDERER_EXCEPTION( "Linker name " + pLinker->getName() + " collides with existing." );
		}
	}
	m_linkers.emplace_back( std::move( pLinker ) );
}

void IPass::setupBinderTarget( const std::string &currentPassBinderName,
	const std::string &targetPassName,
	const std::string &targetPassLinkerName )
{
	auto &binder = getBinder( currentPassBinderName );
	binder.setPassAndLinkerNames( targetPassName, targetPassLinkerName );
}


}//namespace ren