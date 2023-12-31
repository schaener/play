#include "mpeg2/MacroblockTypeITable.h"

using namespace MPEG2;

VLCTABLEENTRY CMacroblockTypeITable::m_pTable[ENTRYCOUNT] =
{
	{ 0x0001,		1,			0x0001	},
	{ 0x0001,		2,			0x0011	},
};

unsigned int CMacroblockTypeITable::m_pIndexTable[MAXBITS] =
{
	0,
	1,
};

CVLCTable* CMacroblockTypeITable::m_pInstance = NULL;

CMacroblockTypeITable::CMacroblockTypeITable() :
CVLCTable(MAXBITS, m_pTable, ENTRYCOUNT, m_pIndexTable)
{

}

CVLCTable* CMacroblockTypeITable::GetInstance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new CMacroblockTypeITable();
	}

	return m_pInstance;
}
