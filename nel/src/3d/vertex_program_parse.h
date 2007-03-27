/** \file vertex_program_parse.h
 * These are a set of classes used to parse a vertex program in proprietary format. This is used when
 * parsing isn't available in the target API (for example, missing OpenGL extension)
 * $Id: vertex_program_parse.h,v 1.4.68.1 2007/03/27 14:01:47 legallo Exp $
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



#ifndef NL_VERTEX_PROGRAM_PARSE_H
#define NL_VERTEX_PROGRAM_PARSE_H

#include "program_parse.h"
#include <vector>

//=================================================================================================
//========================================== CVPParser ============================================
//=================================================================================================
/** A vertex program parser.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CVPParser : public CVertexProgramParser
{

public:
	/** Parse a vertex program, and convert to proprietary format.
	  * It is intended to be use by a driver implementation.
	  * \warning: Only syntax is checked. It doesn't check that a register has been initialised before use.
	  * \param src The input text of a vertex program, in OpenGL format.
	  * \param result The result program.
	  * \param errorOuput If parsing failed, contains the reason
	  * \result true if the parsing succeeded
	  */
	virtual bool parse(const char *src, CVProgram &result, std::string &errorOutput);

	// test if a specific input is used by a vertex program
	static bool isInputUsed(const TVProgram &prg, CVPOperand::EInputRegister input);

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
protected:
	virtual bool parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput);
	//
	virtual bool parseInputRegister(CVPOperand &operand, std::string &errorOutput);
	virtual bool parseOutputRegister(CVPOperand &operand, std::string &errorOutput);
	virtual bool parseConstantRegister(CVPOperand &operand, std::string &errorOutput);
	//
	virtual bool parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered);
	// skip spaces and count lines
	virtual void skipSpacesAndComments();
};

#endif

