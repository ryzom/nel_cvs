/** \file vertex_program_parse.h
 * These are a set of classes used to parse a vertex program in proprietary format. This is used when
 * parsing isn't available in the target API (for example, missing OpenGL extension)
 * $Id: program_parse.h,v 1.1.2.1 2007/03/27 14:01:46 legallo Exp $
 */

/* Copyright, 2000, 2001, 2002 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */



#ifndef NL_PROGRAM_PARSE_H
#define NL_PROGRAM_PARSE_H

#include <vector>


//=================================================================================================
//========================================== CProgramParser =======================================
//=================================================================================================

/// Swizzle of an operand in a program
struct CProgramSwizzle
{
	enum EComp { X = 0, Y = 1, Z = 2, W = 3};
	EComp	Comp[4];
	// Test if all values are the same
	bool isScalar() const
	{
		return    Comp[0] == Comp[1]
			   && Comp[0] == Comp[2]
			   && Comp[0] == Comp[3];
	}
	// Test if no swizzle is applied
	bool isIdentity() const
	{
		return Comp[0] == X
			   && Comp[1] == Y
			   && Comp[2] == Z
			   && Comp[3] == W;
	}
};

//=================================================================================================
struct CProgramOperand
{
	bool Indexed; // true if it is a constant value, and if it is indexed

	// write mask (for output operands)
	uint WriteMask; // b0 -> X, b1 -> Y, b2 -> Z, b3 -> W

	// swizzle & negate 
	bool		Negate;
	CProgramSwizzle	Swizzle;
};

//=================================================================================================
/// An instruction in a program with its operands
struct CProgramInstruction
{
	enum EOpcode
	{
		ADD = 0,
		DP3,
		DP4,
		EXP,
		FRC,
		LOG,
		MAD,
		MAX,
		MIN,
		MOV,
		MUL,
		RCP,
		RSQ,
		SUB,
		M3X2,
		M3X3,
		M3X4,
		M4X3,
		M4X4,
		OpcodeCount
	};
	
	// Get the number of source used depending on the opcode. Might be 1, 2, or 3
	uint getNumUsedSrc(EOpcode opcode) const;
};


//=================================================================================================
class CProgramParser
{
public:
	typedef std::pair< int, std::vector<std::string> > TConstant;
	typedef std::vector<TConstant> TConstantsVector;

protected:
	const char *_CurrChar;
	const char *_LineStart;
	uint		_LineIndex;
	uint		_RegisterMask[96]; // which components of registers have been written

protected:
	
	virtual bool parseVariableRegister(uint &index, std::string &errorOutput);

	virtual bool parseSwizzle(CProgramSwizzle &swizzle, std::string &errorOutput);	
	virtual bool parseWriteMask(uint &mask, std::string &errorOutput);
	
	virtual void skipSpacesAndComments() = 0;

	inline bool letterToSwizzleComp(char letter, CProgramSwizzle::EComp &comp)
	{
		switch (letter)
		{
			case 'x': comp = CProgramSwizzle::X; return true;
			case 'y': comp = CProgramSwizzle::Y; return true;
			case 'z': comp = CProgramSwizzle::Z; return true;
			case 'w': comp = CProgramSwizzle::W; return true;
		}
		return false;
	}

	inline const char *parseUInt(const char *src, uint &dest)
	{
		uint index = 0;
		while (isdigit(*src))
		{
			index = 10 * index + *src - '0';
			++ src;
		}
		dest = index;
		return src;
	}

	inline std::string getStringUntilCR(const char *src)
	{
		nlassert(src);
		std::string result;
		while (*src != '\n' && *src != '\r' && *src != '\0') 
		{
			result += *src;
			++src;
		}
		return result;
	}
};

//=================================================================================================
//==================================== CVertexProgramParser =======================================
//=================================================================================================

/** An operand in a vertex program
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
struct CVPOperand : public CProgramOperand
{
	// type of operand
	enum EOperandType
	{
		Variable = 0, // (R[0], R[1] ..)
		Constant, // (
		InputRegister,
		OutputRegister,
		AddressRegister, // for now, only means a0.x, no additionnal info is needed
		OperandTypeCount
	};
	// input registers
	enum EInputRegister
	{
		IPosition = 0,
		IWeight,
		INormal,
		IPrimaryColor,
		ISecondaryColor,
		IFogCoord,
		IPaletteSkin,
		IEmpty,
		ITex0,
		ITex1,
		ITex2,
		ITex3,
		ITex4,
		ITex5,
		ITex6,
		ITex7,
		InputRegisterCount,
	};
	// output registers
	enum EOutputRegister
	{
		OHPosition = 0,
		OPrimaryColor,
		OSecondaryColor,
		OBackFacePrimaryColor,   // warning : backface colors are not supported on all implementations
		OBackFaceSecondaryColor,
		OFogCoord,
		OPointSize,
		OTex0,
		OTex1,
		OTex2,
		OTex3,
		OTex4,
		OTex5,
		OTex6,
		OTex7,
		OutputRegisterCount
	};

	EOperandType Type;
	
	// Datas for the various types.
	union
	{		
		EOutputRegister OutputRegisterValue;		
		EInputRegister  InputRegisterValue;		
		uint		    VariableValue; // Index from 0 to 11			
		uint            ConstantValue; // Index from 0 to 95		
	} Value;
};

//=================================================================================================
/// An instruction in a vertex program with its operands
struct CVPInstruction : public CProgramInstruction
{
	enum EVPOpcode
	{
		DST = CProgramInstruction::OpcodeCount, 
		LIT,
		SQE,
		SLT,
		ARL,
		OpcodeCount,
	};

	union
	{
		EOpcode   Op;
		EVPOpcode VPOp;
	} Opcode;

	CVPOperand  Dest;
	CVPOperand  Src1;
	CVPOperand  Src2; // if used
	CVPOperand  Src3; // if used

	const CVPOperand &getSrc(uint index) const
	{
		nlassert(index < getNumUsedSrc());
		switch(index)
		{
			case 0: return Src1;
			case 1: return Src2;
			case 2: return Src3;
			default: nlstop;
		}
		return Src1; // avoid warning
	}

	// Get the number of source used depending on the opcode. Might be 1, 2, or 3
	uint getNumUsedSrc() const;
};

//=================================================================================================
class CVertexProgramParser : virtual public CProgramParser
{
public:
	typedef std::vector<CVPInstruction> TVProgram;
	
	struct CVProgram
	{
		TVProgram         _Program;
		TConstantsVector _Constants;
	};

public:

	virtual bool parse(const char *src, CVProgram &result, std::string &errorOutput) = 0;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual bool parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput) = 0;
	//
	virtual bool parseInputRegister(CVPOperand &operand, std::string &errorOutput) = 0;
	virtual bool parseOutputRegister(CVPOperand &operand, std::string &errorOutput) = 0;
	virtual bool parseConstantRegister(CVPOperand &operand, std::string &errorOutput) = 0;
	virtual bool parseVariableRegister(CVPOperand &operand, std::string &errorOutput);
	virtual bool parseAddressRegister(CVPOperand &operand, std::string &errorOutput);
	//
	virtual bool parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered) = 0;
	// parse instruction in the form dest, src1, src2
	virtual bool parseOp2(CVPInstruction &instr, std::string &errorOutput);
	virtual bool parseOp3(CVPInstruction &instr, std::string &errorOutput);
	virtual bool parseOp4(CVPInstruction &instr, std::string &errorOutput);
	// skip spaces and count lines
	virtual void skipSpacesAndComments() = 0;
};

//=================================================================================================
//===================================== CPixelProgramParser =======================================
//=================================================================================================

// An operand in a pixel program
struct CPPOperand : public CProgramOperand
{
	// type of operand
	enum EOperandType
	{
		Variable = 0, // (R[0], R[1] ..)
		Constant, // (
		InputRegister,
		OutputRegister,
		Sampler2DRegister,
		Sampler3DRegister,
		OperandTypeCount
	};
	// input registers
	enum EInputRegister
	{
		IPrimaryColor = 0,
		ISecondaryColor,
		ITex0,
		ITex1,
		ITex2,
		ITex3,
		ITex4,
		ITex5,
		ITex6,
		ITex7,
		InputRegisterCount,
	};
	// output registers
	enum EOutputRegister
	{
		OColor = 0,
		ODepth,
		OutputRegisterCount
	};

	EOperandType Type;
	
	// Datas for the various types.
	union
	{		
		EOutputRegister OutputRegisterValue;		
		EInputRegister  InputRegisterValue;		
		uint		    VariableValue; // Index from 0 to 11			
		uint            ConstantValue; // Index from 0 to 95
		uint			SamplerValue;  // Index from 0 to 15
	} Value;
};

//=================================================================================================
/// An instruction in a pixel program with its operands
struct CPPInstruction : public CProgramInstruction
{
	enum EPPOpcode
	{
		ABS = CProgramInstruction::OpcodeCount, 
		CMP,
		CRS,
		DP2ADD,
		LRP,
		NRM,
		POW,
		SINCOS,
		TEX,
		TEXB,
		TEXP,
		OpcodeCount,
	};

	union
	{
		EOpcode   Op;
		EPPOpcode PPOp;
	} Opcode;

	CPPOperand  Dest;
	CPPOperand  Src1;
	CPPOperand  Src2; // if used
	CPPOperand  Src3; // if used

	const CPPOperand &getSrc(uint index) const
	{
		nlassert(index < getNumUsedSrc());
		switch(index)
		{
			case 0: return Src1;
			case 1: return Src2;
			case 2: return Src3;
			default: nlstop;
		}
		return Src1; // avoid warning
	}

	// Get the number of source used depending on the opcode. Might be 1, 2, or 3
	uint getNumUsedSrc() const;

	bool Sat;
};

//=================================================================================================
class CPixelProgramParser : virtual public CProgramParser
{
public:
	typedef std::vector<CPPInstruction> TPProgram;
	
	struct CPProgram
	{
		TPProgram         _Program;
		TConstantsVector _Constants;

		uint getUsedVariablesNb() const;
	};

public:

	virtual bool parse(const char *src, CPProgram &result, std::string &errorOutput) = 0;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
protected:
	virtual bool parseOperand(CPPOperand &operand, bool outputOperand, std::string &errorOutput) = 0;
	//
	virtual bool parseInputRegister(CPPOperand &operand, std::string &errorOutput) = 0;
	virtual bool parseOutputRegister(CPPOperand &operand, std::string &errorOutput) = 0;
	virtual bool parseConstantRegister(CPPOperand &operand, std::string &errorOutput) = 0;
	virtual bool parseVariableRegister(CPPOperand &operand, std::string &errorOutput);
	virtual bool parseSamplerRegister(CPPOperand &operand, std::string &errorOutput) = 0;
	//
	virtual bool parseInstruction(CPPInstruction &instr, std::string &errorOutput, bool &endEncountered) = 0;
	// parse instruction in the form dest, src1, src2
	virtual bool parseOp2(CPPInstruction &instr, std::string &errorOutput);
	virtual bool parseOp3(CPPInstruction &instr, std::string &errorOutput);
	virtual bool parseOp4(CPPInstruction &instr, std::string &errorOutput);
	// skip spaces and count lines
	virtual void skipSpacesAndComments() = 0;	
};



#endif

