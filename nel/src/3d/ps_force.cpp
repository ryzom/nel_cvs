/** \file ps_force.cpp
 * <File description>
 *
 * $Id: ps_force.cpp,v 1.10 2001/06/25 16:10:08 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "3d/ps_force.h"
#include "3d/driver.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "3d/vertex_buffer.h"
#include "3d/computed_string.h"
#include "3d/font_generator.h"
#include "3d/font_manager.h"
#include "nel/misc/common.h"
#include "3d/ps_util.h"

namespace NL3D {


/*
 * Constructor
 */
CPSForce::CPSForce()
{
}

void CPSForce::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSTargetLocatedBindable::serial(f) ;
	f.serialCheck((uint32) 'FORC') ;
	CPSLocatedBindable::serial(f) ;	
	f.serialVersion(1) ;		
}




void CPSForce::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	switch(pass)
	{
		case PSMotion:
			performMotion(ellapsedTime) ;
		break ;
		case PSToolRender:
			show(ellapsedTime) ;
		break ;
	}
}



////////////////////////////
// gravity implementation //
////////////////////////////


void CPSGravity::performMotion(CAnimationTime ellapsedTime)
{	
	CVector toAddLocal = ellapsedTime * CVector(0, 0, -_G) ;
	// perform the operation on each target

	CVector toAdd ;


	

	for (uint32 k = 0 ; k < _Owner->getSize() ; ++k)
	{	
		for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
		{

			toAdd = CPSLocated::getConversionMatrix(*it, this->_Owner).mulVector(toAddLocal) ; // express this in the target basis			

			uint32 size = (*it)->getSize() ;	
			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2end = (*it)->getSpeed().end() ;
			
			for (; it2 != it2end ; ++it2)
			{
				(*it2) += toAdd ;				
				
			}
		}
	}
}
void CPSGravity::show(CAnimationTime ellapsedTime) 
{	
	CVector I = computeI() ;
	CVector K = CVector(0,0,1) ;	    

	// this is not designed for efficiency (target : edition code)
	CPrimitiveBlock	 pb ;
	CVertexBuffer vb ;
	CMaterial material ;
	IDriver *driver = getDriver() ;
	const float toolSize = 0.2f ;

	vb.setVertexFormat(IDRV_VF_XYZ) ;
	vb.setNumVertices(6) ;
	vb.setVertexCoord(0, -toolSize * I) ;
	vb.setVertexCoord(1, toolSize * I) ;
	vb.setVertexCoord(2, CVector(0, 0, 0)) ;
	vb.setVertexCoord(3, -6.0f * toolSize * K) ;
	vb.setVertexCoord(4, -toolSize * I  - 5.0f * toolSize * K) ;
	vb.setVertexCoord(5, toolSize * I - 5.0f * toolSize * K) ;

	pb.reserveLine(4) ;
	pb.addLine(0, 1) ;
	pb.addLine(2, 3) ;
	pb.addLine(4, 3) ;
	pb.addLine(3, 5) ;	
	
	material.setColor(CRGBA(127, 127, 127)) ;
	material.setLighting(false) ;
	material.setBlendFunc(CMaterial::one, CMaterial::one) ;
	material.setZWrite(false) ;
	material.setBlend(true) ;

	
	CMatrix mat ;

	for (TPSAttribVector::const_iterator it = _Owner->getPos().begin() ; it != _Owner->getPos().end() ; ++it)
	{
		mat.identity() ;
		mat.translate(*it) ;
		if (_Owner->isInSystemBasis())
		{
			mat = getSysMat() * mat ;
		}
		
		driver->setupModelMatrix(mat) ;
		driver->activeVertexBuffer(vb) ;
		driver->render(pb, material) ;
	


		// affiche un g a cote de la force

		CVector pos = *it + CVector(1.5f * toolSize, 0, -1.2f * toolSize) ;

		if (_Owner->isInSystemBasis())
		{
			pos = getSysMat() * pos ;
		}

		// must have set this
		nlassert(getFontGenerator() && getFontGenerator()) ;
		
		CPSUtil::print(driver, std::string("G")
							, *getFontGenerator()
							, *getFontManager()
							, pos
							, 80.0f * toolSize ) ;								
	}

	
}

void CPSGravity::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSForce::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_G) ;
}


/////////////////////////////////
// CPSSpring  implementation   //
/////////////////////////////////



void CPSSpring::performMotion(CAnimationTime ellapsedTime)
{
	// for each spring, and each target, we check if they are in the same basis
	// if not, we need to transform the spring attachment pos into the target basis
	

	const float ellapsedTimexK = ellapsedTime * _K ;

	TPSAttribVector::const_iterator springPosIt = _Owner->getPos().begin()
									, springPosItEnd = _Owner->getPos().end() ;
	for (; springPosIt  != springPosItEnd ; ++springPosIt)
	{	
		for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
		{
			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;
			const CVector center = m * (*springPosIt) ;

			

			uint32 size = (*it)->getSize() ;
			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2End = (*it)->getSpeed().end() ;
			TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin() ;
			TPSAttribVector::const_iterator posIt = (*it)->getPos().begin() ;
			
			for (; it2 != it2End ; ++it2, ++invMassIt, ++posIt)
			{
				// apply the spring equation

				(*it2) += (*invMassIt) * ellapsedTimexK * (center - *posIt) ;								
			}
		}
	}
}


void CPSSpring::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSForce::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_K) ;
}



void CPSSpring::show(CAnimationTime ellapsedTime)
{
	CVector I = CVector::I ;
	CVector J = CVector::J ;

	const CVector tab[] = { -I + 2 * J, I + 2 * J
							, I + 2 * J, -I + J
							, -I + J, I + J
							, I + J, -I
							,  -I, I
							, I, -I - J
							, -I - J, I - J
							, I - J, - I - 2 * J
							, - I - 2 * J, I - 2 * J
							} ;
	const uint tabSize = sizeof(tab) / (2 * sizeof(CVector)) ;

	const float sSize = 0.08f ;
	displayIcon2d(tab, tabSize, sSize) ;
}






} // NL3D
