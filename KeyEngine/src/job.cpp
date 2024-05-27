#include "job.h"
#include "mesh.h"
#include "material.h"
#include "dxgi_info_queue.h"


namespace ren
{

Job::Job( const Mesh *pMesh, const Material *pMaterial )
	:
	m_pMesh{pMesh},
	m_pMaterial{pMaterial}
{

}

void Job::run( Graphics &gfx ) const cond_noex
{
	m_pMesh->bind( gfx );		// bind P.T., V.B., I.B.
	m_pMaterial->bind( gfx );	// bind other bindables
	gfx.drawIndexed( m_pMesh->getIndicesCount() );
	DXGI_GET_QUEUE_INFO( gfx );
}


}//ren