/** \file 3d/quad_grid.h
 * Generic QuadGrid.
 *
 * $Id: quad_grid.h,v 1.5 2002/09/09 09:52:32 coutelas Exp $
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

#ifndef NL_QUAD_GRID_H
#define NL_QUAD_GRID_H

#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stl_block_list.h"
#include "nel/misc/common.h"
#include <vector>


namespace NL3D 
{


using NLMISC::CVector;

// ***************************************************************************
// Default Size of a block for allocation of elements and elements node list in the grid.
#define	NL3D_QUAD_GRID_ALLOC_BLOCKSIZE	16


// ***************************************************************************
/**
 * This container is a simple grid, used to quickly find elements. His purpose is similiar to CQuadTree, but
 * it is a simple grid, so test are in O(1), not in O(log n). It is perfect for local lookup (like in collisions).
 * Use it if you want to select small area, not large. Also, for best use, elements should have approximatively the
 * same size, and this size should be little smaller than the size of a grid element...
 *
 * By default, the quad grid is aligned on XY. (unlike the quadtree!!!)
 *
 * Unlike the quadtree, the quadgrid is NOT geographicly delimited, ie, its limits "tiles"!! This is why no "center"
 * is required. As a direct consequence, when you select something, you are REALLY not sure that what you select is not
 * a mile away from your selection :) ....
 *
 * Also, for memory optimisation, no bbox is stored in the quadgrid. Hence no particular selection is made on the Z 
 * components...
 *
 * For maximum allocation speed Efficiency, it uses a CBlockMemory<CNode> to allocate elements at insert().
 * DefaultBlockSize is 16, but you can change it at construction.
 *
 * because elements may lies in multiples squares, QuadGrid use lists per square which points on elements.
 * For faster allocation, it uses a CSTLBlockList<>. The entire quadGrid has its own CBlockMemory for the 
 * CSTLBlockLists. memoryBlockSize is the same than the blockSize for allocation of nodes.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
template<class T>	class	CQuadGrid
{
public:
	/// Iterator of the contener
	class	CIterator;
	friend class	CIterator;

public:

	/// Default constructor, use axes XY!!!, has a size of 16, and EltSize is 1.
	CQuadGrid(uint memoryBlockSize= NL3D_QUAD_GRID_ALLOC_BLOCKSIZE);

	/// dtor.
	~CQuadGrid();

	/// \name Initialization
	//@{
	/** Change the base matrix of the quad grid. For exemple this code init the grid tree in the plane XZ:
	  * \code
	  * CQuadGrid			grid;
	  * NLMISC::CMatrix		tmp;
	  * NLMISC::CVector		I(1,0,0);
	  * NLMISC::CVector		J(0,0,1);
	  * NLMISC::CVector		K(0,-1,0);
	  * 
	  * tmp.identity();
	  * tmp.setRot(I,J,K, true);
	  * quadTree.changeBase (tmp);
	  * \endcode
	  * 
	  * \param base Base of the quad grid
	  */
	void			changeBase(const NLMISC::CMatrix& base);

	/** Init the container. container is first clear() ed.
	  *
	  * \param size is the width and the height of the initial quad tree, in number of square.
	  *		For performance view, this should be a power of 2, and <=32768. (eg: 256,512, 8, 16 ...)
	  *	\param eltSize is the width and height of an element. Must be >0. Notice that the quadgrid MUST be square!!
	  */
	void			create(uint size, float eltSize);

	// Get creation parameters.
	const NLMISC::CMatrix	&getBasis() const {return _ChangeBasis;}
	uint					getSize() const {return _Size;}
	float					getEltSize() const {return _EltSize;}
	//@}

	/// \name Container operation
	//@{
	/** Clear the container. Elements are deleted, but the quadgrid is not erased.
	 *	Speed is in O(Nelts)
	 */
	void			clear();

	/** Erase an interator from the container
	  *	Speed is in O(1 * L*H) where L*H is the number of squares surrounded by the element
	  *
	  * \param it is the iterator to erase.
	  * \return if element is currently selected, the next selected element is returned, (or end()).
	  * if the element is not selected, end() is returned.
	  */
	CIterator		erase(CIterator it);

	/** Insert a new element in the container.
	  *	Speed is in O(1 * L*H) where L*H is the number of squares surrounded by the element
	  *
	  *	Warning! : bboxmin and bboxmax are multiplied by matrix setuped by changeBase. This work for any
	  *	matrix with 90° rotations (min and max are recomputed internally), but not with any rotation (43° ...)
	  *	because of the nature of AABBox. To do this correclty you should compute the bbox min and max in the 
	  *	basis given in changeBase, and insert() with multiplying min and max with inverse of this basis.
	  *	eg:
	  *		CMatrix					base= getSomeBase();
	  *		CMatrix					invBase= base.inverted();
	  *		// create quadGrid.
	  *		CQuadGrid<CTriangle>	quadGrid;
	  *		quadGrid.changeBase(base);
	  *		quadGrid.create(...);
	  *		// Insert a triangle tri correctly
	  *		CAABBox					bbox;
	  *		bbox.setCenter(base * tri.V0);
	  *		bbox.extend(base * tri.V1);
	  *		bbox.extend(base * tri.V2);
	  *		quadGrid.insert(invBase*bbox.getMin(), invBase*bbox.getMax(), tri);
	  *
	  * \param bboxmin is the corner of the bounding box of the element to insert with minimal coordinates.
	  * \param bboxmax is the corner of the bounding box of the element to insert with maximal coordinates.
	  * \param val is a reference on the value to insert.
	  */
	CIterator		insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val);
	//@}


	/// \name Selection
	//@{
	/** Clear the selection list
	  *	Speed is in O(Nelts)
	  */
	void			clearSelection();

	/** Select all the container.
	  *	Speed is in O(Nelts)
	  */
	void			selectAll();

	/** Select element intersecting a bounding box. Clear the selection first.
	  *	Speed is in O(Nelts * L*H), where L*H is the number of squares surrounded by the selection
	  *
	  * \param bboxmin is the corner of the bounding box used to select
	  * \param bboxmax is the corner of the bounding box used to select
	  */
	void			select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax);


	/** Return the first iterator of the selected element list. begin and end are valid till the next insert.
	  *	Speed is in O(1)
	  */
	CIterator		begin();

	/** Return the end iterator of the selected element list. begin and end are valid till the next insert.
	  *	Speed is in O(1)
	  */
	CIterator		end();
	//@}


// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:// Classes.

	class	CBaseNode
	{
	public:
		CBaseNode	*Prev,*Next;	// For selection.
		bool		Selected;		// true if owned by _SelectedList, or by _UnSelectedList.
		CBaseNode() {Prev= Next= NULL;}
	};

	class	CNode : public CBaseNode
	{
	public:
		T		Elt;
		uint16	x0,x1;			// The location of the elt in the grid. Used for erase().
		uint16	y0,y1;
	};

	class	CQuadNode
	{
	public:
		NLMISC::CSTLBlockList<CNode*>	Nodes;

		CQuadNode(NLMISC::CBlockMemory<CNode*, false> *bm) : Nodes(bm) {}
	};

private:// Atttributes.
	std::vector<CQuadNode>	_Grid;
	sint				_Size;
	sint				_SizePower;
	float				_EltSize;
	NLMISC::CMatrix		_ChangeBasis;
	// Selection. Elements are either in _SelectedList or in _UnSelectedList
	CBaseNode			_SelectedList;		// list of elements selected.
	CBaseNode			_UnSelectedList;	// circular list of elements not selected
	// The memory for nodes
	NLMISC::CBlockMemory<CNode>					_NodeBlockMemory;
	// The memory for node list
	NLMISC::CBlockMemory<CNode*, false>			_NodeListBlockMemory;


private:// Methods.

	// link a node to a root: Selected or UnSelected list
	void		linkToRoot(CBaseNode &root, CBaseNode *ptr)
	{
		ptr->Prev= &root;
		ptr->Next= root.Next;
		ptr->Prev->Next= ptr;
		if(ptr->Next)
			ptr->Next->Prev= ptr;
	}

	// return the coordinates on the grid of what include the bbox.
	void		selectQuads(CVector bmin, CVector bmax, sint &x0, sint &x1, sint &y0, sint &y1)
	{
		CVector		bminp, bmaxp;
		bminp= bmin;
		bmaxp= bmax;
		bmin.minof(bminp, bmaxp);
		bmax.maxof(bminp, bmaxp);
		bmin/= _EltSize;
		bmax/= _EltSize;
		x0= (sint)(floor(bmin.x));
		x1= (sint)(ceil(bmax.x));
		y0= (sint)(floor(bmin.y));
		y1= (sint)(ceil(bmax.y));

		// Very special case where the bbox.size==0 AND position is JUST on an edge of a case.
		if(x0==x1)
			x1++;
		if(y0==y1)
			y1++;

		// Manage tiling.
		if(x1-x0>=_Size)
			x0=0, x1= _Size;
		else
		{
			x0&= _Size-1;
			x1&= _Size-1;
			if(x1<=x0)
				x1+=_Size;
		}
		if(y1-y0>=_Size)
			y0=0, y1= _Size;
		else
		{
			y0&= _Size-1;
			y1&= _Size-1;
			if(y1<=y0)
				y1+=_Size;
		}
	}

	// If not done, add the node to the selection.
	void		addToSelection(CNode	*ptr)
	{
		// if not selected
		if(!ptr->Selected)
		{
			// remove from the unselected list.
			ptr->Prev->Next= ptr->Next;
			if(ptr->Next)
				ptr->Next->Prev= ptr->Prev;

			// Append to front of the _Selected list.
			linkToRoot(_SelectedList, ptr);

			// mark it
			ptr->Selected= true;
		}
	}

	// Try to add each node of the quad node list.
	void		addQuadNodeToSelection(CQuadNode	&quad)
	{
		typename NLMISC::CSTLBlockList<CNode*>::iterator	itNode;
		for(itNode= quad.Nodes.begin();itNode!=quad.Nodes.end();itNode++)
		{
			addToSelection(*itNode);
		}
	}


public:
	// CLASS const_iterator.
	class const_iterator
	{
	public:
		const_iterator()	{_Ptr=NULL;}
		const_iterator(CNode *p) : _Ptr(p) {}
		const_iterator(const CIterator& x) : _Ptr(x._Ptr) {}

		const T&	operator*() const
			{return _Ptr->Elt; }
		// Doesn't work...
		/*const T*	operator->() const
			{return (&**this); }*/
		const_iterator& operator++()
			{_Ptr = (CNode*)(_Ptr->Next); return (*this); }
		const_iterator operator++(int)
			{const_iterator tmp = *this; ++*this; return (tmp); }
		const_iterator& operator--()
			{_Ptr = (CNode*)(_Ptr->Prev); return (*this); }
		const_iterator operator--(int)
			{const_iterator tmp = *this; --*this; return (tmp); }
		bool operator==(const const_iterator& x) const
			{return (_Ptr == x._Ptr); }
		bool operator!=(const const_iterator& x) const
			{return (!(*this == x)); }
	protected:
		CNode	*_Ptr;
		friend class CQuadGrid<T>;
		friend class CIterator;
	};

	// CLASS CIterator
	class CIterator : public const_iterator
	{
	public:
		CIterator()			{_Ptr=NULL;}
		CIterator(CNode *p) : const_iterator(p) {}
		T&	operator*() const
			{return _Ptr->Elt; }
		// Doesn't work...
		/*T*	operator->() const
			{return (&**this); }*/
		CIterator& operator++()
			{_Ptr = (CNode*)(_Ptr->Next); return (*this); }
		CIterator operator++(int)
			{CIterator tmp = *this; ++*this; return (tmp); }
		CIterator& operator--()
			{_Ptr = (CNode*)(_Ptr->Prev); return (*this); }
		CIterator operator--(int)
			{CIterator tmp = *this; --*this; return (tmp); }
		bool operator==(const const_iterator& x) const
			{return (_Ptr == x._Ptr); }
		bool operator!=(const const_iterator& x) const
			{return (!(*this == x)); }
	protected:
		friend class CQuadGrid<T>;
	};


};


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// Template CQuadGrid implementation.
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// Init.
// ***************************************************************************


// ***************************************************************************
template<class T>	CQuadGrid<T>::CQuadGrid(uint memoryBlockSize) : 
	_NodeBlockMemory(memoryBlockSize), _NodeListBlockMemory(memoryBlockSize)
{
	_SizePower=4;
	_Size=1<<_SizePower;
	_EltSize=1;
	_ChangeBasis.identity();
}
// ***************************************************************************
template<class T>	CQuadGrid<T>::~CQuadGrid()
{
	clear();
	// must clear the array, before _NodeListBlockMemory.purge() is called.
	_Grid.clear();
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::changeBase(const NLMISC::CMatrix& base)
{
	_ChangeBasis= base;
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::create(uint size, float eltSize)
{
	clear();

	nlassert(NLMISC::isPowerOf2(size));
	nlassert(size<=32768);
	_SizePower= NLMISC::getPowerOf2(size);
	_Size=1<<_SizePower;
	_Grid.resize(_Size*_Size, CQuadNode(&_NodeListBlockMemory));

	nlassert(eltSize>0);
	_EltSize= eltSize;
}


// ***************************************************************************
// insert/erase.
// ***************************************************************************


// ***************************************************************************
template<class T>	void		CQuadGrid<T>::clear()
{
	CIterator	it;
	selectAll();
	while( (it=begin())!=end())
	{
		erase(it);
	}

	// Clear the 2 selection...
	_SelectedList.Next= NULL;
	_UnSelectedList.Next= NULL;
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator	CQuadGrid<T>::erase(typename CQuadGrid<T>::CIterator it)
{
	sint	x,y;
	CNode	*ptr= it._Ptr;

	if(!ptr)
		return end();

	// First erase all references to it.
	//==================================
	for(y= ptr->y0;y<ptr->y1;y++)
	{
		sint	xe,ye;
		ye= y &(_Size-1);
		for(x= ptr->x0;x<ptr->x1;x++)
		{
			xe= x &(_Size-1);
			CQuadNode	&quad= _Grid[(ye<<_SizePower)+xe];
			typename NLMISC::CSTLBlockList<CNode*>::iterator	itNode;
			for(itNode= quad.Nodes.begin();itNode!=quad.Nodes.end();itNode++)
			{
				if((*itNode)==ptr)
				{
					quad.Nodes.erase(itNode);
					break;
				}
			}
		}
	}

	// Then delete it..., and update selection linked list.
	//=====================================================
	// remove it from _SelectedList or _UnSelectedList
	CBaseNode	*next= NULL;
	next= ptr->Next;
	if(next)
		next->Prev=ptr->Prev;
	ptr->Prev->Next= next;
	// if not selected, then must return NULL
	if(!ptr->Selected)
		next= NULL;
	// delete the object.
	_NodeBlockMemory.free(ptr);


	return CIterator((CNode*)next);
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator	CQuadGrid<T>::insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val)
{
	CVector		bmin,bmax;
	bmin= _ChangeBasis*bboxmin;
	bmax= _ChangeBasis*bboxmax;

	// init the object.
	CNode	*ptr= _NodeBlockMemory.allocate();
	ptr->Elt= val;
	// Link to _Unselected list.
	linkToRoot(_UnSelectedList, ptr);
	// mark as not selected.
	ptr->Selected= false;


	// Find which quad include the object.
	//===================================
	sint	x0,y0;
	sint	x1,y1;
	selectQuads(bmin, bmax, x0,x1, y0,y1);

	ptr->x0= x0;
	ptr->x1= x1;
	ptr->y0= y0;
	ptr->y1= y1;

	// Then for all of them, insert the node in their list.
	//=====================================================
	sint	x,y;
	for(y= ptr->y0;y<ptr->y1;y++)
	{
		sint	xe,ye;
		ye= y &(_Size-1);
		for(x= ptr->x0;x<ptr->x1;x++)
		{
			xe= x &(_Size-1);
			CQuadNode	&quad= _Grid[(ye<<_SizePower)+xe];
			quad.Nodes.push_back(ptr);
		}
	}

	return CIterator(ptr);
}


// ***************************************************************************
// selection.
// ***************************************************************************


// ***************************************************************************
template<class T>	void			CQuadGrid<T>::clearSelection()
{
	CBaseNode	*ptr= _SelectedList.Next;
	while(ptr)
	{
		// next selected.
		CBaseNode	*nextSelected= ptr->Next;

		// Link to _Unselected list.
		linkToRoot(_UnSelectedList, ptr);

		// mark as not selected.
		ptr->Selected= false;

		// next.
		ptr= nextSelected;
	}

	// the selected list is now empty.
	_SelectedList.Next= NULL;
}
// ***************************************************************************
template<class T>	void			CQuadGrid<T>::selectAll()
{
	// This is the opposite of clearSelection(). get all that are in _UnSelectedList,
	// and put them in _SelectedList
	CBaseNode	*ptr= _UnSelectedList.Next;
	while(ptr)
	{
		// next selected.
		CBaseNode	*nextUnSelected= ptr->Next;

		// Link to _Selected list.
		linkToRoot(_SelectedList, ptr);

		// mark as selected.
		ptr->Selected= true;

		// next.
		ptr= nextUnSelected;
	}

	// the Unselected list is now empty.
	_UnSelectedList.Next= NULL;
}
// ***************************************************************************
template<class T>	void			CQuadGrid<T>::select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax)
{
	CVector		bmin,bmax;
	bmin= _ChangeBasis*bboxmin;
	bmax= _ChangeBasis*bboxmax;

	clearSelection();

	// What are the quads to access?
	sint	x0,y0;
	sint	x1,y1;
	selectQuads(bmin, bmax, x0,x1, y0,y1);

	sint	x,y;
	for(y= y0;y<y1;y++)
	{
		sint	xe,ye;
		ye= y &(_Size-1);
		for(x= x0;x<x1;x++)
		{
			xe= x &(_Size-1);
			CQuadNode	&quad= _Grid[(ye<<_SizePower)+xe];
			addQuadNodeToSelection(quad);
		}
	}

}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator		CQuadGrid<T>::begin()
{
	return CIterator((CNode*)(_SelectedList.Next));
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator		CQuadGrid<T>::end()
{
	return CIterator(NULL);
}


} // NL3D


#endif // NL_QUAD_GRID_H

/* End of quad_grid.h */
