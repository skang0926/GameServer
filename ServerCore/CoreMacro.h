#pragma once

/*----------------
	   Crash
-----------------*/

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);    \
	*crash = 0xDEADBEFF;					\
}											\

#define ASSERT_CRASH(expr)					\
{											\
	if (!expr))								\
		{									\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
		}									\
}											\