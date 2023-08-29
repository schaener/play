#pragma once

#include "Test.h"

class CAlu64Test : public CTest
{
public:
	void				Run() override;
	void				Compile(Jitter::CJitter&) override;

private:
	struct CONTEXT
	{
		uint64			value0;
		uint64			value1;
		uint64			value2;
		uint64			value3;

		uint64			resultAdd0;
		uint64			resultAdd1;
		uint64			resultAddCst;
		uint64			resultSub0;
		uint64			resultSub1;
		uint64			resultSubCst0;
		uint64			resultSubCst1;
	};

	CONTEXT				m_context;
	FunctionType		m_function;
};
