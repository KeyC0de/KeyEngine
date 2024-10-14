#include "world.h"
//#include "thread_poolj.h"


void World::update( const float dt )
{

	/*
	ThreadPoolJ &threadPool = ThreadPoolJ::instance( 4u, true );
	threadPool.enqueue( &func_async::doPeriodically, &BindableRegistry::garbageCollect, 5000u, false );

	threadPool.enqueue( &func_async::doLater,
		[this]() -> void
		{
			this->m_testSphere.setScale( 4.0f );
		},
		4000u );

	threadPool.enqueue( &func_async::doLater,
		[this]() -> void
		{
			this->m_testSphere.setScale( 0.25f );
		},
		8000u );
	*/
}

void World::render( const float frameInterpolation )
{
}