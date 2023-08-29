#ifndef _IOSOSTREAM_H_
#define _IOSOSTREAM_H_

#include "Stream.h"
#include <istream>

namespace Framework
{

    class CIosIStream : public CStream
    {
    public:
                        CIosIStream(std::istream&);
        virtual         ~CIosIStream();

		virtual void	Seek(int64, STREAM_SEEK_DIRECTION);
		virtual uint64	Tell();
		virtual uint64	Read(void*, uint64);
		virtual uint64	Write(const void*, uint64);
		virtual bool	IsEOF();

    private:
        std::istream&   m_Stream;
    };

}

#endif
