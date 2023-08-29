#pragma once

#include <memory>
#include "Types.h"
#include "Stream.h"

class CIszImageStream : public Framework::CStream
{
public:
	CIszImageStream(std::unique_ptr<Framework::CStream>);
	virtual ~CIszImageStream();

	virtual void Seek(int64, Framework::STREAM_SEEK_DIRECTION) override;
	virtual uint64 Tell() override;
	virtual uint64 Read(void*, uint64) override;
	virtual uint64 Write(const void*, uint64) override;
	virtual bool IsEOF() override;

private:
#pragma pack(push, 1)
	struct HEADER
	{
		char signature[4];
		uint8 headerSize;
		int8 version;
		uint32 volumeSerialNumber;

		uint16 sectorSize;
		uint32 totalSectors;

		int8 hasPassword;

		int64 segmentSize;

		uint32 blockNumber;
		uint32 blockSize;
		uint8 blockPtrLength;

		int8 segmentNumber;

		uint32 blockPtrOffset;
		uint32 segmentPtrOffset;
		uint32 dataOffset;

		int8 reserved;
	};
#pragma pack(pop)

	struct BLOCKDESCRIPTOR
	{
		uint32 size;
		uint8 storageType;
	};

	enum HASPASSWORD
	{
		ADI_PLAIN = 0,
		ADI_PASSWORD = 1,
		ADI_AES128 = 2,
		ADI_AES192 = 3,
		ADI_AES256 = 4
	};

	enum STORAGETYPE
	{
		ADI_ZERO = 0,
		ADI_DATA = 1,
		ADI_ZLIB = 2,
		ADI_BZ2 = 3
	};

	void ReadBlockDescriptorTable();
	uint64 GetTotalSize() const;
	const BLOCKDESCRIPTOR& SeekToBlock(uint64);
	void SyncCache();

	void ReadZeroBlock(uint32);
	void ReadDataBlock(uint32);
	void ReadGzipBlock(uint32);
	void ReadBz2Block(uint32);

	std::unique_ptr<Framework::CStream> m_baseStream;
	HEADER m_header;
	BLOCKDESCRIPTOR* m_blockDescriptorTable = nullptr;
	int64 m_cachedBlockNumber = -1;
	uint8* m_cachedBlock = nullptr;
	uint8* m_readBuffer = nullptr;
	uint64 m_position = 0;
};
