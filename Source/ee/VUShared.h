#ifndef _VUSHARED_H_
#define _VUSHARED_H_

#include "../MIPSReflection.h"
#include "../MipsJitter.h"
#include "../uint128.h"
#include <string.h>

#undef ABS
#undef MAX

namespace VUShared
{
	//Function to emit code to handle accesses to VU1 area mapped in VU0 address space
	typedef std::function<void(CMipsJitter*, uint8, uint8)> Vu1AreaAccessEmitter;

	enum OP_LATENCY
	{
		LATENCY_MAC = 4,
		LATENCY_DIV = 7,
		LATENCY_SQRT = 7,
		LATENCY_RSQRT = 13
	};

	enum VU_UPPEROP_BIT
	{
		VU_UPPEROP_BIT_I = 0x80000000,
		VU_UPPEROP_BIT_E = 0x40000000
	};

	enum VECTOR_COMP
	{
		VECTOR_COMPX = 0,
		VECTOR_COMPY = 1,
		VECTOR_COMPZ = 2,
		VECTOR_COMPW = 3,
	};

	struct REGISTER_PIPEINFO
	{
		size_t value;
		size_t heldValue;
		size_t counter;
	};

	struct FLAG_PIPEINFO
	{
		size_t value;
		size_t index;
		size_t valueArray;
		size_t timeArray;
	};

	struct OPERANDSET
	{
		unsigned int writeF;
		unsigned int readF0;
		unsigned int readElemF0;
		unsigned int readF1;
		unsigned int readElemF1;
		unsigned int writeI;
		unsigned int writeILsu;
		unsigned int readI0;
		unsigned int readI1;
		bool syncQ;
		bool readQ;
		bool syncP;
		bool readP;
		bool readMACflags;
		bool writeMACflags;

		//When set, means that a branch following the instruction will be
		//able to use the integer value directly
		bool branchValue;
	};

	struct VUINSTRUCTION;

	struct VUSUBTABLE
	{
		uint32 nShift;
		uint32 nMask;
		VUINSTRUCTION* pTable;
	};

	struct VUINSTRUCTION
	{
		const char* name;
		VUSUBTABLE* subTable;
		void (*pGetAffectedOperands)(VUINSTRUCTION* pInstr, CMIPS* pCtx, uint32, uint32, OPERANDSET&);
	};

	enum COMPILEHINT
	{
		COMPILEHINT_SKIPFMACUPDATE = 0x01,
	};

	uint32 MakeDestFromComponent(uint32);
	int32 GetImm11Offset(uint16);
	int32 GetBranch(uint16);

	void VerifyVuReflectionTable(MIPSReflection::INSTRUCTION*, VUShared::VUINSTRUCTION*, size_t);

	bool DestinationHasElement(uint8, unsigned int);
	void ComputeMemAccessAddr(CMipsJitter*, unsigned int, uint32, uint32, uint32);
	uint32 GetDestOffset(uint8);
	uint32* GetVectorElement(CMIPS*, unsigned int, unsigned int);
	size_t GetVectorElement(unsigned int, unsigned int);
	uint32* GetAccumulatorElement(CMIPS*, unsigned int);
	size_t GetAccumulatorElement(unsigned int);

	void PullVector(CMipsJitter*, uint8, size_t);
	void PushIntegerRegister(CMipsJitter*, unsigned int);

	void TestSZFlags(CMipsJitter*, uint8, size_t, uint32, uint32);

	void GetStatus(CMipsJitter*, size_t, uint32);
	void SetStatus(CMipsJitter*, size_t);

	void ADD_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool, uint32, uint32);
	void ADDA_base(CMipsJitter*, uint8, size_t, size_t, bool, uint32, uint32);
	void MADD_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool, uint32, uint32);
	void MADDA_base(CMipsJitter*, uint8, size_t, size_t, bool, uint32, uint32);
	void SUB_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool, uint32, uint32);
	void SUBA_base(CMipsJitter*, uint8, size_t, size_t, bool, uint32, uint32);
	void MSUB_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool, uint32, uint32);
	void MSUBA_base(CMipsJitter*, uint8, size_t, size_t, bool, uint32, uint32);
	void MUL_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool, uint32, uint32);
	void MULA_base(CMipsJitter*, uint8, size_t, size_t, bool, uint32, uint32);

	void MINI_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool);
	void MAX_base(CMipsJitter*, uint8, size_t, size_t, size_t, bool);

	//Shared instructions
	void ABS(CMipsJitter*, uint8, uint8, uint8);
	void ADD(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void ADDbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8, uint32, uint32);
	void ADDi(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void ADDq(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void ADDA(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void ADDAbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void ADDAi(CMipsJitter*, uint8, uint8, uint32, uint32);
	void ADDAq(CMipsJitter*, uint8, uint8, uint32, uint32);
	void CLIP(CMipsJitter*, uint8, uint8, uint32);
	void DIV(CMipsJitter*, uint8, uint8, uint8, uint8, uint32);
	void FTOI0(CMipsJitter*, uint8, uint8, uint8);
	void FTOI4(CMipsJitter*, uint8, uint8, uint8);
	void FTOI12(CMipsJitter*, uint8, uint8, uint8);
	void FTOI15(CMipsJitter*, uint8, uint8, uint8);
	void IADD(CMipsJitter*, uint8, uint8, uint8);
	void IADDI(CMipsJitter*, uint8, uint8, uint8);
	void IAND(CMipsJitter*, uint8, uint8, uint8);
	void ILWbase(CMipsJitter*, uint8);
	void ILWR(CMipsJitter*, uint8, uint8, uint8, uint32);
	void IOR(CMipsJitter*, uint8, uint8, uint8);
	void ISUB(CMipsJitter*, uint8, uint8, uint8);
	void ITOF0(CMipsJitter*, uint8, uint8, uint8);
	void ITOF4(CMipsJitter*, uint8, uint8, uint8);
	void ITOF12(CMipsJitter*, uint8, uint8, uint8);
	void ITOF15(CMipsJitter*, uint8, uint8, uint8);
	void ISWbase(CMipsJitter*, uint8, uint8);
	void ISWR(CMipsJitter*, uint8, uint8, uint8, uint32);
	void LQbase(CMipsJitter*, uint8, uint8);
	void LQD(CMipsJitter*, uint8, uint8, uint8, uint32);
	void LQI(CMipsJitter*, uint8, uint8, uint8, uint32);
	void MADD(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void MADDbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8, uint32, uint32);
	void MADDi(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MADDq(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MADDA(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MADDAbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void MADDAi(CMipsJitter*, uint8, uint8, uint32, uint32);
	void MADDAq(CMipsJitter*, uint8, uint8, uint32, uint32);
	void MAX(CMipsJitter*, uint8, uint8, uint8, uint8);
	void MAXbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8);
	void MAXi(CMipsJitter*, uint8, uint8, uint8);
	void MINI(CMipsJitter*, uint8, uint8, uint8, uint8);
	void MINIbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8);
	void MINIi(CMipsJitter*, uint8, uint8, uint8);
	void MOVE(CMipsJitter*, uint8, uint8, uint8);
	void MR32(CMipsJitter*, uint8, uint8, uint8);
	void MSUB(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void MSUBbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8, uint32, uint32);
	void MSUBi(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MSUBq(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MSUBA(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MSUBAbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void MSUBAi(CMipsJitter*, uint8, uint8, uint32, uint32);
	void MSUBAq(CMipsJitter*, uint8, uint8, uint32, uint32);
	void MFIR(CMipsJitter*, uint8, uint8, uint8);
	void MTIR(CMipsJitter*, uint8, uint8, uint8);
	void MUL(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void MULbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8, uint32, uint32);
	void MULi(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MULq(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MULA(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void MULAbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void MULAi(CMipsJitter*, uint8, uint8, uint32, uint32);
	void MULAq(CMipsJitter*, uint8, uint8, uint32, uint32);
	void OPMSUB(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void OPMULA(CMipsJitter*, uint8, uint8);
	void RINIT(CMipsJitter*, uint8, uint8);
	void RGET(CMipsJitter*, uint8, uint8);
	void RNEXT(CMipsJitter*, uint8, uint8);
	void RSQRT(CMipsJitter*, uint8, uint8, uint8, uint8, uint32);
	void RXOR(CMipsJitter*, uint8, uint8);
	void SQbase(CMipsJitter*, uint8, uint8);
	void SQD(CMipsJitter*, uint8, uint8, uint8, uint32);
	void SQI(CMipsJitter*, uint8, uint8, uint8, uint32, const Vu1AreaAccessEmitter& = Vu1AreaAccessEmitter());
	void SQRT(CMipsJitter*, uint8, uint8, uint32);
	void SUB(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void SUBbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint8, uint32, uint32);
	void SUBi(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void SUBq(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void SUBA(CMipsJitter*, uint8, uint8, uint8, uint32, uint32);
	void SUBAbc(CMipsJitter*, uint8, uint8, uint8, uint8, uint32, uint32);
	void SUBAi(CMipsJitter*, uint8, uint8, uint32, uint32);
	void WAITP(CMipsJitter*);
	void WAITQ(CMipsJitter*);

	void FlushPipeline(const REGISTER_PIPEINFO&, CMipsJitter*);
	void SyncPipeline(const REGISTER_PIPEINFO&, CMipsJitter*, uint32);
	void CheckPipeline(const REGISTER_PIPEINFO&, CMipsJitter*, uint32);
	void QueueInPipeline(const REGISTER_PIPEINFO&, CMipsJitter*, uint32, uint32);
	void CheckFlagPipeline(const FLAG_PIPEINFO&, CMipsJitter*, uint32);
	void QueueInFlagPipeline(const FLAG_PIPEINFO&, CMipsJitter*, uint32, uint32);
	void ResetFlagPipeline(const FLAG_PIPEINFO&, CMipsJitter*);

	//Shared addressing modes
	void ReflOpFdFsI(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFdFsQ(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFdFsFt(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFdFsFtBc(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFsDstItDec(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFsDstItInc(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFtFs(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFtIs(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFtDstIsDec(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFtDstIsInc(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpClip(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpAccFsI(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpAccFsQ(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpAccFsFt(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpAccFsFtBc(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpRFsf(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpFtR(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpQFtf(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpQFsfFtf(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpIdIsIt(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpItFsf(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpItIsDst(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);
	void ReflOpItIsImm5(MIPSReflection::INSTRUCTION*, CMIPS*, uint32, uint32, char*, unsigned int);

	void ReflOpAffNone(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);

	void ReflOpAffWrAMfRdFsFt(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrAMfRdFsFtBc(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrAMfRdFsI(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrAMfRdFsQ(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);

	void ReflOpAffWrCfRdFsFt(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);

	void ReflOpAffWrFdRdFsFt(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrFdRdFsI(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);

	void ReflOpAffWrFdMfRdFsFt(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrFdMfRdFsFtBc(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrFdMfRdFsI(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);
	void ReflOpAffWrFdMfRdFsQ(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);

	void ReflOpAffWrFtRdFs(VUINSTRUCTION*, CMIPS*, uint32, uint32, OPERANDSET&);

	VUINSTRUCTION* DereferenceInstruction(VUSUBTABLE*, uint32);
	void SubTableAffectedOperands(VUINSTRUCTION* pInstr, CMIPS* pCtx, uint32, uint32, OPERANDSET&);

	extern const char* m_sBroadcast[4];
	extern const char* m_sDestination[16];

	extern const REGISTER_PIPEINFO g_pipeInfoQ;
	extern const REGISTER_PIPEINFO g_pipeInfoP;
	extern const FLAG_PIPEINFO g_pipeInfoMac;
	extern const FLAG_PIPEINFO g_pipeInfoSticky;
	extern const FLAG_PIPEINFO g_pipeInfoClip;
}

#endif
