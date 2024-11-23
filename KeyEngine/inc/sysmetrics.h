#pragma once

#if defined _DEBUG && !defined NDEBUG
#	include "winner.h"


///=============================================================
/// \class	SystemMetric
/// \author	KeyC0de
/// \date	2022/10/01 16:59
/// \brief	use GetSystemMetrics( index )
///=============================================================
struct SystemMetric
{
	int m_index;
	const char *m_label;
	const char *m_description;
};

void checkWindowsMetricsTest();
void checkWindowsMetric( const int metricIndex );
void checkAllWindowsMetrics();
#endif