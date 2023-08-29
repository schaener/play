#pragma once

#include "../MIPS.h"
#include "../MA_MIPSIV.h"
#include "../COP_SCU.h"
#include "Iop_BiosBase.h"
#include "Iop_Dev9.h"
#include "Iop_Dmac.h"
#include "Iop_Intc.h"
#include "Iop_Ilink.h"
#include "Iop_RootCounters.h"
#include "Iop_Speed.h"
#include "Iop_SpuBase.h"
#include "Iop_Spu.h"
#include "Iop_Spu2.h"
#include "Iop_Sio2.h"
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipArchiveReader.h"

namespace Iop
{
	class CSubSystem
	{
	public:
		CSubSystem(bool ps2Mode);
		virtual ~CSubSystem();

		void Reset();
		int ExecuteCpu(int);
		bool IsCpuIdle();
		void CountTicks(int);

		void NotifyVBlankStart();
		void NotifyVBlankEnd();

		void SaveState(Framework::CZipArchiveWriter&);
		void LoadState(Framework::CZipArchiveReader&);

		CMIPS m_cpu;
		CMA_MIPSIV m_cpuArch;
		CCOP_SCU m_copScu;
		uint8* m_ram;
		uint8* m_scratchPad;
		uint8* m_spuRam;
		CIntc m_intc;
		CDmac m_dmac;
		CRootCounters m_counters;
		CSpuSampleCache m_spuSampleCache;
		CSpuBase m_spuCore0;
		CSpuBase m_spuCore1;
		CSpu m_spu;
		CSpu2 m_spu2;
		CDev9 m_dev9;
#ifdef _IOP_EMULATE_MODULES
		CSio2 m_sio2;
#endif
		CSpeed m_speed;
		CIlink m_ilink;
		BiosBasePtr m_bios;

	private:
		enum
		{
			SPEED_REG_BEGIN = 0x10000000,
			SPEED_REG_END = 0x1001FFFF,
			HW_REG_BEGIN = 0x1F801000,
			HW_REG_END = 0x1F9FFFFF
		};

		void SetupPageTable();

		uint32 ReadIoRegister(uint32);
		uint32 WriteIoRegister(uint32, uint32);

		void CheckPendingInterrupts();

		int m_dmaUpdateTicks = 0;
		int m_spuIrqUpdateTicks = 0;
	};
}
