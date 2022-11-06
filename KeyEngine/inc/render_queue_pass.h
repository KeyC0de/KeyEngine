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
	//	\function	run	||	\date	2021/06/27 0:11
	//	\brief  call RenderQueuePass::run from derivedPassClass::run as a final task
	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
#if defined _DEBUG && !defined NDEBUG
	int getJobCount() const noexcept;
#endif
};


}//ren