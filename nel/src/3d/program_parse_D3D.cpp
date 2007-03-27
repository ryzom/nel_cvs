/** \file vertex_program_parse.cpp
 *
 * $Id: program_parse_D3D.cpp,v 1.1.2.1 2007/03/27 14:01:46 legallo Exp $
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
#include "program_parse_D3D.h"

//=================================================================================================
//============================ CProgramParserD3D ==================================================
//=================================================================================================
bool CProgramParserD3D::parseConstantDefinition(CProgramParser::TConstant & constantDef, std::string &errorMess, bool &endEncoutered)
{
	// check if the instruction is 'dcl_xxx vi'
	if (   _CurrChar[0] != 'd' 
		|| _CurrChar[1] != 'e'
		|| _CurrChar[2] != 'f')
	{
		endEncoutered = true;
		return true;
	}
	_CurrChar += 3;

	// parse the constant register index
	skipSpacesAndComments();
	//
	if(*_CurrChar != 'c')
	{
		errorMess = "input register 'c<i>' expected.";
		return false;
	}
	++_CurrChar;

	if(isdigit(*_CurrChar))
	{
		// The constant register is expressed as an index
		uint constIndex = *_CurrChar - '0';
		++_CurrChar;
		if(isdigit(*_CurrChar))
		{
			constIndex =  10 * constIndex + (*_CurrChar - '0');
			++_CurrChar;
		}
		if(constIndex > 95)
		{		
			errorMess = "Invalid index for constant register, must be in [0, 95].";
			return false;
		}
		constantDef.first = constIndex;
	}

	if(*_CurrChar != ',')
	{
		errorMess = "',' expected.";
		return false;
	}
	++_CurrChar;

	// parse the constant register values
	for(uint k=0; k<4; k++)
	{
		skipSpacesAndComments();

		char *currChar = (char *)_CurrChar;
		std::string value = "";
		while(*currChar != ',' && *currChar != ' ' && *currChar != '\n')
		{
			if(*currChar=='-')
			{
				value += *currChar;
				currChar++;
			}
			if(!(isdigit(*currChar) || *currChar=='.'))
			{
				errorMess = "Can't parse value for constant register.";
				return false;
			}

			value += *currChar;
			currChar++;
		}
		
		_CurrChar += value.length();
		constantDef.second.push_back(value);

		skipSpacesAndComments();
		if(k!=3)
		{
			if(*_CurrChar != ',')
			{
				errorMess = "',' expected.";
				return false;
			}
			++_CurrChar;
		}
	}

	skipSpacesAndComments();

	return true;
}

//=================================================================================================
// Skip tabulation and space in a source code
void CProgramParserD3D::skipSpacesAndCommentsD3D()
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
			case '/': // comment go till end of line
				if(_CurrChar[1] == '/')
				{
					while (*_CurrChar != '\n' && *_CurrChar != '\0') ++_CurrChar;
					skipSpacesAndComments();
				}
			break;
			default:
				stop = true;
			break;			
		}
	}
	while (!stop);
}

//=================================================================================================
bool CProgramParserD3D::parse(TConstantsVector & constants, std::string &errorOutput)
{
	std::fill(_RegisterMask, _RegisterMask + 96, 0);
	//
	_LineStart = _CurrChar;
	_LineIndex = 1;
	//
	skipSpacesAndComments(); // in fact space are not allowed at the start of the vertex program

	errorOutput.clear();

	// parse constants definition
	bool endEncoutered = false;
	for(;;)
	{
		CProgramParser::TConstant constantDef;
		if (!parseConstantDefinition(constantDef, errorOutput, endEncoutered)) return false;
		if (endEncoutered) break;	
		constants.push_back(constantDef);
	}	

	return true;	
}

//=================================================================================================
bool CProgramParserD3D::parseInstruction(CProgramInstruction::EOpcode & opcode, std::string & instrStr, 
										 std::string & errorOutput)
{
	instrStr = "";
	uint k = 0;
	while(*_CurrChar!=' ')
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
		instrStr += *_CurrChar;
		++ _CurrChar;
		k++;
	}
	
	if(instrStr == "add")
		opcode = CProgramInstruction::ADD;
	else if(instrStr == "dp3")
		opcode = CProgramInstruction::DP3;
	else if(instrStr == "dp4")
		opcode = CProgramInstruction::DP4;
	else if(instrStr == "exp" || instrStr == "expp")
		opcode = CProgramInstruction::EXP;
	else if(instrStr == "frc")
		opcode = CProgramInstruction::FRC;
	else if(instrStr == "log" || instrStr == "logp")
		opcode = CProgramInstruction::LOG;
	else if(instrStr == "mad")
		opcode = CProgramInstruction::MAD;
	else if(instrStr == "max")
		opcode = CProgramInstruction::MAX;
	else if(instrStr == "min")
		opcode = CProgramInstruction::MIN;
	else if(instrStr == "mov")
		opcode = CProgramInstruction::MOV;
	else if(instrStr == "mul")
		opcode = CProgramInstruction::MUL;
	else if(instrStr == "rcp")
		opcode = CProgramInstruction::RCP;
	else if(instrStr == "rsq")
		opcode = CProgramInstruction::RSQ;
	else if(instrStr == "sub")
		opcode = CProgramInstruction::SUB;
	else if(instrStr == "m3x2")
		opcode = CProgramInstruction::M3X2;
	else if(instrStr == "m3x3")
		opcode = CProgramInstruction::M3X3;
	else if(instrStr == "m3x4")
		opcode = CProgramInstruction::M3X4;
	else if(instrStr == "m4x3")
		opcode = CProgramInstruction::M4X3;
	else if(instrStr == "m4x4")
		opcode = CProgramInstruction::M4X4;
	else
		return false;

	// if the instruction isn't supported in OpengL, replace it with others supported instructions
	if(!replaceCurrentInstruction(opcode, errorOutput)) return false;
	
	return true;
}

//=================================================================================================
bool CProgramParserD3D::replaceCurrentInstruction(CProgramInstruction::EOpcode & opcode, std::string & errorOutput)
{
	if(opcode == CProgramInstruction::M3X2)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'm3x2'";
		return false;
	}
	else if(opcode == CProgramInstruction::M3X3)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'm3x3'";
		return false;
	}
	else if(opcode == CProgramInstruction::M3X4)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'm3x4'";
		return false;
	}
	else if(opcode == CProgramInstruction::M4X3)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'm4x3'";
		return false;
	}
	else if(opcode == CProgramInstruction::M4X4)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'm4x4'";
		return false;
	}

	return true;
}

//=================================================================================================
//============================ CVPParserD3D =======================================================
//=================================================================================================
// Skip tabulation and space in a source code
void CVPParserD3D::skipSpacesAndComments()
{
	CProgramParserD3D::skipSpacesAndCommentsD3D();
}

//=================================================================================================
bool CVPParserD3D::parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput)
{
	skipSpacesAndComments();
	bool result;
	if (outputOperand)
	{
		operand.Negate = false;
		switch(*_CurrChar)
		{			
			case 'o': result = parseOutputRegister(operand, errorOutput); break;
			case 'r': 
				result = parseVariableRegister(operand, errorOutput);							
			break;
			case 'a': result = parseAddressRegister(operand, errorOutput); break;
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
			case 'r': result = parseVariableRegister(operand, errorOutput); break;
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
					case 'r': result = parseVariableRegister(operand, errorOutput); break;
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
						//errorOutput = "Can't read a register component before writing to it.";
						//return false;
					}
				}
			}
		}
		return true;
	}	
}

//=================================================================================================
bool CVPParserD3D::parseInputRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::InputRegister;	
	
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

		if(_UsedInputRegister[index]>=0)
		{
			operand.Value.InputRegisterValue = (CVPOperand::EInputRegister) _UsedInputRegister[index];
		}
		else
		{
			errorOutput = "Unknow input register, not found in semantic declarations.";
			return false;
		}
	}
	else
	{
		errorOutput = "Can't parse index for input register.";
		return false;		
	}
	
	skipSpacesAndComments();

	return true;
}

//=================================================================================================
bool CVPParserD3D::parseOutputRegister(CVPOperand &operand, std::string &errorOutput)
{	
	++_CurrChar;
	operand.Type = CVPOperand::OutputRegister;	
	
	// The input register is expressed as a string
	uint32 strValue = 0;
	// read the 3 letters
	uint k;
	for(k = 0; k < 3; ++k)
	{
		if (!isalnum(*_CurrChar))
		{
			if (k < 2) // at least 2 letter in an instruction
			{
				errorOutput = "Syntax error : can't read opcode.";
				return false;
			}
			else break;	
		}
		strValue |= ((uint32) *_CurrChar) << (8 * (2 - k));
		++_CurrChar;
	}
	if(k != 3)
	{
		strValue |= (uint32) ' ';
	}
	// convert to enum
	switch(strValue)
	{
		case 'Pos': operand.Value.OutputRegisterValue = CVPOperand::OHPosition; break;
		case 'D0 ':	operand.Value.OutputRegisterValue = CVPOperand::OPrimaryColor; break;
		case 'D1 ':	operand.Value.OutputRegisterValue = CVPOperand::OSecondaryColor; break;
		case 'Fog': operand.Value.OutputRegisterValue = CVPOperand::OFogCoord; break;
		case 'Pts': operand.Value.OutputRegisterValue = CVPOperand::OPointSize; break;
		case 'T0 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex0; break;
		case 'T1 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex1; break;
		case 'T2 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex2; break;
		case 'T3 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex3; break;
		case 'T4 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex4; break;
		case 'T5 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex5; break;
		case 'T6 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex6; break;
		case 'T7 ':	operand.Value.OutputRegisterValue = CVPOperand::OTex7; break;
		default:
			errorOutput = "Can't read index for output register.";
			return false;
		break;
	}

	skipSpacesAndComments();

	return true;
}

//=================================================================================================
bool CVPParserD3D::parseConstantRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::Constant;	
	
	uint &index = operand.Value.ConstantValue;
	if (isdigit(*_CurrChar))
	{		
		// immediat case : c0 to c95
		_CurrChar = parseUInt(_CurrChar, index);
		if (index > 95)
		{
			errorOutput = "Constant register index must range from 0 to 95.";
			return false;
		}
		operand.Indexed = false;
	}
	else if (*_CurrChar == '[')
	{
		skipSpacesAndComments();

		// indexed case : c[A0.x + 0] to c[A0.x + 95]
		operand.Indexed = true;
		index = 0;
		if (_CurrChar[1] == 'a'
			&& _CurrChar[2] == '0'
			&& _CurrChar[3] == '.'
			&& _CurrChar[4] == 'x')
		{
			_CurrChar += 5;
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

		skipSpacesAndComments();

		if (*_CurrChar != ']')
		{
			errorOutput = "']' expected when parsing an input register.";
			return false;
		}
		++_CurrChar;
	}

	skipSpacesAndComments();
	
	return true;
}

//=================================================================================================
bool CVPParserD3D::parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered)
{
	skipSpacesAndComments();

	if(std::string(_CurrChar)=="")
	{
		endEncountered = true;
		return true;
	}
	endEncountered = false;

	std::string instrStr;
	if(!CProgramParserD3D::parseInstruction(instr.Opcode.Op, instrStr, errorOutput))
	{
		if(instrStr == "dst")
			instr.Opcode.VPOp = CVPInstruction::DST;
		else if(instrStr == "lit")
			instr.Opcode.VPOp = CVPInstruction::LIT;
		else if(instrStr == "sqe")
			instr.Opcode.VPOp = CVPInstruction::SQE;
		else if(instrStr == "slt")
			instr.Opcode.VPOp = CVPInstruction::SLT;
		else
		{
			errorOutput = "Syntax error : unknow opcode.";
			return false;
		}
	}

	switch(instr.getNumUsedSrc())
	{
	case 1: if (!parseOp2(instr, errorOutput)) return false; break;
	case 2: if (!parseOp3(instr, errorOutput)) return false; break;
	case 3: if (!parseOp4(instr, errorOutput)) return false; break;
	}

	if(instr.Opcode.Op==CProgramInstruction::RSQ
		|| instr.Opcode.Op==CProgramInstruction::EXP
		|| instr.Opcode.Op==CProgramInstruction::LOG
		|| instr.Opcode.Op==CProgramInstruction::RCP)
	{
		if (!instr.Src1.Swizzle.isScalar())
		{
			errorOutput = "this instruction need a scalar src value.";
			return false;
		}
	}

	if (instr.Dest.Type == CVPOperand::Variable)
	{
		_RegisterMask[instr.Dest.Value.VariableValue] |= instr.Dest.WriteMask;
	}

	if (instr.Dest.Type == CVPOperand::AddressRegister)
	{
		errorOutput = "Can't write to address register.";
		return false;
	}

	// parse semi-colon
	skipSpacesAndComments();
	
	return true;
}

//=================================================================================================
bool CVPParserD3D::parse(const char *src, CVProgram &result, std::string &errorOutput)
{
	if (!src) return false;	
	//
	_CurrChar = src;
	//
	skipSpacesAndComments(); // in fact space are not allowed at the start of the vertex program

	// parse version
	if (   _CurrChar[0] != 'v' 
		|| _CurrChar[1] != 's'
		|| _CurrChar[2] != '_'
		|| _CurrChar[3] != '1'
		|| _CurrChar[4] != '_'
		|| _CurrChar[5] != '1')
	{
		errorOutput = "Can't parse version.";
		return false;
	}
	_CurrChar += 6;

	// parse constants 
	std::string errorMess;
	if(!CProgramParserD3D::parse(result._Constants, errorMess))
	{
		errorOutput = std::string("CVPParserD3D::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
		return false;
	}

	// parse semantic declarations
	bool endEncoutered = false;
	std::fill(_UsedInputRegister, _UsedInputRegister + 16, -1);
	for(;;)
	{
		if(!parseSemanticDeclaration(errorMess, endEncoutered))
		{
			errorOutput = std::string("CVPParserD3D::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
			return false;
		}
		if (endEncoutered) break;	
	}	

	// parse instructions
	endEncoutered = false;
	for(;;)
	{
		CVPInstruction instr;	
		if (!parseInstruction(instr, errorMess, endEncoutered))
		{
			errorOutput = std::string("CVPParserD3D::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
			return false;
		}
		if (endEncoutered) break;		
		result._Program.push_back(instr);
	}	
	return true;	
}

//=================================================================================================
bool CVPParserD3D::parseSemanticDeclaration(std::string &errorOutput, bool &endEncountered)
{
	CVPOperand::EInputRegister inputRegister;
	uint inputRegIndex;

	skipSpacesAndComments();

	// check if the instruction is 'dcl_xxx vi'
	if (   _CurrChar[0] != 'd' 
		|| _CurrChar[1] != 'c'
		|| _CurrChar[2] != 'l'
		|| _CurrChar[3] != '_')
	{
		endEncountered = true;
		return true;
	}
	_CurrChar += 4;

	// search for semantic word
	std::string semantic;
	while(*_CurrChar != ' ')
	{
		semantic += *_CurrChar;
		_CurrChar++;
	}

	if(semantic=="position")
		inputRegister = CVPOperand::IPosition;
	else if(semantic=="normal")
		inputRegister = CVPOperand::INormal;
	else if(semantic=="color0")
		inputRegister = CVPOperand::IPrimaryColor;
	else if(semantic=="color1")
		inputRegister = CVPOperand::ISecondaryColor;
	else if(semantic=="fog")
		inputRegister = CVPOperand::IFogCoord;
	else if(semantic=="texcoord" || semantic=="texcoord0")
		inputRegister = CVPOperand::ITex0;
	else if(semantic=="texcoord1")
		inputRegister = CVPOperand::ITex1;
	else if(semantic=="texcoord2")
		inputRegister = CVPOperand::ITex2;
	else if(semantic=="texcoord3")
		inputRegister = CVPOperand::ITex3;
	else if(semantic=="texcoord4")
		inputRegister = CVPOperand::ITex4;
	else if(semantic=="texcoord5")
		inputRegister = CVPOperand::ITex5;
	else if(semantic=="texcoord6")
		inputRegister = CVPOperand::ITex6;
	else if(semantic=="texcoord7")
		inputRegister = CVPOperand::ITex7;
	else
	{
		errorOutput = "Syntax error : unknow dcl_usage";
		return false;
	}

	// parse the input register index
	skipSpacesAndComments();
	//
	if (*_CurrChar != 'v')
	{
		errorOutput = "input register 'v<i>' expected.";
		return false;
	}
	++_CurrChar;

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
		inputRegIndex = index;
	}
	else
	{
		errorOutput = "index for input register expeceted.";
		return false;
	}

	_UsedInputRegister[inputRegIndex] = (sint)inputRegister;

	return true;
}

//=================================================================================================
//============================ CPPParserD3D =======================================================
//=================================================================================================
// Skip tabulation and space in a source code
void CPPParserD3D::skipSpacesAndComments()
{
	CProgramParserD3D::skipSpacesAndCommentsD3D();
}

//=================================================================================================
bool CPPParserD3D::parseOperand(CPPOperand &operand, bool outputOperand, std::string &errorOutput)
{
	skipSpacesAndComments();
	bool result;
	if (outputOperand)
	{
		operand.Negate = false;
		switch(*_CurrChar)
		{			
			case 'o': result = parseOutputRegister(operand, errorOutput); break;
			case 'r': 
				result = parseVariableRegister(operand, errorOutput);							
			break;
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
			case 't':
			case 'v': result = parseInputRegister(operand, errorOutput); break;
			case 'r': result = parseVariableRegister(operand, errorOutput); break;
			case 'i': 
			case 'b': 
			case 'c': result = parseConstantRegister(operand, errorOutput); break;
			case 's': result = parseSamplerRegister(operand, errorOutput); break;
			case '-':
			{
				operand.Negate = true;
				// negation
				++ _CurrChar;
				skipSpacesAndComments();
				switch(*_CurrChar)
				{
					case 't':
					case 'v': result = parseInputRegister(operand, errorOutput); break;
					case 'r': result = parseVariableRegister(operand, errorOutput); break;
					case 'i': 
					case 'b': 
					case 'c': result = parseConstantRegister(operand, errorOutput); break;
					case 's': result = parseSamplerRegister(operand, errorOutput); break;
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
		
		if (!parseSwizzle(operand.Swizzle, errorOutput)) return false;		
		if (operand.Type == CPPOperand::Variable)
		{
			for(uint k = 0; k < 4; ++k)
			{
				if (!(_RegisterMask[operand.Value.VariableValue] & (1 << operand.Swizzle.Comp[k])))
				{
					//errorOutput = "Can't read a register component before writing to it.";
					//return false;
				}
			}
		}
		
		return true;
	}	
}

//=================================================================================================
bool CPPParserD3D::parseInputRegister(CPPOperand &operand, std::string &errorOutput)
{
	char inputRegType = *_CurrChar;
	++_CurrChar;

	operand.Type = CPPOperand::InputRegister;
	
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

		if(inputRegType=='v' && _UsedInputColorRegister[index]>=0)
		{
			operand.Value.InputRegisterValue = (CPPOperand::EInputRegister) _UsedInputColorRegister[index];
		}
		else if(inputRegType=='t' && _UsedInputTexCoordRegister[index]>=0)
		{
			operand.Value.InputRegisterValue = (CPPOperand::EInputRegister) _UsedInputTexCoordRegister[index];
		}
		else
		{
			errorOutput = "Unknow input register, not found in semantic declarations.";
			return false;
		}
	}
	else
	{
		errorOutput = "Can't parse index for input register.";
		return false;		
	}
	
	skipSpacesAndComments();

	return true;
}

//=================================================================================================
bool CPPParserD3D::parseOutputRegister(CPPOperand &operand, std::string &errorOutput)
{	
	++_CurrChar;
	operand.Type = CPPOperand::OutputRegister;	

	// search for output register type
	uint outputRegSize = 0;
	char *currChar = (char *)_CurrChar;
	while(*currChar != ' ' && *currChar != ',')
	{
		outputRegSize++;
		currChar++;
	}
	
	char outputReg[20];
	memcpy(outputReg, _CurrChar, outputRegSize);
	_CurrChar += outputRegSize;

	if(!strncmp(outputReg, "C0", outputRegSize))
	{
		operand.Value.OutputRegisterValue = CPPOperand::OColor; 
	}
	else if(!strncmp(outputReg, "Depth", outputRegSize))
	{
		operand.Value.OutputRegisterValue = CPPOperand::ODepth;
	}
	else
	{
		errorOutput = "Can't read or support type for output register.";
		return false;
	}

	skipSpacesAndComments();

	return true;
}

//=================================================================================================
bool CPPParserD3D::parseConstantRegister(CPPOperand &operand, std::string &errorOutput)
{
	operand.Type = CPPOperand::Constant;
	++_CurrChar;
		
	uint &index = operand.Value.ConstantValue;
	if (isdigit(*_CurrChar))
	{		
		// immediat case : c0 to c95
		_CurrChar = parseUInt(_CurrChar, index);
		if (index > 95)
		{
			errorOutput = "Constant register index must range from 0 to 95.";
			return false;
		}
		operand.Indexed = false;
	}
	else
	{
		errorOutput = "Constant register index is expected.";
		return false;
	}

	skipSpacesAndComments();
	
	return true;
}

//=================================================================================================
bool CPPParserD3D::parseInstruction(CPPInstruction &instr, std::string &errorOutput, bool &endEncountered)
{
	skipSpacesAndComments();

	if(std::string(_CurrChar)=="")
	{
		endEncountered = true;
		return true;
	}
	endEncountered = false;

	std::string instrStr;
	if(!CProgramParserD3D::parseInstruction(instr.Opcode.Op, instrStr, errorOutput))
	{
		if(instrStr == "abs")
			instr.Opcode.PPOp = CPPInstruction::ABS;
		else if(instrStr == "cmp")
			instr.Opcode.PPOp = CPPInstruction::CMP;
		else if(instrStr == "dp2add")
			instr.Opcode.PPOp = CPPInstruction::DP2ADD;
		else if(instrStr == "lrp")
			instr.Opcode.PPOp = CPPInstruction::LRP;
		else if(instrStr == "nrm")
			instr.Opcode.PPOp = CPPInstruction::NRM;
		else if(instrStr == "pow")
			instr.Opcode.PPOp = CPPInstruction::POW;
		else if(instrStr == "sin_cos")
			instr.Opcode.PPOp = CPPInstruction::SINCOS;
		else if(instrStr == "texld")
			instr.Opcode.PPOp = CPPInstruction::TEX;
		else if(instrStr == "texldb")
			instr.Opcode.PPOp = CPPInstruction::TEXB;
		else if(instrStr == "texldp")
			instr.Opcode.PPOp = CPPInstruction::TEXP;
		else
		{
			errorOutput = "Syntax error : unknow opcode.";
			return false;
		}

		if(!replaceCurrentInstruction(instr, errorOutput)) return false;
	}

	// parse saturation
	instrStr = "";
	while(*_CurrChar != ' ')
	{
		instrStr += *_CurrChar;
		_CurrChar++;
	}
	instr.Sat = (instrStr=="_sat");
	
	// parse operands
	switch(instr.getNumUsedSrc())
	{
	case 1: if (!parseOp2(instr, errorOutput)) return false; break;
	case 2: if (!parseOp3(instr, errorOutput)) return false; break;
	case 3: if (!parseOp4(instr, errorOutput)) return false; break;
	}

	if(instr.Opcode.Op==CProgramInstruction::RSQ
		|| instr.Opcode.Op==CProgramInstruction::EXP
		|| instr.Opcode.Op==CProgramInstruction::LOG
		|| instr.Opcode.Op==CProgramInstruction::RCP)
	{
		if (!instr.Src1.Swizzle.isScalar())
		{
			errorOutput = "this instruction need a scalar src value.";
			return false;
		}
	}

	if (instr.Dest.Type == CPPOperand::Variable)
	{
		_RegisterMask[instr.Dest.Value.VariableValue] |= instr.Dest.WriteMask;
	}

	// in OpenGL, the order of operands is inversed
	if(instr.Opcode.PPOp==CPPInstruction::CMP)
	{
		CPPOperand tempOperand = instr.Src2;
		instr.Src2 = instr.Src1;
		instr.Src1 = tempOperand;
	}

	// parse semi-colon
	skipSpacesAndComments();
	
	return true;
}

//=================================================================================================
bool CPPParserD3D::replaceCurrentInstruction(CPPInstruction & instr, std::string & errorOutput)
{
	if(instr.Opcode.PPOp == CPPInstruction::NRM)
	{
		skipSpacesAndCommentsD3D();

		// parse dest and src1
		std::string dest, destSwizzle, src1;
		while(*_CurrChar!=' ' && *_CurrChar!=',' && *_CurrChar!='.')
		{
			dest += *_CurrChar;
			_CurrChar++;
		}

		// parse swizzle
		while(*_CurrChar!=' ' && *_CurrChar!=',')
		{
			destSwizzle += *_CurrChar;
			_CurrChar++;
		}

		skipSpacesAndCommentsD3D();
		if(*_CurrChar!=',')
		{
			errorOutput += "expected ','";
			return false;
		}

		_CurrChar++;
		skipSpacesAndCommentsD3D();

		while(*_CurrChar!=' ' && *_CurrChar!='.' && *_CurrChar!='\n')
		{
			src1 += *_CurrChar;
			_CurrChar++;
		}

		skipSpacesAndCommentsD3D();

		// create new D3D code
		std::string currentCode = _CurrChar;
		replaceCode.clear();
		replaceCode = " " + dest + ".x, " + src1 + ", " + src1 + "\n";
		replaceCode += "rsq " + dest + ".x, " + dest + ".x\n";
		replaceCode += "mul " + dest + destSwizzle + ", " + dest + ".x, " + src1 + "\n";
		replaceCode += currentCode;

		_CurrChar = replaceCode.c_str();

		instr.Opcode.Op = CProgramInstruction::DP3;
	}
	else if(instr.Opcode.PPOp == CPPInstruction::DP2ADD)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'dp2add'";
		return false;
	}
	else if(instr.Opcode.PPOp == CPPInstruction::SINCOS)
	{
		//TO DO
		errorOutput += "TODO remplacement of 'sin_cos'";
		return false;
	}

	return true;
}

//=================================================================================================
bool CPPParserD3D::parse(const char *src, CPProgram &result, std::string &errorOutput)
{
	if (!src) return false;	
	//
	_CurrChar = src;
	//
	skipSpacesAndComments(); // in fact space are not allowed at the start of the vertex program

	// parse version
	if (   _CurrChar[0] != 'p' 
		|| _CurrChar[1] != 's'
		|| _CurrChar[2] != '_'
		|| _CurrChar[3] != '2'
		|| _CurrChar[4] != '_'
		|| _CurrChar[5] != '0')
	{
		errorOutput = "Can't parse version.";
		return false;
	}
	_CurrChar += 6;

	// parse constants 
	std::string errorMess;
	if(!CProgramParserD3D::parse(result._Constants, errorMess))
	{
		errorOutput = std::string("CVPParserD3D::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
		return false;
	}

	// parse semantic declarations
	bool endEncoutered = false;
	std::fill(_UsedInputColorRegister, _UsedInputColorRegister + 2, -1);
	std::fill(_UsedInputTexCoordRegister, _UsedInputTexCoordRegister + 16, -1);
	std::fill(_UsedInputSamplerRegister, _UsedInputSamplerRegister + 16, -1);
	for(;;)
	{
		if (!parseSemanticDeclaration(errorMess, endEncoutered))
		{
			errorOutput = std::string("CPPParserD3D::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
			return false;
		}
		if (endEncoutered) break;
	}	

	// parse instructions
	endEncoutered = false;
	for(;;)
	{
		CPPInstruction instr;	
		if (!parseInstruction(instr, errorMess, endEncoutered))
		{
			errorOutput = std::string("CPPParserD3D::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
			return false;
		}
		if (endEncoutered) break;
		
		result._Program.push_back(instr);
	}	
	return true;	
}

//=================================================================================================
bool CPPParserD3D::parseSemanticDeclaration(std::string &errorOutput, bool &endEncountered)
{
	//CPPOperand::EInputRegister inputRegister;
	bool isSampler = false;
	CPPOperand::EOperandType samplerType;

	skipSpacesAndComments();

	// check if the instruction is 'dcl_xxx vi'
	if (   _CurrChar[0] != 'd' 
		|| _CurrChar[1] != 'c'
		|| _CurrChar[2] != 'l')
	{
		endEncountered = true;
		return true;
	}
	_CurrChar += 3;

	if(*_CurrChar!=' ')
	{
		// search for sampler type
		uint semanticSize = 0;
		char *currChar = (char *)_CurrChar;
		while(*currChar != ' ')
		{
			semanticSize++;
			currChar++;
		}
		
		char semantic[20];
		memcpy(semantic, _CurrChar, semanticSize);
		_CurrChar += semanticSize;

		if(    !strncmp(semantic, "_2d", semanticSize) 
			|| !strncmp(semantic, "_cube", semanticSize) 
			|| !strncmp(semantic, "_volume", semanticSize))
		{
			isSampler = true;
			if(!strncmp(semantic, "_2d", semanticSize))
				samplerType = CPPOperand::Sampler2DRegister;
			else
				samplerType = CPPOperand::Sampler3DRegister;
		}
	}
	
	// pass Partial Precision
	while(*_CurrChar != ' ')
	{
		_CurrChar++;
	}

	skipSpacesAndComments();

	if(*_CurrChar=='v' || *_CurrChar=='t' || (*_CurrChar=='s' && samplerType))
	{
		uint inputRegIndex;
		char typeInput = *_CurrChar;
		_CurrChar++;

		if(isdigit(*_CurrChar))
		{
			// The input register is expressed as an index
			uint index = *_CurrChar - '0';
			++_CurrChar;
			if(isdigit(*_CurrChar))
			{
				index =  10 * index + (*_CurrChar - '0');
				++_CurrChar;
			}

			inputRegIndex = index;
		}
		else
		{
			errorOutput = "index for input register expeceted.";
			return false;
		}

		if(typeInput=='v')
		{
			if(inputRegIndex > 1)
			{		
				errorOutput = "Invalid index for input color register, must be in [0, 1].";
				return false;
			}
			_UsedInputColorRegister[inputRegIndex] = (sint)(CPPOperand::IPrimaryColor + inputRegIndex);
		}
		else if(typeInput=='t')
		{
			if(inputRegIndex > 7)
			{		
				errorOutput = "Invalid index for input texture coordinate register, must be in [0, 7].";
				return false;
			}
			_UsedInputTexCoordRegister[inputRegIndex] = (sint)(CPPOperand::ITex0 + inputRegIndex);
		}
		else if(typeInput=='s')
		{
			if(inputRegIndex > 15)
			{		
				errorOutput = "Invalid index for input samplerregister, must be in [0, 15].";
				return false;
			}
			_UsedInputSamplerRegister[inputRegIndex] = (sint)samplerType;
		}

		// pass mask
		while(*_CurrChar != '\n')
		{
			_CurrChar++;
		}
	}
	else
	{
		errorOutput = "Unknown semantic declaration.";
		return false;
	}

	return true;
}

//=================================================================================================
bool CPPParserD3D::parseSamplerRegister(CPPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;

	if (!isdigit(*_CurrChar))
	{
		errorOutput = "Can't parse sampler register.";
		return false;
	}

	uint & index = operand.Value.SamplerValue;
	_CurrChar = parseUInt(_CurrChar, index);
	if (index > 15)
	{
		errorOutput = "Sampler register index must range from 0 to 15.";
		return false;
	}

	if(_UsedInputSamplerRegister[index]>=0)
		operand.Type = CPPOperand::EOperandType(_UsedInputSamplerRegister[index]);

	return true;
}


