#include "reporter_access.h"
#include "reporter_listener_events.h"


ReporterAccess::ReporterAccess()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
ReportingNexus::ReportingNexus( ReporterAccess &access )
	:
	IReporter<SwapChainResized>{access}
	//, IReporter<OtherEventType>{access}
{
	
}

ReportingNexus::~ReportingNexus() noexcept
{
	IReporter<SwapChainResized>::removeThisFromListenersList();
}

ReportingNexus& ReportingNexus::getInstance()
{
	static ReportingNexus instance{s_accessKey};
	return instance;
}

ReporterAccess& ReportingNexus::getReporterAccess()
{
	return s_accessKey;
}
