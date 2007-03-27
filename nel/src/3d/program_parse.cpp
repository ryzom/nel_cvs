/** \file vertex_program_parse.cpp
 *
 * $Id: program_parse.cpp,v 1.1.2.1 2007/03/27 14:01:46 legallo Exp $
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
#include "program_parse.h"


//=================================================================================================
//================================= CProgramInstruction ===========================================
//=================================================================================================
uint CProgramInstruction::getNumUsedSrc(EOpcode opcode) const
{
	switch(opcode)
	{
		case CProgramInstruction::RSQ:						
		case CProgramInstruction::LOG:			
		case CProgramInstruction::RCP:			
		case CProgramInstruction::MOV:			
		case CProgramInstruction::FRC:
		case CProgramInstruction::EXP:
			return 1;
		//
		case CProgramInstruction::MAD:
			return 3;
		//
		case CProgramInstruction::MUL:
		case CProgramInstruction::ADD:
		case CProgramInstruction::DP3:
		case CProgramInstruction::DP4:			
		case CProgramInstruction::MIN:
		case CProgramInstruction::MAX:	
		case CProgramInstruction::SUB:
		case CProgramInstruction::M3X2:
		case CProgramInstruction::M3X3:
		case CProgramInstruction::M3X4:
		case CProgramInstruction::M4X3:
		case CProgramInstruction::M4X4:
			return 2;
		//
		default:
			nlstop;		
	}
	return 0;
}

//=================================================================================================
uint CVPInstruction::getNumUsedSrc() const
{
	switch(Opcode.VPOp)
	{
		case CVPInstruction::LIT:
		case CVPInstruction::ARL:
			return 1;
		//
		case CVPInstruction::MAD:
			return 3;
		//
		case CVPInstruction::DST:
		case CVPInstruction::SQE:
		case CVPInstruction::SLT:
			return 2;
		//
		default:
			return CProgramInstruction::getNumUsedSrc(Opcode.Op);
	}
	return 0;
}

//=================================================================================================
uint CPPInstruction::getNumUsedSrc() const
{
	switch(Opcode.PPOp)
	{
		case CPPInstruction::ABS:
		case CPPInstruction::NRM:
			return 1;
		//
		case CPPInstruction::CMP:
		case CPPInstruction::DP2ADD:
		case CPPInstruction::LRP:
		case CPPInstruction::SINCOS:
			return 3;
		//
		case CPPInstruction::CRS:
		case CPPInstruction::POW:
		case CPPInstruction::TEX:
		case CPPInstruction::TEXB:
		case CPPInstruction::TEXP:
			return 2;
		//
		default:
			return CProgramInstruction::getNumUsedSrc(Opcode.Op);
	}
	return 0;
}

//=================================================================================================
//================================== CProgramParser ===============================================
//=================================================================================================

bool CProgramParser::parseWriteMask(uint &mask, std::string &errorOutput)
{	
	// parse output mask
	if (*_CurrChar != '.')
	{
		// no output masks
		mask = 0xf; //output 4 coordinates
		return true;
	}
	else
	{
		++ _CurrChar;
		mask = 0;
		for(uint k = 0; k < 4; ++k)
		{
			uint maskIndex;
			switch(*_CurrChar)
			{
				case 'x': maskIndex = 0; break;
				case 'y': maskIndex = 1; break;
				case 'z': maskIndex = 2; break;
				case 'w': maskIndex = 3; break;
				default:
					if (k >= 1) return true;
					else
					{					
						errorOutput = "Can't parse output mask.";
						return false;
					}
				break;
			}
			++_CurrChar;
			if (mask & (1 << maskIndex))
			{
				errorOutput = "Duplicated output mask component.";
				return false;
			}
			mask |= 1 << maskIndex;
		}
		return true;
	}
}

//=================================================================================================
bool CProgramParser::parseSwizzle(CProgramSwizzle &swizzle,std::string &errorOutput)
{
	if (*_CurrChar != '.')
	{
		// no swizzle
		swizzle.Comp[0] = CProgramSwizzle::X;	
		swizzle.Comp[1] = CProgramSwizzle::Y;
		swizzle.Comp[2] = CProgramSwizzle::Z;
		swizzle.Comp[3] = CProgramSwizzle::W;
		return true;
	}	
	++_CurrChar;
	// 4 letters case
	for(uint k = 0; k < 4; ++k)
	{
		if (!isalpha(*_CurrChar))
		{
			if (k == 1) // 1 letter case
			{
				switch(*_CurrChar)
				{
					case ',':
					case ';':
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case '#':
						swizzle.Comp[1] = swizzle.Comp[2] = swizzle.Comp[3] = swizzle.Comp[0];
						return true;
					break;
					default:
						errorOutput = "Can't parse swizzle.";

				}
			}
			else
			{
				errorOutput = "Invalid swizzle value.";
				return false;
			}			
		}
		
		if (!letterToSwizzleComp(*_CurrChar, swizzle.Comp[k]))
		{	
			errorOutput = "Invalid swizzle value.";
			return false;
		}
		++ _CurrChar;
	}
	
	return true;
}

//=================================================================================================
bool CProgramParser::parseVariableRegister(uint &index, std::string &errorOutput)
{
	++_CurrChar;
	if (!isdigit(*_CurrChar))
	{
		errorOutput = "Can't parse variable register.";
		return false;
	}

	_CurrChar = parseUInt(_CurrChar, index);
	if (index > 11)
	{
		errorOutput = "Variable register index must range from 0 to 11.";
		return false;
	}
	return true;
}


//=================================================================================================
//================================== CVertexProgramParser =========================================
//=================================================================================================

bool CVertexProgramParser::parseVariableRegister(CVPOperand &operand, std::string &errorOutput)
{
	operand.Type = CVPOperand::Variable;	
	return CProgramParser::parseVariableRegister(operand.Value.VariableValue, errorOutput);
}

//=================================================================================================
bool CVertexProgramParser::parseAddressRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::AddressRegister;
	if (_CurrChar[0] != '0' || _CurrChar[1] != '.' || _CurrChar[2] != 'x')
	{
		errorOutput = "Can't parse address register.";
		return false;
	}
	_CurrChar += 3;
	return true;	
}

//=================================================================================================
bool CVertexProgramParser::parseOp2(CVPInstruction &instr,std::string &errorOutput)
{
	skipSpacesAndComments();
	// parse ouput
	if (!parseOperand(instr.Dest, true, errorOutput)) return false;
	// Can't write in input or consant register
	if (instr.Dest.Type == CVPOperand::Constant || instr.Dest.Type == CVPOperand::InputRegister)
	{
		errorOutput = "Can't write to a constant or input register";
		return false;
	}
	//
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src1
	if (!parseOperand(instr.Src1, false, errorOutput)) return false;
	if (instr.Src1.Type == CVPOperand::AddressRegister 
		|| instr.Src1.Type == CVPOperand::OutputRegister)
	{
		errorOutput = "Src1 must be constant, variable, or input register.";
		return false;
	}
	return true;
}

//=================================================================================================
bool CVertexProgramParser::parseOp3(CVPInstruction &instr, std::string &errorOutput)
{	
	if (!parseOp2(instr, errorOutput)) return false;
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src2
	if (!parseOperand(instr.Src2, false, errorOutput)) return false;
	if (instr.Src2.Type == CVPOperand::AddressRegister 
		|| instr.Src2.Type == CVPOperand::OutputRegister)
	{
		errorOutput = "Src2 must be constant, variable, or input register.";
		return false;
	}
	// make sure we do not have 2 =/= contant register as src (or in put register)

	// 2 constant registers ?
	if (instr.Src1.Type == CVPOperand::Constant
		&& instr.Src2.Type == CVPOperand::Constant)
	{
		// the index must be the same
		if (!
			(
				instr.Src1.Indexed == instr.Src2.Indexed
			 && instr.Src1.Value.ConstantValue == instr.Src2.Value.ConstantValue
			)
		   )
		{
			errorOutput = "Can't read 2 different constant registers in a single instruction.";
			return false;
		}			
	}

	// 2 input registers ?
	if (instr.Src1.Type == CVPOperand::InputRegister
		&& instr.Src2.Type == CVPOperand::InputRegister)
	{
		// the index must be the same
		if (instr.Src1.Value.InputRegisterValue != instr.Src2.Value.InputRegisterValue)
		{
			errorOutput = "Can't read 2 different input registers in a single instruction.";
			return false;
		}			
	}
	return true;
}

//=================================================================================================
bool CVertexProgramParser::parseOp4(CVPInstruction &instr, std::string &errorOutput)
{
	if (!parseOp3(instr, errorOutput)) return false;
	// parse src 3
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src4
	if (!parseOperand(instr.Src3, false, errorOutput)) return false;
	if (instr.Src3.Type == CVPOperand::AddressRegister 
		|| instr.Src3.Type == CVPOperand::OutputRegister)
	{
		errorOutput = "Src3 must be constant, variable, or input register.";
		return false;
	}
	
	///////////////////////////////////////////////////
	// check for different contant / input registers //
	///////////////////////////////////////////////////

	// Duplicated constant register
	if (instr.Src3.Type == CVPOperand::Constant)
	{
		if (instr.Src1.Type == CVPOperand::Constant)
		{
			if (!
			    (
				    instr.Src1.Indexed == instr.Src3.Indexed
			     && instr.Src1.Value.ConstantValue == instr.Src3.Value.ConstantValue
			    )
		       )
			{
				errorOutput = "Can't read 2 different constant registers in a single instruction.";
				return false;
			}
		}
		if (instr.Src2.Type == CVPOperand::Constant)
		{
			if (!
			    (
				    instr.Src2.Indexed == instr.Src3.Indexed
			     && instr.Src2.Value.ConstantValue == instr.Src3.Value.ConstantValue
			    )
		       )
			{
				errorOutput = "Can't read 2 different constant registers in a single instruction.";
				return false;
			}
		}
	}

	// Duplicated input register
	if (instr.Src3.Type == CVPOperand::InputRegister)
	{
		if (instr.Src1.Type == CVPOperand::InputRegister)
		{			
			if (instr.Src1.Value.InputRegisterValue != instr.Src3.Value.InputRegisterValue)
			{
				errorOutput = "Can't read 2 different input registers in a single instruction.";
				return false;
			}
		}
		if (instr.Src2.Type == CVPOperand::InputRegister)
		{
			if (instr.Src2.Value.InputRegisterValue != instr.Src3.Value.InputRegisterValue)
			{
				errorOutput = "Can't read 2 different input registers in a single instruction.";
				return false;
			}
		}
	}

	return true;
}


//=================================================================================================
//================================== CPixelProgramParser ==========================================
//=================================================================================================
uint CPixelProgramParser::CPProgram::getUsedVariablesNb() const
{
	sint varNb = -1;
	for(uint i=0; i<_Program.size(); i++)
	{
		const CPPOperand & op = _Program[i].Dest;
		if(op.Type == CPPOperand::Variable)
		{
			if((sint)op.Value.VariableValue>varNb)
				varNb = op.Value.VariableValue; 
		}
	}

	return (uint)(varNb+1);
}

//=================================================================================================
bool CPixelProgramParser::parseVariableRegister(CPPOperand &operand, std::string &errorOutput)
{
	operand.Type = CPPOperand::Variable;
	return CProgramParser::parseVariableRegister(operand.Value.VariableValue, errorOutput);
}

//=================================================================================================
bool CPixelProgramParser::parseOp2(CPPInstruction &instr,std::string &errorOutput)
{
	skipSpacesAndComments();
	// parse ouput
	if (!parseOperand(instr.Dest, true, errorOutput)) return false;
	// Can't write in input or consant register
	if (instr.Dest.Type == CPPOperand::Constant || instr.Dest.Type == CPPOperand::InputRegister)
	{
		errorOutput = "Can't write to a constant or input register";
		return false;
	}
	//
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src1
	if (!parseOperand(instr.Src1, false, errorOutput)) return false;
	if (instr.Src1.Type == CPPOperand::OutputRegister)
	{
		errorOutput = "Src1 must be constant, variable, or input register.";
		return false;
	}
	return true;
}

//=================================================================================================
bool CPixelProgramParser::parseOp3(CPPInstruction &instr, std::string &errorOutput)
{	
	if (!parseOp2(instr, errorOutput)) return false;
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src2
	if (!parseOperand(instr.Src2, false, errorOutput)) return false;
	if (instr.Src2.Type == CPPOperand::OutputRegister)
	{
		errorOutput = "Src2 must be constant, variable, or input register.";
		return false;
	}
	// make sure we do not have 2 =/= contant register as src (or in put register)

	// 2 constant registers ?
	if (instr.Src1.Type == CPPOperand::Constant
		&& instr.Src2.Type == CPPOperand::Constant)
	{
		// the index must be the same
		if (!
			(
				instr.Src1.Indexed == instr.Src2.Indexed
			 && instr.Src1.Value.ConstantValue == instr.Src2.Value.ConstantValue
			)
		   )
		{
			errorOutput = "Can't read 2 different constant registers in a single instruction.";
			return false;
		}			
	}

	// 2 input registers ?
	if (instr.Src1.Type == CPPOperand::InputRegister
		&& instr.Src2.Type == CPPOperand::InputRegister)
	{
		// the index must be the same
		if (instr.Src1.Value.InputRegisterValue != instr.Src2.Value.InputRegisterValue)
		{
			errorOutput = "Can't read 2 different input registers in a single instruction.";
			return false;
		}			
	}
	return true;
}

//=================================================================================================
bool CPixelProgramParser::parseOp4(CPPInstruction &instr, std::string &errorOutput)
{
	if (!parseOp3(instr, errorOutput)) return false;
	// parse src 3
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src4
	if (!parseOperand(instr.Src3, false, errorOutput)) return false;
	if (instr.Src3.Type == CPPOperand::OutputRegister)
	{
		errorOutput = "Src3 must be constant, variable, or input register.";
		return false;
	}
	
	///////////////////////////////////////////////////
	// check for different contant / input registers //
	///////////////////////////////////////////////////

	// Duplicated constant register
	if (instr.Src3.Type == CPPOperand::Constant)
	{
		if (instr.Src1.Type == CPPOperand::Constant)
		{
			if (!
			    (
				    instr.Src1.Indexed == instr.Src3.Indexed
			     && instr.Src1.Value.ConstantValue == instr.Src3.Value.ConstantValue
			    )
		       )
			{
				errorOutput = "Can't read 2 different constant registers in a single instruction.";
				return false;
			}
		}
		if (instr.Src2.Type == CPPOperand::Constant)
		{
			if (!
			    (
				    instr.Src2.Indexed == instr.Src3.Indexed
			     && instr.Src2.Value.ConstantValue == instr.Src3.Value.ConstantValue
			    )
		       )
			{
				errorOutput = "Can't read 2 different constant registers in a single instruction.";
				return false;
			}
		}
	}

	// Duplicated input register
	if (instr.Src3.Type == CPPOperand::InputRegister)
	{
		if (instr.Src1.Type == CPPOperand::InputRegister)
		{			
			if (instr.Src1.Value.InputRegisterValue != instr.Src3.Value.InputRegisterValue)
			{
				errorOutput = "Can't read 2 different input registers in a single instruction.";
				return false;
			}
		}
		if (instr.Src2.Type == CPPOperand::InputRegister)
		{
			if (instr.Src2.Value.InputRegisterValue != instr.Src3.Value.InputRegisterValue)
			{
				errorOutput = "Can't read 2 different input registers in a single instruction.";
				return false;
			}
		}
	}

	return true;
}
