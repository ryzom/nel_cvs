/** \file move_container.cpp
 * <File description>
 *
 * $Id: move_container.cpp,v 1.3 2001/05/31 13:36:42 corvazier Exp $
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

using namespace NLMISC;
//#define EPSILON_TIME (0.01f)

namespace NLPACS 
{

// ***************************************************************************

CMoveContainer::CMoveContainer (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, 
								double primitiveMaxSize, uint otSize) : _AllocOTInfo (100)
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

void CMoveContainer::init (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, 
						   double primitiveMaxSize, uint otSize)
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
	_CellWidth=(_Xmax - _Xmin)/(double)_CellCountWidth;
	_CellHeight=(_Ymax - _Ymin)/(double)_CellCountHeight;

	// Cell array
	_VectorCell.resize (_CellCountWidth * _CellCountHeight);

	// resize OT
	_OtSize=otSize;
	_TimeOT.resize (otSize);

	// Clear the OT
	clearOT ();

	// Clear test time
	_TestTime=0xffffffff;
}

// ***************************************************************************

void CMoveContainer::evalBegin (double deltaTime)
{
	// Check begin / end 
	nlassert (!_Begin);

	// New test time
	_TestTime++;

	// Delta time
	_DeltaTime=deltaTime;

	// Update the bounding box and position of modified primitives
	updatePrimitives (0.f);

#ifdef NL_DEBUG
	// Check list integrity
	checkSortedList ();
#endif // NL_DEBUG

	// Eval all collisions
	evalAllCollisions (0.f);

	// Clear modified list
	clearModifiedList ();

	// Get first collision
	_PreviousCollisionNode = &_TimeOT[0];

	// Begin
	_Begin=true;
}

// ***************************************************************************

bool CMoveContainer::evalCollision ()
{
	// Check begin / end 
	nlassert (_Begin);

	// Modified list is empty at this point
	nlassert (_ChangedRoot==NULL);

	// Previous node is a 'hard' OT node
	nlassert (!_PreviousCollisionNode->isInfo());

	// Get next collision
	CCollisionOTInfo	*nextCollision=_PreviousCollisionNode->getNextInfo ();

	// Collision ?
	if (nextCollision)
	{
		// Get new previous OT hard node
		_PreviousCollisionNode=nextCollision->getPrevious ();

		// Previous node is a 'hard' OT node
		nlassert (!_PreviousCollisionNode->isInfo());

		// Keep this collision
		if ( reaction (*nextCollision->getFirstPrimitive(), *nextCollision->getSecondPrimitive(), nextCollision->getCollisionDesc()) )
		{
			// Last time
			double newTime=nextCollision->getCollisionDesc ().ContactTime;

			// Remove modified objects from the OT
			removeModifiedFromOT ();

			// Must have been removed
			nlassert (nextCollision->getPrevious ()==NULL);
			nlassert (nextCollision->CCollisionOT::getNext ()==NULL);

			// Update the bounding box and position of modified primitives
			updatePrimitives (newTime);

			// Eval all collisions of modified objects for the new delta t
			evalAllCollisions (newTime);

			// Clear modified list
			clearModifiedList ();
		}
		else
		{
			// Remove this collision info from OT
			nextCollision->unlink ();

			// Remove this collision from the primitives
			nextCollision->getFirstPrimitive()->removeCollisionOTInfo (nextCollision);
			nextCollision->getSecondPrimitive()->removeCollisionOTInfo (nextCollision);
		}

		// Return ok
		return true;
	}
	else
	{
		// OT must be cleared
		checkOT ();

		// TODO remove clearOT.
		//clearOT ();

		// Free ordered table info
		freeAllOTInfo ();

		// Some init
		_PreviousCollisionNode=NULL;

		// End
		_Begin=false;

		return false;
	}
}

// ***************************************************************************

void CMoveContainer::updatePrimitives (double beginTime)
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
	nlassertonce (primitive->getBBXMax() - primitive->getBBXMin() < _CellWidth);

	// Check BB height not too large
	nlassertonce (primitive->getBBYMax() - primitive->getBBYMin() < _CellHeight);

	// Get coordinate in the cell array
	int minx=(int)floor ((primitive->getBBXMin() - _Xmin) / _CellWidth);
	int miny=(int)floor ((primitive->getBBYMin() - _Ymin) / _CellHeight);
	int maxx=(int)floor ((primitive->getBBXMax() - _Xmin) / _CellWidth);
	int maxy=(int)floor ((primitive->getBBYMax() - _Ymin) / _CellHeight);

	// Born
	if (minx<0)
		minx=0;
	if (miny<0)
		miny=0;
	if (maxx>=(int)_CellCountWidth)
		maxx=(int)_CellCountWidth-1;
	if (maxy>=(int)_CellCountHeight)
		maxy=(int)_CellCountHeight-1;

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
			// Check
			nlassert (elm->X>=0);
			nlassert (elm->X<_CellCountWidth);
			nlassert (elm->Y>=0);
			nlassert (elm->Y<_CellCountHeight);

			// Must remove it ?
			if ( (elm->X < minx) || (elm->X > maxx) || (elm->Y < miny) || (elm->Y > maxy) )
			{
				// Yes remove it
				primitive->removeMoveElement (i);
			}
			else
			{
				// Checks
				nlassert (((elm->X - minx)==0)||((elm->X - minx)==1));
				nlassert (((elm->Y - miny)==0)||((elm->Y - miny)==1));

				// Update position
#ifndef TEST_CELL
				_VectorCell[elm->X+elm->Y*_CellCountWidth].updateSortedLists (elm);
#endif TEST_CELL

				// Check found cells
				found[ elm->X - minx + ((elm->Y - miny) << (maxx-minx)) ]=true;
			}
		}
	}

	// For each case selected
	uint x, y;
	i=0;
	for (y=miny; y<=(uint)maxy; y++)
	for (x=minx; x<=(uint)maxx; x++)
	{
		// Check the formula
		nlassert (i == (x - minx + ((y - miny) << (maxx-minx)) ));

		// If the cell is not found
		if (!found[i])
		{
			// Center of the cell
			double cx=((double)x+0.5f)*_CellWidth+_Xmin;
			double cy=((double)y+0.5f)*_CellHeight+_Ymin;

			// Add it in the list
			primitive->addMoveElement (_VectorCell[x+y*_CellCountWidth], (uint16)x, (uint16)y, cx, cy);
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
		primitive->setInModifiedListFlag (false);
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

void CMoveContainer::evalAllCollisions (double beginTime)
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
				// Primitive to the left
#ifdef TEST_CELL
				// Lookup in X sorted list on the left
				CMoveElement	*other=_VectorCell[elm->X+elm->Y*_CellCountWidth].getRootX ();

				while (other)
				{
					CMovePrimitive	*otherP=other->Primitive;
					if ( (!otherP->isInModifiedListFlag ()) || (primitive<otherP) )
					{
						if ((primitive->getBBXMin()<otherP->getBBXMax())&&(otherP->getBBXMin()<primitive->getBBXMax())&&
							(primitive->getBBYMin()<otherP->getBBYMax())&&(otherP->getBBYMin()<primitive->getBBYMax()))
						{
							// Test the primitive
							CCollisionDesc desc;
							if (primitive->evalCollision (*otherP, desc, beginTime, _DeltaTime, _TestTime))
							{
								// Check time interval
								nlassert (beginTime<=desc.ContactTime);
								nlassert (desc.ContactTime<_DeltaTime);

								// OK, collision
								newCollision (primitive, otherP, desc);
							}
						}
					}
					// Next primitive to the left
					other = other->NextX;
				}
#else // TEST_CELL

				// Lookup in X sorted list on the left
				CMoveElement	*other=elm->PreviousX;
				nlassert (other!=elm);

				while (other && (primitive->getBBXMin() - other->Primitive->getBBXMin() < _PrimitiveMaxSize) )
				{
					// Other primitive
					CMovePrimitive	*otherPrimitive=other->Primitive;
					nlassert (otherPrimitive!=primitive);

					// Continue the check if the other primitive is not int the modified list or if its pointer is higher than primitive
					if ( (!otherPrimitive->isInModifiedListFlag ()) || (primitive<otherPrimitive) )
					{
						// Look if valid in X
						if (primitive->getBBXMin() < otherPrimitive->getBBXMax())
						{
							// Look if valid in Y
							if ( (primitive->getBBYMin() < otherPrimitive->getBBYMax()) && (otherPrimitive->getBBYMin() < primitive->getBBYMax()) )
							{
								// Test the primitive
								CCollisionDesc desc;
								if (primitive->evalCollision (*otherPrimitive, desc, beginTime, _DeltaTime, _TestTime))
								{
									// Check time interval
									nlassert (beginTime<=desc.ContactTime);
									nlassert (desc.ContactTime<_DeltaTime);

									// OK, collision
									newCollision (primitive, otherPrimitive, desc);
								}
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
					nlassert (otherPrimitive!=primitive);

					// Continue the check if the other primitive is not in the modified list or if its pointer is higher than primitive
					if ( (!otherPrimitive->isInModifiedListFlag ()) || (primitive<otherPrimitive) )
					{
						// Look if valid in Y
						if ( (primitive->getBBYMin() < otherPrimitive->getBBYMax()) && (otherPrimitive->getBBYMin() < primitive->getBBYMax()) )
						{
							// Test the primitive
							CCollisionDesc desc;
							if (primitive->evalCollision (*otherPrimitive, desc, beginTime, _DeltaTime, _TestTime))
							{
								// OK, collision
								newCollision (primitive, otherPrimitive, desc);
							}
						}
					}

					// Next primitive to the left
					other = other->NextX;
				}
#endif // TEST_CELL
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
	int index=(int)(ceil (desc.ContactTime*(double)_OtSize/_DeltaTime) );

	// Clamp left.
	if (index<0)
		index=0;

	// If in time
	if (index<(int)_OtSize)
	{
		// Rounded time
		double finalTime=(double)index * _DeltaTime / (double)_OtSize;

		// Build info
		CCollisionOTInfo *info = allocateOTInfo ();
		info->init (first, second, desc);

		// Add in the primitive list
		first->addCollisionOTInfo (info);
		second->addCollisionOTInfo (info);

		// Insert in the time ordered table
		nlassert (index<(int)_TimeOT.size());
		_TimeOT[index].link (info);

		// Check it is after the last hard collision
		nlassert (_PreviousCollisionNode<=&_TimeOT[index]);
	}
}

// ***************************************************************************

void CMoveContainer::checkOT ()
{
	// Check
	nlassert (_OtSize==_TimeOT.size());

	// Check linked list
	for (uint i=0; i<_OtSize-1; i++)
	{
		// Check link
		nlassert ( _TimeOT[i].getNext() == (&(_TimeOT[i+1])) );
		nlassert ( _TimeOT[i+1].getPrevious() == (&(_TimeOT[i])) );
	}

	// Check first and last
	nlassert ( _TimeOT[0].getPrevious() == NULL );
	nlassert ( _TimeOT[_OtSize-1].getNext() == NULL );
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

void CMoveContainer::removeModifiedFromOT ()
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot;
	while (changed)
	{
		// Remove from ot list
		changed->removeCollisionOTInfo ();
		
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
	return new CMovePrimitive (this);
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

UMoveContainer *UMoveContainer::createMoveContainer (double xmin, double ymin, double xmax, double ymax, 
		uint widthCellCount, uint heightCellCount, double primitiveMaxSize, uint otSize)
{
	// Create a CMoveContainer
	return new CMoveContainer (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, otSize);
}

// ***************************************************************************

void	UMoveContainer::deleteMoveContainer (UMoveContainer	*container)
{
	delete (CMoveContainer*)container;
}

// ***************************************************************************

UMovePrimitive *CMoveContainer::addPrimitive ()
{
	// Allocate primitive
	CMovePrimitive *primitive=allocatePrimitive ();

	// Add into the set
	_PrimitiveSet.insert (primitive);

	// Flag it
	changed (primitive);

	// Return it
	return primitive;
}

// ***************************************************************************

void CMoveContainer::removePrimitive (UMovePrimitive* primitive)
{
	// CMovePrimitive pointer
	CMovePrimitive *prim=(CMovePrimitive*)primitive;

	// In modified list ?
	if (prim->isInModifiedListFlag ())
		// Remove from modified list
		removeFromModifiedList (prim);

	// Remove from the set
	_PrimitiveSet.erase (prim);

	// Erase it
	delete prim;
}

// ***************************************************************************

void CMoveContainer::removeFromModifiedList (CMovePrimitive* primitive)
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot;
	CMovePrimitive *previous=NULL;

	while (changed)
	{
		// Remove from ot list
		if (changed==primitive)
		{
			// There is a previous primitive ?
			if (previous)
				previous->linkInModifiedList (primitive->getNextModified ());
			else
				_ChangedRoot=primitive->getNextModified ();

			// Unlink
			primitive->linkInModifiedList (NULL);
			primitive->setInModifiedListFlag (false);
			break;
		}
		
		// Next primitive
		previous=changed;
		changed=changed->getNextModified ();
	}
}

// ***************************************************************************

void CMoveContainer::unlinkMoveElement  (CMoveElement *element)
{
	// Some checks
	nlassert (element->X>=0);
	nlassert (element->X<_CellCountWidth);
	nlassert (element->Y>=0);
	nlassert (element->Y<_CellCountHeight);

	// Unlink it
	CMoveCell &cell=_VectorCell[element->X+element->Y*_CellCountWidth];
	cell.unlinkX (element);
	cell.unlinkY (element);
}

// ***************************************************************************

bool CMoveContainer::reaction (CMovePrimitive& first, CMovePrimitive& second, const CCollisionDesc& desc)
{
	// Reaction
	return first.reaction (second, desc);
}

// ***************************************************************************

} // NLPACS
