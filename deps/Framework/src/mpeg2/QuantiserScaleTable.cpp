#include "mpeg2/QuantiserScaleTable.h"

using namespace MPEG2;

unsigned int CQuantiserScaleTable::m_nTable0[0x20] =
{
	0,		2,		4,		6,		8,		10,		12,		14,	
	16,		18,		20,		22,		24,		26,		28,		30,	
	32,		34,		36,		38,		40,		42,		44,		46,	
	48,		50,		52,		54,		56,		58,		60,		62
};

unsigned int CQuantiserScaleTable::m_nTable1[0x20] =
{
	0,		1,		2,		3,		4,		5,		6,		7,
	8,		10,		12,		14,		16,		18,		20,		22,	
	24,		28,		32,		36,		40,		44,		48,		52,
	56,		64,		72,		80,		88,		96,		104,	112,
};
