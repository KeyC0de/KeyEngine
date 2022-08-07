#pragma once

#include "winner.h"


struct Metrics
{
	int index;
	const char *label;
	const char *description;
};


void windowsMetricsCheckTest();
void windowsMetricsCheck( int sm );
void windowsMetricsCheckAll();