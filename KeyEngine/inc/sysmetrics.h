#pragma once

#include "winner.h"


struct Metrics
{
	int index;
	const char *label;
	const char *description;
};


#if defined _DEBUG && !defined NDEBUG
void windowsMetricsCheckTest();
void windowsMetricsCheck( int sm );
void windowsMetricsCheckAll();
#endif