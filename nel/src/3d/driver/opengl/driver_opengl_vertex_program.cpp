/** \file driver_opengl_vertex_program.cpp
 * OpenGL driver implementation for vertex program manipulation.
 *
 * $Id: driver_opengl_vertex_program.cpp,v 1.15 2002/10/25 16:16:08 berenguier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stdopengl.h"

#include "3d/primitive_block.h"
#include "3d/vertex_program.h"
#include "3d/vertex_program_parse.h"
#include <algorithm>


using namespace std;
using namespace NLMISC;

 
namespace NL3D
{

// ***************************************************************************
CVertexProgamDrvInfosGL::CVertexProgamDrvInfosGL (CDriverGL *drv, ItVtxPrgDrvInfoPtrList it) : IVertexProgramDrvInfos (drv, it) 
{
	// Extension must exist
	nlassert (drv->_Extensions.NVVertexProgram
		      || drv->_Extensions.EXTVertexShader
		     );

	if (drv->_Extensions.NVVertexProgram) // nvidia implemntation
	{	
		// Generate a program
		nglGenProgramsNV (1, &ID);
	}
	else
	{
		ID = nglGenVertexShadersEXT(1);
	}
}


// ***************************************************************************
bool CDriverGL::isVertexProgramSupported () const
{	
	return _Extensions.NVVertexProgram || _Extensions.EXTVertexShader;
}

// ***************************************************************************
bool CDriverGL::isVertexProgramEmulated () const
{
	return _Extensions.NVVertexProgramEmulated;
}



// ***************************************************************************
bool CDriverGL::activeNVVertexProgram (CVertexProgram *program)
{
	// Setup or unsetup ?
	if (program)
	{
		// Enable vertex program
		glEnable (GL_VERTEX_PROGRAM_NV);
		_VertexProgramEnabled= true;


		// Driver info
		CVertexProgamDrvInfosGL *drvInfo;

		// Program setuped ?
		if (program->_DrvInfo==NULL)
		{
			/** Check with our parser if the program will works with other implemented extensions, too. (EXT_vertex_shader ..).
			  * There are some incompatibilities.
			  */
			CVPParser parser;
			CVPParser::TProgram parsedProgram;
			std::string errorOutput;
			bool result = parser.parse(program->getProgram().c_str(), parsedProgram, errorOutput);
			if (!result)
			{
				nlwarning("Unable to parse a vertex program :");
				nlwarning(errorOutput.c_str());
				#ifdef NL_DEBUG
					nlassert(0);
				#endif
				return false;
			}

			// Insert into driver list. (so it is deleted when driver is deleted).
			ItVtxPrgDrvInfoPtrList	it= _VtxPrgDrvInfos.insert(_VtxPrgDrvInfos.end());

			// Create a driver info
			*it = drvInfo = new CVertexProgamDrvInfosGL (this, it);

			// Set the pointer
			program->_DrvInfo=drvInfo;						

			// Compile the program
			nglLoadProgramNV (GL_VERTEX_PROGRAM_NV, drvInfo->ID, program->getProgram().length(), (const GLubyte*)program->getProgram().c_str());

			// Get loading error code
			GLint errorOff;
			glGetIntegerv (GL_PROGRAM_ERROR_POSITION_NV, &errorOff);

			// Compilation error ?
			if (errorOff>=0)
			{
				// String length
				uint length = program->getProgram ().length();
				const char* sString= program->getProgram ().c_str();

				// Line count and char count
				uint line=1;
				uint charC=1;

				// Find the line
				uint offset=0;
				while ((offset<length)&&(offset<(uint)errorOff))
				{
					if (sString[offset]=='\n')
					{
						line++;
						charC=1;
					}
					else
						charC++;

					// Next character
					offset++;
				}

				// Show the error
				nlinfo ("Vertex program syntax error line %d character %d\n", line, charC);

				// Disable vertex program
				glDisable (GL_VERTEX_PROGRAM_NV);
				_VertexProgramEnabled= false;

				// Setup not ok
				return false;
			}

			// Setup ok
			return true;
		}
		else
		{
			// Cast the driver info pointer
			drvInfo=safe_cast<CVertexProgamDrvInfosGL*>((IVertexProgramDrvInfos*)program->_DrvInfo);
		}

		// Setup this program
		nglBindProgramNV (GL_VERTEX_PROGRAM_NV, drvInfo->ID);
		_LastSetuppedVP = program;

		// Ok
		return true;
	}
	else // Unsetup
	{
		// Disable vertex program
		glDisable (GL_VERTEX_PROGRAM_NV);
		_VertexProgramEnabled= false;
		// Ok
		return true;
	}
}


// ***************************************************************************
static 
inline GLenum convSwizzleToGLFormat(CVPSwizzle::EComp comp, bool negate)
{
	if (!negate)
	{
		switch(comp)
		{
			case CVPSwizzle::X: return GL_X_EXT;
			case CVPSwizzle::Y: return GL_Y_EXT;
			case CVPSwizzle::Z: return GL_Z_EXT;
			case CVPSwizzle::W: return GL_W_EXT;
			default:
				nlstop;
				return 0;
			break;
		}
	}
	else
	{
		switch(comp)
		{
			case CVPSwizzle::X: return GL_NEGATIVE_X_EXT;
			case CVPSwizzle::Y: return GL_NEGATIVE_Y_EXT;
			case CVPSwizzle::Z: return GL_NEGATIVE_Z_EXT;
			case CVPSwizzle::W: return GL_NEGATIVE_W_EXT;
			default:
				nlstop;
				return 0;
			break;
		}
	}
}

// ***************************************************************************
/** Convert an output register to a EXTVertexShader register
  */ 
static GLuint convOutputRegisterToEXTVertexShader(CVPOperand::EOutputRegister r)
{
	switch (r)
	{
		case 	CVPOperand::OHPosition:			return GL_OUTPUT_VERTEX_EXT;
		case    CVPOperand::OPrimaryColor:		return GL_OUTPUT_COLOR0_EXT;
		case    CVPOperand::OSecondaryColor:	return GL_OUTPUT_COLOR1_EXT;
		case    CVPOperand::OBackFacePrimaryColor:
			nlwarning("Backface color used in a vertex program is not supported by device, defaulting to front color");
			return GL_OUTPUT_COLOR0_EXT;
		break;
		case    CVPOperand::OBackFaceSecondaryColor:
			nlwarning("Backface color used in a vertex program is not supported by device, defaulting to front color");
			return GL_OUTPUT_COLOR1_EXT;
		break;
		case    CVPOperand::OFogCoord:			return GL_OUTPUT_FOG_EXT;
		case    CVPOperand::OPointSize:			nlstop; return 0; // sorry, not supported
		case	CVPOperand::OTex0:				return GL_OUTPUT_TEXTURE_COORD0_EXT;
		case	CVPOperand::OTex1:				return GL_OUTPUT_TEXTURE_COORD1_EXT;
		case	CVPOperand::OTex2:				return GL_OUTPUT_TEXTURE_COORD2_EXT;
		case	CVPOperand::OTex3:				return GL_OUTPUT_TEXTURE_COORD3_EXT;
		case	CVPOperand::OTex4:				return GL_OUTPUT_TEXTURE_COORD4_EXT;
		case	CVPOperand::OTex5:				return GL_OUTPUT_TEXTURE_COORD5_EXT;
		case	CVPOperand::OTex6:				return GL_OUTPUT_TEXTURE_COORD6_EXT;
		case	CVPOperand::OTex7:				return GL_OUTPUT_TEXTURE_COORD7_EXT;
		default:
			nlstop;
		break;
	}
	return 0;
}

// ***************************************************************************
/** Convert an input register to a vertex buffer flag
  */ 
static uint convInputRegisterToVBFlag(uint index)
{
	switch (index)
	{	
		case CVPOperand::IPosition:				return CVertexBuffer::PositionFlag;
		case CVPOperand::IWeight:				return CVertexBuffer::WeightFlag;
		case CVPOperand::INormal:				return CVertexBuffer::NormalFlag;
		case CVPOperand::IPrimaryColor:			return CVertexBuffer::PrimaryColorFlag;
		case CVPOperand::ISecondaryColor:		return CVertexBuffer::SecondaryColorFlag;
		case CVPOperand::IFogCoord:				return CVertexBuffer::FogFlag;
		case CVPOperand::IPaletteSkin:			return CVertexBuffer::PaletteSkinFlag;
		case CVPOperand::IEmpty: nlassert(0); break;
		case CVPOperand::ITex0:
		case CVPOperand::ITex1: 
		case CVPOperand::ITex2:
		case CVPOperand::ITex3:
		case CVPOperand::ITex4:
		case CVPOperand::ITex5:
		case CVPOperand::ITex6:
		case CVPOperand::ITex7:
		case CVPOperand::ITex8:
			return CVertexBuffer::TexCoord0Flag << (index - CVPOperand::ITex0);
		default:
			nlassert(0);
		break;
	}
	return 0;
}



// A macro to debug the generated instruction
#define DEBUG_SETUP_EXT_VERTEX_SHADER

#ifdef DEBUG_SETUP_EXT_VERTEX_SHADER
	#define EVS_INFO(what) nlinfo(what)
#else
	#define EVS_INFO(what)
#endif


// For debugging with swizzling
static void doSwizzle(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)
{
	nglSwizzleEXT(res, in, outX, outY, outZ, outW);
#ifdef DEBUG_SETUP_EXT_VERTEX_SHADER
	std::string swzStr = "Swizzle : ";
	GLenum swz[] = { outX, outY, outZ, outW };		
	for(uint k = 0; k < 4; ++k)
	{
		switch(swz[k])
		{
			case GL_X_EXT:
				swzStr +=" X";
			break;
			case GL_NEGATIVE_X_EXT:
				swzStr +=" -X";
			break;
			case GL_Y_EXT:
				swzStr +=" Y";
			break;
			case GL_NEGATIVE_Y_EXT:
				swzStr +=" -Y";
			break;
			break;
			case GL_Z_EXT:
				swzStr +=" Z";
			break;
			case GL_NEGATIVE_Z_EXT:
				swzStr +=" -Z";
			break;
			case GL_W_EXT:
				swzStr +=" W";
			break;
			case GL_NEGATIVE_W_EXT:
				swzStr +=" -W";
			break;
			case GL_ZERO_EXT:
				swzStr +="0";
			break;
			case GL_ONE_EXT:
				swzStr +="1";
			break;
		}
	}
	EVS_INFO(swzStr.c_str());
#endif

}

// Perform write mask and output de bug informations
static void doWriteMask(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)
{
	nglWriteMaskEXT(res, in, outX, outY, outZ, outW);
	#ifdef DEBUG_SETUP_EXT_VERTEX_SHADER
	nlinfo("Write Mask : %c%c%c%c",
		   outX ? 'x' : '-',
		   outY ? 'y' : '-',
		   outZ ? 'z' : '-',
		   outW ? 'w' : '-'		   
		  );
	#endif
}

// ***************************************************************************
/** Setup a vertex shader from its parsed program
  */ 
bool CDriverGL::setupEXTVertexShader(const CVPParser::TProgram &program, GLuint id, uint variants[EVSNumVariants], uint16 &usedInputRegisters)
{
	// counter to see what is generated
	uint numOp = 0;
	uint numOpIndex = 0;
	uint numSwizzle = 0;
	uint numEC = 0; // extract component
	uint numIC = 0; // insert component
	uint numWM = 0; // write maks

	
	#ifdef DEBUG_SETUP_EXT_VERTEX_SHADER
		nlinfo("**********************************************************");
	#endif

	// clear last error
	GLenum glError = glGetError();

	// allocate the symbols
	nglBindVertexShaderEXT(id);
	nglBeginVertexShaderEXT();
	{				
		// Allocate register and variant
		GLuint firstRegister = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_LOCAL_EXT, GL_FULL_RANGE_EXT, 12); // 12 register
		GLuint firstTempRegister = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_LOCAL_EXT, GL_FULL_RANGE_EXT, 4); // 4  temp register used for swizzle & indexing
		GLuint firstTempScalar = nglGenSymbolsEXT(GL_SCALAR_EXT, GL_LOCAL_EXT, GL_FULL_RANGE_EXT, 3); // 3  temp scalars used for EXPP & LOGG emulation
		GLuint firstAddressRegister = nglGenSymbolsEXT(GL_SCALAR_EXT, GL_LOCAL_EXT, GL_FULL_RANGE_EXT, 1);

		// allocate variants
		variants[EVSSecondaryColorVariant] = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_VARIANT_EXT, GL_NORMALIZED_RANGE_EXT, 1);
		variants[EVSFogCoordsVariant] = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_VARIANT_EXT, GL_FULL_RANGE_EXT, 1);
		variants[EVSSkinWeightVariant] = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_VARIANT_EXT, GL_NORMALIZED_RANGE_EXT, 1);
		variants[EVSPaletteSkinVariant] = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_VARIANT_EXT, GL_FULL_RANGE_EXT, 1);

		// allocate one temporary register for fog before conversion
		GLuint fogTemp = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_LOCAL_EXT, GL_FULL_RANGE_EXT, 1);


		// local constant : 0 and 1
		GLuint cteOne = nglGenSymbolsEXT(GL_SCALAR_EXT, GL_LOCAL_CONSTANT_EXT, GL_FULL_RANGE_EXT, 1);
		GLuint cteZero = nglGenSymbolsEXT(GL_SCALAR_EXT, GL_LOCAL_CONSTANT_EXT, GL_FULL_RANGE_EXT, 1);


		float oneValue = 1.f;
		float zeroValue = 0.f;

		nglSetLocalConstantEXT( cteOne, GL_FLOAT, &oneValue);
		nglSetLocalConstantEXT( cteZero, GL_FLOAT, &zeroValue);


		
		if (firstRegister == 0)
		{
			nlwarning("Unable to allocate local registers for EXT_vertex_shader");
			return false;
		}
		if (firstTempRegister == 0)
		{
			nlwarning("Unable to allocate local temp registers for EXT_vertex_shader");
			return false;
		}
		if (firstTempScalar == 0)
		{
			nlwarning("Unable to allocate temp scalar registers for EXT_vertex_shader");
			return false;
		}
		if (firstAddressRegister == 0)
		{
			nlwarning("Unable to allocate address register for EXT_vertex_shader");
			return false;
		}

		uint k;
		for(k = 0; k < EVSNumVariants; ++k)
		{
			if (variants[k] == 0)
			{
				nlwarning("Unable to allocate variants for EXT_vertex_shader");
				return false;
			}
		}

		// Mask of output component that are being written
		uint componentWritten[16];
		std::fill(componentWritten, componentWritten + 16, 0);
		
		//
		GLuint srcValue[3];
		//
		GLuint destValue;
		GLuint maskedDestValue;
		

		uint l;
		// convert each instruction of the vertex program
		for(k = 0; k < program.size(); ++k)
		{
			// get src values, eventually applying swizzle, negate, and index on them
			uint numSrc = program[k].getNumUsedSrc();
			for(l = 0; l < numSrc; ++l)
			{
				EVS_INFO(("Build source " + toString(l)).c_str());
				const CVPOperand &operand = program[k].getSrc(l);
				switch (operand.Type)
				{
					case CVPOperand::InputRegister: 
					{
						switch(operand.Value.InputRegisterValue)
						{
							case  0: srcValue[l] = _EVSPositionHandle; EVS_INFO("Src = position"); break;
							case  1: srcValue[l] = variants[EVSSkinWeightVariant]; EVS_INFO("Src = skin weight"); break;
							case  2: srcValue[l] = _EVSNormalHandle; EVS_INFO("Src = normal"); break;
							case  3: srcValue[l] = _EVSColorHandle; EVS_INFO("Src = color 0"); break;
							case  4: srcValue[l] = variants[EVSSecondaryColorVariant]; EVS_INFO("Src = color 1"); break;
							case  5: srcValue[l] = variants[EVSFogCoordsVariant]; EVS_INFO("Src = fog coord"); break;
							case  6: srcValue[l] = variants[EVSPaletteSkinVariant]; EVS_INFO("Src = palette skin"); break;
							case  7: nlstop; // not supported
							case  8:
							case  9:
							case  10:
							case  11:
							case  12:
							case  13:
							case  14:
							case  15:
							{								
								EVS_INFO(("Src = Tex" + toString(operand.Value.InputRegisterValue - 8)).c_str());
								srcValue[l] = _EVSTexHandle[operand.Value.InputRegisterValue - 8];
								if (srcValue[l] == 0)
								{								
									nlwarning("Trying to read an unaccessible texture coords for the device when using EXT_vertex_shader, shader loading failed");
									return false;
								}
							}
							break;
							default:
								nlstop; // invalid value
							break;
						}
					}
					break;
					case CVPOperand::Constant: 
						srcValue[l] = _EVSConstantHandle + operand.Value.ConstantValue; 
						EVS_INFO(("Src = constant" + toString(operand.Value.ConstantValue)).c_str());
					break;
					case CVPOperand::Variable: 
						srcValue[l] = firstRegister + operand.Value.VariableValue;
						EVS_INFO(("Src = variable register" + toString(operand.Value.VariableValue)).c_str());
					break;
					default:
						nlassert(0);
					break;
				}
				// test if indexed access is used (can be used on one register only)
				if (operand.Type == CVPOperand::Constant && operand.Indexed)
				{
					nglShaderOp2EXT(GL_OP_INDEX_EXT, firstTempRegister + 3, firstAddressRegister, srcValue[l]);
					EVS_INFO("GL_OP_INDEX_EXT");
					++ numOpIndex;
					srcValue[l] = firstTempRegister + 3;
					glError = glGetError();
					nlassert(glError == GL_NO_ERROR)
				}

				// test if swizzle or negate is used
				if (!operand.Swizzle.isIdentity() || operand.Negate)
				{
					// if the instruction reads a scalar, no need for swizzle (except if negate is used)
					if (!						
						(
						 (program[k].Opcode == CVPInstruction::RSQ
						  || program[k].Opcode == CVPInstruction::RCP
						  || program[k].Opcode == CVPInstruction::LOG
						  || program[k].Opcode == CVPInstruction::EXPP
					     ) 
						 && 
						 !operand.Negate
						)
					)
					{					
						// need a temp register for swizzle and/or negate
						doSwizzle(firstTempRegister + l, srcValue[l],
									  convSwizzleToGLFormat(operand.Swizzle.Comp[0], operand.Negate),
									  convSwizzleToGLFormat(operand.Swizzle.Comp[1], operand.Negate),
									  convSwizzleToGLFormat(operand.Swizzle.Comp[2], operand.Negate),
									  convSwizzleToGLFormat(operand.Swizzle.Comp[3], operand.Negate));						
						++numSwizzle;
						srcValue[l] = firstTempRegister + l;
						glError = glGetError();
						nlassert(glError == GL_NO_ERROR)
					}
				}
			}

			// get dest value
			const CVPOperand &destOperand = program[k].Dest;
			switch(destOperand.Type)
			{
				case CVPOperand::Variable: 
					destValue = firstRegister + destOperand.Value.VariableValue; 
				break;
				case CVPOperand::OutputRegister:
					if (destOperand.Value.OutputRegisterValue != CVPOperand::OFogCoord)
					{					
						destValue = convOutputRegisterToEXTVertexShader(destOperand.Value.OutputRegisterValue);
					}
					else
					{
						destValue = fogTemp;
					}
				break;
				case CVPOperand::AddressRegister:
					destValue = firstAddressRegister;
				break;
				default:
					nlassert(0);
				break;
			}

			uint writeMask = program[k].Dest.WriteMask;
			CVPInstruction::EOpcode opcode = program[k].Opcode;
			uint outputRegisterIndex = destOperand.Value.OutputRegisterValue;
			nlassert(outputRegisterIndex < 16);

			// Tells wether the output has already been written before the final write mask. This happens with instructions LOG, EXPP, LIT, RSQ and RCP,
			// because they write their output component by components
			bool outputWritten = false;

			// test for write mask						
			if (writeMask != 0x0f) 
			{					
				/** Don't know why, but on some implementation of EXT_vertex_shader, can't write a single components to the fog coordinate..
				 * So we force the mask to 0xf (only the x coordinate is used anyway).			  
				 */			
				if (!(destOperand.Type == CVPOperand::OutputRegister && destValue == fogTemp))
				{
					// For instructions that write their output components by components, we don't need an intermediary register
					if (opcode == CVPInstruction::LOG
						|| opcode == CVPInstruction::EXPP
						|| opcode == CVPInstruction::LIT
						|| opcode == CVPInstruction::RSQ
						|| opcode == CVPInstruction::RCP
					   )
					{
						outputWritten = true;	
					}
					else
					{					
						maskedDestValue = destValue;
						// use a temp register before masking
						destValue = firstTempRegister;
					}
				}
				else
				{
					componentWritten[outputRegisterIndex] = 0xf;
				}
			}
			else
			{
				if (destOperand.Type == CVPOperand::OutputRegister)
				{
					componentWritten[outputRegisterIndex] = 0xf; // say all components have been written for that output				
				}
			}			
			
			// generate opcode
			switch (opcode)
			{
				case  CVPInstruction::ARL:
				{					
					nlassert(program[k].Src1.Swizzle.isScalar());
					GLuint index = program[k].Src1.Swizzle.Comp[0];
					nglExtractComponentEXT(firstAddressRegister, srcValue[0],  index);
					EVS_INFO("Extract component");
					++numEC;
				}
				break;
				case  CVPInstruction::MOV:
				{											
					nglShaderOp1EXT(GL_OP_MOV_EXT, destValue, srcValue[0]);
					EVS_INFO("GL_OP_MOV_EXT");
					++numOp;					
				}
				break;
				case  CVPInstruction::MUL: 
					nglShaderOp2EXT(GL_OP_MUL_EXT, destValue, srcValue[0], srcValue[1]); 
					EVS_INFO("GL_OP_MUL_EXT");
					++numOp;
				break;
				case  CVPInstruction::ADD: 
					nglShaderOp2EXT(GL_OP_ADD_EXT, destValue, srcValue[0], srcValue[1]); 
					EVS_INFO("GL_OP_ADD_EXT");
					++numOp;
				break;
				case  CVPInstruction::MAD: 
					nglShaderOp3EXT(GL_OP_MADD_EXT, destValue, srcValue[0], srcValue[1], srcValue[2]); 
					EVS_INFO("GL_OP_MADD_EXT");
					++numOp;
				break;
				case  CVPInstruction::RSQ:
				{
					nlassert(program[k].Src1.Swizzle.isScalar());
					// extract the component we need
					GLuint index = program[k].Src1.Swizzle.Comp[0];
					nglExtractComponentEXT(firstTempScalar, srcValue[0],  index);
					EVS_INFO("Extract component");
					++numEC;
					nglShaderOp1EXT(GL_OP_RECIP_SQRT_EXT, firstTempScalar + 1, firstTempScalar);
					EVS_INFO("GL_OP_RECIP_SQRT_EXT");
					++numOp;
					// duplicate result in destination
					for(uint l = 0; l < 4; ++l)
					{
						if (writeMask & (1 << l))
						{						
							nglInsertComponentEXT(destValue, firstTempScalar + 1, l);
							EVS_INFO("Insert component");
							nlassert(glGetError() == GL_NO_ERROR);
						}
					}
				}
				break;
				case  CVPInstruction::DP3: 
					nglShaderOp2EXT(GL_OP_DOT3_EXT, destValue, srcValue[0], srcValue[1]);
					EVS_INFO("GL_OP_DOT3_EXT");
					++numOp;
				break;
				case  CVPInstruction::DP4: 
					nglShaderOp2EXT(GL_OP_DOT4_EXT, destValue, srcValue[0], srcValue[1]); 
					EVS_INFO("GL_OP_DOT4_EXT");
					++numOp;
				break;
				case  CVPInstruction::DST: 
					doSwizzle(firstTempRegister, srcValue[0], GL_ONE_EXT, GL_Y_EXT, GL_Z_EXT, GL_ONE_EXT);
					EVS_INFO("GL_OP_DOT4_EXT");
					++numOp;
					doSwizzle(firstTempRegister + 1, srcValue[1], GL_ONE_EXT, GL_Y_EXT, GL_ONE_EXT, GL_W_EXT);					
					++numSwizzle;
					nglShaderOp2EXT(GL_OP_MUL_EXT, destValue, firstTempRegister, firstTempRegister + 1);
					EVS_INFO("GL_OP_MUL_EXT");
					++numOp;
				break;					
				case  CVPInstruction::LIT:
				{
					uint writeMask = program[k].Dest.WriteMask;					
					nglExtractComponentEXT(firstTempScalar, srcValue[0], 0); // extract X from the source
					if (writeMask & 4)
					{					
						nglExtractComponentEXT(firstTempScalar + 1, srcValue[0], 1); // extract Y from the source
						EVS_INFO("Extract component");
						++numEC;
						nglExtractComponentEXT(firstTempScalar + 2, srcValue[0], 3); // extract W from the source
						EVS_INFO("Extract component");
						++numEC;
						// result = X > 0 ? Y^W : 0					
						nglShaderOp2EXT(GL_OP_POWER_EXT, firstTempScalar + 2, firstTempScalar + 1, firstTempScalar + 2);
						EVS_INFO("GL_OP_POWER_EXT");
						++numOp;
						nglShaderOp2EXT(GL_OP_SET_GE_EXT, firstTempScalar + 1, firstTempScalar, cteZero);
						EVS_INFO("GL_OP_SET_GE_EXT");
						++numOp;
						nglShaderOp2EXT(GL_OP_MUL_EXT, firstTempScalar + 2, firstTempScalar + 2, firstTempScalar + 1);
						EVS_INFO("GL_OP_MUL_EXT");
						++numOp;
						// store result
						nglInsertComponentEXT(destValue, firstTempScalar + 2, 2);
						EVS_INFO("Insert component");
						++numIC;
					}
					if (writeMask & 2) 
					{
						// clamp N.L to [0, 1]
						nglShaderOp3EXT(GL_OP_CLAMP_EXT, firstTempScalar, firstTempScalar, cteZero, cteOne);
						EVS_INFO("GL_OP_CLAMP_EXT");
						++numOp;
						nglInsertComponentEXT(destValue, firstTempScalar, 1);
						EVS_INFO("Insert component");
						++numIC;
					}
					// set x and w to 1 if they are not masked
					if (writeMask & (1 + 8)) 
					{					
						doSwizzle(destValue, destValue, 
									  (writeMask & 1) ? GL_ONE_EXT : GL_X_EXT,
									  GL_Y_EXT,
									  GL_Z_EXT,
									  (writeMask & 8) ? GL_ONE_EXT : GL_W_EXT);						
						++numSwizzle;
					}					
						 
				}
				break;
				case  CVPInstruction::MIN: 
					nglShaderOp2EXT(GL_OP_MIN_EXT, destValue, srcValue[0], srcValue[1]);
					EVS_INFO("GL_OP_MIN_EXT");
					++numOp;
				break;
				case  CVPInstruction::MAX: 
					nglShaderOp2EXT(GL_OP_MAX_EXT, destValue, srcValue[0], srcValue[1]);
					EVS_INFO("GL_OP_MAX_EXT");
					++numOp;
				break;
				case  CVPInstruction::SLT: 
					nglShaderOp2EXT(GL_OP_SET_LT_EXT, destValue, srcValue[0], srcValue[1]);
					EVS_INFO("GL_OP_SET_LT_EXT");
					++numOp;
				break;
				case  CVPInstruction::SGE: 
					nglShaderOp2EXT(GL_OP_SET_GE_EXT, destValue, srcValue[0], srcValue[1]);
					EVS_INFO("GL_OP_SET_GE_EXT");
					++numOp;
				break;
				case  CVPInstruction::EXPP:
				{			
					uint writeMask = program[k].Dest.WriteMask;
					nlassert(program[k].Src1.Swizzle.isScalar());
					GLuint compIndex = program[k].Src1.Swizzle.Comp[0];
					nglExtractComponentEXT(firstTempScalar + 2, srcValue[0], compIndex); // extract W from the source
					EVS_INFO("Extract component");
					++numEC;
					if (writeMask & 1)
					{					
						nglShaderOp1EXT(GL_OP_FLOOR_EXT, firstTempScalar, firstTempScalar + 2); // (int) W
						EVS_INFO("GL_OP_FLOOR_EXT");
						++numOp;
						nglShaderOp1EXT(GL_OP_EXP_BASE_2_EXT, firstTempScalar, firstTempScalar); // 2 ^ (int) W		
						EVS_INFO("GL_OP_EXP_BASE_2_EXT");
						++numOp;
					}
					if (writeMask & 2) 
					{					
						nglShaderOp1EXT(GL_OP_FRAC_EXT, firstTempScalar + 1, firstTempScalar + 2); // frac(W)
						EVS_INFO("GL_OP_FRAC_EXT");
						++numOp;
					}
					if (writeMask & 4) nglShaderOp1EXT(GL_OP_EXP_BASE_2_EXT, firstTempScalar + 2, firstTempScalar + 2); // 2 ^W
					// store the results
					if (writeMask & 1) { nglInsertComponentEXT(destValue, firstTempScalar, 0); EVS_INFO("Insert component"); ++numIC;  }
					if (writeMask & 2) { nglInsertComponentEXT(destValue, firstTempScalar + 1, 1); EVS_INFO("Insert component"); ++numIC; }
					if (writeMask & 4) { nglInsertComponentEXT(destValue, firstTempScalar + 2, 2); EVS_INFO("Insert component"); ++numIC; }
					// set W to 1 and leave other values unchanged
					if (writeMask & 8) { doSwizzle(destValue, destValue, GL_X_EXT, GL_Y_EXT, GL_Z_EXT, GL_ONE_EXT); ++numSwizzle; }
				}
				break;
				case  CVPInstruction::LOG:
				{					
					uint writeMask = program[k].Dest.WriteMask;
					nlassert(program[k].Src1.Swizzle.isScalar());
					// extract the component we need
					nglExtractComponentEXT(firstTempScalar, srcValue[0], (GLuint) program[k].Src1.Swizzle.Comp[0]);
					EVS_INFO("Extract component");
					++numEC;
					// get abs(src) : abs(src) = max(src, -src)
					nglShaderOp1EXT(GL_OP_NEGATE_EXT, firstTempScalar + 1, firstTempScalar);
					EVS_INFO("GL_OP_NEGATE_EXT");
					++numOp;
					nglShaderOp2EXT(GL_OP_MAX_EXT, firstTempScalar, firstTempScalar, firstTempScalar + 1);
					EVS_INFO("GL_OP_MAX_EXT");
					++numOp;
					nglShaderOp1EXT(GL_OP_LOG_BASE_2_EXT, firstTempScalar, firstTempScalar); // (int) W
					EVS_INFO("GL_OP_LOG_BASE_2_EXT");
					++numOp;
					// store the results
					for(uint l = 0; l < 4; ++l)
					{
						if (writeMask & (1 << l))
						{						
							nglInsertComponentEXT(destValue, firstTempScalar, l);
							EVS_INFO("Insert component");
							nlassert(glGetError() == GL_NO_ERROR);
						}
					}					
				}
				break;
				case  CVPInstruction::RCP:
				{
					uint writeMask = program[k].Dest.WriteMask;
					nlassert(program[k].Src1.Swizzle.isScalar());
					// extract the component we need
					nglExtractComponentEXT(firstTempScalar, srcValue[0], (GLuint) program[k].Src1.Swizzle.Comp[0]);
					EVS_INFO("Extract component");
					++numEC;
					nglShaderOp1EXT(GL_OP_RECIP_EXT, firstTempScalar + 1, firstTempScalar);
					EVS_INFO("GL_OP_RECIP_EXT");
					++numOp;
					// duplicate result in destination
					for(uint l = 0; l < 4; ++l)
					{
						if (writeMask & (1 << l))
						{						
							nglInsertComponentEXT(destValue, firstTempScalar + 1, l);
							EVS_INFO("insert component");
							++numIC;
						}
					}
				}					
				break;
			}

			glError = glGetError();
			nlassert(glError == GL_NO_ERROR)

				
			// apply write mask if any
			if (writeMask != 0x0f)
			{									
				if (destOperand.Type == CVPOperand::OutputRegister && destValue != fogTemp)
				{
					uint &outputMask = componentWritten[outputRegisterIndex];
					// is a texture coordinate or a color being written ?
					if ((maskedDestValue >= GL_OUTPUT_TEXTURE_COORD0_EXT && maskedDestValue <= GL_OUTPUT_TEXTURE_COORD7_EXT)
						|| maskedDestValue == GL_OUTPUT_COLOR0_EXT
						|| maskedDestValue == GL_OUTPUT_COLOR1_EXT
					   )
					{					
						// test if this is the last time this output will be written
						bool found = false;
						// if this was the last write for this output, must set unfilled component
						// NB : this loop could be optimized, but vertex program are rather short for now ..
						for(uint m = k + 1; m < program.size(); ++m)
						{
							if (program[m].Dest.Type == CVPOperand::OutputRegister) // another output to this texture ?
							{
								if (program[m].Dest.Value.OutputRegisterValue == program[k].Dest.Value.OutputRegisterValue)
								{
									found = true;
									break;
								}
							}
						}

						if (found)
						{
							if (!outputWritten)
							{							
								// write values
								doWriteMask(maskedDestValue, destValue,
												writeMask & 1 ? GL_TRUE : GL_FALSE,
												writeMask & 2 ? GL_TRUE : GL_FALSE,
												writeMask & 4 ? GL_TRUE : GL_FALSE,
												writeMask & 8 ? GL_TRUE : GL_FALSE);
								++numWM;
							}
						}
						else // this is the last write, check if the mask is complete
						{
							if ((outputMask | writeMask) == 0xf)
							{
								if (!outputWritten)
								{
									// ok, after this call everything has been written
									// write values
									    doWriteMask(maskedDestValue, destValue,
													writeMask & 1 ? GL_TRUE : GL_FALSE,
													writeMask & 2 ? GL_TRUE : GL_FALSE,
													writeMask & 4 ? GL_TRUE : GL_FALSE,
													writeMask & 8 ? GL_TRUE : GL_FALSE);
									++numWM;
								}
							}
							else
							{
								uint prevMask = outputMask;
								uint newMask  = writeMask | outputMask;
								
								// complete unused entries

								// if primary color is output, then the default color is white
								if (maskedDestValue == GL_OUTPUT_COLOR0_EXT)
								{
									doSwizzle(firstTempRegister, destValue,
												  newMask & 1 ? GL_X_EXT : GL_ONE_EXT,
												  newMask & 2 ? GL_Y_EXT : GL_ONE_EXT,
												  newMask & 4 ? GL_Z_EXT : GL_ONE_EXT,
												  newMask & 8 ? GL_W_EXT : GL_ONE_EXT);
								}
								else
								{								
									doSwizzle(firstTempRegister, destValue,
												  newMask & 1 ? GL_X_EXT : GL_ZERO_EXT,
												  newMask & 2 ? GL_Y_EXT : GL_ZERO_EXT,
												  newMask & 4 ? GL_Z_EXT : GL_ZERO_EXT,
												  newMask & 8 ? GL_W_EXT : GL_ONE_EXT);
								}
								if (!outputWritten)
								{							
									++numWM;
									    doWriteMask(maskedDestValue, firstTempRegister,
													prevMask & 1 ? GL_FALSE : GL_TRUE,
													prevMask & 2 ? GL_FALSE : GL_TRUE,
													prevMask & 4 ? GL_FALSE : GL_TRUE,
													prevMask & 8 ? GL_FALSE : GL_TRUE
												  );									
									++numWM;
								}
								outputMask = 0xf;
							}
						}										
					}
					else
					{
						if (!outputWritten)
						{
							    doWriteMask(maskedDestValue, destValue,
											writeMask & 1 ? GL_TRUE : GL_FALSE,
											writeMask & 2 ? GL_TRUE : GL_FALSE,
											writeMask & 4 ? GL_TRUE : GL_FALSE,
											writeMask & 8 ? GL_TRUE : GL_FALSE);
							++numWM;
						}
					}
					// complete the mask 
					outputMask |= writeMask;
				}
				else if (destOperand.Type != CVPOperand::OutputRegister)
				{
					if (!outputWritten)
					{
						    doWriteMask(maskedDestValue, destValue,
										writeMask & 1 ? GL_TRUE : GL_FALSE,
										writeMask & 2 ? GL_TRUE : GL_FALSE,
										writeMask & 4 ? GL_TRUE : GL_FALSE,
										writeMask & 8 ? GL_TRUE : GL_FALSE);
						++numWM;
					}
				}				
			}

			glError = glGetError();
			nlassert(glError == GL_NO_ERROR)
		}

		

		// if color have not been written, write with default values
		if (componentWritten[CVPOperand::OPrimaryColor] == 0)
		{
			// we specify vertex coord has input for swizzle, but we don't read any component.. 
			doSwizzle(GL_OUTPUT_COLOR0_EXT, _EVSPositionHandle, GL_ONE_EXT, GL_ONE_EXT, GL_ONE_EXT, GL_ONE_EXT);
			EVS_INFO("Swizzle (Complete primary color)");
			++numSwizzle;
		}
		else
		{
			nlassert(componentWritten[CVPOperand::OPrimaryColor] == 0xf)
		}
		if (componentWritten[CVPOperand::OSecondaryColor] == 0)
		{
			// we specify vertex coord has input for swizzle, but we don't read any component..
			doSwizzle(GL_OUTPUT_COLOR1_EXT, _EVSPositionHandle, GL_ZERO_EXT, GL_ZERO_EXT, GL_ZERO_EXT, GL_ONE_EXT);
			EVS_INFO("Swizzle (Complete secondary color)");
			++numSwizzle;
		}
		else
		{
			nlassert(componentWritten[CVPOperand::OSecondaryColor] == 0xf)
		}
		nlassert(componentWritten[CVPOperand::OHPosition] == 0xf); // should have written all component of position	

		glError = glGetError();
		nlassert(glError == GL_NO_ERROR);

		// if fog has been written, perform conversion
		if (componentWritten[CVPOperand::OFogCoord] == 0xf)
		{
			// Well this could be avoided, but we should make 2 cases for each vertex program.. :(
			doSwizzle(firstTempRegister, _EVSConstantHandle + 96, GL_X_EXT, GL_X_EXT, GL_X_EXT, GL_X_EXT);
			doSwizzle(firstTempRegister + 1, _EVSConstantHandle + 96, GL_Y_EXT, GL_Y_EXT, GL_Y_EXT, GL_Y_EXT);			
			nglShaderOp3EXT(GL_OP_MADD_EXT, firstTempRegister + 2, fogTemp, firstTempRegister, firstTempRegister + 1);
			EVS_INFO("Use MAD for fog conversion");
			nglExtractComponentEXT(GL_OUTPUT_FOG_EXT, firstTempRegister + 2, 0);
			EVS_INFO("Extract component to fog");
		}

		glError = glGetError();
		nlassert(glError == GL_NO_ERROR);
	}
	nglEndVertexShaderEXT();

	glError = glGetError();
	nlassert(glError == GL_NO_ERROR);

	GLboolean optimizedShader;
	glGetBooleanv(GL_VERTEX_SHADER_OPTIMIZED_EXT, &optimizedShader);
	if (!optimizedShader)
	{
		nlwarning("Failed to optimize a vertex program with the EXT_vertex_shader extension, this shader will be disabled");
		return false;
	}	

	// see which input registers are used
	usedInputRegisters = 0;
	
	uint k, l;
	// convert each instruction of the vertex program
	for(k = 0; k < program.size(); ++k)
	{
		uint numSrc = program[k].getNumUsedSrc();
		for(l = 0; l < numSrc; ++l)
		{
			const CVPOperand &op = program[k].getSrc(l);
			if (op.Type == CVPOperand::InputRegister)
			{
				usedInputRegisters |= convInputRegisterToVBFlag(op.Value.InputRegisterValue);
			}
		}		
	}

#ifdef DEBUG_SETUP_EXT_VERTEX_SHADER
	nlinfo("========================");
	nlinfo("num Opcode  = %d", numOp);
	nlinfo("num Indexing = %d", numOpIndex);
	nlinfo("num Swizzle = %d", numSwizzle);
	nlinfo("num extract component = %d", numEC);
	nlinfo("num insert component = %d", numIC);	
	nlinfo("num write mask = %d", numWM);
#endif

	return true;
			
}

// ***************************************************************************
bool CDriverGL::activeEXTVertexShader (CVertexProgram *program)
{
	// Setup or unsetup ?
	if (program)
	{		
		// Driver info
		CVertexProgamDrvInfosGL *drvInfo;

		// Program setuped ?
		if (program->_DrvInfo==NULL)
		{
			// try to parse the program
			CVPParser parser;
			CVPParser::TProgram parsedProgram;
			std::string errorOutput;
			bool result = parser.parse(program->getProgram().c_str(), parsedProgram, errorOutput);
			if (!result)
			{
				nlwarning("Unable to parse a vertex program.");
				#ifdef NL_DEBUG
					nlerror(errorOutput.c_str());
				#endif
				return false;
			}

			/* 
			FILE *f = fopen("c:\\test.txt", "wb");
			if (f)
			{
				std::string vpText;
				CVPParser::dump(parsedProgram, vpText);
				fwrite(vpText.c_str(), vpText.size(), 1, f);
				fclose(f);
			}
			*/

			// Insert into driver list. (so it is deleted when driver is deleted).
			ItVtxPrgDrvInfoPtrList	it= _VtxPrgDrvInfos.insert(_VtxPrgDrvInfos.end());

			// Create a driver info
			*it = drvInfo = new CVertexProgamDrvInfosGL (this, it);
			// Set the pointer
			program->_DrvInfo=drvInfo;

			if (!setupEXTVertexShader(parsedProgram, drvInfo->ID, drvInfo->Variants, drvInfo->UsedVertexComponents))
			{
				delete drvInfo;
				program->_DrvInfo = NULL;
				_VtxPrgDrvInfos.erase(it);
				return false;
			}
		}
		else
		{
			// Cast the driver info pointer
			drvInfo=safe_cast<CVertexProgamDrvInfosGL*>((IVertexProgramDrvInfos*)program->_DrvInfo);
		}

		glEnable( GL_VERTEX_SHADER_EXT);
		_VertexProgramEnabled = true;
		nglBindVertexShaderEXT( drvInfo->ID );
		_LastSetuppedVP = program;
	}
	else
	{
		glDisable( GL_VERTEX_SHADER_EXT );
		_VertexProgramEnabled = false;
	}
	return true;	
}

// ***************************************************************************
bool CDriverGL::activeVertexProgram (CVertexProgram *program)
{
	// Extension here ?
	if (_Extensions.NVVertexProgram)
	{
		return activeNVVertexProgram(program);
	}
	else if (_Extensions.EXTVertexShader)
	{
		return activeEXTVertexShader(program);		
	}

	// Can't do anything
	return false;
}


// ***************************************************************************

void CDriverGL::setConstant (uint index, float f0, float f1, float f2, float f3)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		// Setup constant
		nglProgramParameter4fNV (GL_VERTEX_PROGRAM_NV, index, f0, f1, f2, f3);
	}
	else if (_Extensions.EXTVertexShader)
	{
		float datas[] = { f0, f1, f2, f3 };
		nglSetInvariantEXT(_EVSConstantHandle + index, GL_FLOAT, datas);
	}		 
}


// ***************************************************************************

void CDriverGL::setConstant (uint index, double d0, double d1, double d2, double d3)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		// Setup constant
		nglProgramParameter4dNV (GL_VERTEX_PROGRAM_NV, index, d0, d1, d2, d3);
	}
	else if (_Extensions.EXTVertexShader)
	{
		double datas[] = { d0, d1, d2, d3 };
		nglSetInvariantEXT(_EVSConstantHandle + index, GL_DOUBLE, datas);
	}
}


// ***************************************************************************

void CDriverGL::setConstant (uint index, const NLMISC::CVector& value)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		// Setup constant
		nglProgramParameter4fNV (GL_VERTEX_PROGRAM_NV, index, value.x, value.y, value.z, 0);	
	}
	else if (_Extensions.EXTVertexShader)
	{
		float datas[] = { value.x, value.y, value.z, 0 };
		nglSetInvariantEXT(_EVSConstantHandle + index, GL_FLOAT, datas);
	}
}


// ***************************************************************************

void CDriverGL::setConstant (uint index, const NLMISC::CVectorD& value)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		// Setup constant
		nglProgramParameter4dNV (GL_VERTEX_PROGRAM_NV, index, value.x, value.y, value.z, 0);
	}
	else if (_Extensions.EXTVertexShader)
	{
		double datas[] = { value.x, value.y, value.z, 0 };
		nglSetInvariantEXT(_EVSConstantHandle + index, GL_DOUBLE, datas);
	}
}


// ***************************************************************************
void	CDriverGL::setConstant (uint index, uint num, const float *src)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		nglProgramParameters4fvNV(GL_VERTEX_PROGRAM_NV, index, num, src);
	}	
	else if (_Extensions.EXTVertexShader)
	{
		for(uint k = 0; k < num; ++k)
		{					
			nglSetInvariantEXT(_EVSConstantHandle + index + k, GL_FLOAT, (void *) (src + 4 * k));
		}
	}	
}

// ***************************************************************************
void	CDriverGL::setConstant (uint index, uint num, const double *src)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		nglProgramParameters4dvNV(GL_VERTEX_PROGRAM_NV, index, num, src);
	}
	else if (_Extensions.EXTVertexShader)
	{
		for(uint k = 0; k < num; ++k)
		{					
			nglSetInvariantEXT(_EVSConstantHandle + index + k, GL_DOUBLE, (void *) (src + 4 * k));
		}
	}
}

// ***************************************************************************

const uint CDriverGL::GLMatrix[IDriver::NumMatrix]=
{
	GL_MODELVIEW,
	GL_PROJECTION,
	GL_MODELVIEW_PROJECTION_NV
};


// ***************************************************************************

const uint CDriverGL::GLTransform[IDriver::NumTransform]=
{
	GL_IDENTITY_NV,
	GL_INVERSE_NV,
	GL_TRANSPOSE_NV,
	GL_INVERSE_TRANSPOSE_NV
};


// ***************************************************************************

void CDriverGL::setConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		// First, ensure that the render setup is correclty setuped.
		refreshRenderSetup();

		// Track the matrix
		nglTrackMatrixNV (GL_VERTEX_PROGRAM_NV, index, GLMatrix[matrix], GLTransform[transform]);
		// Release Track => matrix data is copied.
		nglTrackMatrixNV (GL_VERTEX_PROGRAM_NV, index, GL_NONE, GL_IDENTITY_NV);
	}
	else if (_Extensions.EXTVertexShader)
	{
		// First, ensure that the render setup is correctly setuped.
		refreshRenderSetup();		
		CMatrix mat;		
		switch (matrix)
		{
			case IDriver::ModelView:
				mat = _ModelViewMatrix;
			break;
			case IDriver::Projection:
			{
				refreshProjMatrixFromGL();
				mat = _GLProjMat;
			}
			break;
			case IDriver::ModelViewProjection:
				refreshProjMatrixFromGL();				
				mat = _GLProjMat * _ModelViewMatrix;
			break;
		}

		switch(transform)
		{
			case IDriver::Identity: break;
			case IDriver::Inverse:
				mat.invert();
			break;		
			case IDriver::Transpose:
				mat.transpose();								
			break;
			case IDriver::InverseTranspose:
				mat.invert();
				mat.transpose();				
			break;
		}
		mat.transpose();
		float matDatas[16];
		mat.get(matDatas);
		nglSetInvariantEXT(_EVSConstantHandle + index, GL_FLOAT, matDatas);
		nglSetInvariantEXT(_EVSConstantHandle + index + 1, GL_FLOAT, matDatas + 4);
		nglSetInvariantEXT(_EVSConstantHandle + index + 2, GL_FLOAT, matDatas + 8);
		nglSetInvariantEXT(_EVSConstantHandle + index + 3, GL_FLOAT, matDatas + 12);
	}		 
}

// ***************************************************************************

void CDriverGL::enableVertexProgramDoubleSidedColor(bool doubleSided)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		// change mode (not cached because supposed to be rare)
		if(doubleSided)
			glEnable (GL_VERTEX_PROGRAM_TWO_SIDE_NV);
		else
			glDisable (GL_VERTEX_PROGRAM_TWO_SIDE_NV);
	}	
}


// ***************************************************************************
bool CDriverGL::supportVertexProgramDoubleSidedColor() const
{
	// currenlty only supported by NV_VERTEX_PROGRAM
	return _Extensions.NVVertexProgram;
}


} // NL3D
