#include "material.h"
#include <memory>
#include "mesh.h"
#include "renderer.h"
#include "render_queue_pass.h"
#include "assertions_console.h"


Material::Material( const size_t channels,
	const std::string &targetPassName,
	const bool bStartActive ) noexcept
	:
	m_renderingChannels(channels),
	m_bActive{bStartActive},
	m_targetPassName{targetPassName}
{

}

Material::Material( const Material &rhs )
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

Material::Material( Material &&rhs ) noexcept
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

void Material::addBindable( std::shared_ptr<IBindable> pBindable ) noexcept
{
	m_bindables.emplace_back( std::move( pBindable ) );
}


void Material::render( const Mesh &mesh,
	const size_t channels ) const noexcept
{
	if ( m_bActive && ( ( m_renderingChannels & channels ) != 0 ) )
	{
		m_pTargetPass->addJob( ren::Job{&mesh, this}, mesh.getDistanceFromActiveCamera() );
	}
}

void Material::bind( Graphics &gfx ) const cond_noex
{
	for ( const auto &bindable : m_bindables )
	{
		bindable->bind( gfx );
	}
}

bool Material::isEnabled() const noexcept
{
	return m_bActive;
}

void Material::setEnabled( const bool b ) noexcept
{
	m_bActive = b;
}

void Material::setEnabled( const size_t channels,
	const bool bEnabled ) noexcept
{
	if ( (m_renderingChannels & channels) != 0 )
	{
		m_bActive = bEnabled;
	}
}

const std::string& Material::getTargetPassName() const noexcept
{
	return m_targetPassName;
}

void Material::setMesh( const Mesh &parent ) noexcept
{
	for ( auto &bindable : m_bindables )
	{
		bindable->setMesh( parent );
	}
}

#ifndef FINAL_RELEASE
void Material::accept(IImGuiConstantBufferVisitor &ev)
{
	ev.setMaterial( this );
	for ( auto &bindable : m_bindables )
	{
		bindable->accept( ev );
	}
}
#endif

void Material::connectPass( ren::Renderer &r )
{
	m_pTargetPass = &r.getRenderQueuePass( m_targetPassName );
}

std::vector<std::shared_ptr<IBindable>>& Material::getBindables()
{
	return m_bindables;
}

const std::vector<std::shared_ptr<IBindable>>& Material::getBindables() const noexcept
{
	return m_bindables;
}

size_t Material::getChannelMask() const noexcept
{
	return m_renderingChannels;
}
