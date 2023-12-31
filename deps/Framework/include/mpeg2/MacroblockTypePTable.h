#ifndef _MPEG2_MACROBLOCKTYPEPTABLE_H_
#define _MPEG2_MACROBLOCKTYPEPTABLE_H_

#include "mpeg2/VLCTable.h"

namespace MPEG2
{

	class CMacroblockTypePTable : public MPEG2::CVLCTable
	{
	public:
									CMacroblockTypePTable();
		static MPEG2::CVLCTable*	GetInstance();

		enum MAXBITS
		{
			MAXBITS = 6,
		};

		enum ENTRYCOUNT
		{
			ENTRYCOUNT = 7,
		};

	private:
		static MPEG2::VLCTABLEENTRY	m_pTable[ENTRYCOUNT];
		static unsigned int			m_pIndexTable[MAXBITS];
		static MPEG2::CVLCTable*	m_pInstance;
	};

}

#endif
