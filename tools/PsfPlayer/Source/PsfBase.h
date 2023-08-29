#ifndef _PSFBASE_H_
#define _PSFBASE_H_

#include <map>
#include <string>
#include "Stream.h"
#include "Types.h"

class CPsfBase
{
public:
	typedef std::map<std::string, std::string> TagMap;
	typedef TagMap::const_iterator ConstTagIterator;

	CPsfBase(Framework::CStream&);
	virtual ~CPsfBase();

	enum PSFVERSION
	{
		VERSION_PLAYSTATION = 0x01,
		VERSION_PLAYSTATION2 = 0x02,
		VERSION_PLAYSTATIONPORTABLE = 0x03,
		VERSION_SATURN = 0x11,
		VERSION_DREAMCAST = 0x12,
		VERSION_GENESIS = 0x13,
		VERSION_NINTENDO64 = 0x21,
		VERSION_GAMEBOYADVANCE = 0x22,
		VERSION_SUPERNES = 0x23,
		VERSION_CAPCOMQSOUND = 0x41
	};

	uint8 GetVersion() const;
	uint8* GetProgram() const;
	uint32 GetProgramUncompressedSize() const;
	uint8* GetReserved() const;
	uint32 GetReservedSize() const;
	const char* GetTagValue(const char*) const;
	ConstTagIterator GetTagsBegin() const;
	ConstTagIterator GetTagsEnd() const;

private:
	void ReadProgram(Framework::CStream&);
	void ReadTags(Framework::CStream&);

	uint8 m_version;
	uint32 m_reservedSize;
	uint32 m_programSize;
	uint32 m_programCrc;

	uint32 m_uncompProgramSize;

	uint8* m_reserved;
	uint8* m_program;

	TagMap m_tags;
};

#endif
