/** \file driver_opengl_vertex_program.cpp
 * OpenGL driver implementation for vertex program manipulation.
 *
 * $Id: driver_opengl_vertex_program.cpp,v 1.12 2002/08/21 09:37:12 lecroart Exp $
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


using namespace std;
using namespace NLMISC;

 
namespace NL3D
{

// ***************************************************************************
class CVertexProgamDrvInfosGL : public IVertexProgramDrvInfos
{
public:
	// The GL Id.
	GLuint					ID;

	// The gl id is auto created here.
	CVertexProgamDrvInfosGL (CDriverGL *drv, ItVtxPrgDrvInfoPtrList it);
};


// ***************************************************************************
CVertexProgamDrvInfosGL::CVertexProgamDrvInfosGL (CDriverGL *drv, ItVtxPrgDrvInfoPtrList it) : IVertexProgramDrvInfos (drv, it) 
{
	// Extension must exist
	nlassert (drv->_Extensions.NVVertexProgram);

	// Generate a program
	nglGenProgramsNV (1, &ID);
}


// ***************************************************************************
bool CDriverGL::isVertexProgramSupported () const
{	
	return _Extensions.NVVertexProgram;
}

// ***************************************************************************
bool CDriverGL::isVertexProgramEmulated () const
{
	return _Extensions.NVVertexProgramEmulated;
}


// ***************************************************************************
bool CDriverGL::activeVertexProgram (CVertexProgram *program)
{
	// Extension here ?
	if (_Extensions.NVVertexProgram)
	{
		// Setup or unsetup ?
		if (program)
		{
			// Enable vertex program
			glEnable (GL_VERTEX_PROGRAM_NV);

			// Driver info
			CVertexProgamDrvInfosGL *drvInfo;

			// Program setuped ?
			if (program->_DrvInfo==NULL)
			{
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

			// Ok
			return true;
		}
		else // Unsetup
		{
			// Disable vertex program
			glDisable (GL_VERTEX_PROGRAM_NV);

			// Ok
			return true;
		}
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
}


// ***************************************************************************
void	CDriverGL::setConstant (uint index, uint num, const float *src)
{
	// Vertex program exist ?
	if (_Extensions.NVVertexProgram)
	{
		nglProgramParameters4fvNV(GL_VERTEX_PROGRAM_NV, index, num, src);
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

} // NL3D
