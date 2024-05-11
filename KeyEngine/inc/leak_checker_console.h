#pragma once

#if defined _DEBUG && !defined NDEBUG


namespace debugLeak
{

bool anyMemoryLeaks();

class LeakChecker final
{
public:
	LeakChecker();
	~LeakChecker() noexcept;
};


};// namespace debugLeak


#endif