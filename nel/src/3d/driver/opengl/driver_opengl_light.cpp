/** \file driver_opengl_light.cpp
 * OpenGL driver implementation : light
 *
 * $Id: driver_opengl_light.cpp,v 1.8 2002/06/20 09:45:04 berenguier Exp $
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

#include "3d/light.h"

namespace NL3D 
{


// ***************************************************************************
uint	CDriverGL::getMaxLight () const
{
	// return min(maxLight supported by openGL, MaxLight=8).
	return _MaxDriverLight;
}

// ***************************************************************************

void	CDriverGL::setLight (uint8 num, const CLight& light)
{
	// Check light count is good
//	nlassert (num<_MaxDriverLight);
	
	// Set the light
	if (num<_MaxDriverLight)
	{
		// GL light number
		GLenum lightNum=(GLenum)(GL_LIGHT0+num);

		// Get light mode
		CLight::TLightMode mode=light.getMode ();

		// Copy the mode
		_LightMode[num]=mode;

		// Set the ambiant color
		GLfloat colorGL[4];
		CRGBA colorNeL=light.getAmbiant ();
		colorGL[0]=(float)colorNeL.R/255.f;
		colorGL[1]=(float)colorNeL.G/255.f;
		colorGL[2]=(float)colorNeL.B/255.f;
		colorGL[3]=1.f;
		glLightfv (lightNum, GL_AMBIENT, colorGL);

		// Set the diffuse color
		colorNeL=light.getDiffuse ();
		colorGL[0]=(float)colorNeL.R/255.f;
		colorGL[1]=(float)colorNeL.G/255.f;
		colorGL[2]=(float)colorNeL.B/255.f;
		colorGL[3]=1.f;
		glLightfv (lightNum, GL_DIFFUSE, colorGL);

		// Set the specular color
		colorNeL=light.getSpecular ();
		colorGL[0]=(float)colorNeL.R/255.f;
		colorGL[1]=(float)colorNeL.G/255.f;
		colorGL[2]=(float)colorNeL.B/255.f;
		colorGL[3]=1.f;
		glLightfv (lightNum, GL_SPECULAR, colorGL);

		// Set light attenuation
		glLightf (lightNum, GL_CONSTANT_ATTENUATION, light.getConstantAttenuation());
		glLightf (lightNum, GL_LINEAR_ATTENUATION, light.getLinearAttenuation());
		glLightf (lightNum, GL_QUADRATIC_ATTENUATION, light.getQuadraticAttenuation());

		// Set the position
		if ((mode==CLight::DirectionalLight)||(mode==CLight::SpotLight))
		{
			// Get the direction of the light
			_WorldLightDirection[num]=light.getDirection ();
		}

		if (mode!=CLight::DirectionalLight)
		{
			// Get the position of the light
			_WorldLightPos[num]=light.getPosition ();
		}

		if (mode==CLight::SpotLight)
		{
			// Get the exponent of the spot
			float exponent=light.getExponent ();

			// Set it
			glLightf (lightNum, GL_SPOT_EXPONENT, exponent);

			// Get the cutoff of the spot
			float cutoff=180.f*(light.getCutoff ()/(float)NLMISC::Pi);

			// Set it
			glLightf (lightNum, GL_SPOT_CUTOFF, cutoff);
		}
		else
		{
			// Disactive spot properties
			glLighti (lightNum, GL_SPOT_CUTOFF, 180);
			glLighti (lightNum, GL_SPOT_EXPONENT, 0);
		}

		// dirt the lightSetup and hence the render setup
		_LightSetupDirty= true;
		_RenderSetupDirty=true;
	}
}

// ***************************************************************************

void	CDriverGL::enableLight (uint8 num, bool enable)
{
	// Check light count is good
//	nlassert (num<_MaxDriverLight);

	// Enable glLight
	if (num<_MaxDriverLight)
	{
		// Enable the light
		_LightEnable[num]=enable;

		// Enable GL
		if (enable)
			glEnable ((GLenum)(GL_LIGHT0+num));
		else
			glDisable ((GLenum)(GL_LIGHT0+num));
	}
}

// ***************************************************************************

void	CDriverGL::setAmbientColor (CRGBA color)
{
	// Gl array
	GLfloat array[4];
	array[0]=(float)color.R/255.f;
	array[1]=(float)color.G/255.f;
	array[2]=(float)color.B/255.f;
	array[3]=1.f;

	// Set the color
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, array);
}


// ***************************************************************************
void				CDriverGL::cleanLightSetup ()
{
	// Should be dirty
	nlassert (_LightSetupDirty);
	
	// First light
	bool first=true;

	// For each lights
	for (uint i=0; i<_MaxDriverLight; i++)
	{
		// Is this light enabled ?
		if (_LightEnable[i])
		{
			// If first light
			if (first)
			{
				first=false;

				// Push the matrix
				glPushMatrix ();

				// Load the view matrix
				glLoadMatrixf (_ViewMtx.get());
			}

			// Light is directionnal ?
			if (_LightMode[i]==(uint)CLight::DirectionalLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=-_WorldLightDirection[i].x;
				vectorGL[1]=-_WorldLightDirection[i].y;
				vectorGL[2]=-_WorldLightDirection[i].z;
				vectorGL[3]=0.f;
	
				// Set it
				glLightfv ((GLenum)(GL_LIGHT0+i), (GLenum)GL_POSITION, vectorGL);
			}

			// Spotlight ?
			if (_LightMode[i]==(uint)CLight::SpotLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=_WorldLightDirection[i].x;
				vectorGL[1]=_WorldLightDirection[i].y;
				vectorGL[2]=_WorldLightDirection[i].z;
	
				// Set it
				glLightfv ((GLenum)(GL_LIGHT0+i), (GLenum)GL_SPOT_DIRECTION, vectorGL);
			}

			// Position 
			if (_LightMode[i]!=(uint)CLight::DirectionalLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				// Must Substract CameraPos, because ViewMtx may not be the exact view.
				vectorGL[0]=_WorldLightPos[i].x - _PZBCameraPos.x;
				vectorGL[1]=_WorldLightPos[i].y - _PZBCameraPos.y;
				vectorGL[2]=_WorldLightPos[i].z - _PZBCameraPos.z;
				vectorGL[3]=1.f;
	
				// Set it
				glLightfv ((GLenum)(GL_LIGHT0+i), (GLenum)GL_POSITION, vectorGL);
			}
		}
	}

	// Pop old matrix
	if (!first)
		glPopMatrix ();

	// Clean flag
	_LightSetupDirty=false;
}


} // NL3D
