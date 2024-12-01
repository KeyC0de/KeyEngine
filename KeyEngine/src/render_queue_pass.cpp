#include "render_queue_pass.h"
#include <algorithm>
#include <iterator>


namespace ren
{

RenderQueuePass::RenderQueuePass( const std::string &name,
	const std::vector<std::shared_ptr<IBindable>> &bindables /*= {}*/,
	const bool bTransparentMeshPass /*= false*/ )
	:
	IBindablePass{name, bindables},
	m_bTransparent{bTransparentMeshPass}
{

}

void RenderQueuePass::addJob( Job job,
	const float meshDistanceFromActiveCamera ) noexcept
{
	m_jobs.emplace_back( job, meshDistanceFromActiveCamera );
}

void RenderQueuePass::run( Graphics &gfx ) const cond_noex
{
	IBindablePass::bind( gfx );

	const_cast<RenderQueuePass*>( this )->sortJobs();

	for ( const auto &job : m_jobs )
	{
		job.first.run( gfx );
	}
}

void RenderQueuePass::reset() cond_noex
{
	m_jobs.clear();
}

size_t RenderQueuePass::getNumMeshes() const noexcept
{
	return m_jobs.size();
}

void RenderQueuePass::sortJobs()
{
	if ( m_bTransparent )
	{
		// if meshes are transparent sort them back to front (those with the largest distance from camera are rendered first)
		std::sort( m_jobs.begin(), m_jobs.end(),
		[] ( const auto &itLeft, const auto &itRight ) -> bool
			{
				const float distLeft = itLeft.second;
				const float distRight = itRight.second;
				if ( distLeft != distRight )
				{
					return distLeft > distRight;
				}
				return false;
			} );
	}
	else
	{
		// if meshes are opaque sort them front to back (those with least distance from camera are rendered first)
		std::sort( m_jobs.begin(), m_jobs.end(),
		[] ( const auto &itLeft, const auto &itRight ) -> bool
			{
				const float distLeft = itLeft.second;
				const float distRight = itRight.second;
				if ( distLeft != distRight )
				{
					return distLeft < distRight;
				}
				return false;
			} );
	}
}


}//namespace ren