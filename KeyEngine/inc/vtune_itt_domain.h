#pragma once

#ifdef _PROFILE

// Intel Instrumentation & Tracing Technology
#	include <ittnotify.h>
#	pragma comment( lib, "libittnotify.lib" )

__itt_domain *ittDomain = __itt_domain_create( L"KeyEngine.Domain.Global" );

// ITT task begin
#	define PROFILE_VTUNE_ITT_TASK_BEGIN( strHandle )			__itt_task_begin( ittDomain, __itt_null, __itt_null, strHandle );

// Tasks can be nested to your heart's content.

// ITT task end
#	define PROFILE_VTUNE_ITT_TASK_END							__itt_task_end( ittDomain );

// Set a thread's name so it shows up in the UI as something meaningful
#	define PROFILE_VTUNE_ITT_SET_THREAD_NAME( threadName )		__itt_thread_set_name( threadName );

// ITT VTUNE Pause Profiler
#	define PROFILE_VTUNE_ITT_PAUSE								__itt_pause()

// ITT VTUNE Resume Profiler
#	define PROFILE_VTUNE_ITT_RESUME							__itt_resume()

#else

#	define PROFILE_VTUNE_ITT_TASK_BEGIN( strHandle )			(void) 0;
#	define PROFILE_VTUNE_ITT_TASK_END							(void) 0;
#	define PROFILE_VTUNE_ITT_SET_THREAD_NAME( threadName )		(void) 0;
#	define PROFILE_VTUNE_ITT_PAUSE								(void) 0;
#	define PROFILE_VTUNE_ITT_RESUME								(void) 0;

#endif