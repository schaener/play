#pragma once

#include "PspModule.h"
#include "Psp_IoDevice.h"

namespace Psp
{
	class CIoFileMgrForUser : public CModule
	{
	public:
		enum
		{
			FD_STDOUT = 1,
			FD_STDIN = 2,
			FD_STDERR = 3,
			FD_FIRSTUSERID,
		};

		enum OPENFLAGS
		{
			OPEN_READ = 0x01,
			OPEN_WRITE = 0x02,
		};

		CIoFileMgrForUser(uint8*);

		void Invoke(uint32, CMIPS&) override;
		std::string GetName() const override;

		void RegisterDevice(const char*, const IoDevicePtr&);
		uint32 Open(const char*, uint32, uint32);
		Framework::CStream* GetFileStream(uint32);

		uint32 IoClose(uint32);

	private:
		typedef std::shared_ptr<Framework::CStream> StreamPtr;
		typedef std::map<std::string, IoDevicePtr> IoDeviceList;
		typedef std::map<unsigned int, StreamPtr> FileList;

		uint32 IoOpen(uint32, uint32, uint32);
		uint32 IoRead(uint32, uint32, uint32);
		uint32 IoWrite(uint32, uint32, uint32);
		uint32 IoLseek(uint32, uint32, uint32);

		uint8* m_ram = nullptr;
		uint32 m_nextFileId;
		IoDeviceList m_devices;
		FileList m_files;
	};

	typedef std::shared_ptr<CIoFileMgrForUser> IoFileMgrForUserModulePtr;
}
