#include "render_queue_pass.h"


namespace ren
{

void RenderQueuePass::addJob( Job job ) noexcept
{
	m_jobs.push_back( job );
}

void RenderQueuePass::run( Graphics &gph ) const cond_noex
{
	bindPass( gph );
	for ( const auto& job : m_jobs )
	{
		job.run( gph );
	}
}

void RenderQueuePass::reset() cond_noex
{
	m_jobs.clear();
}


}//ren