#pragma once

#include <vector>
#include "bindable_pass.h"
#include "job.h"


namespace ren
{

class RenderQueuePass
	: public IBindablePass
{
	std::vector<std::pair<Job, float>> m_opaques;
	std::vector<std::pair<Job, float>> m_transparents;
public:
	using IBindablePass::IBindablePass;	// pass name & bindables vector

	void addJob( Job job, const float meshDistanceFromActiveCamera, const bool bTransparent = false ) noexcept;
	//	\function	run	||	\date	2021/06/27 0:11
	//	\brief  call RenderQueuePass::run from derivedPassClass::run as a final task
	void run( Graphics &gph ) const cond_noex override;
	void reset() cond_noex override;
	int getNumOpaques() const noexcept;
	int getNumTransparents() const noexcept;
	int getJobCount() const noexcept;
private:
	void sortJobs();
};


}//ren