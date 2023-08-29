#pragma once

#include "Iop_Module.h"

class CIopBios;

namespace Iop
{
	class CModload : public CModule
	{
	public:
		CModload(CIopBios&, uint8*);
		virtual ~CModload() = default;

		std::string GetId() const override;
		std::string GetFunctionName(unsigned int) const override;
		void Invoke(CMIPS&, unsigned int) override;

	private:
		uint32 LoadStartModule(uint32, uint32, uint32, uint32);
		uint32 StartModule(uint32, uint32, uint32, uint32, uint32);
		uint32 LoadModuleBufferAddress(uint32, uint32, uint32);
		uint32 LoadModuleBuffer(uint32);
		uint32 GetModuleIdList(uint32, uint32, uint32);
		int32 ReferModuleStatus(uint32, uint32);
		int32 LoadModuleWithOption(uint32, uint32);
		int32 StopModule(uint32, uint32, uint32, uint32);
		int32 UnloadModule(uint32);
		int32 SearchModuleByName(uint32);
		int32 AllocLoadMemory(uint32, uint32, uint32);

		CIopBios& m_bios;
		uint8* m_ram = nullptr;
	};

	typedef std::shared_ptr<CModload> ModloadPtr;
}
