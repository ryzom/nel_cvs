/** \file ps_edit.h
 * <File description>
 *
 * $Id: ps_edit.h,v 1.4 2001/05/30 10:05:58 vizerie Exp $
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

#ifndef NL_PS_EDIT_H
#define NL_PS_EDIT_H

#include "nel/misc/types_nl.h"



namespace NLMISC
{
	class CMatrix ;
	class CVector ;
} ;


namespace NL3D {


/**
 * In this file, we define interfaces to interact more precisely with located in a particle system
 * It allows to manipulate individual elements of a located
 * For example, it allows you to move a collision plane after its creation,
 * or to move a particular particle that you've instancied yourself.
 * This is needed because of the packed format used for data representation
 * (each attribute is packed into its own table, see ps_attrib.h )
 */



/**
 * This interface allow to move a particle system element given its index in a located
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */

struct IPSMover
{
	/** Send back true if non isometric transformation can be applied
	 *  If it sends false, the use of non isometric transformation leads to undefine results
	 */
	virtual bool supportNonOrthonormalBasis(void) const { return false ; }


	/** Send back true if uniform scaling can be applied
	 *  If it sends false, uniform scaling leads to undefine results
	 */
	virtual bool supportUniformScaling(void) const { return false ; }

	/** Send back true if non-uniform (general) scaling can be applied
	 *  If it sends false, non-uniform scaling leads to undefine results
	 */
	virtual bool supportScaling(void) const { return false ; }

	/// send back true if non-orthogonal transforms are supported
	virtual bool supportNonOrthogonalTransform(void) const { return false ; }

	
	// left multiply the current matrix by the given one. No valid index -> assert
	virtual void applyMatrix(uint32 index, const CMatrix &m) = 0 ;
	// return a matrix of the system. No valid index -> assert
	virtual CMatrix getMatrix(uint32 index) const = 0 ;

	// Shortcut to make a translation. Index must be valid
	virtual void translate(uint32 index, const CVector &v)
	{
		CMatrix m ;
		m.translate(v) ;
		applyMatrix(index, m) ;
	}
};



} // NL3D


#endif // NL_PS_EDIT_H

/* End of ps_edit.h */
