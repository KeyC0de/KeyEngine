#include "effect.h"
#include "renderer.h"
#include "effect_visitor.h"
#include "render_queue_pass.h"
#include "assertions_console.h"


Effect::Effect( size_t channels,
	const std::string &targetPassName,
	bool bStartActive ) noexcept
	:
	m_renderingChannels(channels),
	m_active(bStartActive),
	m_targetPassName(targetPassName)
{

}

Effect::Effect( const Effect &rhs ) noexcept
	:
	m_renderingChannels(rhs.m_renderingChannels),
	m_active(rhs.m_active),
	m_targetPassName(rhs.m_targetPassName)
{
	m_pBindables.reserve( rhs.m_pBindables.size() );
	for ( auto &bindable : rhs.m_pBindables )
	{
		if ( auto *pClone = dynamic_cast<const IBindableCloning*>( bindable.get() ) )
		{
			m_pBindables.push_back( pClone->clone() );
		}
		else
		{
			m_pBindables.push_back( bindable );
		}
	}
}

void Effect::addBindable( std::shared_ptr<IBindable> bindable ) noexcept
{
	m_pBindables.push_back( std::move( bindable ) );
}

void Effect::render( const Drawable &drawable,
	size_t channels ) const noexcept
{
	if ( m_active && (m_renderingChannels & channels) != 0 )
	{
		m_pTargetPass->addJob( ren::Job{this, &drawable} );
	}
}

void Effect::bind( Graphics &gph ) const cond_noex
{
	for ( const auto &bi : m_pBindables )
	{
		bi->bind( gph );
	}
}

bool Effect::isEnabled() const noexcept
{
	return m_active;
}

void Effect::setEnabled( bool b ) noexcept
{
	m_active = b;
}

const std::string& Effect::getTargetPassName() const noexcept
{
	return m_targetPassName;
}

void Effect::setParentDrawable( const Drawable &parent ) noexcept
{
	for ( auto &bi : m_pBindables )
	{
		bi->setParentDrawable( parent );
	}
}

void Effect::accept( IEffectVisitor &ev )
{
	ev.setEffect( this );
	for ( auto &bindable : m_pBindables )
	{
		bindable->accept( ev );
	}
}

void Effect::connectPass( ren::Renderer &r )
{
	ASSERT( m_pTargetPass == nullptr, "Pass already set!" );
	m_pTargetPass = &r.getRenderQueuePass( m_targetPassName );
}

std::vector<std::shared_ptr<IBindable>>& Effect::getBindables() noexcept
{
	return m_pBindables;
}