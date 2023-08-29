#include <cstring>
#include <stdio.h>
#include "../Log.h"
#include "../states/RegisterStateFile.h"
#include "Timer.h"
#include "Ps2Const.h"
#include "gs/GSHandler.h"

#define LOG_NAME ("ee_timer")

#define STATE_REGS_XML ("timer/regs.xml")

CTimer::CTimer(CINTC& intc, CGSHandler*& gs)
    : m_intc(intc)
    , m_gs(gs)
{
	Reset();
}

void CTimer::Reset()
{
	memset(m_timer, 0, sizeof(TIMER) * 4);
}

void CTimer::Count(unsigned int ticks)
{
	for(unsigned int i = 0; i < MAX_TIMER; i++)
	{
		auto& timer = m_timer[i];

		if(!(timer.nMODE & MODE_COUNT_ENABLE)) continue;

		uint32 previousCount = timer.nCOUNT;
		uint32 nextCount = timer.nCOUNT;

		uint32 divider = 1;
		//BUSCLOCK runs at half EE frequency
		switch(timer.nMODE & MODE_CLOCK_SELECT)
		{
		case MODE_CLOCK_SELECT_BUSCLOCK:
			divider = 1 * 2;
			break;
		case MODE_CLOCK_SELECT_BUSCLOCK16:
			divider = 16 * 2;
			break;
		case MODE_CLOCK_SELECT_BUSCLOCK256:
			divider = 256 * 2;
			break;
		case MODE_CLOCK_SELECT_EXTERNAL:
		{
			assert(m_gs);
			uint32 hSyncFreq = m_gs->GetCrtHSyncFrequency();
			divider = PS2::EE_CLOCK_FREQ / hSyncFreq;
		}
		break;
		}

		//Compute increment
		uint32 totalTicks = timer.clockRemain + ticks;
		uint32 countAdd = totalTicks / divider;
		timer.clockRemain = totalTicks % divider;
		nextCount = previousCount + countAdd;

		uint32 compare = (timer.nCOMP == 0) ? 0x10000 : timer.nCOMP;
		uint32 newFlags = 0;

		//Check if it hit the reference value
		if((previousCount < compare) && (nextCount >= compare))
		{
			newFlags |= MODE_EQUAL_FLAG;
			if(timer.nMODE & MODE_ZERO_RETURN)
			{
				timer.nCOUNT = nextCount - compare;
			}
			else
			{
				timer.nCOUNT = nextCount;
			}
		}
		else
		{
			timer.nCOUNT = nextCount;
		}

		if(timer.nCOUNT >= 0x10000)
		{
			newFlags |= MODE_OVERFLOW_FLAG;
			timer.nCOUNT &= 0xFFFF;
		}
		timer.nMODE |= newFlags;

		uint32 nMask = (timer.nMODE & 0x300) << 2;
		bool interruptPending = (newFlags & nMask) != 0;
		if(interruptPending)
		{
			m_intc.AssertLine(CINTC::INTC_LINE_TIMER0 + i);
		}
	}
}

uint32 CTimer::GetRegister(uint32 nAddress)
{
	DisassembleGet(nAddress);

	unsigned int nTimerId = (nAddress >> 11) & 0x3;

	switch(nAddress & 0x7FF)
	{
	case 0x00:
		return m_timer[nTimerId].nCOUNT & 0xFFFF;
		break;
	case 0x04:
	case 0x08:
	case 0x0C:
		break;

	case 0x10:
		return m_timer[nTimerId].nMODE;
		break;
	case 0x14:
	case 0x18:
	case 0x1C:
		break;

	case 0x20:
		return m_timer[nTimerId].nCOMP;
		break;
	case 0x24:
	case 0x28:
	case 0x2C:
		break;

	case 0x30:
		return m_timer[nTimerId].nHOLD;
		break;
	case 0x34:
	case 0x38:
	case 0x3C:
		break;

	default:
		CLog::GetInstance().Warn(LOG_NAME, "Read an unhandled IO port (0x%08X).\r\n", nAddress);
		break;
	}

	return 0;
}

void CTimer::SetRegister(uint32 nAddress, uint32 nValue)
{
	DisassembleSet(nAddress, nValue);

	unsigned int nTimerId = (nAddress >> 11) & 0x3;

	switch(nAddress & 0x7FF)
	{
	case 0x00:
		m_timer[nTimerId].nCOUNT = nValue & 0xFFFF;
		break;
	case 0x04:
	case 0x08:
	case 0x0C:
		break;

	case 0x10:
		m_timer[nTimerId].nMODE &= ~(nValue & 0xC00);
		m_timer[nTimerId].nMODE &= 0xC00;
		m_timer[nTimerId].nMODE |= nValue & ~0xC00;
		break;
	case 0x14:
	case 0x18:
	case 0x1C:
		break;

	case 0x20:
		m_timer[nTimerId].nCOMP = nValue & 0xFFFF;
		break;
	case 0x24:
	case 0x28:
	case 0x2C:
		break;

	case 0x30:
		m_timer[nTimerId].nHOLD = nValue & 0xFFFF;
		break;
	case 0x34:
	case 0x38:
	case 0x3C:
		break;

	default:
		CLog::GetInstance().Warn(LOG_NAME, "Wrote to an unhandled IO port (0x%08X, 0x%08X).\r\n", nAddress, nValue);
		break;
	}
}

void CTimer::DisassembleGet(uint32 nAddress)
{
	unsigned int nTimerId = (nAddress >> 11) & 0x3;

	switch(nAddress & 0x7FF)
	{
	case 0x00:
		CLog::GetInstance().Print(LOG_NAME, "= T%i_COUNT\r\n", nTimerId);
		break;

	case 0x10:
		CLog::GetInstance().Print(LOG_NAME, "= T%i_MODE\r\n", nTimerId);
		break;

	case 0x20:
		CLog::GetInstance().Print(LOG_NAME, "= T%i_COMP\r\n", nTimerId);
		break;

	case 0x30:
		CLog::GetInstance().Print(LOG_NAME, "= T%i_HOLD\r\n", nTimerId);
		break;
	}
}

void CTimer::DisassembleSet(uint32 nAddress, uint32 nValue)
{
	unsigned int nTimerId = (nAddress >> 11) & 0x3;

	switch(nAddress & 0x7FF)
	{
	case 0x00:
		CLog::GetInstance().Print(LOG_NAME, "T%i_COUNT = 0x%08X\r\n", nTimerId, nValue);
		break;

	case 0x10:
		CLog::GetInstance().Print(LOG_NAME, "T%i_MODE = 0x%08X\r\n", nTimerId, nValue);
		break;

	case 0x20:
		CLog::GetInstance().Print(LOG_NAME, "T%i_COMP = 0x%08X\r\n", nTimerId, nValue);
		break;

	case 0x30:
		CLog::GetInstance().Print(LOG_NAME, "T%i_HOLD = 0x%08X\r\n", nTimerId, nValue);
		break;
	}
}

void CTimer::LoadState(Framework::CZipArchiveReader& archive)
{
	CRegisterStateFile registerFile(*archive.BeginReadFile(STATE_REGS_XML));
	for(unsigned int i = 0; i < MAX_TIMER; i++)
	{
		auto& timer = m_timer[i];
		std::string timerPrefix = "TIMER" + std::to_string(i) + "_";
		timer.nCOUNT = registerFile.GetRegister32((timerPrefix + "COUNT").c_str());
		timer.nMODE = registerFile.GetRegister32((timerPrefix + "MODE").c_str());
		timer.nCOMP = registerFile.GetRegister32((timerPrefix + "COMP").c_str());
		timer.nHOLD = registerFile.GetRegister32((timerPrefix + "HOLD").c_str());
		timer.clockRemain = registerFile.GetRegister32((timerPrefix + "REM").c_str());
	}
}

void CTimer::SaveState(Framework::CZipArchiveWriter& archive)
{
	auto registerFile = std::make_unique<CRegisterStateFile>(STATE_REGS_XML);
	for(unsigned int i = 0; i < MAX_TIMER; i++)
	{
		const auto& timer = m_timer[i];
		std::string timerPrefix = "TIMER" + std::to_string(i) + "_";
		registerFile->SetRegister32((timerPrefix + "COUNT").c_str(), timer.nCOUNT);
		registerFile->SetRegister32((timerPrefix + "MODE").c_str(), timer.nMODE);
		registerFile->SetRegister32((timerPrefix + "COMP").c_str(), timer.nCOMP);
		registerFile->SetRegister32((timerPrefix + "HOLD").c_str(), timer.nHOLD);
		registerFile->SetRegister32((timerPrefix + "REM").c_str(), timer.clockRemain);
	}
	archive.InsertFile(std::move(registerFile));
}

void CTimer::NotifyVBlankStart()
{
	ProcessGateEdgeChange(MODE_GATE_SELECT_VBLANK, MODE_GATE_MODE_HIGHEDGE);
}

void CTimer::NotifyVBlankEnd()
{
	ProcessGateEdgeChange(MODE_GATE_SELECT_VBLANK, MODE_GATE_MODE_LOWEDGE);
}

void CTimer::ProcessGateEdgeChange(uint32 gate, uint32 edgeMode)
{
	for(unsigned int i = 0; i < MAX_TIMER; i++)
	{
		auto& timer = m_timer[i];
		if(!(timer.nMODE & MODE_COUNT_ENABLE)) continue;
		if(!(timer.nMODE & MODE_GATE_ENABLE)) continue;

		if((timer.nMODE & MODE_GATE_SELECT) != gate) continue;

		uint32 gateMode = (timer.nMODE & MODE_GATE_MODE);
		if((edgeMode & gateMode) == edgeMode)
		{
			timer.nCOUNT = 0;
			timer.clockRemain = 0;
		}
	}
}
