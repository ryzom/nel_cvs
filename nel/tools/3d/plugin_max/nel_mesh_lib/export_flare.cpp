/** \file export_flare.cpp
 *
 * $Id: export_flare.cpp,v 1.1 2002/07/03 09:15:24 vizerie Exp $
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



#include "StdAfx.h"
#include "export_nel.h"
#include <3d/flare_shape.h>


using namespace NL3D;

IShape* CExportNel::buildFlare(INode& node, TimeValue time)
{
	// build the shape
	CFlareShape *fshape = new CFlareShape;
	Point3 col;
	float persistence, size, spacing, attenuationRange, pos, maxViewDist = 1000.f, maxViewDistRatio = 0.9f;
	int   attenuable;
	int	  firstFlareKeepSize;
	int   hasDazzle;
	// retrieve the color of the flare from the node
	CExportNel::getValueByNameUsingParamBlock2(node, "ColorParam", (ParamType2)TYPE_RGBA, &col, 0);
	fshape->setColor(NLMISC::CRGBA((uint) (255.f * col.x), (uint) (255.f * col.y), (uint) (255.f * col.z)));
	// retrieve the persistence of the flare
	CExportNel::getValueByNameUsingParamBlock2(node, "PersistenceParam", (ParamType2)TYPE_FLOAT, &persistence, 0);
	fshape->setPersistence(persistence);
	// retrieve spacing of the flare
	CExportNel::getValueByNameUsingParamBlock2(node, "Spacing", (ParamType2)TYPE_FLOAT, &spacing, 0);
	fshape->setFlareSpacing(spacing);
	// retrieve use of radial attenuation
	CExportNel::getValueByNameUsingParamBlock2(node, "Attenuable", (ParamType2) TYPE_BOOL, &attenuable, 0);			
	if (attenuable)
	{
		fshape->setAttenuable();
		CExportNel::getValueByNameUsingParamBlock2(node, "AttenuationRange", (ParamType2) TYPE_FLOAT, &attenuationRange, 0);			
		fshape->setAttenuationRange(attenuationRange);
	}			
	CExportNel::getValueByNameUsingParamBlock2(node, "FirstFlareKeepSize", (ParamType2) TYPE_BOOL, &firstFlareKeepSize, 0);			
	fshape->setFirstFlareKeepSize(firstFlareKeepSize ? true : false); // avoid VC++ warning
	/// check for dazzle
	CExportNel::getValueByNameUsingParamBlock2(node, "HasDazzle", (ParamType2) TYPE_BOOL, &hasDazzle, 0);			
	if (hasDazzle)
	{
		fshape->enableDazzle();
		// get dazzle color
		CExportNel::getValueByNameUsingParamBlock2(node, "DazzleColor", (ParamType2) TYPE_RGBA, &col, 0);			
		fshape->setDazzleColor(NLMISC::CRGBA((uint) (255.f * col.x), (uint) (255.f * col.y), (uint) (255.f * col.z)));
		// get dazzle attenuation range
		CExportNel::getValueByNameUsingParamBlock2(node, "DazzleAttenuationRange", (ParamType2) TYPE_FLOAT, &attenuationRange, 0);			
		fshape->setDazzleAttenuationRange(attenuationRange);
	}
	/// retrieve maxViewDistRatio
	CExportNel::getValueByNameUsingParamBlock2(node, "MaxViewDist", (ParamType2) TYPE_FLOAT, &maxViewDist, 0);
	fshape->setMaxViewDist(maxViewDist);
	/// retrieve maxViewDistRatio
	CExportNel::getValueByNameUsingParamBlock2(node, "MaxViewDistRatio", (ParamType2) TYPE_FLOAT, &maxViewDistRatio, 0);
	fshape->setMaxViewDistRatio(maxViewDistRatio);
	// retrieve sizes & tex
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
		char out[16]; sprintf(out, "size%d", k);
		CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2)TYPE_FLOAT, &size, 0);
		fshape->setSize(k, size);
		// get relative position
		sprintf(out, "pos%d", k);
		CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2)TYPE_FLOAT, &pos, 0);
		fshape->setRelativePos(k, pos);

		// check wether the flare is used
		int texUsed;
		sprintf(out, "flareUsed%d", k);
		CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2) TYPE_BOOL, &texUsed, 0);			
		if (texUsed)
		{
			sprintf(out, "texFileName%d", k);
			// retrieve the texture name
			std::string fileName;
			CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2) TYPE_STRING, &fileName, 0);
			if (_AbsolutePath)
			{
				fshape->setTexture(k, new NL3D::CTextureFile(fileName.c_str()));
			}
			else
			{
				char fName[_MAX_FNAME];
				// get file name only
				::_splitpath(fileName.c_str(), NULL, NULL, fName, NULL);
				fshape->setTexture(k, new NL3D::CTextureFile(fName));
			}
		}
		else
		{
			fshape->setTexture(k, NULL);
		}	
	}
	// Get the node matrix
	Matrix3 localTM;
	getLocalMatrix (localTM, node, time);			
	Point3  fp = localTM.GetTrans();
	// export default transformation
	fshape->getDefaultPos()->setValue( CVector(fp.x, fp.y, fp.z) );						
	return fshape;
}