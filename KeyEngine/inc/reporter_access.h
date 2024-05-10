#pragma once

#include "reporter_listener.h"


/////////
struct SwapChainResized;
/////////

class ReportingNexus;

class ReporterAccess
{
	friend class ReportingNexus;
public:
	ReporterAccess();
};

class ReportingNexus
	: public IReporter<SwapChainResized>
	//, public IReporter<OtherEventType>
{
	static inline ReporterAccess s_accessKey{};
private:
	ReportingNexus( ReporterAccess &access );
public:
	~ReportingNexus() noexcept;

	static ReportingNexus& getInstance();
	static ReporterAccess& getReporterAccess();
};