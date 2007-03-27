/** \file vertex_program_parse.cpp
 *
 * $Id: vertex_program_parse.cpp,v 1.5.50.1 2007/03/27 14:01:47 legallo Exp $
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

#include "std3d.h"
#include "vertex_program_parse.h"


//=================================================================================================
//================================== CVPParser ====================================================
//=================================================================================================
// Skip tabulation and space in a source code
void CVPParser::skipSpacesAndComments()
{
	bool stop = false;
	do
	{	
		switch(*_CurrChar)
		{
			case '\t':
			case '\r':
			case ' ' : 
				++_CurrChar; 
			break;
			//
			case '\n': 
				++_CurrChar; 
				++_LineIndex; 
				_LineStart = _CurrChar;
			break;
			case '#': // comment go till end of line
				while (*_CurrChar != '\n' && *_CurrChar != '\0') ++_CurrChar;
				skipSpacesAndComments();
			break;
			default:
				stop = true;
			break;			
		}
	}
	while (!stop);
}

//=================================================================================================
bool CVPParser::parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput)
{
	skipSpacesAndComments();
	bool result;
	if (outputOperand)
	{
		operand.Negate = false;
		switch(*_CurrChar)
		{			
			case 'o': result = parseOutputRegister(operand, errorOutput); break;
			case 'R': 
				result = parseVariableRegister(operand, errorOutput);							
			break;
			case 'A': result = parseAddressRegister(operand, errorOutput); break;
			case '-':
				errorOutput = "Negation not allowed on ouput register.";
				return false;
			default:
				errorOutput = "Output, Address, or Temporary register expected as an output operand.";
				return false;			
		}
		if (!result) return false;

		// parse the write mask
		return parseWriteMask(operand.WriteMask, errorOutput);				
	}
	else
	{
		operand.Negate = false;
		switch(*_CurrChar)
		{
			case 'v': result = parseInputRegister(operand, errorOutput); break;
			case 'R': result = parseVariableRegister(operand, errorOutput); break;
			case 'c': result = parseConstantRegister(operand, errorOutput); break;
			case 'a': result = parseAddressRegister(operand, errorOutput); break;
			case '-':
			{
				operand.Negate = true;
				// negation
				++ _CurrChar;
				skipSpacesAndComments();
				switch(*_CurrChar)
				{
					case 'v': result = parseInputRegister(operand, errorOutput); break;
					case 'R': result = parseVariableRegister(operand, errorOutput); break;
					case 'c': result = parseConstantRegister(operand, errorOutput); break;
					default:
						errorOutput = "Negation must be followed by an input register, a variable register, or a constant.";
						return false;
					break;
				}
			}
			break;
			default:
				errorOutput = "Syntax error.";
				return false;
			break;
		}
		if (!result) return false;		
		if (operand.Type != CVPOperand::AddressRegister)
		{
			if (!parseSwizzle(operand.Swizzle, errorOutput)) return false;		
			if (operand.Type == CVPOperand::Variable)
			{
				for(uint k = 0; k < 4; ++k)
				{
					if (!(_RegisterMask[operand.Value.VariableValue] & (1 << operand.Swizzle.Comp[k])))
					{
						errorOutput = "Can't read a register component before writing to it.";
						return false;
					}
				}
			}
		}
		return true;
	}
}

//=================================================================================================
bool CVPParser::parseInputRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::InputRegister;	
	if (*_CurrChar != '[')
	{
		errorOutput = "'[' expected when parsing an input register.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	if (isdigit(*_CurrChar))
	{
		// The input register is expressed as an index
		uint index = *_CurrChar - '0';
		++_CurrChar;
		if (isdigit(*_CurrChar))
		{
			index =  10 * index + (*_CurrChar - '0');
			++_CurrChar;
		}
		if (index > 15)
		{		
			errorOutput = "Invalid index for input register, must be in [0, 15].";
			return false;
		}
		operand.Value.InputRegisterValue = (CVPOperand::EInputRegister) index;
	}
	else
	{
		// The input register is expressed as a string
		uint32 strValue = 0;
		// read the 4 letters
		for(uint k = 0; k < 4; ++k)
		{
			if (!isalnum(*_CurrChar))
			{
				errorOutput = "Can't parse index for input register.";
				return false;
			}
			strValue |= ((uint32) *_CurrChar) << (8 * (3 - k));
			++_CurrChar;
		}
		switch (strValue)
		{
			case 'OPOS': operand.Value.InputRegisterValue = CVPOperand::IPosition; break;
			case 'WGHT': operand.Value.InputRegisterValue = CVPOperand::IWeight; break;
			case 'NRML': operand.Value.InputRegisterValue = CVPOperand::INormal; break;
			case 'COL0': operand.Value.InputRegisterValue = CVPOperand::IPrimaryColor; break;
			case 'COL1': operand.Value.InputRegisterValue = CVPOperand::ISecondaryColor; break;			
			case 'FOGC': operand.Value.InputRegisterValue = CVPOperand::IFogCoord; break;
			// texture argument
			case 'TEX0':
			case 'TEX1':
			case 'TEX2':
			case 'TEX3':
			case 'TEX4':
			case 'TEX5':
			case 'TEX6':
			case 'TEX7':
				operand.Value.InputRegisterValue = (CVPOperand::EInputRegister) (CVPOperand::ITex0 + strValue & 0xff - '0'); 
			break;
			default:
				errorOutput = "Can't parse index for input register.";
				return false;
		}		
	}
	skipSpacesAndComments();
	if (*_CurrChar != ']')
	{
		errorOutput = "']' expected when parsing an input register.";
		return false;
	}
	++ _CurrChar;
	return true;
}

//=================================================================================================
bool CVPParser::parseOutputRegister(CVPOperand &operand, std::string &errorOutput)
{	
	++_CurrChar;
	operand.Type = CVPOperand::OutputRegister;	
	if (*_CurrChar != '[')
	{
		errorOutput = "'[' expected when parsing an ouput register.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// The input register is expressed as a string
	uint32 strValue = 0;
	// read the 4 letters
	for(uint k = 0; k < 4; ++k)
	{
		if (!isalnum(*_CurrChar))
		{
			errorOutput = "Can't parse index for output register.";
			return false;
		}
		strValue |= ((uint32) *_CurrChar) << (8 * (3 - k));
		++_CurrChar;
	}
	// convert to enum
	switch(strValue)
	{
		case 'HPOS': operand.Value.OutputRegisterValue = CVPOperand::OHPosition; break;
		case 'COL0': operand.Value.OutputRegisterValue = CVPOperand::OPrimaryColor; break;
		case 'COL1': operand.Value.OutputRegisterValue = CVPOperand::OSecondaryColor; break;
		case 'BFC0': operand.Value.OutputRegisterValue = CVPOperand::OBackFacePrimaryColor; break;
		case 'BFC1': operand.Value.OutputRegisterValue = CVPOperand::OBackFaceSecondaryColor; break;
		case 'FOGC': operand.Value.OutputRegisterValue = CVPOperand::OFogCoord; break;
		case 'PSIZ': operand.Value.OutputRegisterValue = CVPOperand::OPointSize; break;
		case 'TEX0': operand.Value.OutputRegisterValue = CVPOperand::OTex0; break;
		case 'TEX1': operand.Value.OutputRegisterValue = CVPOperand::OTex1; break;
		case 'TEX2': operand.Value.OutputRegisterValue = CVPOperand::OTex2; break;
		case 'TEX3': operand.Value.OutputRegisterValue = CVPOperand::OTex3; break;
		case 'TEX4': operand.Value.OutputRegisterValue = CVPOperand::OTex4; break;
		case 'TEX5': operand.Value.OutputRegisterValue = CVPOperand::OTex5; break;
		case 'TEX6': operand.Value.OutputRegisterValue = CVPOperand::OTex6; break;
		case 'TEX7': operand.Value.OutputRegisterValue = CVPOperand::OTex7; break;
		default:
			errorOutput = "Can't read index for output register.";
			return false;
		break;
	}
	skipSpacesAndComments();
	if (*_CurrChar != ']')
	{
		errorOutput = "']' expected when parsing an output register.";
		return false;
	}
	++_CurrChar;
	return true;
}

//=================================================================================================
bool CVPParser::parseConstantRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::Constant;	
	if (*_CurrChar != '[')
	{
		errorOutput = "'[' expected when parsing a constant register.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	uint &index = operand.Value.ConstantValue;
	if (isdigit(*_CurrChar))
	{		
		// immediat case : c[0] to c[95]
		_CurrChar = parseUInt(_CurrChar, index);
		if (index > 95)
		{
			errorOutput = "Constant register index must range from 0 to 95.";
			return false;
		}
		operand.Indexed = false;
	}
	else if (*_CurrChar == 'A')
	{
		// indexed case : c[A0.x + 0] to c[A0.x + 95]
		operand.Indexed = true;
		index = 0;
		if (_CurrChar[1] == '0'
			&& _CurrChar[2] == '.'
			&& _CurrChar[3] == 'x')
		{
			_CurrChar += 4;
			skipSpacesAndComments();
			if (*_CurrChar == '+')
			{
				++ _CurrChar;
				skipSpacesAndComments();
				if (isdigit(*_CurrChar))
				{
					_CurrChar = parseUInt(_CurrChar, index);
					if (index > 95)
					{
						errorOutput = "Constant register index must range from 0 to 95.";
						return false;
					}
				}
				else
				{
					errorOutput = "Can't parse offset for constant register.";
					return false;
				}
			}
		}
		else
		{
			errorOutput = "Can't parse constant register index.";
			return false;
		}
	}
	skipSpacesAndComments();
	if (*_CurrChar != ']')
	{
		errorOutput = "']' expected when parsing an input register.";
		return false;
	}
	++_CurrChar;
	return true;
}

//=================================================================================================
bool CVPParser::parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered)
{
	skipSpacesAndComments();
	endEncountered = false;
	uint32 instrStr = 0;
	uint k;
	for(k = 0; k < 4; ++k)
	{
		if (!isalnum(*_CurrChar))
		{
			if (k < 3) // at least 3 letter in an instruction
			{
				errorOutput = "Syntax error : can't read opcode.";
				return false;
			}
			else break;			
		}
		instrStr |= ((uint) *_CurrChar) << (8 * (3 - k));
		++ _CurrChar;
	}
	if (k != 4)
	{
		instrStr |= (uint32) ' ';
	}
	switch (instrStr)
	{
		case 'ADD ':
			instr.Opcode.Op = CProgramInstruction::ADD;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'DP3 ':
			instr.Opcode.Op = CProgramInstruction::DP3;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'DP4 ':
			instr.Opcode.Op = CProgramInstruction::DP4;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'EXP ':
		case 'EXPP':
			instr.Opcode.Op = CProgramInstruction::EXP;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "EXP need a scalar src value.";
				return false;
			}
		break;
		/////////////////
		case 'LOG ':
			instr.Opcode.Op = CProgramInstruction::LOG;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "LOG need a scalar src value.";
				return false;
			}
		break;
		/////////////////
		case 'MAD ':
			instr.Opcode.Op = CProgramInstruction::MAD;
			if (!parseOp4(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'MAX ':
			instr.Opcode.Op = CProgramInstruction::MAX;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'MIN ':
			instr.Opcode.Op = CProgramInstruction::MIN;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'MOV ':
			instr.Opcode.Op = CProgramInstruction::MOV;
			if (!parseOp2(instr, errorOutput)) return false;
			
		break;
		/////////////////
		case 'MUL ':
			instr.Opcode.Op = CProgramInstruction::MUL;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'RCP ':
			instr.Opcode.Op = CProgramInstruction::RCP;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "RCP need a scalar src value.";
				return false;
			}
		break;
		/////////////////
		case 'RSQ ':
			instr.Opcode.Op = CProgramInstruction::RSQ;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "RSQ need a scalar src value.";
				return false;
			}
		break;
		/////////////////
		case 'DST ':
			instr.Opcode.VPOp = CVPInstruction::DST;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'LIT ':
			instr.Opcode.VPOp = CVPInstruction::LIT;
			if (!parseOp2(instr, errorOutput)) return false;
		break;		
		////////////////
		case 'SGE ':
			instr.Opcode.VPOp = CVPInstruction::SQE;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'SLT ':
			instr.Opcode.VPOp = CVPInstruction::SLT;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case 'ARL ':
			instr.Opcode.VPOp = CVPInstruction::ARL;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "ARL need a scalar src value.";
				return false;
			}
		break;	
		/////////////////
		case 'END ':
			endEncountered = true;
			return true;
		break;
		default:
			errorOutput = "Syntax error : unknow opcode.";
			return false;
		break;
	}

	if (instr.Dest.Type == CVPOperand::Variable)
	{
		_RegisterMask[instr.Dest.Value.VariableValue] |= instr.Dest.WriteMask;
	}

	// it is not allowed to write to an adress register except for ARL
	if (instrStr != 'ARL ')
	{
		if (instr.Dest.Type == CVPOperand::AddressRegister)
		{
			errorOutput = "Can't write to address register.";
			return false;
		}
	}

	// parse semi-colon
	skipSpacesAndComments();
	//
	if (*_CurrChar != ';')
	{
		errorOutput = "';' expected.";
		return false;
	}
	++_CurrChar;
	return true;
}


//=================================================================================================
bool CVPParser::isInputUsed(const TVProgram &prg, CVPOperand::EInputRegister input)
{
	for(uint k = 0; k < prg.size(); ++k)
	{
		uint numSrc = prg[k].getNumUsedSrc();
		for(uint l = 0; l < numSrc; ++l)
		{
			const CVPOperand &src =  prg[k].getSrc(l);
			if (src.Type == CVPOperand::InputRegister && src.Value.InputRegisterValue == input) return true;
		}
	}
	return false;
}

//=================================================================================================
bool CVPParser::parse(const char *src, CVProgram &result, std::string &errorOutput)
{
	if (!src) return false;	
	//
	std::fill(_RegisterMask, _RegisterMask + 96, 0);

	//
	_CurrChar = src;
	_LineStart = src;
	_LineIndex = 1;
	//
	//skipSpacesAndComments(); // in fact space are not allowed at the start of the vertex program

	// parse version
	if (   _CurrChar[0] != '!' 
		|| _CurrChar[1] != '!'
		|| _CurrChar[2] != 'V'
		|| _CurrChar[3] != 'P'
		|| _CurrChar[4] != '1'
		|| _CurrChar[5] != '.'
		|| (_CurrChar[6] != '0' && _CurrChar[6] != '1'))
	{
		errorOutput = "Can't parse version.";
		return false;
	}
	_CurrChar += 7;

	errorOutput.clear();
	// parse instructions
	bool endEncoutered = false;
	
	std::string errorMess;
	for(;;)
	{
		CVPInstruction instr;		
		if (!parseInstruction(instr, errorMess, endEncoutered))
		{
			errorOutput = std::string("CVPParser::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
			return false;
		}
		if (endEncoutered) break;		
		result._Program.push_back(instr);
	}	
	return true;	
}


