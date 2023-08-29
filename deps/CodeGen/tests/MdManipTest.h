#pragma once

#include "Test.h"
#include "Align16.h"

class CMdManipTest : public CTest
{
public:
	void				Compile(Jitter::CJitter&) override;
	void				Run() override;

private:
	struct CONTEXT
	{
		ALIGN16

		float			src0[4];
		float			src1[4];
		float			src2[4];

		float			dstMasked[4];

		float			dstExpandRel[4];
		float			dstExpandCst[4];
	};

	FunctionType		m_function;
};
