#pragma once

#include <random>
#include <limits>


class KeyRandom final
{
	static inline std::mt19937 m_device;
	static inline bool m_started = false;
	static inline std::uniform_int_distribution<std::mt19937::result_type> m_distr;
public:
	static void init();
	explicit operator bool() const noexcept;
	// 0-1 float
	static float getRandomFloat();
	// 0 - 1 double
	static double getRandomDouble();
	//===================================================
	//	\function	getRandomInRange
	//	\brief  returns random float from 0 to 1 by default
	//			start = specify lower limit on range of output values
	//			end = specify upper limit on range of output
	//	\date	2020/12/18 1:49
	static float getRandomFloatInRange( int start = 0, int end = 1 );
	static double getRandomDoubleInRange( int start = 0, int end = 1 );
};