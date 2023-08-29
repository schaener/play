#pragma once

#include "Convertible.h"
#include "Types.h"
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipArchiveReader.h"
#include <functional>

namespace Iop
{
	class CDmac;

	namespace Dmac
	{
		class CChannel
		{
		public:
			typedef std::function<uint32(uint8*, uint32, uint32, uint32)> ReceiveFunctionType;

			enum
			{
				REG_MADR = 0x00,
				REG_BCR = 0x04,
				REG_CHCR = 0x08
			};

			enum CHCR_DIR
			{
				CHCR_DR_TO = 0,
				CHCR_DR_FROM = 1,
			};

			struct BCR : public convertible<uint32>
			{
				unsigned int bs : 16;
				unsigned int ba : 16;
			};
			static_assert(sizeof(BCR) == sizeof(uint32), "Size of BCR struct must be 4 bytes.");

			struct CHCR : public convertible<uint32>
			{
				unsigned int dr : 1;
				unsigned int unused0 : 8;
				unsigned int co : 1;
				unsigned int li : 1;
				unsigned int unused1 : 13;
				unsigned int tr : 1;
				unsigned int unused2 : 7;
			};
			static_assert(sizeof(CHCR) == sizeof(uint32), "Size of CHCR struct must be 4 bytes.");

			CChannel(uint32, unsigned int, unsigned int, CDmac&);
			virtual ~CChannel() = default;

			void SaveState(Framework::CZipArchiveWriter&);
			void LoadState(Framework::CZipArchiveReader&);

			void Reset();
			void SetReceiveFunction(const ReceiveFunctionType&);
			void ResumeDma();
			uint32 ReadRegister(uint32);
			void WriteRegister(uint32, uint32);

		private:
			ReceiveFunctionType m_receiveFunction;
			CDmac& m_dmac;
			unsigned int m_number;
			unsigned int m_intrLine;
			uint32 m_baseAddress;
			uint32 m_MADR;
			BCR m_BCR;
			CHCR m_CHCR;
		};
	}
}
