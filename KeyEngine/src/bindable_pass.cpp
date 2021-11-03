#include "bindable_pass.h"
#include "render_target.h"
#include "depth_stencil_view.h"


namespace ren
{

IBindablePass::IBindablePass( const std::string& name,
	const std::vector<std::shared_ptr<IBindable>>& bindables )
	:
	IPass{name},
	m_bindables(bindables)
{

}

void IBindablePass::addPassSharedBindable( std::shared_ptr<IBindable> bindable ) noexcept
{
	m_bindables.emplace_back( bindable );
}

void IBindablePass::bindPassShared( Graphics& gph ) const cond_noex
{
	if ( m_pRtv )
	{
		m_pRtv->bindRenderSurface( gph,
			m_pDsv.get() );
	}
	else
	{
		m_pDsv->bindRenderSurface( gph );
	}

	for ( auto& b : m_bindables )
	{
		b->bind( gph );
	}
}

void IBindablePass::validate()
{
	IPass::validate();
	if ( !m_pRtv && !m_pDsv )
	{
		throwRendererException( "IBindablePass: Both IRenderTargetView & IDepthStencilView are null!"
			+ getName() );
	}
}


}//ren