#include "job.h"
#include "drawable.h"
#include "effect.h"
#include "dxgi_info_queue.h"


namespace ren
{

Job::Job( const Effect *pEffect,
	const Drawable *pDrawable )
	:
	m_pDrawable{pDrawable},
	m_pEffect{pEffect}
{

}

void Job::run( Graphics &gph ) const cond_noex
{
	m_pDrawable->bind( gph );	// bind P.T., I.B., V.B.
	m_pEffect->bind( gph );		// bind other bindables
	gph.drawIndexed( m_pDrawable->getIndicesCount() );
	DXGI_GET_QUEUE_INFO( gph );
}


}//ren