/** \file move_container.cpp
 * <File description>
 *
 * $Id: move_container.cpp,v 1.4 2001/06/06 09:34:03 corvazier Exp $
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

#define NELPACS_ALLOC_DYNAMIC_INFO 100
#define NELPACS_ALLOC_STATIC_INFO 100

namespace NLPACS 
{

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
						   double primitiveMaxSize, uint maxIteration, uint otSize)
{
	// Clear arrays
	clear ();

	// Not in test mode
	_ChangedRoot=NULL;
	_Retriever=NULL;

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
	_MaxTestIteration=maxIteration;

	// Resize trigger array
	_Triggers.resize (NELPACS_CONTAINER_TRIGGER_DEFAULT_SIZE);
}

// ***************************************************************************

void CMoveContainer::init (CGlobalRetriever* retriever, uint widthCellCount, uint heightCellCount, double primitiveMaxSize, 
		uint maxIteration, uint otSize)
{
	// Get min max of the global retriever BB
	CVector min=retriever->getBBox().getMin();
	CVector max=retriever->getBBox().getMax();

	// Setup min max
	double xmin=min.x;
	double ymin=min.y;
	double xmax=max.x;
	double ymax=max.y;

	// Init
	init (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, maxIteration, otSize);
}

// ***************************************************************************

void  CMoveContainer::evalCollision (double deltaTime)
{
	// New test time
	_TestTime++;

	// Delta time
	_DeltaTime=deltaTime;

	// Clear triggers
	_Triggers.clear ();

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

	// Modified list is empty at this point
	nlassert (_ChangedRoot==NULL);

	// Previous node is a 'hard' OT node
	nlassert (!_PreviousCollisionNode->isInfo());

	// Get next collision
	CCollisionOTInfo	*nextCollision=_PreviousCollisionNode->getNextInfo ();

	// Collision ?
	while (nextCollision)
	{
		// Get new previous OT hard node
		_PreviousCollisionNode=nextCollision->getPrevious ();

		// Previous node is a 'hard' OT node
		nlassert (!_PreviousCollisionNode->isInfo());

		// Keep this collision
		reaction (*nextCollision);

		// Last time
		double newTime=nextCollision->getCollisionTime ();

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

		// Get next collision
		nextCollision=_PreviousCollisionNode->getNextInfo ();
	}

#ifdef NL_DEBUG
	// OT must be cleared
	checkOT ();
#endif // NL_DEBUG

	// Free ordered table info
	freeAllOTInfo ();

	// Some init
	_PreviousCollisionNode=NULL;
}

// ***************************************************************************

bool CMoveContainer::testMove (UMovePrimitive* primitive, const CVectorD& speed, double deltaTime)
{
	// Cast
	nlassert (dynamic_cast<CMovePrimitive*>(primitive));
	CMovePrimitive* prim=static_cast<CMovePrimitive*>(primitive);

	// New test time
	_TestTime++;

	// Delta time
	_DeltaTime=deltaTime;

	// Backup speed
	CVectorD oldSpeed=prim->getSpeed ();

	// Set speed
	prim->move (speed);

	// Update the bounding box and position of the primitive
	prim->update (0, _DeltaTime);

	// Compute cells overlaped by the primitive
	updateCells (prim);

#ifdef NL_DEBUG
	// Check list integrity
	checkSortedList ();
#endif // NL_DEBUG

	// Eval collisions
	bool result=evalOneCollision (0, prim, true);

	// Backup speed
	prim->move (oldSpeed);

#ifdef NL_DEBUG
	// OT must be cleared
	checkOT ();
#endif // NL_DEBUG

	// Free ordered table info
	freeAllOTInfo ();

	// Some init
	_PreviousCollisionNode=NULL;

	// Return result
	return result;
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
	int x, y;
	i=0;
	for (y=miny; y<=(int)maxy; y++)
	for (x=minx; x<=(int)maxx; x++)
	{
		// Check the formula
		nlassert ((int)i == (x - minx + ((y - miny) << (maxx-minx)) ));

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

bool CMoveContainer::evalOneCollision (double beginTime, CMovePrimitive *primitive, bool testMove)
{
	// Find its collisions
	bool found=false;

	// Test its static collision
	if (_Retriever)
	{
		// Delta pos..
		CVector delta=primitive->getSpeed ();
		delta*=(float)(_DeltaTime-beginTime);

		// Test retriever with the primitive
		const TCollisionSurfaceDescVector *result=primitive->evalCollision (*_Retriever, _SurfaceTemp, delta, _TestTime, _MaxTestIteration);
		if (result)
		{
			// Size of the array
			uint size=result->size();

			// For each detected collisions
			for (uint c=0; c<size; c++)
			{
				// Ref on the collision
				const CCollisionSurfaceDesc &desc=(*result)[c];

				// ptr on the surface
				const CRetrievableSurface *surf= _Retriever->getSurfaceById (desc.ContactSurface);

				// TODO: check surface flags  against primitive flags HERE:
				// Is a wall ?
				bool isWall;
				if(!surf)
					isWall= true;
				else
					isWall= !(surf->isFloor() || surf->isCeiling());
				// stop on a wall.
				if(isWall)
				{
					// Check time interval
					nlassert (beginTime<=desc.ContactTime);
					nlassert (desc.ContactTime<_DeltaTime);

					// Test move ?
					if (testMove)
						return true;
					else
					{
						// OK, collision
						newCollision (primitive, desc, delta);

						// One collision found
						found=true;
					}
				}
			}
		}
	}

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

			// Lookup in X sorted list on the left
			CMoveElement	*other=elm->PreviousX;
			nlassert (other!=elm);

			while (other && (primitive->getBBXMin() - other->Primitive->getBBXMin() < _PrimitiveMaxSize) )
			{
				// Other primitive
				CMovePrimitive	*otherPrimitive=other->Primitive;
				nlassert (otherPrimitive!=primitive);

				// Continue the check if the other primitive is not int the modified list or if its pointer is higher than primitive
				if ( testMove || ( (!otherPrimitive->isInModifiedListFlag ()) || (primitive<otherPrimitive) ) )
				{
					// Look if valid in X
					if (primitive->getBBXMin() < otherPrimitive->getBBXMax())
					{
						// Look if valid in Y
						if ( (primitive->getBBYMin() < otherPrimitive->getBBYMax()) && (otherPrimitive->getBBYMin() < primitive->getBBYMax()) )
						{
							// Test the primitive
							CCollisionDesc desc;
							double firstTime, lastTime;

							// Collision
							if (primitive->evalCollision (*otherPrimitive, desc, beginTime, _DeltaTime, _TestTime, _MaxTestIteration, 
														firstTime, lastTime))
							{
								// Enter or exit
								bool enter = (beginTime<=firstTime) && (firstTime<_DeltaTime);
								bool exit = (beginTime<=lastTime) && (lastTime<_DeltaTime);
								bool collision = ( beginTime<=((firstTime+lastTime)/2) ) && (firstTime<=_DeltaTime);

								// Test move ?
								if (collision)
								{
									// TODO: make new collision when collision==false to raise triggers
									// OK, collision
									newCollision (primitive, otherPrimitive, desc, collision, enter, exit);

									if (testMove) 
										return true;
									else
									{
										// Collision
										found=true;
									}
								}
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
				if ( testMove || ( (!otherPrimitive->isInModifiedListFlag ()) || (primitive<otherPrimitive) ) )
				{
					// Look if valid in Y
					if ( (primitive->getBBYMin() < otherPrimitive->getBBYMax()) && (otherPrimitive->getBBYMin() < primitive->getBBYMax()) )
					{
						// Test the primitive
						CCollisionDesc desc;
						double firstTime, lastTime;

						// Collision
						if (primitive->evalCollision (*otherPrimitive, desc, beginTime, _DeltaTime, _TestTime, _MaxTestIteration, 
													firstTime, lastTime))
						{
							// Enter or exit
							bool enter = (beginTime<=firstTime) && (firstTime<_DeltaTime);
							bool exit = (beginTime<=lastTime) && (lastTime<_DeltaTime);
							bool collision = ( beginTime<=((firstTime+lastTime)/2) ) && (firstTime<=_DeltaTime);

							// Test move ?
							if (collision)
							{
								// TODO: make new collision when collision==false to raise triggers
								// OK, collision
								newCollision (primitive, otherPrimitive, desc, collision, enter, exit);

								if (testMove) 
									return true;
								else
								{
									// Collision
									found=true;
								}
							}
						}
					}
				}

				// Next primitive to the left
				other = other->NextX;
			}
		}
	}

	// No collision ?
	if ( (!found) && (!testMove) )
	{
		if (_Retriever)
		{
			// Do move
			primitive->doMove (*_Retriever, _SurfaceTemp, _DeltaTime);
		}
		else
		{
			// Do move
			primitive->doMove (_DeltaTime);
		}
	}

	return found;
}

// ***************************************************************************

void CMoveContainer::evalAllCollisions (double beginTime)
{
	// First primitive
	CMovePrimitive	*primitive=_ChangedRoot;

	// For each modified primitive
	while (primitive)
	{
		// Eval collision
		evalOneCollision (beginTime, primitive);

		// Next primitive
		primitive=primitive->getNextModified ();
	}
}

// ***************************************************************************

void CMoveContainer::newCollision (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc, bool collision, bool enter, bool exit)
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
		CCollisionOTDynamicInfo *info = allocateOTDynamicInfo ();
		info->init (first, second, desc, collision, enter, exit);

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

void CMoveContainer::newCollision (CMovePrimitive* first, const CCollisionSurfaceDesc& desc, const CVector& delta)
{
	// Check
	nlassert (_Retriever);

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
		CCollisionOTStaticInfo *info = allocateOTStaticInfo ();

		// Setup new speed
		double t=desc.ContactTime;

		// Time of the collision.
		t-=NELPACS_DIST_BACK/first->getSpeed().norm();
		t=std::max(t, 0.0);
		t/=_DeltaTime;

		// Make a new globalposition
		CGlobalRetriever::CGlobalPosition endPosition=_Retriever->doMove (first->getGlobalPosition(), delta, (float)t, _SurfaceTemp, false);

		// Init the info descriptor
		info->init (first, desc, endPosition, t);

		// Add in the primitive list
		first->addCollisionOTInfo (info);

		// Insert in the time ordered table
		nlassert (index<(int)_TimeOT.size());
		_TimeOT[index].link (info);

		// Check it is after the last hard collision
		nlassert (_PreviousCollisionNode<=&_TimeOT[index]);
	}
}

// ***************************************************************************

void CMoveContainer::newTrigger (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc)
{
	// Element index
	uint index=_Triggers.size();

	// Add one element
	_Triggers.resize (index+1);

	// Fill info
	_Triggers[index].Object0=first->UserPointer;
	_Triggers[index].Object1=second->UserPointer;
	_Triggers[index].CollisionDesc=desc;
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

CCollisionOTDynamicInfo *CMoveContainer::allocateOTDynamicInfo ()
{
	return _AllocOTDynamicInfo.allocate ();
}

// ***************************************************************************

CCollisionOTStaticInfo *CMoveContainer::allocateOTStaticInfo ()
{
	return _AllocOTStaticInfo.allocate ();
}

// ***************************************************************************

// Free all ordered table info
void CMoveContainer::freeAllOTInfo ()
{
	_AllocOTDynamicInfo.free ();
	_AllocOTStaticInfo.free ();
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

void CMoveContainer::reaction (const CCollisionOTInfo& first)
{
	// Static collision ?
	if (first.isCollisionAgainstStatic())
	{
		// Check mode
		nlassert (_Retriever);

		// Check type
		nlassert (dynamic_cast<const CCollisionOTStaticInfo*>(&first));

		// Cast
		const CCollisionOTStaticInfo *staticInfo=static_cast<const CCollisionOTStaticInfo*> (&first);

		// Dynamic collision
		staticInfo->getPrimitive ()->reaction ( staticInfo->getCollisionDesc (), staticInfo->getGlobalPosition (),
														*_Retriever, staticInfo->getDeltaTime() );
	}
	else
	{
		// Check type
		nlassert (dynamic_cast<const CCollisionOTDynamicInfo*>(&first));

		// Cast
		const CCollisionOTDynamicInfo *dynInfo=static_cast<const CCollisionOTDynamicInfo*> (&first);

		// Dynamic collision
		dynInfo->getFirstPrimitive ()->reaction ( *(dynInfo->getSecondPrimitive ()), dynInfo->getCollisionDesc (),
			_Retriever, _SurfaceTemp, dynInfo->isCollision());

		// Trigger ?
		if (dynInfo->getFirstPrimitive ()->isTriggered (*dynInfo->getSecondPrimitive (), dynInfo->isEnter(), dynInfo->isExit()))
			newTrigger (dynInfo->getFirstPrimitive (), dynInfo->getSecondPrimitive (), dynInfo->getCollisionDesc ());
	}
}

// ***************************************************************************

UMoveContainer *UMoveContainer::createMoveContainer (double xmin, double ymin, double xmax, double ymax, 
		uint widthCellCount, uint heightCellCount, double primitiveMaxSize, uint maxIteration, uint otSize)
{
	// Create a CMoveContainer
	return new CMoveContainer (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, maxIteration, otSize);
}

// ***************************************************************************

UMoveContainer *UMoveContainer::createMoveContainer (UGlobalRetriever* retriever, uint widthCellCount, 
	uint heightCellCount, double primitiveMaxSize, uint maxIteration, uint otSize)
{
	// Cast
	nlassert (dynamic_cast<CGlobalRetriever*>(retriever));
	CGlobalRetriever* r=static_cast<CGlobalRetriever*>(retriever);

	// Create a CMoveContainer
	return new CMoveContainer (r, widthCellCount, heightCellCount, primitiveMaxSize, maxIteration, otSize);
}

// ***************************************************************************

} // NLPACS
