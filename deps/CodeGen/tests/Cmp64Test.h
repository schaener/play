#pragma once

#include "Test.h"

class CCmp64Test : public CTest
{
public:
						CCmp64Test(bool, bool, uint64, uint64);

	void				Run() override;
	void				Compile(Jitter::CJitter&) override;

private:
	struct CONTEXT
	{
		uint64			value0;
		uint64			value1;

		uint32			resultEq;
		uint32			resultNe;
		uint32			resultBl;
		uint32			resultLt;
		uint32			resultLe;
		uint32			resultAb;
		uint32			resultGt;
	};

	bool				m_useConstant0 = false;
	bool				m_useConstant1 = false;
	uint64				m_value0 = 0;
	uint64				m_value1 = 0;
	CONTEXT				m_context;
	FunctionType		m_function;
};
