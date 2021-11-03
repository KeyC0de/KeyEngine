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
	using IBindablePass::IBindablePass;

	void addJob( Job job ) noexcept;
	//===================================================
	//	\function	run
	//	\brief  always call RenderQueuePass::run from derivedPassClass::run as a final task
	//	\date	2021/06/27 0:11
	void run( Graphics& gph ) const cond_noex override;
	void reset() cond_noex override;
};


}//ren