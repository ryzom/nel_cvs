/** \file vertex_program_parse.h
 * These are a set of classes used to parse a vertex program in proprietary format. This is used when
 * parsing isn't available in the target API (for example, missing OpenGL extension)
 * $Id: program_parse_D3D.h,v 1.1.2.1 2007/03/27 14:01:47 legallo Exp $
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



#ifndef NL_PROGRAM_PARSE_D3D_H
#define NL_PROGRAM_PARSE_D3D_H

#include <vector>
#include "program_parse.h"

//=================================================================================================
//=================================== CProgramParserD3D ===========================================
//=================================================================================================
class CProgramParserD3D : virtual public CProgramParser
{

protected:

	virtual bool parseConstantDefinition(CProgramParser::TConstant & constantDef, std::string &errorMess, bool &endEncoutered);
	void skipSpacesAndCommentsD3D();
	bool parse(TConstantsVector & constants, std::string &errorOutput);
	bool parseInstruction(/*CProgramInstruction &instr*/CProgramInstruction::EOpcode & opcode, 
		std::string & instrStr, std::string & errorOutput);

	bool replaceCurrentInstruction(/*CProgramInstruction & instr*/CProgramInstruction::EOpcode & opcode, std::string & errorOutput);

	std::string replaceCode;
};

//=================================================================================================
//=================================== CVPParserD3D ================================================
//=================================================================================================
/** A vertex program parser for ASM vs1.1.
  * \author Nevrax France
  * \date 2007
  */
class CVPParserD3D : public CVertexProgramParser, public CProgramParserD3D
{
public:
	
	virtual bool parse(const char *src, CVProgram &result, std::string &errorOutput);

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
protected:
	sint		_UsedInputRegister[16]; // which input register are used

protected:
	virtual bool parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput);
	//
	virtual bool parseInputRegister(CVPOperand &operand, std::string &errorOutput);
	virtual bool parseOutputRegister(CVPOperand &operand, std::string &errorOutput);
	virtual bool parseConstantRegister(CVPOperand &operand, std::string &errorOutput);
	//
	virtual bool parseSemanticDeclaration(std::string &errorOutput, bool &endEncountered);
	virtual bool parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered);
	// skip spaces and count lines
	virtual void skipSpacesAndComments();
};

//=================================================================================================
//=================================== CPPParserD3D ================================================
//=================================================================================================
/** A pixel program parser for ASM ps.2.0.
  * \author Nevrax France
  * \date 2007
  */
class CPPParserD3D : public CPixelProgramParser, public CProgramParserD3D
{
public:
	
	virtual bool parse(const char *src, CPProgram &result, std::string &errorOutput);

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
protected:
	sint		_UsedInputColorRegister[2]; // which input color register are used
	sint		_UsedInputTexCoordRegister[16]; // which input texture coordinates register are used
	sint		_UsedInputSamplerRegister[16]; // which input sampler register are used

protected:
	virtual bool parseOperand(CPPOperand &operand, bool outputOperand, std::string &errorOutput);
	//
	virtual bool parseInputRegister(CPPOperand &operand, std::string &errorOutput);
	virtual bool parseOutputRegister(CPPOperand &operand, std::string &errorOutput);
	virtual bool parseConstantRegister(CPPOperand &operand, std::string &errorOutput);
	virtual bool parseSamplerRegister(CPPOperand &operand, std::string &errorOutput);
	//
	virtual bool parseSemanticDeclaration(std::string &errorOutput, bool &endEncountered);
	virtual bool parseInstruction(CPPInstruction &instr, std::string &errorOutput, bool &endEncountered);
	// skip spaces and count lines
	virtual void skipSpacesAndComments();

	bool replaceCurrentInstruction(CPPInstruction & instr, std::string & errorOutput);
};

#endif

