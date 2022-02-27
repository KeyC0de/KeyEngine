#pragma once

#include <ittnotify.h>
#pragma comment( lib, "libittnotify.lib" )

__itt_domain* ittDomain = __itt_domain_create( L"KeyEngine.Domain.Global" );

// ITT task begin
#define VTUNE_ITT_TASK_BEGIN( strHandle )			__itt_task_begin( ittDomain,\
	__itt_null,\
	__itt_null,\
	strHandle );

// Tasks can be nested to your heart's content.

// ITT task end
#define VTUNE_ITT_TASK_END( strHandle )				__itt_task_end( ittDomain,\
	__itt_null,\
	__itt_null,\
	strHandle );

// Set a thread's name so it shows up in the UI as something meaningful
#define VTUNE_ITT_SET_THREAD_NAME( threadName )		__itt_thread_set_name( threadName );

// ITT VTUNE Pause Profiler
#define VTUNE_ITT_PAUSE								__itt_pause()

// ITT VTUNE Resume Profiler
#define VTUNE_ITT_RESUME							__itt_resume()