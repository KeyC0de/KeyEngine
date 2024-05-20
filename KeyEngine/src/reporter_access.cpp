#include "reporter_access.h"
#include "reporter_listener_events.h"


ReporterAccess::ReporterAccess()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
ReportingNexus::ReportingNexus( ReporterAccess &access )
	:
	IReporter<SwapChainResizedEvent>{access},
	IReporter<UISoundEvent>{access},
	IReporter<UserPropertyChanged>{access},
	IReporter<UiMsg>{access}
{
	
}

ReportingNexus::~ReportingNexus() noexcept
{
	IReporter<SwapChainResizedEvent>::removeThisFromListenersList();
	IReporter<UISoundEvent>::removeThisFromListenersList();
	IReporter<UserPropertyChanged>::removeThisFromListenersList();
	IReporter<UiMsg>::removeThisFromListenersList();
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
