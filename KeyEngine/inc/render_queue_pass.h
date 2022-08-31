#pragma once

#include <vector>
#include "bindable_pass.h"
#include "job.h"


namespace ren
{

class RenderQueuePass
	: public IBindablePass
{
	std::vector<Job> m_jobs;
public:
	using IBindablePass::IBindablePass;	// pass name & bindables vector

	void addJob( Job job ) noexcept;
	//===================================================
	//	\function	run
	//	\brief  call RenderQueuePass::run from derivedPassClass::run as a final task
	//			Do that for a complete shader cycle:
	//				eg Outline Mask is 1st "part" of Outline Effect, and Outline Draw is 2nd part of Outline Effect, so override `run only on the last part of the Effect.
	//	\date	2021/06/27 0:11
	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
};


}//ren