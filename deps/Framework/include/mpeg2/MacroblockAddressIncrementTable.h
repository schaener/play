#ifndef _MPEG2_MACROBLOCKADDRESSINCREMENTTABLE_H_
#define _MPEG2_MACROBLOCKADDRESSINCREMENTTABLE_H_

#include "mpeg2/VLCTable.h"

namespace MPEG2
{
	class CMacroblockAddressIncrementTable : public MPEG2::CVLCTable
	{
	public:
										CMacroblockAddressIncrementTable();
		static MPEG2::CVLCTable*		GetInstance();

		enum MAXBITS
		{
			MAXBITS = 11,
		};

		enum ENTRYCOUNT
		{
			ENTRYCOUNT = 35,
		};

	private:
		static MPEG2::VLCTABLEENTRY		m_pTable[ENTRYCOUNT];
		static unsigned int				m_pIndexTable[MAXBITS];
		static MPEG2::CVLCTable*		m_pInstance;
	};

}

#endif
