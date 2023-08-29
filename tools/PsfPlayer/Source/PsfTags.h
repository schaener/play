#ifndef _PSFTAGMANAGER_H_
#define _PSFTAGMANAGER_H_

#include <functional>
#include "PsfBase.h"

class CPsfTags
{
public:
	typedef std::function<std::wstring(const std::string&)> StringConvertFunction;
	typedef CPsfBase::TagMap TagMap;
	typedef CPsfBase::ConstTagIterator ConstTagIterator;

	enum CHAR_ENCODING
	{
		CE_INVALID = 0,
		CE_WINDOWS_1252 = 1,
		CE_SHIFT_JIS = 2,
		CE_UTF8 = 3,
	};

	CPsfTags();
	CPsfTags(const TagMap&);
	virtual ~CPsfTags();
	void SetDefaultCharEncoding(const CHAR_ENCODING&);
	bool HasTag(const char*) const;
	std::string GetRawTagValue(const char*) const;
	std::wstring GetTagValue(const char*) const;
	std::wstring DecodeTagValue(const char*) const;

	static double ConvertTimeString(const wchar_t*);

	ConstTagIterator GetTagsBegin() const;
	ConstTagIterator GetTagsEnd() const;

private:
	void Init();
	void UpdateStringConverter();
	void SetStringConverter(const CHAR_ENCODING&);

	CHAR_ENCODING m_encoding;
	CHAR_ENCODING m_defaultEncoding;
	StringConvertFunction m_stringConverter;
	TagMap m_tags;
};

#endif
