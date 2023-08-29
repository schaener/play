#pragma once

#include "Test.h"

class CRandomAluTest : public CTest
{
public:
						CRandomAluTest(bool);

	void				Compile(Jitter::CJitter&) override;
	void				Run() override;

private:
	struct CONTEXT
	{
		uint32 number;
	};

	bool				m_useConstant;
	CONTEXT				m_context;
	FunctionType		m_function;
};
