#include "render_queue_pass.h"
#include <algorithm>
#include <iterator>


namespace ren
{

void RenderQueuePass::addJob( Job job,
	const float meshDistanceFromActiveCamera,
	const bool bTransparent /*= false*/ ) noexcept
{
	if ( bTransparent )
	{
		m_transparents.emplace_back( job, meshDistanceFromActiveCamera );
	}
	else
	{
		m_opaques.emplace_back( job, meshDistanceFromActiveCamera );
	}
}

void RenderQueuePass::run( Graphics &gph ) const cond_noex
{
	IBindablePass::bind( gph );

	const_cast<RenderQueuePass*>( this )->sortJobs();

	for ( const auto &job : m_opaques )
	{
		job.first.run( gph );
	}

	for ( const auto &job : m_transparents )
	{
		job.first.run( gph );
	}
}

void RenderQueuePass::reset() cond_noex
{
	m_opaques.clear();
	m_transparents.clear();
}

int RenderQueuePass::getNumOpaques() const noexcept
{
	return m_opaques.size();
}

int RenderQueuePass::getNumTransparents() const noexcept
{
	return m_transparents.size();
}

int RenderQueuePass::getJobCount() const noexcept
{
	return getNumOpaques() + getNumTransparents();
}

void RenderQueuePass::sortJobs()
{
	// sort opaques front to back (those with least distance from camera are rendered first)
	std::sort( m_opaques.begin(), m_opaques.end(),
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

	// sort transparents back to front (those with largest distance from camera are rendered first)
	std::sort( m_transparents.begin(), m_transparents.end(),
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


}//ren