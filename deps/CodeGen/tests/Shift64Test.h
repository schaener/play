#pragma once

#include "Test.h"

class CShift64Test : public CTest
{
public:
						CShift64Test(uint32);
			
	void				Run() override;
	void				Compile(Jitter::CJitter&) override;

private:
	struct CONTEXT
	{
		uint64			value0;
		uint64			value1;

		uint32			shiftAmount;

		uint64			resultSra0;
		uint64			resultSra1;

		uint64			resultSrl0;
		uint64			resultSrl1;

		uint64			resultShl0;
		uint64			resultShl1;

		uint64			resultSraVar0;
		uint64			resultSraVar1;

		uint64			resultSrlVar0;
		uint64			resultSrlVar1;

		uint64			resultShlVar0;
		uint64			resultShlVar1;
	};

	CONTEXT				m_context;
	FunctionType		m_function;
	uint32				m_shiftAmount = 0;
};
