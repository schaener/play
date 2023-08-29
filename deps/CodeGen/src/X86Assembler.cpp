#include "X86Assembler.h"
#include <cassert>
#include <stdexcept>
#include "LiteralPool.h"
#include "maybe_unused.h"

void CX86Assembler::Begin()
{
	m_nextLabelId = 1;
	m_nextLiteral128Id = 1;
	m_currentLabel = nullptr;
	m_tmpStream.ResetBuffer();
	m_labels.clear();
	m_labelOrder.clear();
}

void CX86Assembler::End()
{
	//Mark last label
	if(m_currentLabel != nullptr)
	{
		uint32 currentPos = static_cast<uint32>(m_tmpStream.Tell());
		m_currentLabel->size = currentPos - m_currentLabel->start;
	}

	//Initialize
	for(auto& labelPair : m_labels)
	{
		auto& label = labelPair.second;
		label.projectedStart = label.start;
	}

	while(1)
	{
		bool changed = false;

		for(LabelArray::const_iterator labelIterator(m_labelOrder.begin());
			labelIterator != m_labelOrder.end(); ++labelIterator)
		{
			auto& label = m_labels[*labelIterator];

			for(LabelRefArray::iterator labelRefIterator(label.labelRefs.begin());
				labelRefIterator != label.labelRefs.end(); ++labelRefIterator)
			{
				auto& labelRef(*labelRefIterator);
				switch(labelRef.length)
				{
					case JMP_NOTSET:
						{
							//Assume a short jump first.
							unsigned int jumpSize = GetJumpSize(labelRef.type, JMP_NEAR);
							labelRef.length = JMP_NEAR;
							IncrementJumpOffsetsLocal(label, labelRefIterator + 1, jumpSize);
							IncrementJumpOffsets(labelIterator + 1, jumpSize);
							changed = true;
						}
						break;
					case JMP_NEAR:
						{
							//We need to verify if the jump is still good for us, since sizes might have changed
							LABELINFO& referencedLabel(m_labels[labelRef.label]);
							unsigned int smallJumpSize = GetJumpSize(labelRef.type, JMP_NEAR);
							uint32 offset = referencedLabel.projectedStart - (labelRef.offset + smallJumpSize);
							uint32 offsetSize = GetMinimumConstantSize(offset);
							if(offsetSize != 1)
							{
								//Doesn't fit, recompute offsets
								labelRef.length = JMP_FAR;
								unsigned int longJumpSize = GetJumpSize(labelRef.type, JMP_FAR);
								unsigned int incrementAmount = longJumpSize - smallJumpSize;
								IncrementJumpOffsetsLocal(label, labelRefIterator + 1, incrementAmount);
								IncrementJumpOffsets(labelIterator + 1, incrementAmount);
								changed = true;
							}
						}
						break;
					case JMP_FAR:
						break;
					default:
						assert(false);
						break;
				}
			}
		}

		if(!changed) break;
	}

	assert(m_outputStream != nullptr);
	m_tmpStream.Seek(0, Framework::STREAM_SEEK_SET);

	for(const auto& labelId : m_labelOrder)
	{
		const auto& label = m_labels[labelId];

		unsigned int currentPos = label.start;
		unsigned int currentProjectedPos = label.projectedStart;
		unsigned int endPos = label.start + label.size;

		for(const auto& labelRef : label.labelRefs)
		{
			const auto& referencedLabel(m_labels[labelRef.label]);

			unsigned int readSize = labelRef.offset - currentProjectedPos;
			if(readSize != 0)
			{
				m_copyBuffer.resize(readSize);
				m_tmpStream.Read(m_copyBuffer.data(), readSize);
				m_outputStream->Write(m_copyBuffer.data(), readSize);
			}

			//Write our jump here.
			unsigned int jumpSize = GetJumpSize(labelRef.type, labelRef.length);
			uint32 distance = referencedLabel.projectedStart - (labelRef.offset + jumpSize);
			WriteJump(m_outputStream, labelRef.type, labelRef.length, distance);

			currentProjectedPos += readSize + jumpSize;
			currentPos += readSize;
		}

		unsigned int lastCopySize = endPos - currentPos;
		if(lastCopySize != 0)
		{
			m_copyBuffer.resize(lastCopySize);
			m_tmpStream.Read(m_copyBuffer.data(), lastCopySize);
			m_outputStream->Write(m_copyBuffer.data(), lastCopySize);
		}
	}

	ResolveLiteralReferences();
}

void CX86Assembler::IncrementJumpOffsets(LabelArray::const_iterator startLabel, unsigned int amount)
{
	for(LabelArray::const_iterator labelIterator(startLabel);
		labelIterator != m_labelOrder.end(); ++labelIterator)
	{
		LABELINFO& label = m_labels[*labelIterator];
		label.projectedStart += amount;
		IncrementJumpOffsetsLocal(label, label.labelRefs.begin(), amount);
	}
}

void CX86Assembler::IncrementJumpOffsetsLocal(LABELINFO& label, LabelRefArray::iterator startJump, unsigned int amount)
{
	for(LabelRefArray::iterator labelRefIterator(startJump);
		labelRefIterator != label.labelRefs.end(); ++labelRefIterator)
	{
		auto& labelRef(*labelRefIterator);

		//Make sure any literal ref happens before a label ref
		for(const auto& literalRefPair : label.literal128Refs)
		{
			FRAMEWORK_MAYBE_UNUSED const auto& literalRef = literalRefPair.second;
			assert(literalRef.offset < labelRef.offset);
		}

		labelRef.offset += amount;
	}
}

void CX86Assembler::SetStream(Framework::CStream* stream)
{
	m_outputStream = stream;
}

CX86Assembler::CAddress CX86Assembler::MakeRegisterAddress(REGISTER nRegister)
{
	CAddress Address;

	if(nRegister > 7)
	{
		Address.nIsExtendedModRM = true;
		nRegister = static_cast<REGISTER>(nRegister & 7);
	}

	Address.ModRm.nMod = 3;
	Address.ModRm.nRM = nRegister;

	return Address;
}

CX86Assembler::CAddress CX86Assembler::MakeXmmRegisterAddress(XMMREGISTER registerId)
{
	return MakeRegisterAddress(static_cast<REGISTER>(registerId));
}

CX86Assembler::CAddress CX86Assembler::MakeByteRegisterAddress(BYTEREGISTER registerId)
{
	auto address = MakeRegisterAddress(static_cast<REGISTER>(registerId));
	address.usesLegacyByteRegister = true;
	return address;
}

CX86Assembler::CAddress CX86Assembler::MakeIndRegAddress(REGISTER registerId)
{
	CAddress address;

	//Cannot be done with rBP
	assert(registerId != rBP);
	if(registerId == r13)
	{
		return MakeIndRegOffAddress(registerId, 0);
	}

	if(registerId == rSP)
	{
		registerId = static_cast<REGISTER>(4);
		address.sib.scale = 0;
		address.sib.index = 4;
		address.sib.base = 4;
	}
	else if(registerId == r12)
	{
		registerId = static_cast<REGISTER>(4);
		address.sib.scale = 0;
		address.sib.index = 4;
		address.sib.base = 4;
		address.nIsExtendedModRM = true;
	}

	if(registerId > 7)
	{
		address.nIsExtendedModRM = true;
		registerId = static_cast<REGISTER>(registerId & 7);
	}

	address.ModRm.nMod = 0;
	address.ModRm.nRM = registerId;
	return address;
}

CX86Assembler::CAddress CX86Assembler::MakeIndRegOffAddress(REGISTER nRegister, uint32 nOffset)
{
	if(
		(nOffset == 0) &&
		(nRegister != CX86Assembler::rBP) &&
		(nRegister != CX86Assembler::r13)
		)
	{
		return MakeIndRegAddress(nRegister);
	}
	
	CAddress Address;

	if(nRegister == rSP)
	{
		nRegister = static_cast<REGISTER>(4);
		Address.sib.scale = 0;
		Address.sib.index = 4;
		Address.sib.base = 4;
	}
	else if(nRegister == r12)
	{
		nRegister = static_cast<REGISTER>(4);
		Address.nIsExtendedModRM = true;
		Address.sib.scale = 0;
		Address.sib.index = 4;
		Address.sib.base = 4;
	}

	if(nRegister > 7)
	{
		Address.nIsExtendedModRM = true;
		nRegister = static_cast<REGISTER>(nRegister & 7);
	}

	if(GetMinimumConstantSize(nOffset) == 1)
	{
		Address.ModRm.nMod = 1;
		Address.nOffset = static_cast<uint8>(nOffset);
	}
	else
	{
		Address.ModRm.nMod = 2;
		Address.nOffset = nOffset;
	}

	Address.ModRm.nRM = nRegister;

	return Address;
}

CX86Assembler::CAddress CX86Assembler::MakeBaseOffIndexScaleAddress(REGISTER base, uint32 offset, REGISTER index, uint8 scale)
{
	if(index == rSP)
	{
		throw std::runtime_error("Invalid index.");
	}

	CAddress address;
	address.ModRm.nRM = 4;

	if(base > 7)
	{
		address.nIsExtendedModRM = true;
		base = static_cast<REGISTER>(base & 7);
	}
	if(index > 7)
	{
		address.nIsExtendedSib = true;
		index = static_cast<REGISTER>(index & 7);
	}
	address.sib.base = base;
	address.sib.index = index;
	switch(scale)
	{
	case 1:
		address.sib.scale = 0;
		break;
	case 2:
		address.sib.scale = 1;
		break;
	case 4:
		address.sib.scale = 2;
		break;
	case 8:
		address.sib.scale = 3;
		break;
	default:
		throw std::runtime_error("Invalid scale.");
		break;
	}

	//If base is rBP or r13 (collapsed to rBP above), we can't use offset-less encoding
	if((offset == 0) && (base != rBP))
	{
		address.ModRm.nMod = 0;
	}
	else if(GetMinimumConstantSize(offset) == 1)
	{
		address.ModRm.nMod = 1;
		address.nOffset = static_cast<uint8>(offset);
	}
	else
	{
		address.ModRm.nMod = 2;
		address.nOffset = offset;
	}

	return address;
}

CX86Assembler::CAddress CX86Assembler::MakeLiteral128Address(LITERAL128ID literalId)
{
	CAddress address;
	address.nOffset = 0;
	address.ModRm.nMod = 0;
	address.ModRm.nRM = 5;
	address.literal128Id = literalId;
	return address;
}

CX86Assembler::LABEL CX86Assembler::CreateLabel()
{
	LABEL newLabelId = m_nextLabelId++;
	m_labels[newLabelId] = LABELINFO();
	return newLabelId;
}

void CX86Assembler::MarkLabel(LABEL label, int32 offset)
{
	uint32 currentPos = static_cast<uint32>(m_tmpStream.Tell()) + offset;

	if(m_currentLabel != NULL)
	{
		m_currentLabel->size = currentPos - m_currentLabel->start;
	}

	auto labelIterator(m_labels.find(label));
	assert(labelIterator != m_labels.end());
	auto& labelInfo(labelIterator->second);
	labelInfo.start = currentPos;
	m_currentLabel = &labelInfo;
	m_labelOrder.push_back(label);
}

uint32 CX86Assembler::GetLabelOffset(LABEL label) const
{
	auto labelIterator(m_labels.find(label));
	assert(labelIterator != m_labels.end());
	const auto& labelInfo(labelIterator->second);
	return labelInfo.projectedStart;
}

CX86Assembler::LITERAL128ID CX86Assembler::CreateLiteral128(const LITERAL128& literal)
{
	auto literalId = m_nextLiteral128Id++;
	LITERAL128REF literalRef;
	literalRef.value = literal;
	assert(m_currentLabel);
	m_currentLabel->literal128Refs.insert(std::make_pair(literalId, literalRef));
	return literalId;
}

void CX86Assembler::ResolveLiteralReferences()
{
	CLiteralPool literalPool(m_outputStream);
	literalPool.AlignPool();

	for(const auto& labelId : m_labelOrder)
	{
		const auto& label = m_labels[labelId];
		assert(label.projectedStart >= label.start);
		uint32 projectedDiff = label.projectedStart - label.start;
		for(const auto& literalRefPair : label.literal128Refs)
		{
			const auto& literal = literalRefPair.second;
			auto literalPos = static_cast<uint32>(literalPool.GetLiteralPosition(literal.value));
			uint32 projectedOffset = literal.offset + projectedDiff;
			m_outputStream->Seek(projectedOffset, Framework::STREAM_SEEK_SET);
			static const uint32 opcodeSize = 4;
			auto offset = literalPos - projectedOffset - opcodeSize;
			m_outputStream->Write32(offset);
		}
	}

	m_outputStream->Seek(0, Framework::STREAM_SEEK_END);
}

void CX86Assembler::AdcEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x13, false, address, registerId);
}

void CX86Assembler::AdcId(const CAddress& address, uint32 constant)
{
	WriteEvId(0x02, address, constant);
}

void CX86Assembler::AddEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x03, false, address, registerId);
}

void CX86Assembler::AddEq(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x03, true, address, registerId);
}

void CX86Assembler::AddId(const CAddress& Address, uint32 nConstant)
{
	WriteEvId(0x00, Address, nConstant);
}

void CX86Assembler::AddIq(const CAddress& address, uint64 constant)
{
	WriteEvIq(0x00, address, constant);
}

void CX86Assembler::AndEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x23, false, address, registerId);
}

void CX86Assembler::AndEq(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x23, true, address, registerId);
}

void CX86Assembler::AndIb(const CAddress& address, uint8 constant)
{
	assert(!address.NeedsExtendedByteAddress());
	WriteEvIb(0x04, address, constant);
}

void CX86Assembler::AndId(const CAddress& address, uint32 constant)
{
	WriteEvId(0x04, address, constant);
}

void CX86Assembler::AndIq(const CAddress& address, uint64 constant)
{
	assert(0);
}

void CX86Assembler::BsrEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp0F(0xBD, false, address, registerId);
}

void CX86Assembler::CallEd(const CAddress& address)
{
	WriteEvOp(0xFF, 0x02, false, address);
}

void CX86Assembler::CmovsEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp0F(0x48, false, address, registerId);
}

void CX86Assembler::CmovnsEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp0F(0x49, false, address, registerId);
}

void CX86Assembler::CmpEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x3B, false, address, registerId);
}

void CX86Assembler::CmpEq(REGISTER nRegister, const CAddress& Address)
{
	WriteEvGvOp(0x3B, true, Address, nRegister);
}

void CX86Assembler::CmpIb(const CAddress& address, uint8 constant)
{
	assert(!address.NeedsExtendedByteAddress());
	WriteEvIb(0x07, address, constant);
}

void CX86Assembler::CmpId(const CAddress& address, uint32 constant)
{
	WriteEvId(0x07, address, constant);
}

void CX86Assembler::CmpIq(const CAddress& Address, uint64 nConstant)
{
	WriteEvIq(0x07, Address, nConstant);
}

void CX86Assembler::Cdq()
{
	WriteByte(0x99);
}

void CX86Assembler::DivEd(const CAddress& address)
{
	WriteEvOp(0xF7, 0x06, false, address);
}

void CX86Assembler::IdivEd(const CAddress& address)
{
	WriteEvOp(0xF7, 0x07, false, address);
}

void CX86Assembler::ImulEw(const CAddress& address)
{
	WriteByte(0x66);
	WriteEvOp(0xF7, 0x05, false, address);
}

void CX86Assembler::ImulEd(const CAddress& address)
{
	WriteEvOp(0xF7, 0x05, false, address);
}

void CX86Assembler::Int3()
{
	WriteByte(0xCC);
}

void CX86Assembler::JbJx(LABEL label)
{
	CreateLabelReference(label, JMP_B);
}

void CX86Assembler::JbeJx(LABEL label)
{
	CreateLabelReference(label, JMP_BE);
}

void CX86Assembler::JnbJx(LABEL label)
{
	CreateLabelReference(label, JMP_NB);
}

void CX86Assembler::JlJx(LABEL label)
{
	CreateLabelReference(label, JMP_L);
}

void CX86Assembler::JleJx(LABEL label)
{
	CreateLabelReference(label, JMP_LE);
}

void CX86Assembler::JnlJx(LABEL label)
{
	CreateLabelReference(label, JMP_NL);
}

void CX86Assembler::JnleJx(LABEL label)
{
	CreateLabelReference(label, JMP_NLE);
}

void CX86Assembler::JmpEd(const CAddress& address)
{
	WriteEvOp(0xFF, 0x04, false, address);
}

void CX86Assembler::JmpJx(LABEL label)
{
	CreateLabelReference(label, JMP_ALWAYS);
}

void CX86Assembler::JzJx(LABEL label)
{
	CreateLabelReference(label, JMP_Z);
}

void CX86Assembler::JnzJx(LABEL label)
{
	CreateLabelReference(label, JMP_NZ);
}

void CX86Assembler::JnbeJx(LABEL label)
{
	CreateLabelReference(label, JMP_NBE);
}

void CX86Assembler::JnoJx(LABEL label)
{
	CreateLabelReference(label, JMP_NO);
}

void CX86Assembler::JnsJx(LABEL label)
{
	CreateLabelReference(label, JMP_NS);
}

void CX86Assembler::LeaGd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x8D, false, address, registerId);
}

void CX86Assembler::LeaGq(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x8D, true, address, registerId);
}

void CX86Assembler::MovEw(REGISTER registerId, const CAddress& address)
{
	WriteByte(0x66);
	WriteEvGvOp(0x8B, false, address, registerId);
}

void CX86Assembler::MovEd(REGISTER nRegister, const CAddress& Address)
{
	WriteEvGvOp(0x8B, false, Address, nRegister);
}

void CX86Assembler::MovEq(REGISTER nRegister, const CAddress& Address)
{
	WriteEvGvOp(0x8B, true, Address, nRegister);
}

void CX86Assembler::MovGb(const CAddress& Address, BYTEREGISTER nRegister)
{
	if(Address.NeedsExtendedByteAddress())
	{
		throw std::runtime_error("Invalid operation.");
	}
	WriteEbGbOp(0x88, false, Address, nRegister);
}

void CX86Assembler::MovGb(const CAddress& Address, REGISTER nRegister)
{
	WriteEbGbOp(0x88, false, Address, nRegister);
}

void CX86Assembler::MovGw(const CAddress& Address, REGISTER nRegister)
{
	WriteByte(0x66);
	WriteEvGvOp(0x89, false, Address, nRegister);
}

void CX86Assembler::MovGd(const CAddress& Address, REGISTER nRegister)
{
	WriteEvGvOp(0x89, false, Address, nRegister);
}

void CX86Assembler::MovGq(const CAddress& Address, REGISTER nRegister)
{
	WriteEvGvOp(0x89, true, Address, nRegister);
}

void CX86Assembler::MovId(REGISTER nRegister, uint32 nConstant)
{
	CAddress Address(MakeRegisterAddress(nRegister));
	WriteRexByte(false, Address);
	WriteByte(0xB8 | Address.ModRm.nRM);
	WriteDWord(nConstant);
}

void CX86Assembler::MovIq(REGISTER registerId, uint64 constant)
{
	CAddress address(MakeRegisterAddress(registerId));
	WriteRexByte(true, address);
	WriteByte(0xB8 | address.ModRm.nRM);
	WriteDWord(static_cast<uint32>(constant & 0xFFFFFFFF));
	WriteDWord(static_cast<uint32>(constant >> 32));
}

void CX86Assembler::MovIb(const CX86Assembler::CAddress& address, uint8 constant)
{
	assert(!address.NeedsExtendedByteAddress());

	WriteRexByte(false, address);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = 0x00;

	WriteByte(0xC6);
	newAddress.Write(&m_tmpStream);
	WriteByte(constant);
}

void CX86Assembler::MovIw(const CX86Assembler::CAddress& address, uint16 constant)
{
	WriteByte(0x66);
	WriteRexByte(false, address);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = 0x00;

	WriteByte(0xC7);
	newAddress.Write(&m_tmpStream);
	WriteWord(constant);
}

void CX86Assembler::MovId(const CX86Assembler::CAddress& address, uint32 constant)
{
	WriteRexByte(false, address);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = 0x00;

	WriteByte(0xC7);
	newAddress.Write(&m_tmpStream);
	WriteDWord(constant);
}

void CX86Assembler::MovIq(const CX86Assembler::CAddress& address, uint32 constant)
{
	WriteRexByte(true, address);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = 0x00;

	WriteByte(0xC7);
	newAddress.Write(&m_tmpStream);
	WriteDWord(constant);
}

void CX86Assembler::MovsxEb(REGISTER registerId, const CAddress& address)
{
	WriteEbGvOp0F(0xBE, false, address, registerId);
}

void CX86Assembler::MovsxEw(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp0F(0xBF, false, address, registerId);
}

void CX86Assembler::MovzxEb(REGISTER registerId, const CAddress& address)
{
	WriteEbGvOp0F(0xB6, false, address, registerId);
}

void CX86Assembler::MovzxEw(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp0F(0xB7, false, address, registerId);
}

void CX86Assembler::MulEd(const CAddress& address)
{
	WriteEvOp(0xF7, 0x04, false, address);
}

void CX86Assembler::NegEd(const CAddress& address)
{
	WriteEvOp(0xF7, 0x03, false, address);
}

void CX86Assembler::Nop()
{
	WriteByte(0x90);
}

void CX86Assembler::NotEd(const CAddress& address)
{
	WriteEvOp(0xF7, 0x02, false, address);
}

void CX86Assembler::OrEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x0B, false, address, registerId);
}

void CX86Assembler::OrId(const CAddress& address, uint32 constant)
{
	WriteEvId(0x01, address, constant);
}

void CX86Assembler::Pop(REGISTER registerId)
{
	CAddress Address(MakeRegisterAddress(registerId));
	WriteRexByte(false, Address);
	WriteByte(0x58 | Address.ModRm.nRM);
}

void CX86Assembler::Push(REGISTER registerId)
{
	CAddress Address(MakeRegisterAddress(registerId));
	WriteRexByte(false, Address);
	WriteByte(0x50 | Address.ModRm.nRM);
}

void CX86Assembler::PushEd(const CAddress& address)
{
	WriteEvOp(0xFF, 0x06, false, address);
}

void CX86Assembler::PushId(uint32 value)
{
	WriteByte(0x68);
	WriteDWord(value);
}

void CX86Assembler::RclEd(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x02, false, address);
	WriteByte(amount);
}

void CX86Assembler::RepMovsb()
{
	WriteByte(0xF3);
	WriteByte(0xA4);
}

void CX86Assembler::Ret()
{
	WriteByte(0xC3);
}

void CX86Assembler::SarEd(const CAddress& address)
{
	WriteEvOp(0xD3, 0x07, false, address);
}

void CX86Assembler::SarEd(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x07, false, address);
	WriteByte(amount);
}

void CX86Assembler::SarEq(const CAddress& address)
{
	WriteEvOp(0xD3, 0x07, true, address);
}

void CX86Assembler::SarEq(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x07, true, address);
	WriteByte(amount);
}

void CX86Assembler::SbbEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x1B, false, address, registerId);
}

void CX86Assembler::SbbId(const CAddress& Address, uint32 nConstant)
{
	WriteEvId(0x03, Address, nConstant);
}

void CX86Assembler::SetaEb(const CAddress& address)
{
	WriteEbOp_0F(0x97, 0x00, address);
}

void CX86Assembler::SetaeEb(const CAddress& address)
{
	throw std::exception();
}

void CX86Assembler::SetbEb(const CAddress& address)
{
	WriteEbOp_0F(0x92, 0x00, address);
}

void CX86Assembler::SetbeEb(const CAddress& address)
{
	WriteEbOp_0F(0x96, 0x00, address);
}

void CX86Assembler::SeteEb(const CAddress& address)
{
	WriteEbOp_0F(0x94, 0x00, address);
}

void CX86Assembler::SetneEb(const CAddress& address)
{
	WriteEbOp_0F(0x95, 0x00, address);
}

void CX86Assembler::SetlEb(const CAddress& address)
{
	WriteEbOp_0F(0x9C, 0x00, address);
}

void CX86Assembler::SetleEb(const CAddress& address)
{
	WriteEbOp_0F(0x9E, 0x00, address);
}

void CX86Assembler::SetgEb(const CAddress& address)
{
	WriteEbOp_0F(0x9F, 0x00, address);
}

void CX86Assembler::ShlEd(const CAddress& address)
{
	WriteEvOp(0xD3, 0x04, false, address);
}

void CX86Assembler::ShlEd(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x04, false, address);
	WriteByte(amount);
}

void CX86Assembler::ShlEq(const CAddress& address)
{
	WriteEvOp(0xD3, 0x04, true, address);
}

void CX86Assembler::ShlEq(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x04, true, address);
	WriteByte(amount);
}

void CX86Assembler::ShrEd(const CAddress& address)
{
	WriteEvOp(0xD3, 0x05, false, address);
}

void CX86Assembler::ShrEd(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x05, false, address);
	WriteByte(amount);
}

void CX86Assembler::ShrEq(const CAddress& address)
{
	WriteEvOp(0xD3, 0x05, true, address);
}

void CX86Assembler::ShrEq(const CAddress& address, uint8 amount)
{
	WriteEvOp(0xC1, 0x05, true, address);
	WriteByte(amount);
}

void CX86Assembler::ShldEd(const CAddress& address, REGISTER registerId)
{
	WriteByte(0x0F);
	WriteEvGvOp(0xA5, false, address, registerId);
}

void CX86Assembler::ShldEd(const CAddress& address, REGISTER registerId, uint8 amount)
{
	WriteByte(0x0F);
	WriteEvGvOp(0xA4, false, address, registerId);
	WriteByte(amount);
}

void CX86Assembler::ShrdEd(const CAddress& address, REGISTER registerId)
{
	WriteByte(0x0F);
	WriteEvGvOp(0xAD, false, address, registerId);
}

void CX86Assembler::ShrdEd(const CAddress& address, REGISTER registerId, uint8 amount)
{
	WriteByte(0x0F);
	WriteEvGvOp(0xAC, false, address, registerId);
	WriteByte(amount);
}

void CX86Assembler::SubEd(REGISTER nRegister, const CAddress& Address)
{
	WriteEvGvOp(0x2B, false, Address, nRegister);
}

void CX86Assembler::SubEq(REGISTER nRegister, const CAddress& Address)
{
	WriteEvGvOp(0x2B, true, Address, nRegister);
}

void CX86Assembler::SubId(const CAddress& Address, uint32 nConstant)
{
	WriteEvId(0x05, Address, nConstant);
}

void CX86Assembler::SubIq(const CAddress& address, uint64 constant)
{
	WriteEvIq(0x05, address, constant);
}

void CX86Assembler::TestEb(BYTEREGISTER registerId, const CAddress& address)
{
	if(address.NeedsExtendedByteAddress())
	{
		throw std::runtime_error("Invalid operation.");
	}
	WriteEbGbOp(0x84, false, address, registerId);
}

void CX86Assembler::TestEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x85, false, address, registerId);
}

void CX86Assembler::TestEq(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x85, true, address, registerId);
}

void CX86Assembler::XorEd(REGISTER registerId, const CAddress& address)
{
	WriteEvGvOp(0x33, false, address, registerId);
}

void CX86Assembler::XorId(const CAddress& address, uint32 constant)
{
	WriteEvId(0x06, address, constant);
}

void CX86Assembler::XorGd(const CAddress& Address, REGISTER nRegister)
{
	WriteEvGvOp(0x31, false, Address, nRegister);
}

void CX86Assembler::XorGq(const CAddress& Address, REGISTER nRegister)
{
	WriteEvGvOp(0x31, true, Address, nRegister);
}

void CX86Assembler::WriteRexByte(bool nIs64, const CAddress& Address)
{
	REGISTER nTemp = rAX;
	WriteRexByte(nIs64, Address, nTemp);
}

void CX86Assembler::WriteRexByte(bool nIs64, const CAddress& Address, REGISTER& nRegister, bool forceWrite)
{
	if((nIs64) || (Address.nIsExtendedModRM) || (Address.nIsExtendedSib) || (nRegister > 7) || forceWrite)
	{
		uint8 nByte = 0x40;
		nByte |= nIs64 ? 0x8 : 0x0;
		nByte |= (nRegister > 7) ? 0x04 : 0x0;
		nByte |= Address.nIsExtendedSib ? 0x02 : 0x0;
		nByte |= Address.nIsExtendedModRM ? 0x1 : 0x0;

		nRegister = static_cast<REGISTER>(nRegister & 7);

		WriteByte(nByte);
	}
}

void CX86Assembler::WriteEbOp_0F(uint8 opcode, uint8 subOpcode, const CAddress& address)
{
	auto dummyReg = CX86Assembler::rAX;
	bool forcedWrite = address.NeedsExtendedByteAddress();
	WriteRexByte(false, address, dummyReg, forcedWrite);
	WriteByte(0x0F);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = subOpcode;
	WriteByte(opcode);
	newAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEbGbOp(uint8 nOp, bool nIs64, const CAddress& Address, REGISTER nRegister)
{
	WriteRexByte(nIs64, Address, nRegister, true);
	CAddress NewAddress(Address);
	NewAddress.ModRm.nFnReg = nRegister;
	WriteByte(nOp);
	NewAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEbGbOp(uint8 nOp, bool nIs64, const CAddress& Address, BYTEREGISTER nRegister)
{
	CAddress NewAddress(Address);
	NewAddress.ModRm.nFnReg = nRegister;
	WriteByte(nOp);
	NewAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEbGvOp0F(uint8 op, bool is64, const CAddress& address, REGISTER registerId)
{
	assert(!(address.usesLegacyByteRegister && (registerId > 7)));
	bool forcedWrite = address.NeedsExtendedByteAddress();
	WriteRexByte(is64, address, registerId, forcedWrite);
	WriteByte(0x0F);
	CAddress NewAddress(address);
	NewAddress.ModRm.nFnReg = registerId;
	WriteByte(op);
	NewAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEvOp(uint8 opcode, uint8 subOpcode, bool is64, const CAddress& address)
{
	WriteRexByte(is64, address);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = subOpcode;
	WriteByte(opcode);
	newAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEvGvOp(uint8 nOp, bool nIs64, const CAddress& Address, REGISTER nRegister)
{
	WriteRexByte(nIs64, Address, nRegister);
	CAddress NewAddress(Address);
	NewAddress.ModRm.nFnReg = nRegister;
	WriteByte(nOp);
	NewAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEvGvOp0F(uint8 nOp, bool nIs64, const CAddress& Address, REGISTER nRegister)
{
	WriteRexByte(nIs64, Address, nRegister);
	WriteByte(0x0F);
	CAddress NewAddress(Address);
	NewAddress.ModRm.nFnReg = nRegister;
	WriteByte(nOp);
	NewAddress.Write(&m_tmpStream);
}

void CX86Assembler::WriteEvIb(uint8 op, const CAddress& address, uint8 constant)
{
	WriteRexByte(false, address);
	CAddress newAddress(address);
	newAddress.ModRm.nFnReg = op;
	WriteByte(0x80);
	newAddress.Write(&m_tmpStream);
	WriteByte(constant);
}

void CX86Assembler::WriteEvId(uint8 nOp, const CAddress& Address, uint32 nConstant)
{
	//0x81 -> Id
	//0x83 -> Ib
 
	WriteRexByte(false, Address);
	CAddress NewAddress(Address);
	NewAddress.ModRm.nFnReg = nOp;

	if(GetMinimumConstantSize(nConstant) == 1)
	{
		WriteByte(0x83);
		NewAddress.Write(&m_tmpStream);
		WriteByte(static_cast<uint8>(nConstant));
	}
	else
	{
		WriteByte(0x81);
		NewAddress.Write(&m_tmpStream);
		WriteDWord(nConstant);
	}
}

void CX86Assembler::WriteEvIq(uint8 nOp, const CAddress& Address, uint64 nConstant)
{
	unsigned int nConstantSize(GetMinimumConstantSize64(nConstant));
	assert(nConstantSize <= 4);

	WriteRexByte(true, Address);
	CAddress NewAddress(Address);
	NewAddress.ModRm.nFnReg = nOp;

	if(nConstantSize == 1)
	{
		WriteByte(0x83);
		NewAddress.Write(&m_tmpStream);
		WriteByte(static_cast<uint8>(nConstant));
	}
	else
	{
		WriteByte(0x81);
		NewAddress.Write(&m_tmpStream);
		WriteDWord(static_cast<uint32>(nConstant));
	}
}

void CX86Assembler::CreateLabelReference(LABEL label, JMP_TYPE type)
{
	assert(m_currentLabel != NULL);

	LABELREF reference;
	reference.label			= label;
	reference.offset		= static_cast<uint32>(m_tmpStream.Tell());
	reference.type			= type;
	
	m_currentLabel->labelRefs.push_back(reference);
}

bool CX86Assembler::HasByteRegister(REGISTER registerId)
{
	return (registerId < rSP);
}

CX86Assembler::BYTEREGISTER CX86Assembler::GetByteRegister(REGISTER registerId)
{
	assert(HasByteRegister(registerId));
	return static_cast<BYTEREGISTER>(registerId);
}

unsigned int CX86Assembler::GetMinimumConstantSize(uint32 nConstant)
{
	if((static_cast<int32>(nConstant) >= -128) && (static_cast<int32>(nConstant) <= 127))
	{
		return 1;
	}
	return 4;
}

unsigned int CX86Assembler::GetMinimumConstantSize64(uint64 nConstant)
{
	if((static_cast<int64>(nConstant) >= -128) && (static_cast<int64>(nConstant) <= 127))
	{
		return 1;
	}
	if((static_cast<int64>(nConstant) >= -2147483647) && (static_cast<int64>(nConstant) <= 2147483647))
	{
		return 4;
	}
	return 8;
}

unsigned int CX86Assembler::GetJumpSize(JMP_TYPE type, JMP_LENGTH length)
{
	if(length == JMP_NEAR)
	{
		return 2;
	}
	else
	{
		if(type == JMP_ALWAYS)
		{
			return 5;
		}
		else
		{
			return 6;
		}
	}
}

void CX86Assembler::WriteJump(Framework::CStream* stream, JMP_TYPE type, JMP_LENGTH length, uint32 offset)
{
	if(length == JMP_FAR)
	{
		switch(type)
		{
		case JMP_ALWAYS:
			stream->Write8(0xE9);
			break;
		default:
			stream->Write8(0x0F);
			stream->Write8(0x80 | static_cast<uint8>(type));
			break;
		}
		stream->Write32(offset);
	}
	else
	{
		switch(type)
		{
		case JMP_ALWAYS:
			stream->Write8(0xEB);
			break;
		default:
			stream->Write8(0x70 | static_cast<uint8>(type));
			break;
		}
		stream->Write8(static_cast<uint8>(offset));
	}
}

void CX86Assembler::WriteByte(uint8 nByte)
{
	m_tmpStream.Write8(nByte);
}

void CX86Assembler::WriteWord(uint16 word)
{
	m_tmpStream.Write16(word);
}

void CX86Assembler::WriteDWord(uint32 nDWord)
{
	//Endianess should be good, unless we target a different processor...
	m_tmpStream.Write32(nDWord);
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////

CX86Assembler::CAddress::CAddress()
{
	nOffset = 0;
	ModRm.nByte = 0;
	sib.byteValue = 0;
	nIsExtendedModRM = false;
	nIsExtendedSib = false;
	usesLegacyByteRegister = false;
}

void CX86Assembler::CAddress::Write(Framework::CStream* stream)
{
	stream->Write8(ModRm.nByte);

	if(HasSib())
	{
		stream->Write8(sib.byteValue);
	}

	if(ModRm.nMod == 1)
	{
		stream->Write8(static_cast<uint8>(nOffset));
	}
	else if(ModRm.nMod == 2)
	{
		stream->Write32(nOffset);
	}
}

bool CX86Assembler::CAddress::HasSib() const
{
	if(ModRm.nMod == 3) return false;
	return ModRm.nRM == 4;
}

bool CX86Assembler::CAddress::NeedsExtendedByteAddress() const
{
	return (ModRm.nMod == 0x03) && !usesLegacyByteRegister;
}
