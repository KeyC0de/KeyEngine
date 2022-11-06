#include "render_queue_pass.h"


namespace ren
{

void RenderQueuePass::addJob( Job job ) noexcept
{
	m_jobs.emplace_back( job );
}

void RenderQueuePass::run( Graphics &gph ) const cond_noex
{
	bind( gph );
	for ( const auto &job : m_jobs )
	{
		job.run( gph );
	}
}

void RenderQueuePass::reset() cond_noex
{
	m_jobs.clear();
}

#if defined _DEBUG && !defined NDEBUG
int RenderQueuePass::getJobCount() const noexcept
{
	return m_jobs.size();
}
#endif


}//ren