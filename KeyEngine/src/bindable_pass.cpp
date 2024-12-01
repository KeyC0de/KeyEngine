#include "bindable_pass.h"
#include "render_target_view.h"
#include "depth_stencil_view.h"


namespace ren
{

IBindablePass::IBindablePass( const std::string &name,
	const std::vector<std::shared_ptr<IBindable>> &bindables )
	:
	IPass{name},
	m_bindables(bindables)
{

}

void IBindablePass::addBindable( std::shared_ptr<IBindable> bindable ) noexcept
{
	m_bindables.emplace_back( bindable );
}

void IBindablePass::bind( Graphics &gfx ) const cond_noex
{
	if ( m_pRtv )
	{
		m_pRtv->bindRenderSurface( gfx, m_pDsv.get() );
	}
	else if ( m_pDsv )
	{
		m_pDsv->bindRenderSurface( gfx );
	}

	for ( auto &b : m_bindables )
	{
		b->bind( gfx );
	}
}

void IBindablePass::validate()
{
	IPass::validate();
	if ( !m_pRtv && !m_pDsv && getName() != "shadow" /*Shadow Pass's render surfaces are bound during run()*/ )
	{
		THROW_RENDERER_EXCEPTION( "IBindablePass: Both IRenderTargetView & IDepthStencilView are null!" + getName() );
	}
}


}//namespace ren