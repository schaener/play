#pragma once

#include "Types.h"
#include "MA_ALLEGREX.h"
#include "COP_SCU.h"
#include "COP_FPU.h"
#include "MIPS.h"
#include "../PsfVmSubSystem.h"
#include "Psp_PsfBios.h"
#include "MemStream.h"

namespace Psp
{
	class CPsfSubSystem : public CPsfVmSubSystem
	{
	public:
		enum
		{
			DEFAULT_RAMSIZE = 0x02000000,
		};

		CPsfSubSystem(uint32 = DEFAULT_RAMSIZE);
		virtual ~CPsfSubSystem();

		void Reset() override;
		CMIPS& GetCpu() override;
		uint8* GetRam() override;
		Iop::CSpuBase& GetSpuCore(unsigned int) override;

#ifdef DEBUGGER_INCLUDED
		bool MustBreak() override;
		void DisableBreakpointsOnce() override;
		CBiosDebugInfoProvider* GetBiosDebugInfoProvider() override;
		void LoadDebugTags(Framework::Xml::CNode*) override;
		void SaveDebugTags(Framework::Xml::CNode*) override;
#endif

		void Update(bool, CSoundHandler*) override;

		Psp::CPsfBios& GetBios();

	private:
		enum SPURAMSIZE
		{
			SPURAMSIZE = 0x400000,
		};

		int ExecuteCpu(bool);

		CMIPS m_cpu;
		CMA_ALLEGREX m_cpuArch;
		CCOP_SCU m_copScu;
		CCOP_FPU m_copFpu;
		uint8* m_ram;
		uint32 m_ramSize;
		uint8* m_spuRam;
		Iop::CSpuSampleCache m_spuSampleCache;
		Iop::CSpuBase m_spuCore0;
		Iop::CSpuBase m_spuCore1;
		Framework::CMemStream m_audioStream;
		int m_samplesToFrame;

		Psp::CPsfBios m_bios;
	};

	typedef std::shared_ptr<CPsfSubSystem> PsfSubSystemPtr;
}
