#pragma once

#include "Types.h"
#include "Convertible.h"
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipArchiveReader.h"

namespace Iop
{
	class CIntc;

	class CRootCounters
	{
	public:
		struct MODE : public convertible<uint32>
		{
			unsigned int en : 1;
			unsigned int unused0 : 2;
			unsigned int tar : 1;
			unsigned int iq1 : 1;
			unsigned int unused1 : 1;
			unsigned int iq2 : 1;
			unsigned int unused2 : 1;
			unsigned int clc : 1;
			unsigned int div : 2; //Assuming it's two bits here, but not sure about that
			unsigned int unused3 : 21;
		};
		static_assert(sizeof(MODE) == sizeof(uint32), "MODE structure size is too small");

		CRootCounters(unsigned int, Iop::CIntc&);
		virtual ~CRootCounters() = default;

		void Reset();

		void LoadState(Framework::CZipArchiveReader&);
		void SaveState(Framework::CZipArchiveWriter&);

		void Update(unsigned int);

		uint32 ReadRegister(uint32);
		uint32 WriteRegister(uint32, uint32);

		enum
		{
			ADDR_BEGIN1 = 0x1F801100,
			ADDR_END1 = 0x1F80112F,

			ADDR_BEGIN2 = 0x1F801480,
			ADDR_END2 = 0x1F8014AF,
		};

		enum
		{
			CNT0_BASE = 0x1F801100,
			CNT1_BASE = 0x1F801110,
			CNT2_BASE = 0x1F801120,

			CNT3_BASE = 0x1F801480,
			CNT4_BASE = 0x1F801490,
			CNT5_BASE = 0x1F8014A0,
		};

		enum
		{
			CNT_COUNT = 0,
			CNT_MODE = 4,
			CNT_TARGET = 8,
		};

		enum
		{
			MAX_COUNTERS = 6,
		};

		enum COUNTER_SOURCE
		{
			COUNTER_SOURCE_SYSCLOCK = 1,
			COUNTER_SOURCE_PIXEL = 2,
			COUNTER_SOURCE_HLINE = 4,
			COUNTER_SOURCE_HOLD = 8
		};

		enum COUNTER_SCALE
		{
			COUNTER_SCALE_1 = 0,
			COUNTER_SCALE_8 = 1,
			COUNTER_SCALE_16 = 2,
			COUNTER_SCALE_256 = 3
		};

		static const uint32 g_counterInterruptLines[MAX_COUNTERS];
		static const uint32 g_counterBaseAddresses[MAX_COUNTERS];
		static const uint32 g_counterSources[MAX_COUNTERS];
		static const uint32 g_counterSizes[MAX_COUNTERS];
		static const uint32 g_counterMaxScales[MAX_COUNTERS];

	private:
		struct COUNTER
		{
			uint32 count;
			MODE mode;
			uint32 target;
			uint32 clockRemain;
		};

		void DisassembleRead(uint32);
		void DisassembleWrite(uint32, uint32);

		static unsigned int GetCounterIdByAddress(uint32);

		COUNTER m_counter[MAX_COUNTERS];
		unsigned int m_hsyncClocks;
		unsigned int m_pixelClocks;
		Iop::CIntc& m_intc;
	};
}
