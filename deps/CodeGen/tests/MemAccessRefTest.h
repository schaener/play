#pragma once

#include "Test.h"

class CMemAccessRefTest : public CTest
{
public:
	void    Run() override;
	void    Compile(Jitter::CJitter&) override;

private:
	void    EmitNullTest(Jitter::CJitter&, uint32, size_t);
	void    EmitNullComparison(Jitter::CJitter&, uint32, size_t);
	void    EmitNotNullComparison(Jitter::CJitter&, uint32, size_t);
	
	enum
	{
		MEMORY_SIZE = 0x20,
	};

	struct CONTEXT
	{
		void*     memory;
		uint32    readIdx;
		uint32    readValue;
		uint32    readValueResult;
		uint32    readValueCstIdxResult;
		uint32    readValueVarIdxResult;
		uint32    nullCheck0;
		uint32    nullCheck1;
		uint32    nullCheck2;
		uint32    nullCheck3;
		uint32    nullCheck4;
		uint32    nullCheck5;
	};

	CONTEXT            m_context;
	void*              m_memory[MEMORY_SIZE];
	FunctionType       m_function;
};
