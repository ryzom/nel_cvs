/** \file landscape_face_vector_manager.h
 * <File description>
 *
 * $Id: landscape_face_vector_manager.h,v 1.1 2001/09/14 09:44:26 berenguier Exp $
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

#ifndef NL_LANDSCAPE_FACE_VECTOR_MANAGER_H
#define NL_LANDSCAPE_FACE_VECTOR_MANAGER_H

#include "nel/misc/types_nl.h"
#include <vector>


namespace NL3D {


class	CLandscapeFaceVectorManager;


// ***************************************************************************
/**
 * Fast Allocate blocks of faces, according to the size of the block.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CLandscapeFaceVector
{
public:
	CLandscapeFaceVector();
	// can't copy a faceVector (assert).
	CLandscapeFaceVector(const CLandscapeFaceVector &o);
	CLandscapeFaceVector &operator=(const CLandscapeFaceVector &o);
	~CLandscapeFaceVector();

	/// Array of triangles indices.
	uint32		*TriPtr;
	/// Number of triangles.
	uint32		NumTri;

private:
	friend	class	CLandscapeFaceVectorManager;
	sint					_BlockId;
	CLandscapeFaceVector	*_Next;
};


// ***************************************************************************
/**
 * Fast Allocate blocks of faces, according to the size of the block.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeFaceVectorManager
{
public:

	/// Constructor
	CLandscapeFaceVectorManager();
	~CLandscapeFaceVectorManager();


	// Empty the Free List. All FaceVector must be deleted.
	void					purge();
	CLandscapeFaceVector	*createFaceVector(uint numTri);
	void					deleteFaceVector(CLandscapeFaceVector	*fv);


private:
	// Array of List.
	std::vector<CLandscapeFaceVector*>	_Blocks;


};


} // NL3D


#endif // NL_LANDSCAPE_FACE_VECTOR_MANAGER_H

/* End of landscape_face_vector_manager.h */
