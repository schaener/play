#pragma once

#include "PsfArchive.h"
#include "StdStream.h"
#include "zip/ZipArchiveReader.h"

class CPsfZipArchive : public CPsfArchive
{
public:
	virtual ~CPsfZipArchive() = default;

	virtual void Open(const fs::path&) override;
	virtual void ReadFileContents(const char*, void*, unsigned int) override;

private:
	typedef std::unique_ptr<Framework::CZipArchiveReader> ZipArchiveReaderPtr;

	Framework::CStdStream m_inputFile;
	ZipArchiveReaderPtr m_archive;
};
