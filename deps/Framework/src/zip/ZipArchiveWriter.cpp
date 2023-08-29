#include <string.h>
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipDeflateStream.h"
#include "zip/ZipDefs.h"
#include <zstd_zlibwrapper.h>

using namespace Framework;
using namespace Framework::Zip;

void CZipArchiveWriter::Write(CStream& stream)
{
	typedef std::pair<std::string, ZIPDIRFILEHEADER> DirectoryEntry;
	typedef std::list<DirectoryEntry> DirectoryEntryList;

	DirectoryEntryList directoryEntries;

	for(const auto& file : m_files)
	{
		std::string fileName(file->GetName());
		uint32 relativePosition = static_cast<uint32>(stream.Tell());

		//Build an incomplete file header
		ZIPFILEHEADER fileHeader = {};
		fileHeader.signature = FILEHEADER_SIG;
		fileHeader.versionNeeded = 0x14;
		fileHeader.compressedSize = 0;
		fileHeader.uncompressedSize = 0;
		fileHeader.compressionMethod = ZWRAP_isUsingZSTDcompression() ? COMPRESSION_METHOD::ZSTD : COMPRESSION_METHOD::DEFLATE;
		fileHeader.fileNameLength = static_cast<uint16>(fileName.length());
		fileHeader.crc = 0;

		//Write file header
		stream.Write(&fileHeader, sizeof(ZIPFILEHEADER));

		//Write file name
		stream.Write(fileName.c_str(), fileName.length());

		//Write body
		CZipDeflateStream proxyStream(stream);
		file->Write(proxyStream);
		proxyStream.Flush();

		//Update header with info
		fileHeader.crc = proxyStream.GetCrc();
		fileHeader.compressedSize = static_cast<uint32>(proxyStream.GetCompressedLength());
		fileHeader.uncompressedSize = static_cast<uint32>(proxyStream.GetUncompressedLength());

		//Write back old header
		stream.Seek(relativePosition, STREAM_SEEK_SET);
		stream.Write(&fileHeader, sizeof(ZIPFILEHEADER));
		stream.Seek(0, STREAM_SEEK_END);

		//Create directory entry for this file
		ZIPDIRFILEHEADER dirFileHeader;
		memset(&dirFileHeader, 0, sizeof(ZIPDIRFILEHEADER));
		dirFileHeader.signature = DIRFILEHEADER_SIG;
		dirFileHeader.versionMadeBy = 0x14;
		dirFileHeader.versionNeeded = 0x14;
		dirFileHeader.crc = fileHeader.crc;
		dirFileHeader.fileStartOffset = relativePosition;
		dirFileHeader.compressedSize = fileHeader.compressedSize;
		dirFileHeader.uncompressedSize = fileHeader.uncompressedSize;
		dirFileHeader.fileNameLength = fileHeader.fileNameLength;
		dirFileHeader.compressionMethod = fileHeader.compressionMethod;

		directoryEntries.push_back(DirectoryEntry(fileName, dirFileHeader));
	}

	//Write directory
	uint64 dirStart = stream.Tell();

	for(const auto& entry : directoryEntries)
	{
		//Write file header
		stream.Write(&entry.second, sizeof(ZIPDIRFILEHEADER));

		//Write file name
		stream.Write(entry.first.c_str(), entry.first.length());
	}

	uint64 dirEnd = stream.Tell();

	//Write directory header
	{
		ZIPDIRENDHEADER dirHeader;
		memset(&dirHeader, 0, sizeof(ZIPDIRENDHEADER));
		dirHeader.signature = DIRENDHEADER_SIG;
		dirHeader.dirEntryCount = static_cast<uint16>(directoryEntries.size());
		dirHeader.totalDirEntryCount = static_cast<uint16>(directoryEntries.size());
		dirHeader.dirSize = static_cast<uint32>(dirEnd - dirStart);
		dirHeader.dirStartOffset = static_cast<uint32>(dirStart);

		//Write file header
		stream.Write(&dirHeader, sizeof(ZIPDIRENDHEADER));
	}
}

void CZipArchiveWriter::InsertFile(ZipFilePtr file)
{
	m_files.emplace_back(std::move(file));
}
