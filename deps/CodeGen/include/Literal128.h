#pragma once

#include "Types.h"

struct LITERAL128
{
	LITERAL128(uint32 w0, uint32 w1, uint32 w2, uint32 w3)
		: w0(w0), w1(w1), w2(w2), w3(w3)
	{

	}

	LITERAL128(uint64 lo, uint64 hi)
		: lo(lo), hi(hi)
	{
	
	}

	union
	{
		struct
		{
			uint32 w0;
			uint32 w1;
			uint32 w2;
			uint32 w3;
		};
		struct
		{
			uint64 lo;
			uint64 hi;
		};
	};

	bool operator < (const LITERAL128& rhs) const
	{
		if(hi == rhs.hi) return lo < rhs.lo;
		return (hi < rhs.hi);
	}

	bool operator == (const LITERAL128& rhs) const
	{
		return (hi == rhs.hi) && (lo == rhs.lo);
	}
};
