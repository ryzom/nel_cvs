/** \file trav_scene.cpp
 * <File description>
 *
 * $Id: trav_scene.cpp,v 1.2 2002/04/15 12:03:05 lecroart Exp $
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

#include "std3d.h"

#include "3d/trav_scene.h"
#include "3d/scene.h"



namespace	NL3D
{

void ITravScene::addedToMOT(CMOT *mot)
{
	// this traversal has been added to a mot object
	// make sure this is a scene, and make sure it is attached to ONE scene only
	nlassert(dynamic_cast<CScene *>(mot));
	nlassert(Scene == NULL); // ERROR : traversal already added to a scene		
	Scene = static_cast<CScene *>(mot);

}

} // NL3D




/* End of trav_scene.cpp */
