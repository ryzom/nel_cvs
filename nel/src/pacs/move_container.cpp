/** \file move_container.cpp
 * <File description>
 *
 * $Id: move_container.cpp,v 1.25 2002/06/06 15:29:20 legros Exp $
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

#include "stdpacs.h"

#include "pacs/move_primitive.h"
#include "pacs/move_element.h"
#include "pacs/primitive_block.h"

#include "nel/misc/hierarchical_timer.h"

#include "nel/misc/i_xml.h"
#include <math.h>

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

	// Clear root changed
	_ChangedRoot.clear ();

	// Clear static world image set
	_StaticWorldImage.clear ();

	// Clear cell array
	_VectorCell.clear ();

	// Clear time ot
	_TimeOT.clear ();
}

// ***************************************************************************

void CMoveContainer::init (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, 
						   double primitiveMaxSize, uint8 numWorldImage, uint maxIteration, uint otSize)
{
	// Clear arrays
	clear ();

	// Create world images
	_ChangedRoot.resize (numWorldImage);
	for (uint i=0; i<numWorldImage; i++)
		_ChangedRoot[i]=NULL;

	// Not in test mode
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
	_VectorCell.resize (numWorldImage);
	for (uint j=0; j<numWorldImage; j++)
		_VectorCell[j].resize (_CellCountWidth * _CellCountHeight);

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
		uint8 numWorldImage, uint maxIteration, uint otSize)
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
	init (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, numWorldImage, maxIteration, otSize);

	// Init the retriever
	_Retriever=retriever;
}

// ***************************************************************************

void  CMoveContainer::evalCollision (double deltaTime, uint8 worldImage)
{
//	H_AUTO(PACS_MC_evalCollision);

	// New test time
	_TestTime++;

	// Delta time
	_DeltaTime=deltaTime;

	// Clear triggers
	_Triggers.clear ();

	// Update the bounding box and position of modified primitives
	updatePrimitives (0.f, worldImage);

#ifdef NL_DEBUG
	// Check list integrity
	checkSortedList ();
#endif // NL_DEBUG

	// Get first collision
	_PreviousCollisionNode = &_TimeOT[0];

	// Eval all collisions
	evalAllCollisions (0.f, worldImage);

	// Clear modified list
	clearModifiedList (worldImage);

	// Modified list is empty at this point
	nlassert (_ChangedRoot[worldImage]==NULL);

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
		removeModifiedFromOT (worldImage);

		// Must have been removed
		nlassert (nextCollision->getPrevious ()==NULL);
		nlassert (nextCollision->CCollisionOT::getNext ()==NULL);

		// Update the bounding box and position of modified primitives
		updatePrimitives (newTime, worldImage);

		// Eval all collisions of modified objects for the new delta t
		evalAllCollisions (newTime, worldImage);

		// Clear modified list
		clearModifiedList (worldImage);

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

bool CMoveContainer::testMove (UMovePrimitive* primitive, const CVectorD& speed, double deltaTime, uint8 worldImage)
{
//	H_AUTO(PACS_MC_testMove);

	// Cast
	nlassert (dynamic_cast<CMovePrimitive*>(primitive));
	CMovePrimitive* prim=static_cast<CMovePrimitive*>(primitive);

	// New test time
	_TestTime++;

	// Delta time
	_DeltaTime=deltaTime;

	// Get the world image primitive
	uint8 primitiveWorldImage;
	CPrimitiveWorldImage *wI;
	if (prim->isNonCollisionable ())
	{
		wI=prim->getWorldImage (0);
		primitiveWorldImage=worldImage;
	}
	else
	{
		wI=prim->getWorldImage (worldImage);
		primitiveWorldImage=worldImage;
	}

	// Backup speed
	CVectorD oldSpeed=wI->getSpeed ();

	// Set speed
	wI->move (speed, *this, *prim, primitiveWorldImage);

	// Update the bounding box and position of the primitive
	wI->update (0, _DeltaTime, *prim);

	// Compute cells overlaped by the primitive
	if (!prim->isNonCollisionable ())
		updateCells (prim, worldImage);

#ifdef NL_DEBUG
	// Check list integrity
	checkSortedList ();
#endif // NL_DEBUG

	// Result
	bool result=false;
	bool testMoveValid;
	double collisionTime;

	// Eval first each static world images
	result=evalOneTerrainCollision (0, prim, primitiveWorldImage, true, testMoveValid, collisionTime);

	// Eval first each static world images
	if (!result)
	{
		std::set<uint8>::iterator ite=_StaticWorldImage.begin();
		while (ite!=_StaticWorldImage.end())
		{

			// Eval in this world image
			result=evalOnePrimitiveCollision (0, prim, *ite, primitiveWorldImage, true, true, testMoveValid, collisionTime);

			// If found, abort
			if (result)
				break;

			// Next world image
			ite++;
		}
	}

	// Eval collisions if not found and not tested
	if ((!result) && (_StaticWorldImage.find (worldImage)==_StaticWorldImage.end()))
		result=evalOnePrimitiveCollision (0, prim, worldImage, primitiveWorldImage, true, false, testMoveValid, collisionTime);

	// Backup speed only if the primitive is inserted in the world image
	if (prim->isInserted (primitiveWorldImage))
		wI->move (oldSpeed, *this, *prim, primitiveWorldImage);

#ifdef NL_DEBUG
	// OT must be cleared
	checkOT ();
#endif // NL_DEBUG

	// Free ordered table info
	freeAllOTInfo ();

	// Some init
	_PreviousCollisionNode=NULL;

	// Return result
	return !result;
}

// ***************************************************************************

void CMoveContainer::updatePrimitives (double beginTime, uint8 worldImage)
{
//	H_AUTO(PACS_MC_updatePrimitives);

	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot[worldImage];
	while (changed)
	{
		// Get the primitive world image
		CPrimitiveWorldImage *wI;
		if (changed->isNonCollisionable())
			wI=changed->getWorldImage (0);
		else
			wI=changed->getWorldImage (worldImage);

		// Force the build of the bounding box
		wI->update (beginTime, _DeltaTime, *changed);

		// Is inserted in this world image ?
		if (changed->isInserted (worldImage))
		{

			// Compute cells overlaped by the primitive
			updateCells (changed, worldImage);
		}

		// Next primitive
		changed=wI->getNextModified ();
	}
}

// ***************************************************************************

void CMoveContainer::updateCells (CMovePrimitive *primitive, uint8 worldImage)
{
//	H_AUTO(PACS_MC_updateCells);

	// Get the primitive world image
	CPrimitiveWorldImage *wI=primitive->getWorldImage (worldImage);

	// Check BB width not too large
	if (wI->getBBXMax() - wI->getBBXMin() > _CellWidth)
	{
		nlwarning ("Primitives have moved more than a cell.");
	}

	// Check BB height not too large
	if (wI->getBBYMax() - wI->getBBYMin() > _CellHeight)
	{
		nlwarning ("Primitives have moved more than a cell.");
	}

	// Get coordinate in the cell array
	sint minx=(int)floor ((wI->getBBXMin() - _Xmin) / _CellWidth);
	sint miny=(int)floor ((wI->getBBYMin() - _Ymin) / _CellHeight);
	sint maxx=(int)floor ((wI->getBBXMax() - _Xmin) / _CellWidth);
	sint maxy=(int)floor ((wI->getBBYMax() - _Ymin) / _CellHeight);

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
		CMoveElement *elm = wI->getMoveElement (i);

		// Old element in this cell ?
		if ( elm )
		{
			// Check
			nlassert (elm->X<_CellCountWidth);
			nlassert (elm->Y<_CellCountHeight);

			// Must remove it ?
			if ( (elm->X < minx) || (elm->X > maxx) || (elm->Y < miny) || (elm->Y > maxy) )
			{
				// Yes remove it
				wI->removeMoveElement (i, *this, worldImage);
			}
			else
			{
				// Checks
				nlassert (((elm->X - minx)==0)||((elm->X - minx)==1));
				nlassert (((elm->Y - miny)==0)||((elm->Y - miny)==1));

				// Update position
#ifndef TEST_CELL
				_VectorCell[worldImage][elm->X+elm->Y*_CellCountWidth].updateSortedLists (elm, worldImage);
#endif

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
			wI->addMoveElement (_VectorCell[worldImage][x+y*_CellCountWidth], (uint16)x, (uint16)y, cx, cy, primitive, *this, worldImage);
		}

		// Next cell
		i++;
	}
}

// ***************************************************************************

void CMoveContainer::getCells (CMovePrimitive *primitive, uint8 worldImage, uint8 primitiveWorldImage, CMoveElement **elementArray)
{
//	H_AUTO(PACS_MC_getCells);

	// Get the primitive world image
	CPrimitiveWorldImage *wI;
	if (primitive->isNonCollisionable())
		wI=primitive->getWorldImage (0);	
	else
		wI=primitive->getWorldImage (primitiveWorldImage);

	// Check BB width not too large
	if (wI->getBBXMax() - wI->getBBXMin() > _CellWidth)
	{
		nlwarning ("Primitives have moved more than a cell.");
	}

	// Check BB height not too large
	if (wI->getBBYMax() - wI->getBBYMin() > _CellHeight)
	{
		nlwarning ("Primitives have moved more than a cell.");
	}

	// Get coordinate in the cell array
	int minx=(int)floor ((wI->getBBXMin() - _Xmin) / _CellWidth);
	int miny=(int)floor ((wI->getBBYMin() - _Ymin) / _CellHeight);
	int maxx=(int)floor ((wI->getBBXMax() - _Xmin) / _CellWidth);
	int maxy=(int)floor ((wI->getBBYMax() - _Ymin) / _CellHeight);

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

	// For each case selected
	int x, y;
	int i=0;
	for (y=miny; y<=(int)maxy; y++)
	for (x=minx; x<=(int)maxx; x++)
	{
		// Check the formula
		nlassert ((int)i == (x - minx + ((y - miny) << (maxx-minx)) ));

		// Center of the cell
		double cx=((double)x+0.5f)*_CellWidth+_Xmin;

		// Primitive center
		double pcx=(wI->getBBXMin()+wI->getBBXMax())/2.f;

		elementArray[i]->Primitive=primitive;
		elementArray[i]->X=x;
		elementArray[i]->Y=y;
		// Insert in left or right ?
		if (pcx<cx)
		{
			// In the left
			elementArray[i]->NextX=_VectorCell[worldImage][x+y*_CellCountWidth].getFirstX ();
			elementArray[i]->PreviousX=NULL;
		}
		else
		{
			// In the right
			elementArray[i]->PreviousX=_VectorCell[worldImage][x+y*_CellCountWidth].getLastX ();
			elementArray[i]->NextX=NULL;
		}

		// Next cell
		i++;
	}

	// Erase last array element
	for (; i<4; i++)
	{
		elementArray[i]=NULL;
	}
}

// ***************************************************************************

void CMoveContainer::clearModifiedList (uint8 worldImage)
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot[worldImage];
	while (changed)
	{
		// Get the world image primitive
		CPrimitiveWorldImage *wI;
		if (changed->isNonCollisionable())
			wI=changed->getWorldImage (0);
		else
			wI=changed->getWorldImage (worldImage);
		
		// Next primitive
		changed=wI->getNextModified ();

		// Remove it from the list
		wI->setInModifiedListFlag (false);
	}

	// Empty list
	_ChangedRoot[worldImage]=NULL;
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

bool CMoveContainer::evalOneTerrainCollision (double beginTime, CMovePrimitive *primitive, uint8 primitiveWorldImage, 
									   bool testMove, bool &testMoveValid, double &collisionTime)
{
//	H_AUTO(PACS_MC_evalOneCollision);

	// Find its collisions
	bool found=false;

	// Get the primitive world image
	CPrimitiveWorldImage *wI;
	if (primitive->isNonCollisionable())
		wI=primitive->getWorldImage (0);
	else
		wI=primitive->getWorldImage (primitiveWorldImage);

	// Begin time must be the same as beginTime
	nlassert (wI->getInitTime()==beginTime);

	// Test its static collision
	if (_Retriever)
	{
		// Delta pos..
		// Test retriever with the primitive
		const TCollisionSurfaceDescVector *result=wI->evalCollision (*_Retriever, _SurfaceTemp, _TestTime, _MaxTestIteration, *primitive);
		if (result)
		{
			// TEST MOVE MUST BE OK !!
			testMoveValid=true;

			// Size of the array
			uint size=result->size();

			// For each detected collisions
			for (uint c=0; c<size; c++)
			{
				// Ref on the collision
				CCollisionSurfaceDesc desc=(*result)[c];
				desc.ContactTime=(_DeltaTime-beginTime)*desc.ContactTime+beginTime;

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
					// Return the collision time
					collisionTime = desc.ContactTime;

					// Test move ?
					if (testMove)
						return true;
					else
					{
						// OK, collision if we are a collisionable primitive
						if (primitive->isCollisionable ())
							newCollision (primitive, desc, primitiveWorldImage, beginTime);

						// One collision found
						found=true;
						break;
					}
				}
			}
		}
		else
			// More than maxtest made, exit
			return false;
	}
	return found;
}

// ***************************************************************************

bool CMoveContainer::evalOnePrimitiveCollision (double beginTime, CMovePrimitive *primitive, uint8 worldImage, uint8 primitiveWorldImage, 
									   bool testMove, bool secondIsStatic, bool &testMoveValid, double &collisionTime)
{
//	H_AUTO(PACS_MC_evalOneCollision);

	// Find its collisions
	bool found=false;

	// Get the primitive world image
	CPrimitiveWorldImage *wI;
	if (primitive->isNonCollisionable())
		wI=primitive->getWorldImage (0);
	else
		wI=primitive->getWorldImage (primitiveWorldImage);

	// Begin time must be the same as beginTime
	nlassert (wI->getInitTime()==beginTime);

	// Element table
	CMoveElement	tableNotInserted[4];
	CMoveElement	*table[4];

	// Single test ?
	bool singleTest=testMove;
	
	// Is in world image
	if ((worldImage==primitiveWorldImage) && wI->isInWorldImageFlag())
	{
		// Get move element table from the primitive
		table[0]=wI->getMoveElement (0);
		table[1]=wI->getMoveElement (1);
		table[2]=wI->getMoveElement (2);
		table[3]=wI->getMoveElement (3);
	}
	else
	{
		// Set table pointers
		table[0]=tableNotInserted+0;
		table[1]=tableNotInserted+1;
		table[2]=tableNotInserted+2;
		table[3]=tableNotInserted+3;

		// Get cells
		getCells (primitive, worldImage, primitiveWorldImage, table);

		// Force the test
		singleTest=true;
	}

	// For each move element
	for (uint i=0; i<4; i++)
	{
		// Get the element
		CMoveElement	*elm=table[i];

		// Element valid ?
		if (elm)
		{
			// Check
			nlassert (elm->Primitive==primitive);
			// Primitive to the left

			// Lookup in X sorted list on the left
			CMoveElement	*other=elm->PreviousX;
			nlassert (other!=elm);

			while (other && (wI->getBBXMin() - other->Primitive->getWorldImage(worldImage)->getBBXMin() < _PrimitiveMaxSize) )
			{
				// Other primitive
				CMovePrimitive	*otherPrimitive=other->Primitive;
				CPrimitiveWorldImage *otherWI=otherPrimitive->getWorldImage (worldImage);
				nlassert (otherPrimitive!=primitive);

				// Continue the check if the other primitive is not int the modified list or if its pointer is higher than primitive
				if ( singleTest || ( (!otherWI->isInModifiedListFlag ()) || (primitive<otherPrimitive) ) )
				{
					// Look if valid in X
					if (wI->getBBXMin() < otherWI->getBBXMax())
					{
						// Look if valid in Y
						if ( (wI->getBBYMin() < otherWI->getBBYMax()) && (otherWI->getBBYMin() < wI->getBBYMax()) )
						{
							if (evalPrimAgainstPrimCollision (beginTime, primitive, otherPrimitive, wI, otherWI, testMove,
								primitiveWorldImage, worldImage, secondIsStatic, collisionTime))
							{
								if (testMove)
									return true;
								found=true;
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
			while (other && (other->Primitive->getWorldImage(worldImage)->getBBXMin() < wI->getBBXMax()) )
			{
				// Other primitive
				CMovePrimitive	*otherPrimitive=other->Primitive;
				CPrimitiveWorldImage *otherWI=otherPrimitive->getWorldImage (worldImage);
				nlassert (otherPrimitive!=primitive);

				// Continue the check if the other primitive is not in the modified list or if its pointer is higher than primitive
				if ( singleTest || ( (!otherWI->isInModifiedListFlag ()) || (primitive<otherPrimitive) ) )
				{
					// Look if valid in Y
					if ( (wI->getBBYMin() < otherWI->getBBYMax()) && (otherWI->getBBYMin() < wI->getBBYMax()) )
					{
						if (evalPrimAgainstPrimCollision (beginTime, primitive, otherPrimitive, wI, otherWI, testMove,
							primitiveWorldImage, worldImage, secondIsStatic, collisionTime))
						{
							if (testMove)
								return true;
							found=true;
						}
					}
				}

				// Next primitive to the left
				other = other->NextX;
			}
		}
	}

	return found;
}

// ***************************************************************************

bool CMoveContainer::evalPrimAgainstPrimCollision (double beginTime, CMovePrimitive *primitive, CMovePrimitive *otherPrimitive, 
											CPrimitiveWorldImage *wI, CPrimitiveWorldImage *otherWI, bool testMove,
											uint8 firstWorldImage, uint8 secondWorldImage, bool secondIsStatic, double &collisionTime)
{
//	H_AUTO(PACS_MC_evalPrimAgainstPrimCollision);

	// Test the primitive
	CCollisionDesc desc;
	double firstTime, lastTime;

	// Collision
	if (wI->evalCollision (*otherWI, desc, beginTime, _DeltaTime, _TestTime, _MaxTestIteration, 
								firstTime, lastTime, *primitive, *otherPrimitive))
	{
		// Enter or exit
		bool enter = (beginTime<=firstTime) && (firstTime<_DeltaTime);
		bool exit = (beginTime<=lastTime) && (lastTime<_DeltaTime);
		bool collision = ( beginTime<=((firstTime+lastTime)/2) ) && (firstTime<=_DeltaTime);

		// Test move ?
		if (collision)
		{
			// Return collision time
			collisionTime = firstTime;

			if (testMove) 
				return true;
			else
			{
				// TODO: make new collision when collision==false to raise triggers
				// OK, collision
				if (primitive->isCollisionable ())
					newCollision (primitive, otherPrimitive, desc, collision, enter, exit, firstWorldImage, secondWorldImage, secondIsStatic);

				// Collision
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************

void CMoveContainer::evalAllCollisions (double beginTime, uint8 worldImage)
{
//	H_AUTO(PACS_MC_evalAllCollisions);

	// First primitive
	CMovePrimitive	*primitive=_ChangedRoot[worldImage];

	// For each modified primitive
	while (primitive)
	{
		// Get the primitive world image
		uint8 primitiveWorldImage;
		CPrimitiveWorldImage *wI;
		if (primitive->isNonCollisionable ())
		{
			wI=primitive->getWorldImage (0);
			primitiveWorldImage=worldImage;
		}
		else
		{
			wI=primitive->getWorldImage (worldImage);
			primitiveWorldImage=worldImage;
		}

		CVectorD d0=wI->getDeltaPosition();
		CVector f0=wI->getDeltaPosition();
		CVector f0bis=_SurfaceTemp.PrecDeltaPos;

		// Find a collision
		bool found=false;
		bool testMoveValid=false;
		double collisionTime;

		// Eval collision on the terrain
		found|=evalOneTerrainCollision (beginTime, primitive, primitiveWorldImage, false, testMoveValid, collisionTime);

		// Eval collision in each static world image
		std::set<uint8>::iterator ite=_StaticWorldImage.begin();
		while (ite!=_StaticWorldImage.end())
		{
			// Eval in this world image
			found|=evalOnePrimitiveCollision (beginTime, primitive, *ite, primitiveWorldImage, false, true, testMoveValid, collisionTime);

			// Next world image
			ite++;
		}

		CVectorD d1=wI->getDeltaPosition();
		CVector f1=_SurfaceTemp.PrecDeltaPos;

		// Eval collision in the world image if not already tested
		if (_StaticWorldImage.find (worldImage)==_StaticWorldImage.end())
			found|=evalOnePrimitiveCollision (beginTime, primitive, worldImage, primitiveWorldImage, false, false, testMoveValid, collisionTime);

		CVectorD d2=wI->getDeltaPosition();
		CVector f2=_SurfaceTemp.PrecDeltaPos;

		// No collision ?
		if (!found)
		{
			nlassert ((d0==d1)&&(d0==d2));
//			nlassert (f1==f2);
			if (_Retriever&&testMoveValid)
			{								
				// Do move
				wI->doMove (*_Retriever, _SurfaceTemp, _DeltaTime, _DeltaTime, primitive->getDontSnapToGround());				
			}
			else
			{
				// Do move
				wI->doMove (_DeltaTime);
			}
		}

		// Next primitive
		primitive=wI->getNextModified ();
	}
}

// ***************************************************************************

void CMoveContainer::newCollision (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc, bool collision, bool enter, bool exit,
								   uint firstWorldImage, uint secondWorldImage, bool secondIsStatic)
{
//	H_AUTO(PACS_MC_newCollision_short);

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
		info->init (first, second, desc, collision, enter, exit, firstWorldImage, secondWorldImage, secondIsStatic);

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

void CMoveContainer::newCollision (CMovePrimitive* first, const CCollisionSurfaceDesc& desc, uint8 worldImage, double beginTime)
{
//	H_AUTO(PACS_MC_newCollision_long);

	// Check
	nlassert (_Retriever);

	// Get the world image
	CPrimitiveWorldImage *wI;
	if (first->isNonCollisionable())
		wI=first->getWorldImage (0);
	else
		wI=first->getWorldImage (worldImage);

	// Time
	double time=desc.ContactTime;

	// Check time interval
	nlassert (beginTime<=time);
	nlassert (time<_DeltaTime);

	// Time of the collision.
	time-=NELPACS_DIST_BACK/wI->getSpeed().norm();
	time=std::max(time, beginTime);
	double ratio=(time-beginTime)/(_DeltaTime-beginTime);
	nlassert (ratio>=0);
	nlassert (ratio<=1);

	// Get an ordered time index. Always round to the future.
	int index=(int)(ceil (time*(double)_OtSize/_DeltaTime) );

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

		// Make a new globalposition
		UGlobalPosition endPosition=_Retriever->doMove (wI->getGlobalPosition(), wI->getDeltaPosition(), 
			(float)ratio, _SurfaceTemp, false);

		// Init the info descriptor
		info->init (first, desc, endPosition, ratio, worldImage);

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
	_Triggers[index].Object0=first->UserData;
	_Triggers[index].Object1=second->UserData;
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

void CMoveContainer::removeModifiedFromOT (uint8 worldImage)
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot[worldImage];
	while (changed)
	{
		// Remove from ot list
		changed->removeCollisionOTInfo ();

		// Get the primitive world image
		CPrimitiveWorldImage *wI;
		if (changed->isNonCollisionable())
			wI=changed->getWorldImage (0);
		else
			wI=changed->getWorldImage (worldImage);
		
		// Next primitive
		changed=wI->getNextModified ();
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

CMovePrimitive *CMoveContainer::allocatePrimitive (uint8 firstWorldImage, uint8 numWorldImage)
{
	// Simply allocate
	return new CMovePrimitive (this, firstWorldImage, numWorldImage);
}

// ***************************************************************************

void CMoveContainer::freePrimitive (CMovePrimitive *primitive)
{
	// Simply deallocate
	delete primitive;
}

// ***************************************************************************

CPrimitiveWorldImage	**CMoveContainer::allocateWorldImagesPtrs (uint numPtrs)
{
	return new CPrimitiveWorldImage*[numPtrs];
}

// ***************************************************************************

void CMoveContainer::freeWorldImagesPtrs (CPrimitiveWorldImage **ptrs)
{
		delete [] ptrs;
}

// ***************************************************************************

CPrimitiveWorldImage	*CMoveContainer::allocateWorldImage ()
{
	return new CPrimitiveWorldImage;
}

// ***************************************************************************

void CMoveContainer::freeWorldImage (CPrimitiveWorldImage *worldImage)
{
	delete worldImage;
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

UMovePrimitive *CMoveContainer::addCollisionablePrimitive (uint8 firstWorldImage, uint8 numWorldImage, const UMovePrimitive *copyFrom)
{
	// Allocate primitive
	CMovePrimitive *primitive=allocatePrimitive (firstWorldImage, numWorldImage);

	// Add into the set
	_PrimitiveSet.insert (primitive);

	// if copy from primitive is not null, copy attributes
	if (copyFrom != NULL)
	{
		primitive->setPrimitiveType(copyFrom->getPrimitiveType());
		primitive->setReactionType(copyFrom->getReactionType());
		primitive->setTriggerType(copyFrom->getTriggerType());
		primitive->setCollisionMask(copyFrom->getCollisionMask());
		primitive->setOcclusionMask(copyFrom->getOcclusionMask());
		primitive->setObstacle(copyFrom->getObstacle());
		primitive->setAbsorbtion(copyFrom->getAbsorbtion());
		primitive->setHeight(copyFrom->getHeight());
		if (primitive->getPrimitiveType() == UMovePrimitive::_2DOrientedBox)
		{
			float	width=0.0f, height=0.0f;
			copyFrom->getSize(width, height);
			primitive->setSize(width, height);
		}
		else
		{
			primitive->setRadius(copyFrom->getRadius());
		}
	}

	// Return it
	return primitive;
}

// ***************************************************************************

UMovePrimitive *CMoveContainer::addNonCollisionablePrimitive (const UMovePrimitive *copyFrom)
{
	// Allocate primitive
	CMovePrimitive *primitive=allocatePrimitive (0, 1);
	
	// Set as noncollisionable
	primitive->setNonCollisionable (true);
	
	// Add into the set
	_PrimitiveSet.insert (primitive);

	// if copy from primitive is not null, copy attributes
	if (copyFrom != NULL)
	{
		primitive->setPrimitiveType(copyFrom->getPrimitiveType());
		primitive->setReactionType(copyFrom->getReactionType());
		primitive->setTriggerType(copyFrom->getTriggerType());
		primitive->setCollisionMask(copyFrom->getCollisionMask());
		primitive->setOcclusionMask(copyFrom->getOcclusionMask());
		primitive->setObstacle(copyFrom->getObstacle());
		primitive->setAbsorbtion(copyFrom->getAbsorbtion());
		primitive->setHeight(copyFrom->getHeight());
		if (primitive->getPrimitiveType() == UMovePrimitive::_2DOrientedBox)
		{
			float	width=0.0f, height=0.0f;
			copyFrom->getSize(width, height);
			primitive->setSize(width, height);
		}
		else
		{
			primitive->setRadius(copyFrom->getRadius());
		}
	}

	// Return it
	return primitive;
}

// ***************************************************************************

void CMoveContainer::removePrimitive (UMovePrimitive* primitive)
{
	// CMovePrimitive pointer
	CMovePrimitive *prim=(CMovePrimitive*)primitive;

	// Get the primitive world image
	for (uint8 i=0; i<prim->getNumWorldImage (); i++)
	{
		// World image
		uint8 worldImage=prim->getFirstWorldImage ()+i;

		// Get primitive world image
		CPrimitiveWorldImage *wI=prim->getWorldImage (worldImage);

		// In modified list ?
		if (wI->isInModifiedListFlag ())
		{
			// Non collisionable primitive ?
			if (prim->isNonCollisionable())
			{
				// Remove from all world image
				removeNCFromModifiedList (prim, worldImage);
			}
			else
			{
				// Remove from modified list
				removeFromModifiedList (prim, worldImage);
			}
		}
	}

	// Remove from the set
	_PrimitiveSet.erase (prim);

	// Erase it
	freePrimitive (prim);
}

// ***************************************************************************

void CMoveContainer::removeNCFromModifiedList (CMovePrimitive* primitive, uint8 worldImage)
{
	// For each world image
	uint i;
	uint worldImageCount = _ChangedRoot.size();
	for (i=0; i<worldImageCount; i++)
	{
		// For each changed primitives
		CMovePrimitive *changed=_ChangedRoot[i];
		CPrimitiveWorldImage *previous=NULL;
		CPrimitiveWorldImage *wI=primitive->getWorldImage (worldImage);

		while (changed)
		{
			// Get the primitive world image
			CPrimitiveWorldImage *changedWI=changed->getWorldImage (worldImage);

			// Remove from ot list
			if (changed==primitive)
			{
				// There is a previous primitive ?
				if (previous)
					previous->linkInModifiedList (wI->getNextModified ());
				else
					_ChangedRoot[i]=wI->getNextModified ();

				// Unlink
				wI->linkInModifiedList (NULL);
				wI->setInModifiedListFlag (false);
				break;
			}
			
			// Next primitive
			previous=changedWI;
			changed=changedWI->getNextModified ();
		}

		// Breaked ?
		if (changed==primitive)
			break;
	}
}

// ***************************************************************************
 
void CMoveContainer::removeFromModifiedList (CMovePrimitive* primitive, uint8 worldImage)
{
	// For each changed primitives
	CMovePrimitive *changed=_ChangedRoot[worldImage];
	CPrimitiveWorldImage *previous=NULL;
	CPrimitiveWorldImage *wI=primitive->getWorldImage (worldImage);

	while (changed)
	{
		// Get the primitive world image
		CPrimitiveWorldImage *changedWI=changed->getWorldImage (worldImage);

		// Remove from ot list
		if (changed==primitive)
		{
			// There is a previous primitive ?
			if (previous)
				previous->linkInModifiedList (wI->getNextModified ());
			else
				_ChangedRoot[worldImage]=wI->getNextModified ();

			// Unlink
			wI->linkInModifiedList (NULL);
			wI->setInModifiedListFlag (false);
			break;
		}
		
		// Next primitive
		previous=changedWI;
		changed=changedWI->getNextModified ();
	}
}

// ***************************************************************************
 
void CMoveContainer::unlinkMoveElement  (CMoveElement *element, uint8 worldImage)
{
	// Some checks
	nlassert (element->X<_CellCountWidth);
	nlassert (element->Y<_CellCountHeight);

	// Unlink it
	CMoveCell &cell=_VectorCell[worldImage][element->X+element->Y*_CellCountWidth];
	cell.unlinkX (element);
	//cell.unlinkY (element);
}

// ***************************************************************************

void CMoveContainer::reaction (const CCollisionOTInfo& first)
{
//	H_AUTO(PACS_MC_reaction);

	// Static collision ?
	if (first.isCollisionAgainstStatic())
	{
		// Check mode
		nlassert (_Retriever);

		// Check type
		nlassert (dynamic_cast<const CCollisionOTStaticInfo*>(&first));

		// Cast
		const CCollisionOTStaticInfo *staticInfo=static_cast<const CCollisionOTStaticInfo*> (&first);

		// Get the primitive world image
		CMovePrimitive *movePrimitive=staticInfo->getPrimitive ();
		CPrimitiveWorldImage *wI;
		if (movePrimitive->isNonCollisionable ())
			wI=movePrimitive->getWorldImage (0);
		else
			wI=movePrimitive->getWorldImage (staticInfo->getWorldImage());

		// Dynamic collision
		wI->reaction ( staticInfo->getCollisionDesc (), staticInfo->getGlobalPosition (),
						*_Retriever, staticInfo->getDeltaTime(), _DeltaTime, *staticInfo->getPrimitive (), *this, staticInfo->getWorldImage());
	}
	else
	{
		// Check type
		nlassert (dynamic_cast<const CCollisionOTDynamicInfo*>(&first));

		// Cast
		const CCollisionOTDynamicInfo *dynInfo=static_cast<const CCollisionOTDynamicInfo*> (&first);

		// Get the primitives world image
		CPrimitiveWorldImage *firstWI;
		if (dynInfo->getFirstPrimitive ()->isNonCollisionable ())
			firstWI=dynInfo->getFirstPrimitive ()->getWorldImage (0);
		else
			firstWI=dynInfo->getFirstPrimitive ()->getWorldImage (dynInfo->getFirstWorldImage());

		CPrimitiveWorldImage *secondWI;
		if (dynInfo->getSecondPrimitive ()->isNonCollisionable ())
			secondWI=dynInfo->getSecondPrimitive ()->getWorldImage (0);
		else
			secondWI=dynInfo->getSecondPrimitive ()->getWorldImage (dynInfo->getSecondWorldImage());

		// Dynamic collision
		firstWI->reaction ( *secondWI, dynInfo->getCollisionDesc (), _Retriever, _SurfaceTemp, dynInfo->isCollision(), 
							*dynInfo->getFirstPrimitive (), *dynInfo->getSecondPrimitive (), this, dynInfo->getFirstWorldImage(),
							dynInfo->getSecondWorldImage(), dynInfo->isSecondStatic());

		// Trigger ?
		if (dynInfo->getFirstPrimitive ()->isTriggered (*dynInfo->getSecondPrimitive (), dynInfo->isEnter(), dynInfo->isExit()))
			newTrigger (dynInfo->getFirstPrimitive (), dynInfo->getSecondPrimitive (), dynInfo->getCollisionDesc ());
	}
}

// ***************************************************************************

void CMoveContainer::setAsStatic (uint8 worldImage)
{
	// Add this world image in the static set of world image
	_StaticWorldImage.insert (worldImage);
}

// ***************************************************************************

void CMoveContainer::duplicateWorldImage (uint8 source, uint8 dest)
{
	// Cell count
	uint cellCount=_CellCountWidth*_CellCountHeight;

	// Clear dest modified list
	clearModifiedList (dest);

	// Clear destination cells
	uint i;
	for (i=0; i<cellCount; i++)
	{
		// Get first X
		CMoveElement *elm;
		while (elm=_VectorCell[dest][i].getFirstX ())
		{
			// Get primitive world image
			CPrimitiveWorldImage *wI=elm->Primitive->getWorldImage (dest);

			// Remove the primitive
			int i;
			for (i=0; i<4; i++)
			{
				if (wI->getMoveElement(i))
					wI->removeMoveElement (i, *this, dest);
			}
		}
	}

	// Duplicate destination cells
	for (i=0; i<cellCount; i++)
	{
		// Get first X
		CMoveElement *elm=_VectorCell[source][i].getFirstX ();
		while (elm)
		{
			// Get primitive world image
			CPrimitiveWorldImage *wISource=elm->Primitive->getWorldImage (source);
			CPrimitiveWorldImage *wIDest=elm->Primitive->getWorldImage (dest);

			// First time the primitive is visited ?
			if (wIDest->getMoveElement (0)==NULL)
			{
				wIDest->copy (*wISource);
			}
		
			// Add at the end of the list
			wIDest->addMoveElementendOfList (_VectorCell[dest][i], elm->X, elm->Y, elm->Primitive, *this);

			// Added ?
			nlassert (wIDest->getMoveElement (0)!=NULL);

			// Next primitive
			elm=elm->NextX;
		}
	}
}

// ***************************************************************************

UMoveContainer *UMoveContainer::createMoveContainer (double xmin, double ymin, double xmax, double ymax, 
		uint widthCellCount, uint heightCellCount, double primitiveMaxSize, uint8 numWorldImage, 
		uint maxIteration, uint otSize)
{
	// Create a CMoveContainer
	return new CMoveContainer (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, numWorldImage, maxIteration, otSize);
}

// ***************************************************************************

UMoveContainer *UMoveContainer::createMoveContainer (UGlobalRetriever* retriever, uint widthCellCount, 
	uint heightCellCount, double primitiveMaxSize, uint8 numWorldImage, uint maxIteration, uint otSize)
{
	// Cast
	nlassert (dynamic_cast<CGlobalRetriever*>(retriever));
	CGlobalRetriever* r=static_cast<CGlobalRetriever*>(retriever);

	// Create a CMoveContainer
	return new CMoveContainer (r, widthCellCount, heightCellCount, primitiveMaxSize, numWorldImage, maxIteration, otSize);
}

// ***************************************************************************

void UCollisionDesc::serial (NLMISC::IStream& stream)
{
	stream.serial (ContactPosition);
	stream.serial (ContactNormal0);
	stream.serial (ContactNormal1);
	stream.serial (ContactTime);
};

// ***************************************************************************

void UTriggerInfo::serial (NLMISC::IStream& stream)
{
	stream.serial (Object0);
	stream.serial (Object1);
	stream.serial (CollisionDesc);
}



// ***************************************************************************
void CMoveContainer::addCollisionnablePrimitiveBlock(UPrimitiveBlock *pb,uint8 firstWorldImage,uint8 numWorldImage,std::vector<UMovePrimitive*> *primitives,float orientation,const NLMISC::CVector &position, bool dontSnapToGround /* = false*/)
{
	CPrimitiveBlock *block = NLMISC::safe_cast<CPrimitiveBlock *>(pb);
	// Reserve the pointer array
	if (primitives)
		primitives->reserve (block->Primitives.size());

	// For each primitive
	uint prim;
	for (prim=0; prim<block->Primitives.size(); prim++)
	{
		// Create a collisionable primitive
		UMovePrimitive *primitive = addCollisionablePrimitive (firstWorldImage, numWorldImage);
		
		// Ref on the block descriptor
		CPrimitiveDesc &desc = block->Primitives[prim];

		// Set its properties
		primitive->setPrimitiveType (desc.Type);
		primitive->setReactionType (desc.Reaction);
		primitive->setTriggerType (desc.Trigger);
		primitive->setCollisionMask (desc.CollisionMask);
		primitive->setOcclusionMask (desc.OcclusionMask);
		primitive->setObstacle (desc.Obstacle);
		primitive->setAbsorbtion (desc.Attenuation);
		primitive->setDontSnapToGround(dontSnapToGround);
		if (desc.Type == UMovePrimitive::_2DOrientedBox)
		{
			primitive->setSize (desc.Length[0], desc.Length[1]);
		}
		else
		{
			nlassert (desc.Type == UMovePrimitive::_2DOrientedCylinder);
			primitive->setRadius (desc.Length[0]);
		}
		primitive->setHeight (desc.Height);

		// Insert the primitives

		// For each world image
		uint wI;
		for (wI=firstWorldImage; wI<(uint)(firstWorldImage+numWorldImage); wI++)
		{
			// Insert the primitive
			primitive->insertInWorldImage (wI);

			// Final position
			float cosa = (float) cos (orientation);
			float sina = (float) sin (orientation);
			CVector finalPos;
			finalPos.x = cosa * desc.Position.x - sina * desc.Position.y + position.x;
			finalPos.y = sina * desc.Position.y + cosa * desc.Position.y + position.y;
			finalPos.z = desc.Position.z + position.z;

			// Set the primtive orientation
			if (desc.Type == UMovePrimitive::_2DOrientedBox)
				primitive->setOrientation ((float)fmod (desc.Orientation + orientation, 2*Pi), wI);

			// Set the primitive global position
			primitive->setGlobalPosition (finalPos, wI);
		}

		// Feedback asked ?
		if (primitives)
		{
			// Add the pointer
			primitives->push_back (primitive);
		}
	}			
}


// ***************************************************************************

bool CMoveContainer::loadCollisionablePrimitiveBlock (const char *filename, uint8 firstWorldImage, uint8 numWorldImage, std::vector<UMovePrimitive*> *primitives, float orientation, const NLMISC::CVector &position, bool dontSnapToGround /*= false*/)
{
	// Check world image
	if ( (uint)(firstWorldImage+numWorldImage) > _ChangedRoot.size() )
	{
		nlwarning ("Invalid world image number.");
		return false;
	}

	// Try to load the file
	CIFile file;
	if (file.open (filename))
	{
		// Create the XML stream
		CIXml input;

		// Init
		if (input.init (file))
		{
			// The primitive block
			CPrimitiveBlock block;

			// Serial it
			file.serial (block);

			// add primitives
			addCollisionnablePrimitiveBlock(&block, firstWorldImage, numWorldImage, primitives, orientation, position, dontSnapToGround);
		
			return true;
		}
		else
		{
			// Warning
			nlwarning ("Can't init XML stream with file %s.", filename);

			return false;
		}
	}
	else
	{
		// Warning
		nlwarning ("Can't load primitive block %s.", filename);

		return false;
	}
}


// ***************************************************************************
void CMoveContainer::getPrimitives(std::vector<const UMovePrimitive *> &dest) const
{	
	dest.resize(_PrimitiveSet.size());
	std::copy(_PrimitiveSet.begin(), _PrimitiveSet.end(), dest.begin());
}


// ***************************************************************************
void UMoveContainer::getPACSCoordsFromMatrix(NLMISC::CVector &pos,float &angle,const NLMISC::CMatrix &mat)
{
	pos = mat.getPos();
	CVector orient = mat.mulVector(NLMISC::CVector::I);
	orient.z = 0.f;
	orient.normalize();
	angle = orient.y >= 0.f ? ::acosf(orient.x)  
							: 2.f * (float) NLMISC::Pi - ::acosf(orient.x);

}

// ***************************************************************************
bool CMoveContainer::evalNCPrimitiveCollision (double deltaTime, UMovePrimitive *primitive, uint8 worldImage)
{
	// New test time
	_TestTime++;

	// Only non-collisionable primitives
	if (!primitive->isCollisionable())
	{
		// Delta time
		_DeltaTime=deltaTime;

		// Begin of the time slice to compute
		double beginTime = 0;

		// Get the world image
		CPrimitiveWorldImage *wI = ((CMovePrimitive*)primitive)->getWorldImage (0);

		bool found;
		do
		{
			// Update the primitive
			wI->update (beginTime, deltaTime, *(CMovePrimitive*)primitive);

			CVectorD d0=wI->getDeltaPosition();
			CVector f0=wI->getDeltaPosition();
			CVector f0bis=_SurfaceTemp.PrecDeltaPos;

			// Eval collision again the terrain
			bool testMoveValid = false;
			double collisionTime = deltaTime;
			found = evalOneTerrainCollision (beginTime, (CMovePrimitive*)primitive, worldImage, false, testMoveValid, collisionTime);

			// Eval collision again the static primitives
			std::set<uint8>::iterator ite=_StaticWorldImage.begin();
			while (ite!=_StaticWorldImage.end())
			{
				// Eval in this world image
				double tmp;
				if (evalOnePrimitiveCollision (beginTime, (CMovePrimitive*)primitive, *ite, worldImage, false, true, testMoveValid, tmp))
				{
					// First collision..
					if (found)
						collisionTime = std::min (collisionTime, tmp);
					else
					{
						found = true;
						collisionTime = tmp;
					}
				}

				// Next world image
				ite++;
			}

			// Checks
			CVectorD d1=wI->getDeltaPosition();
			CVector f1=_SurfaceTemp.PrecDeltaPos;

			// Eval collision again the world image
			double tmp;
			if (_StaticWorldImage.find (worldImage)==_StaticWorldImage.end())
			{
				if (evalOnePrimitiveCollision (beginTime, (CMovePrimitive*)primitive, worldImage, worldImage, false, false, testMoveValid, tmp))
				{
					// First collision..
					if (found)
						collisionTime = std::min (collisionTime, tmp);
					else
					{
						found = true;
						collisionTime = tmp;
					}
				}
			}

			// Checks
			CVectorD d2=wI->getDeltaPosition();
			CVector f2=_SurfaceTemp.PrecDeltaPos;
			nlassert ((d0==d1)&&(d0==d2));

//			if (found)
//				nlstop;

			// Retriever mode ?
			if (_Retriever&&testMoveValid)
			{								
				// Do move
				wI->doMove (*_Retriever, _SurfaceTemp, deltaTime, collisionTime, ((CMovePrimitive*)primitive)->getDontSnapToGround());
			}
			else
			{
				// Do move
				wI->doMove (_DeltaTime);
			}

			beginTime = collisionTime;
		}
		while (found);
	}
	else
		return false;

	return true;
}


} // NLPACS
