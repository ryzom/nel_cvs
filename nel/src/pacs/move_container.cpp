/** \file move_container.cpp
 * <File description>
 *
 * $Id: move_container.cpp,v 1.2 2001/05/22 08:35:19 corvazier Exp $
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

#include "nel/pacs/move_primitive.h"
#include "nel/pacs/move_element.h"
#include "nel/pacs/collision_callback.h"

using namespace NLMISC;

namespace NLPACS 
{

// ***************************************************************************

CMoveContainer::CMoveContainer (float xmin, float ymin, float xmax, float ymax, uint widthCellCount, uint heightCellCount, 
								float primitiveMaxSize, uint otSize) : _AllocOTInfo (100)
{
	init (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, otSize);
}

// ***************************************************************************

CMoveContainer::~CMoveContainer ()
{
	clear ();
}

// ***************************************************************************

void CMoveContainer::clear ()
{
	// Clear all primitives
	std::set<CMovePrimitive*>::iterator ite=_PrimitiveSet.begin();
	while (ite!=_PrimitiveSet.end ())
	{
		freePrimitive (*ite);
		ite++;
	}

	// Clear primitive set
	_PrimitiveSet.clear ();

	// Clear cell array
	_VectorCell.clear ();
}

// ***************************************************************************

void CMoveContainer::init (float xmin, float ymin, float xmax, float ymax, uint widthCellCount, uint heightCellCount, 
						   float primitiveMaxSize, uint otSize)
{
	// Clear arrays
	clear ();

	// Not in test mode
	_Begin=false;
	_ChangedRoot=NULL;

	// Element size
	_PrimitiveMaxSize=primitiveMaxSize;

	// BB
	_Xmin=xmin;
	_Ymin=ymin;
	_Xmax=xmax;
	_Ymax=ymax;

	// Cells count
	_CellCountWidth=widthCellCount;
	_CellCountHeight=heightCellCount;

	// Cells size
	_CellWidth=(_Xmax - _Xmin)/(float)_CellCountWidth;
	_CellHeight=(_Ymax - _Ymin)/(float)_CellCountHeight;

	// Cell array
	_VectorCell.resize (_CellCountWidth * _CellCountHeight);

	// resize OT
	_OtSize=otSize;
	_TimeOT.resize (otSize);
}

// ***************************************************************************

void CMoveContainer::evalBegin (float deltaTime, ICollisionCallback& callBack)
{
	// Check begin / end 
	nlassert (!_Begin);

	// Delta time
	_DeltaTime=deltaTime;

	// Update the bounding box and position of modified primitives
	updatePrimitives (0.f);

	// Clear the OT
	clearOT ();

	// Clear modified primitive's collision list
	clearModifiedPrimitiveCollisionList ();

#ifdef NL_DEBUG
	// Check list integrity
	checkSortedList ();
#endif // NL_DEBUG

	// Eval all collisions
	evalAllCollisions (0.f);

	// Clear modified list
	clearModifiedList ();

	// Get first collision
	_NextCollision = _TimeOT[0].getNextInfo ();

	// Begin
	_Begin=false;
}

// ***************************************************************************

bool CMoveContainer::evalCollision ()
{
	// Check begin / end 
	nlassert (_Begin);

	// Collision ?
	if (_NextCollision)
	{
		// Keep this collision
		if ( _Callback->reaction (*_NextCollision->getFirstPrimitive(), *_NextCollision->getSecondPrimitive(), _NextCollision->getCollisionDesc()) )
		{
			// Last time
			float newTime=_NextCollision->getCollisionDesc ().ContactTime;

			// Get next collision
			nlassert (_NextCollision!=NULL);
			_NextCollision = _NextCollision->getNextInfo ();

			// Remove modified objects from the OT
			removeModifiedFromOT (newTime);

			// Update the bounding box and position of modified primitives
			updatePrimitives (newTime);

			// Eval all collisions of modified objects for the new delta t
			evalAllCollisions (newTime);

			// Clear modified list
			clearModifiedList ();
		}

		// Return ok
		return true;
	}
	else
	{
		// Free ordered table info
		freeAllOTInfo ();

		// Some init
		_NextCollision=NULL;

		// End
		_Begin=false;

		return false;
	}
}

// ***************************************************************************

void CMoveContainer::updatePrimitives (float beginTime)
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot;
	while (changed)
	{
		// Force the build of the bounding box
		changed->update (beginTime, _DeltaTime);

		// Compute cells overlaped by the primitive
		updateCells (changed);

		// Next primitive
		changed=changed->getNextModified();
	}
}

// ***************************************************************************

void CMoveContainer::updateCells (CMovePrimitive *primitive)
{
	// Check BB width not too large
	nlassert (primitive->getBBXMax() - primitive->getBBXMin() < _CellWidth);

	// Check BB height not too large
	nlassert (primitive->getBBYMax() - primitive->getBBYMin() < _CellHeight);

	// Get coordinate in the cell array
	int minx=(int)floor ((primitive->getBBXMin() - _Xmin) / _CellWidth);
	int miny=(int)floor ((primitive->getBBYMin() - _Ymin) / _CellHeight);
	int maxx=(int)floor ((primitive->getBBXMax() - _Ymin) / _CellWidth);
	int maxy=(int)floor ((primitive->getBBYMax() - _Ymin) / _CellHeight);

	// Born
	clamp (minx, 0, (int)(_CellCountWidth-1));
	clamp (miny, 0, (int)(_CellCountHeight-1));
	clamp (maxx, 0, (int)(_CellCountWidth-1));
	clamp (maxy, 0, (int)(_CellCountHeight-1));
	maxx=std::min (minx+1, maxx);
	maxy=std::min (miny+1, maxy);

	// flags founded
	bool found[4]={false, false, false, false};

	// For each old cells
	uint i;
	for (i=0; i<4; i++)
	{
		// Element
		CMoveElement *elm = primitive->getMoveElement (i);

		// Old element in this cell ?
		if ( elm )
		{
			// Must remove it ?
			if ( (elm->X < minx) && (elm->X > maxx) && (elm->Y < miny) && (elm->Y > maxy) )
			{
				// Yes remove it
				primitive->removeMoveElement (i);
			}
			else
			{
				// Check found cells
				found[elm->X - minx + ((elm->Y - miny) << (maxx-minx)) ]=true;
			}
		}
	}

	// For each case selected
	int x, y;
	i=0;
	for (y=miny; y<maxy; y++)
	for (x=minx; x<maxx; x++)
	{
		// If the cell is not found
		if (!found[i])
		{
			// Center of the cell
			float cx=((float)x)*_CellWidth+_Xmin;
			float cy=((float)y)*_CellHeight+_Ymin;

			// Add it in the list
			primitive->addMoveElement (_VectorCell[x+y*_CellCountWidth], cx, cy);
		}

		// Next cell
		i++;
	}
}

// ***************************************************************************

void CMoveContainer::clearModifiedList ()
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot;
	while (changed)
	{
		// Copy pointer
		CMovePrimitive *primitive=changed;
		
		// Next primitive
		changed=changed->getNextModified ();

		// Remove it from the list
		primitive->clearInModifiedListFlag ();
	}

	// Empty list
	_ChangedRoot=NULL;
}

// ***************************************************************************

void CMoveContainer::checkSortedList ()
{
	// Check each primitives in the set
	std::set<CMovePrimitive*>::iterator ite=_PrimitiveSet.begin();
	while (ite!=_PrimitiveSet.end())
	{
		// Check
		(*ite)->checkSortedList ();

		ite++;
	}
}

// ***************************************************************************

void CMoveContainer::evalAllCollisions (float beginTime)
{
	// First primitive
	CMovePrimitive	*primitive=_ChangedRoot;

	// For each modified primitive
	while (primitive)
	{
		// Find its collisions
		
		// For each move element
		for (uint i=0; i<4; i++)
		{
			// Get the element
			CMoveElement	*elm=primitive->getMoveElement (i);

			// Element valid ?
			if (elm)
			{
				// Check
				nlassert (elm->Primitive==primitive);

				// Lookup in X sorted list on the left
				CMoveElement	*other=elm->PreviousX;

				// Primitive to the left
				while (other && (primitive->getBBXMin() - other->Primitive->getBBXMin() < _PrimitiveMaxSize) )
				{
					// Other primitive
					CMovePrimitive	*otherPrimitive=other->Primitive;

					// Look if valid in X
					if (primitive->getBBXMin() < otherPrimitive->getBBXMax())
					{
						// Look if valid in Y
						if ( (primitive->getBBYMin() < otherPrimitive->getBBYMax()) && (otherPrimitive->getBBYMin() < primitive->getBBYMax()) )
						{
							// Test the primitive
							CCollisionDesc desc;
							if (primitive->evalCollision (*otherPrimitive, desc, beginTime, _DeltaTime))
							{
								// OK, collision
								newCollision (primitive, otherPrimitive, desc);
							}
						}
					}

					// Next primitive to the left
					other = other->PreviousX;
				}

				// Lookup in X sorted list on the right
				other=elm->NextX;

				// Primitive to the right
				while (other && (other->Primitive->getBBXMin() < primitive->getBBXMax()) )
				{
					// Other primitive
					CMovePrimitive	*otherPrimitive=other->Primitive;

					// Look if valid in Y
					if ( (primitive->getBBYMin() < otherPrimitive->getBBYMax()) && (otherPrimitive->getBBYMin() < primitive->getBBYMax()) )
					{
						// Test the primitive
						CCollisionDesc desc;
						if (primitive->evalCollision (*otherPrimitive, desc, beginTime, _DeltaTime))
						{
							// OK, collision
							newCollision (primitive, otherPrimitive, desc);
						}
					}

					// Next primitive to the left
					other = other->NextX;
				}
			}
		}

		// Next primitive
		primitive=primitive->getNextModified ();
	}
}

// ***************************************************************************

void CMoveContainer::newCollision (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc)
{
	// Get an ordered time index. Always round to the future.
	int index=(int)(ceil (desc.ContactTime*(float)_OtSize/_DeltaTime) );

	// Clamp left.
	if (index<0)
		index=0;

	// If in time
	if (index<(int)_OtSize)
	{
		// Rounded time
		float finalTime=(float)index * _DeltaTime / (float)_OtSize;

		// Build info
		CCollisionOTInfo *info = allocateOTInfo ();
		info->init (first, second, desc);

		// Add in the primitive list
		first->addCollisionOTInfo (info);
		second->addCollisionOTInfo (info);

		// Insert in the time ordered table
		nlassert (index<(int)_TimeOT.size());
		_TimeOT[index].link (info);
	}
}

// ***************************************************************************

void CMoveContainer::clearOT ()
{
	// Check
	nlassert (_OtSize==_TimeOT.size());

	// clear the list
	uint i;
	for (i=0; i<_OtSize; i++)
		_TimeOT[i].clear ();

	// Relink the list
	for (i=0; i<_OtSize-1; i++)
		// Link the two cells
		_TimeOT[i].link (&(_TimeOT[i+1]));
}

// ***************************************************************************

void CMoveContainer::removeModifiedFromOT (float beginTime)
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot;
	while (changed)
	{
		// Remove from ot list
		changed->removeCollisionOTInfo (beginTime);
		
		// Next primitive
		changed=changed->getNextModified ();
	}
}

// ***************************************************************************

void CMoveContainer::clearModifiedPrimitiveCollisionList ()
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot;
	while (changed)
	{
		// Remove from ot list
		changed->clearCollisionList ();
		
		// Next primitive
		changed=changed->getNextModified ();
	}
}

// ***************************************************************************

CCollisionOTInfo *CMoveContainer::allocateOTInfo ()
{
	return _AllocOTInfo.allocate ();
}

// ***************************************************************************

// Free all ordered table info
void CMoveContainer::freeAllOTInfo ()
{
	_AllocOTInfo.free ();
}

// ***************************************************************************

CMovePrimitive *CMoveContainer::allocatePrimitive ()
{
	// Simply allocate
	return new CMovePrimitive;
}

// ***************************************************************************

void CMoveContainer::freePrimitive (CMovePrimitive *primitive)
{
	// Simply deallocate
	delete primitive;
}

// ***************************************************************************

CMoveElement *CMoveContainer::allocateMoveElement ()
{
	// Simply allocate
	return new CMoveElement;
}

// ***************************************************************************

void CMoveContainer::freeMoveElement (CMoveElement *element)
{
	// Simply deallocate
	delete element;
}

// ***************************************************************************

} // NLPACS
