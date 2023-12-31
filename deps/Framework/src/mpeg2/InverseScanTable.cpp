#include "mpeg2/InverseScanTable.h"

using namespace MPEG2;

unsigned int CInverseScanTable::m_nTable0[0x40] = 
{
	0,	1,	5,	6,	14,	15,	27,	28,
	2,	4,	7,	13,	16,	26,	29,	42,
	3,	8,	12,	17,	25,	30,	41,	43,
	9,	11,	18,	24,	31,	40,	44,	53,
	10,	19,	23,	32,	39,	45,	52,	54,
	20,	22,	33,	38,	46,	51,	55,	60,
	21,	34,	37,	47,	50,	56,	59,	61,
	35,	36,	48,	49,	57,	58,	62,	63
};

unsigned int CInverseScanTable::m_nTable1[0x40] =
{
	0,	4,	6,	20,	22,	36,	38,	52,
	1,	5,	7,	21,	23,	37,	39,	53,
	2,	8,	19,	24,	34,	40,	50,	54,
	3,	9,	18,	25,	35,	41,	51,	55,
	10,	17,	26,	30,	42,	46,	56,	60,
	11,	16,	27,	31,	43,	47,	57,	61,
	12,	15,	28,	32,	44,	48,	58,	62,
	13,	14,	29,	33,	45,	49,	59,	63
};
