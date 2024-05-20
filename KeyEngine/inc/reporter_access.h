#pragma once

#include "reporter_listener.h"


/////////
struct SwapChainResizedEvent;
struct UISoundEvent;
struct UserPropertyChanged;
struct UiMsg;
/////////

class ReportingNexus;

class ReporterAccess
{
	friend class ReportingNexus;
public:
	ReporterAccess();
};

class ReportingNexus
	: public IReporter<SwapChainResizedEvent>
	, public IReporter<UISoundEvent>
	, public IReporter<UserPropertyChanged>
	, public IReporter<UiMsg>
{
	static inline ReporterAccess s_accessKey{};
private:
	ReportingNexus( ReporterAccess &access );
public:
	~ReportingNexus() noexcept;

	static ReportingNexus& getInstance();
	static ReporterAccess& getReporterAccess();
};