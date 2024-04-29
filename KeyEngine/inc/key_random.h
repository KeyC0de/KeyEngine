#pragma once

#include <random>
#include <limits>


class KeyRandom final
{
	std::mt19937 m_device;
	bool m_bStarted = false;
	std::uniform_int_distribution<std::mt19937::result_type> m_distr;
public:
	KeyRandom( bool started = true );

	void start() noexcept;
	explicit operator bool() noexcept;
	// like c rand() function
	int rand();
	// 0 - 1 int
	int getRandomInt();
	// 0.0f - 1.0f float
	float getRandomFloat();
	// 0.0 - 1.0 double
	double getRandomDouble();
	int getRandomIntInRange( const int start, const int end );
	//===================================================
	//	\function	getRandomInRange
	//	\brief  returns random float from 0 to 1 by default
	//			start = specify lower limit on range of output values
	//			end = specify upper limit on range of output
	//	\date	2020/12/18 1:49
	float getRandomFloatInRange( const int start = 0, const int end = 1 );
	double getRandomDoubleInRange( const int start = 0, const int end = 1 );
};