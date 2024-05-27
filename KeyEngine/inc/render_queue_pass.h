#pragma once

#include <vector>
#include "bindable_pass.h"
#include "job.h"


namespace ren
{

class RenderQueuePass
	: public IBindablePass
{
	bool m_bTransparent;
	std::vector<std::pair<Job, float>> m_meshes;
public:
	RenderQueuePass( const std::string &name, const std::vector<std::shared_ptr<IBindable>> &bindables = {}, const bool bTransparentMeshPass = false );

	void addJob( Job job, const float meshDistanceFromActiveCamera ) noexcept;
	//	\function	run	||	\date	2021/06/27 0:11
	//	\brief	call RenderQueuePass::run from derivedPassClass::run as a final task
	//			first binds common/shared Pass bindables, then binds individual Mesh bindables (Job), then binds Material bindables (Job), then executes draw call
	void run( Graphics &gfx ) const cond_noex override;
	void reset() cond_noex override;
	size_t getNumMeshes() const noexcept;
private:
	void sortJobs();
};


}//ren