#pragma once

#include "winner.h"


//=============================================================
//	\class	SystemMetric
//	\author	KeyC0de
//	\date	2022/10/01 16:59
//	\brief	use GetSystemMetrics( index )
//=============================================================
struct SystemMetric
{
	int m_index;
	const char *m_label;
	const char *m_description;
};


#if defined _DEBUG && !defined NDEBUG
void windowsMetricsCheckTest();
void windowsMetricsCheck( int sm );
void windowsMetricsCheckAll();
#endif