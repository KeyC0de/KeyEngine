#include "effect.h"
#include <memory>
#include "mesh.h"
#include "renderer.h"
#include "imgui_visitors.h"
#include "render_queue_pass.h"
#include "assertions_console.h"


Effect::Effect( const size_t channels,
	const std::string &targetPassName,
	const bool bStartActive ) noexcept
	:
	m_renderingChannels(channels),
	m_bActive{bStartActive},
	m_targetPassName{targetPassName}
{

}

Effect::Effect( const Effect &rhs )
	:
	m_renderingChannels{rhs.m_renderingChannels},
	m_bActive{rhs.m_bActive},
	m_targetPassName{rhs.m_targetPassName},
	m_pTargetPass{rhs.m_pTargetPass}
{
	m_bindables.reserve( rhs.m_bindables.size() );
	for ( auto &pBindable : rhs.m_bindables )
	{
		if ( const auto*pClone = dynamic_cast<const IBindableCloning*>( pBindable.get() ) )
		{
			m_bindables.emplace_back( pClone->clone() );
		}
		else
		{
			m_bindables.emplace_back( pBindable );
		}
	}
}

Effect::Effect( Effect &&rhs ) noexcept
	:
	m_renderingChannels{rhs.m_renderingChannels},
	m_bActive{rhs.m_bActive},
	m_targetPassName{std::move( rhs.m_targetPassName )},
	m_pTargetPass{rhs.m_pTargetPass}
{
	std::swap( m_bindables, rhs.m_bindables );

	rhs.m_renderingChannels = 0u;
	rhs.m_bActive = false;
	rhs.m_pTargetPass = nullptr;
}

void Effect::addBindable( std::shared_ptr<IBindable> pBindable ) noexcept
{
	m_bindables.emplace_back( std::move( pBindable ) );
}

void Effect::render( const Mesh &mesh,
	const size_t channels ) const noexcept
{
	if ( m_bActive && (m_renderingChannels & channels) != 0 )
	{
		m_pTargetPass->addJob( ren::Job{&mesh, this}, mesh.getDistanceFromActiveCamera() );
	}
}

void Effect::bind( Graphics &gfx ) const cond_noex
{
	for ( const auto &bindable : m_bindables )
	{
		bindable->bind( gfx );
	}
}

bool Effect::isEnabled() const noexcept
{
	return m_bActive;
}

void Effect::setEnabled( const bool b ) noexcept
{
	m_bActive = b;
}

void Effect::setEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	if ( (m_renderingChannels & channels) != 0 )
	{
		m_bActive = bEnabled;
	}
}

const std::string& Effect::getTargetPassName() const noexcept
{
	return m_targetPassName;
}

void Effect::setMesh( const Mesh &parent ) noexcept
{
	for ( auto &bindable : m_bindables )
	{
		bindable->setMesh( parent );
	}
}

void Effect::accept( IImGuiEffectVisitor &ev )
{
	ev.setEffect( this );
	for ( auto &bindable : m_bindables )
	{
		bindable->accept( ev );
	}
}

void Effect::connectPass( ren::Renderer &r )
{
	ASSERT( m_pTargetPass == nullptr, "Effect already connected to Renderer!" );
	m_pTargetPass = &r.getRenderQueuePass( m_targetPassName );
}

std::vector<std::shared_ptr<IBindable>>& Effect::getBindables()
{
	return m_bindables;
}

const std::vector<std::shared_ptr<IBindable>>& Effect::getBindables() const noexcept
{
	return m_bindables;
}

size_t Effect::getChannelMask() const noexcept
{
	return m_renderingChannels;
}
