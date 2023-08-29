#pragma once

#include "Types.h"
#include "BasicUnion.h"
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipArchiveReader.h"

namespace Iop
{
	class CIntc
	{
	public:
		enum LINES
		{
			LINE_VBLANK = 0x00,
			LINE_SBUS = 0x01,
			LINE_CDROM = 0x02,
			LINE_DMAC = 0x03,
			LINE_RTC0 = 0x04,
			LINE_RTC1 = 0x05,
			LINE_RTC2 = 0x06,
			LINE_SPU2 = 0x09,
			LINE_EVBLANK = 0x0B,
			LINE_DEV9 = 0x0D,
			LINE_RTC3 = 0x0E,
			LINE_RTC4 = 0x0F,
			LINE_RTC5 = 0x10,
			LINE_SIO2 = 0x11,
			LINE_USB = 0x16,
			LINE_ILINK = 0x18,
			LINE_DMA_BASE = 0x20,
			LINE_DMA_SIF2 = 0x22,
			LINE_DMA_SPU0 = 0x24,
			LINE_DMA_SPU1 = 0x28,
			LINE_DMA_DEV9 = 0x29,
			LINE_DMA_SIF0 = 0x2A,
			LINE_DMA_SIF1 = 0x2B,
			LINE_DMA_SIO2in = 0x2C,
			LINE_DMA_SIO2out = 0x2D,
			LINES_MAX
		};

		enum
		{
			ADDR_BEGIN = 0x1F801070,
			ADDR_END = 0x1F80107F
		};

		enum
		{
			STATUS0 = 0x1F801070,
			MASK0 = 0x1F801074,
			STATUS1 = 0x1F801078,
			MASK1 = 0x1F80107C,
		};

		virtual ~CIntc() = default;

		void Reset();

		void LoadState(Framework::CZipArchiveReader&);
		void SaveState(Framework::CZipArchiveWriter&);

		uint32 ReadRegister(uint32);
		uint32 WriteRegister(uint32, uint32);

		void AssertLine(unsigned int);
		void ClearLine(unsigned int);
		void SetStatus(uint64);
		void SetMask(uint64);
		bool HasPendingInterrupt();

	private:
		UNION64_32 m_status = 0;
		UNION64_32 m_mask = 0;
	};
}
