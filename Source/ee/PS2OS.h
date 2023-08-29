#pragma once

#include <string>
#include <memory>
#include "filesystem_def.h"
#include "signal/Signal.h"
#include "../ELF.h"
#include "../MIPS.h"
#include "../BiosDebugInfoProvider.h"
#include "../OsStructManager.h"
#include "../OsVariableWrapper.h"
#include "../OsStructQueue.h"
#include "../gs/GSHandler.h"
#include "SIF.h"
#include "Ee_IdleEvaluator.h"
#include "Ee_LibMc2.h"

#define INTERRUPTS_ENABLED_MASK (CMIPS::STATUS_IE | CMIPS::STATUS_EIE)

class CIopBios;

class CPS2OS : public CBiosDebugInfoProvider
{
public:
	typedef std::vector<std::string> ArgumentList;

	typedef Framework::CSignal<void(const char*, const ArgumentList&)> RequestLoadExecutableEvent;

	CPS2OS(CMIPS&, uint8*, uint8*, uint8*, CGSHandler*&, CSIF&, CIopBios&);
	virtual ~CPS2OS();

	void Initialize(uint32);
	void Release();

	bool IsIdle() const;

	void BootFromFile(const fs::path&);
	void BootFromVirtualPath(const char*, const ArgumentList&);
	void BootFromCDROM();
	CELF32* GetELF();
	const char* GetExecutableName() const;
	std::pair<uint32, uint32> GetExecutableRange() const;
	uint32 LoadExecutable(const char*, const char*);

	Ee::CLibMc2& GetLibMc2();

	void HandleInterrupt(int32);
	void HandleSyscall();
	void HandleReturnFromException();
	void HandleTLBException();
	bool CheckVBlankFlag();

	uint32 SuspendCurrentThread();
	void ResumeThread(uint32);

	uint8* GetStructPtr(uint32) const;

	void UpdateTLBEnabledState();

	static uint32 TranslateAddress(CMIPS*, uint32);
	static uint32 TranslateAddressTLB(CMIPS*, uint32);
	static uint32 CheckTLBExceptions(CMIPS*, uint32, uint32);

#ifdef DEBUGGER_INCLUDED
	BiosDebugModuleInfoArray GetModulesDebugInfo() const override;
	BiosDebugObjectInfoMap GetBiosObjectsDebugInfo() const override;
	BiosDebugObjectArray GetBiosObjects(uint32) const override;
#endif

	Framework::CSignal<void()> OnExecutableChange;
	Framework::CSignal<void()> OnExecutableUnloading;
	Framework::CSignal<void()> OnRequestInstructionCacheFlush;
	RequestLoadExecutableEvent OnRequestLoadExecutable;
	Framework::CSignal<void()> OnRequestExit;
	Framework::CSignal<void()> OnCrtModeChange;

private:
	struct SEMAPHOREPARAM
	{
		uint32 count;
		uint32 maxCount;
		uint32 initCount;
		uint32 waitThreads;
		uint32 attributes;
		uint32 option;
	};

	struct THREADPARAM
	{
		uint32 status;
		uint32 threadProc;
		uint32 stackBase;
		uint32 stackSize;
		uint32 gp;
		uint32 initPriority;
		uint32 currPriority;
		uint32 attr;
		uint32 option;
	};

	struct THREADSTATUS : public THREADPARAM
	{
		uint32 waitType;
		uint32 waitId;
		uint32 wakeupCount;
	};
	static_assert(sizeof(THREADSTATUS) == 0x30, "Thread status must be 48 bytes long.");

	struct SEMAPHORE
	{
		uint32 isValid;
		uint32 count;
		uint32 maxCount;
		uint32 waitCount;
		uint32 waitNextId;
		uint32 option;
	};

	struct THREAD
	{
		uint32 isValid;
		uint32 nextId;
		uint32 status;
		uint32 contextPtr;
		uint32 stackBase;
		uint32 heapBase;
		uint32 threadProc;
		uint32 epc;
		uint32 gp;
		uint32 initPriority;
		uint32 currPriority;
		uint32 semaWait;
		uint32 wakeUpCount;
		uint32 stackSize;
	};

	enum STACKRES
	{
		STACKRES = 0x2A0,
		STACK_FRAME_RESERVE_SIZE = 0x20
	};

	//Castlevania: CoD relies on the fact that the GPRs are stored
	//in order starting from R0 all the way up to RA because it read/writes
	//values directly in the thread context
	struct THREADCONTEXT
	{
		uint128 gpr[0x20];
		uint32 cop1[0x20];
		uint32 fcsr;
		uint32 cop1a;
		uint32 unknown[6];
	};
	static_assert(sizeof(THREADCONTEXT) == STACKRES, "Size of THREADCONTEXT must be STACKRES");

	struct DMACHANDLER
	{
		uint32 isValid;
		uint32 nextId;
		uint32 channel;
		uint32 address;
		uint32 arg;
		uint32 gp;
	};

	struct INTCHANDLER
	{
		uint32 isValid;
		uint32 nextId;
		uint32 cause;
		uint32 address;
		uint32 arg;
		uint32 gp;
	};

	struct DECI2HANDLER
	{
		uint32 valid;
		uint32 device;
		uint32 bufferAddr;
	};

	struct ALARM
	{
		uint32 isValid;
		uint32 delay;
		uint32 compare;
		uint32 callback;
		uint32 callbackParam;
		uint32 gp;
	};

	enum class OSD_VERSION
	{
		V1,
		V2,
		V2_EXT,
	};

	enum class OSD_LANGUAGE
	{
		JAPANESE,
		ENGLISH,
		FRENCH,
		SPANISH,
		GERMAN,
		ITALIAN,
		DUTCH,
		PORTUGUESE,
		RUSSIAN,
		KOREAN,
		CHINESE_TRADITIONAL,
		CHINESE_SIMPLIFIED
	};

	enum class OSD_SCREENTYPE
	{
		RATIO_4_3,
		FULLSCREEN,
		RATIO_16_9,
	};

	struct OSDCONFIGPARAM : public convertible<uint32>
	{
		uint32 spdifMode : 1;
		uint32 screenType : 2;
		uint32 videoOutput : 1;
		uint32 jpLanguage : 1;
		uint32 ps1drvConfig : 8;
		uint32 version : 3;
		uint32 language : 5;
		uint32 timezoneOffset : 11;
	};

#ifdef DEBUGGER_INCLUDED
	struct SYSCALL_NAME
	{
		uint32 id;
		const char* name;
	};
#endif

	enum SYSCALL_REGS
	{
		SC_RETURN = 2,
		SC_PARAM0 = 4,
		SC_PARAM1 = 5,
		SC_PARAM2 = 6,
		SC_PARAM3 = 7,
		SC_PARAM4 = 8
	};

	enum MAX
	{
		MAX_THREAD = 256,
		MAX_SEMAPHORE = 256,
		MAX_DMACHANDLER = 128,
		MAX_INTCHANDLER = 128,
		MAX_DECI2HANDLER = 32,
		MAX_ALARM = 4,
	};

	//TODO: Use "refer" status enum values
	enum THREAD_STATUS
	{
		THREAD_RUNNING = 0x01,
		THREAD_SLEEPING = 0x02,
		THREAD_WAITING = 0x03,
		THREAD_SUSPENDED = 0x04,
		THREAD_SUSPENDED_WAITING = 0x05,
		THREAD_SUSPENDED_SLEEPING = 0x06,
		THREAD_ZOMBIE = 0x07,
	};

	enum THREAD_STATUS_REFER
	{
		THS_RUN = 0x01,
		THS_READY = 0x02,
		THS_WAIT = 0x04,
		THS_SUSPEND = 0x08,
		THS_DORMANT = 0x10,
	};

	typedef COsStructManager<THREAD> ThreadList;
	typedef COsStructManager<SEMAPHORE> SemaphoreList;
	typedef COsStructManager<INTCHANDLER> IntcHandlerList;
	typedef COsStructManager<DMACHANDLER> DmacHandlerList;
	typedef COsStructManager<ALARM> AlarmList;

	typedef COsStructQueue<THREAD> ThreadQueue;
	typedef COsStructQueue<INTCHANDLER> IntcHandlerQueue;
	typedef COsStructQueue<DMACHANDLER> DmacHandlerQueue;

	typedef void (CPS2OS::*SystemCallHandler)();

	void LoadELF(Framework::CStream*, const char*, const ArgumentList&);

	void LoadExecutableInternal();
	void UnloadExecutable();

	void ApplyPatches();

	void DisassembleSysCall(uint8);
	std::string GetSysCallDescription(uint8);

	static SystemCallHandler m_sysCall[0x80];

	void AssembleCustomSyscallHandler();
	void AssembleInterruptHandler();
	void AssembleDmacHandler();
	void AssembleIntcHandler();
	void AssembleAlarmHandler();
	void AssembleThreadEpilog();
	void AssembleIdleThreadProc();

	uint32* GetCustomSyscallTable();

	void CreateIdleThread();
	void LinkThread(uint32);
	void UnlinkThread(uint32);
	void ThreadShakeAndBake();
	void ThreadSwitchContext(uint32);
	void ThreadSaveContext(THREAD*, bool);
	void ThreadLoadContext(THREAD*, bool);
	void ThreadReset(uint32);
	void CheckLivingThreads();

	void SemaLinkThread(uint32, uint32);
	void SemaUnlinkThread(uint32, uint32);
	void SemaReleaseSingleThread(uint32, bool);

	void AlarmUpdateCompare();

	std::pair<uint32, uint32> GetVsyncFlagPtrs() const;
	void SetVsyncFlagPtrs(uint32, uint32);

	uint32 GetNextAvailableDeci2HandlerId();
	DECI2HANDLER* GetDeci2Handler(uint32);

	//Various system calls
	void sc_GsSetCrt();
	void sc_Exit();
	void sc_LoadExecPS2();
	void sc_ExecPS2();
	void sc_SetVTLBRefillHandler();
	void sc_SetVCommonHandler();
	void sc_AddIntcHandler();
	void sc_RemoveIntcHandler();
	void sc_AddDmacHandler();
	void sc_RemoveDmacHandler();
	void sc_EnableIntc();
	void sc_DisableIntc();
	void sc_EnableDmac();
	void sc_DisableDmac();
	void sc_SetAlarm();
	void sc_ReleaseAlarm();
	void sc_CreateThread();
	void sc_DeleteThread();
	void sc_StartThread();
	void sc_ExitThread();
	void sc_ExitDeleteThread();
	void sc_TerminateThread();
	void sc_ChangeThreadPriority();
	void sc_RotateThreadReadyQueue();
	void sc_ReleaseWaitThread();
	void sc_GetThreadId();
	void sc_ReferThreadStatus();
	void sc_SleepThread();
	void sc_WakeupThread();
	void sc_CancelWakeupThread();
	void sc_SuspendThread();
	void sc_ResumeThread();
	void sc_SetupThread();
	void sc_SetupHeap();
	void sc_EndOfHeap();
	void sc_CreateSema();
	void sc_DeleteSema();
	void sc_SignalSema();
	void sc_WaitSema();
	void sc_PollSema();
	void sc_ReferSemaStatus();
	void sc_GetOsdConfigParam();
	void sc_FlushCache();
	void sc_GsGetIMR();
	void sc_GsPutIMR();
	void sc_SetVSyncFlag();
	void sc_SetSyscall();
	void sc_SifDmaStat();
	void sc_SifSetDma();
	void sc_SifSetDChain();
	void sc_SifSetReg();
	void sc_SifGetReg();
	void sc_Deci2Call();
	void sc_MachineType();
	void sc_GetMemorySize();
	void sc_Unhandled();

	CMIPS& m_ee;
	CGSHandler*& m_gs;

	uint8* m_ram = nullptr;
	uint32 m_ramSize = 0;
	uint8* m_bios = nullptr;
	uint8* m_spr = nullptr;

	CSIF& m_sif;
	Ee::CLibMc2 m_libMc2;
	CIopBios& m_iopBios;

	std::unique_ptr<CELF32> m_elf;
	ThreadList m_threads;
	SemaphoreList m_semaphores;
	IntcHandlerList m_intcHandlers;
	DmacHandlerList m_dmacHandlers;
	AlarmList m_alarms;

	OsVariableWrapper<uint32> m_currentThreadId;
	OsVariableWrapper<uint32> m_idleThreadId;
	OsVariableWrapper<uint32> m_tlblExceptionHandler;
	OsVariableWrapper<uint32> m_tlbsExceptionHandler;
	OsVariableWrapper<uint32> m_trapExceptionHandler;
	OsVariableWrapper<uint32> m_sifDmaNextIdx;

	uint32* m_sifDmaTimes = nullptr;

	ThreadQueue m_threadSchedule;
	IntcHandlerQueue m_intcHandlerQueue;
	DmacHandlerQueue m_dmacHandlerQueue;

	ArgumentList m_currentArguments;

	//For display purposes only
	std::string m_executableName;

	Ee::CIdleEvaluator m_idleEvaluator;

#ifdef DEBUGGER_INCLUDED
	static const SYSCALL_NAME g_syscallNames[];
#endif
};
