#include "job.h"
#include "mesh.h"
#include "effect.h"
#include "dxgi_info_queue.h"


namespace ren
{

Job::Job( const Mesh *pMesh, const Effect *pEffect )
	:
	m_pMesh{pMesh},
	m_pEffect{pEffect}
{

}

void Job::run( Graphics &gfx ) const cond_noex
{
	m_pMesh->bind( gfx );		// bind P.T., I.B., V.B.
	m_pEffect->bind( gfx );	// bind other bindables
	gfx.drawIndexed( m_pMesh->getIndicesCount() );
	DXGI_GET_QUEUE_INFO( gfx );
}


}//ren