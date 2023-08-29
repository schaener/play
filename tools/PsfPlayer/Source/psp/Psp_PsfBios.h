#pragma once

#include "PspBios.h"
#include "Psp_PsfDevice.h"

namespace Psp
{
	class CPsfBios : public CBiosDebugInfoProvider
	{
	public:
		CPsfBios(CMIPS&, uint8*, uint32);
		virtual ~CPsfBios();

		void AppendArchive(const CPsfBase&);
		void Start();
		void Reset();

		CSasCore* GetSasCore();
		CAudio* GetAudio();

		void HandleException();
		void HandleInterrupt();
		void CountTicks(uint32);

		bool IsIdle();

#ifdef DEBUGGER_INCLUDED
		void LoadDebugTags(Framework::Xml::CNode*);
		void SaveDebugTags(Framework::Xml::CNode*);

		BiosDebugModuleInfoArray GetModulesDebugInfo() const override;
		BiosDebugObjectInfoMap GetBiosObjectsDebugInfo() const override;
		BiosDebugObjectArray GetBiosObjects(uint32) const override;
#endif

	private:
		CBios m_bios;
		PsfDevicePtr m_psfDevice;
	};
}
