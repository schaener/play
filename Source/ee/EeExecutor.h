#pragma once

#ifdef _WIN32
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <thread>
#elif defined(__unix__)
#include <signal.h>
#endif

#include "../GenericMipsExecutor.h"

class CEeExecutor : public CGenericMipsExecutor<BlockLookupTwoWay>
{
public:
	CEeExecutor(CMIPS&, uint8*);
	virtual ~CEeExecutor() = default;

	void AddExceptionHandler();
	void RemoveExceptionHandler();

	void AttachExceptionHandlerToThread();

	void Reset() override;
	void ClearActiveBlocksInRange(uint32, uint32, bool) override;

	BasicBlockPtr BlockFactory(CMIPS&, uint32, uint32) override;

private:
	typedef std::pair<uint128, uint32> CachedBlockKey;
	typedef std::map<CachedBlockKey, BasicBlockPtr> CachedBlockMap;
	CachedBlockMap m_cachedBlocks;

	uint8* m_ram = nullptr;
	size_t m_pageSize = 0;

	bool HandleAccessFault(intptr_t);
	void SetMemoryProtected(void*, size_t, bool);

#if defined(_WIN32)
	static LONG CALLBACK HandleException(_EXCEPTION_POINTERS*);
	LONG HandleExceptionInternal(_EXCEPTION_POINTERS*);

	LPVOID m_handler = NULL;
#elif defined(__unix__) || defined(__ANDROID__)
	static void HandleException(int, siginfo_t*, void*);
	void HandleExceptionInternal(int, siginfo_t*, void*);
#elif defined(__APPLE__)
	void HandlerThreadProc();

	mach_port_t m_port = MACH_PORT_NULL;
	std::thread m_handlerThread;
	std::atomic<bool> m_running = false;
#endif
};
