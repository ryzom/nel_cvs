/** \file ps_emitter.cpp
 * <File description>
 *
 * $Id: ps_emitter.cpp,v 1.5 2001/05/09 14:31:02 vizerie Exp $
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

#include "nel/3d/ps_emitter.h"


namespace NL3D {





/////////////////////////////////
// CPSFrequency implementation //
/////////////////////////////////
void CPSFrequency::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32)'NLPS') ;
	f.serialVersion(1) ;
	f.serialEnum(_FreqType) ;
	switch (_FreqType)
	{
		case CPSFrequency::regular:
			f.serial(_Period) ;
		break ;
		default:
		break ;
	}
	f.serial(_GenNb) ;
}




///////////////////////////////
// CPSEmitter implementation //
///////////////////////////////


CPSEmitter::CPSEmitter() : _EmittedType(NULL)
{
}


void CPSEmitter::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
if (pass != PSMotion) return ;
	// our behaviour depend of the frequency
	switch (_Freq._FreqType)
	{
		case (CPSFrequency::regular):
			{
				uint32 k, l;
				// we don't use an iterator here
				// because it could be invalidated if size change (a located ould generate itself)	

				const uint32 size = _Owner->getSize() ;

				for ( k = 0 ; k < size; ++k) 												   
				{
					_Phase[k] += ellapsedTime ;
					if ( _Phase[k] >= _Freq._Period) // phase is greater than period -> must emit
					{
						_Phase[k] -= _Freq._Period ;
						for (l = 0 ; l < _Freq._GenNb ; ++l) // emit one or several located
						{
							emit(k) ;					
						}
					}			
				}		
			}
		break ;	
	}
}


void CPSEmitter::newElement(void)
{	
	nlassert(_Phase.getSize() != _Phase.getMaxSize()) ;	

	_Phase.insert(0.f) ;

	if (_Freq._FreqType == CPSFrequency::once)
	{
		for (uint32 l = 0 ; l < _Freq._GenNb ; ++l) // emit one or several located
		{
			emit(_Owner->getNewElementIndex()) ;
		}
	}		
}

void CPSEmitter::deleteElement(uint32 index)
{	
	if (_Freq._FreqType == CPSFrequency::onDeath)
	{
		for (uint32 l = 0 ; l < _Freq._GenNb ; ++l) // emit one or several located
		{
			emit(index) ;
		}
	}
	_Phase.remove(index) ;
}

void CPSEmitter::resize(uint32 size)
{
	_Phase.resize(size) ;
}


void CPSEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'EMIT') ;
	CPSLocatedBindable::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Freq) ;
	f.serial(_Phase) ;
	f.serialPtr(_EmittedType) ;

}



void CPSEmitterOmni::emit(uint32 index)
{
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType) ;	
	
	CVector v( ((rand() % 1000) -500) / 500.0f
				   , ((rand() % 1000) -500) / 500.0f
				   , /*((rand() % 1000) -500) / 500.0f*/ (rand() % 1000) / 1000.0f) ;
	v.normalize() ;
	v *= 3.0f ;

		


	// retrieve the pos from the located that hold us, and the index of the emitter 
	_EmittedType->newElement(_Owner->getPos()[index], v, this) ;	
}



void CPSEmitterOmni::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSEmitter::serial(f) ;
}


} // NL3D
