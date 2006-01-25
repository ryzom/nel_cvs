/** \file array_2d.h
 * A simple 2D array
 *
 * $Id: array_2d.h
 */

/* Copyright, 2000, 2006 Nevrax Ltd.
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

#ifndef NL_ARRAY_2D_H
#define NL_ARRAY_2D_H

#include "nel/misc/rect.h"
#include "nel/misc/stream.h"

namespace NLMISC
{

/** A simple 2D array.
  *
  * Access is done using the () operator
  *
  * Example :
  * 
  * CArray2D<uint> myArray;  
  * myArray.init(10, 10, 0); // fille with zero's
  * myArray(5, 5) = 1;
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2005
  */
template <class T> class CArray2D
{
public:	
	typedef typename std::vector<T> TArrayContainer;
	typedef typename TArrayContainer::iterator iterator;
	typedef typename TArrayContainer::const_iterator const_iterator;
	CArray2D() : _Width(0), _Height(0) {}
	void init(uint width, uint height);
	void init(uint width, uint height, const T &defaultValue);
	bool empty() const { return _Array.empty(); }
	void clear();
	typename iterator begin() { return _Array.begin(); }
	typename iterator end() { return _Array.end(); }
	typename const_iterator begin() const { return _Array.begin(); }
	typename const_iterator end() const { return _Array.end(); }
	
	// access element by column/row
	T &operator()(uint x, uint y)
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif		
		return _Array[x + y * _Width];
	}
	// access element by column/row (const version)
	const T &operator()(uint x, uint y) const
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif		
		return _Array[x + y * _Width];		
	}
	// Return width of array
	uint getWidth() const { return _Width; }
	// Return height of array
	uint getHeight() const { return _Height; }
	/** Move array content of the given offset. No wrapping is applied
	  * Example : move(1, 0) will move the array of one column to the left. The latest column is lost. The first column remains unchanged
	  */
	void move(sint offsetX, sint offsetY);
	// Move a part of the array. Values are clamped as necessary
	void moveSubArray(sint dstX, sint dstY, sint srcX, sint srcY, sint width, sint height);
	// get an iterator to the start of a row
	iterator beginRow(uint row)
	{
		nlassert(row < _Height);
		return _Array.begin() + row * _Width;
	}
	const_iterator beginRow(uint row) const
	{
		nlassert(row < _Height);
		return _Array.begin() + row * _Width;
	}
	iterator endRow(uint row)
	{
		nlassert(row < _Height);
		return _Array.begin() + (row + 1) * _Width;
	}
	const_iterator endRow(uint row) const
	{
		nlassert(row < _Height);
		return _Array.begin() + (row + 1) * _Width;
	}	
	// get an iterator at the given position 
	iterator getIteratorAt(uint x, uint y)
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif
		return _Array.begin() + x + (y * _Width);
	}
	// Get a const iterator at the given position
	const_iterator getIteratorAt(uint x, uint y) const
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif
		return _Array.begin() + x + (y * _Width);
	}
	/** See which part of array should be updated after its content has been displaced by the given offset (by a call to move for example).
	  * Example: getUpdateRects(0, 1, result) will result the first row as a result
	  */
	void getUpdateRects(sint moveOffsetX, sint moveOffsetY, std::vector<NLMISC::CRect> &rectsToUpdate);
	// See which parts of array will be discarded if the array is displaced by the given offset
	void getDiscardRects(sint moveOffsetX, sint moveOffsetY, std::vector<NLMISC::CRect> &discardedRects);
	//
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	TArrayContainer _Array;	
	uint _Width;	
	uint _Height;
private:
	inline void checkRect(const NLMISC::CRect &r) const
	{
		nlassert(r.X >= 0 && r.X < (sint32) _Width);
		nlassert(r.Y >= 0 && r.Y < (sint32) _Height);
		nlassert(r.X + r.Width >= 0 && r.X + (sint32)  r.Width <= (sint32) _Width);
		nlassert(r.Y + r.Height >= 0 && r.Y + (sint32) r.Height <= (sint32) _Height);
	}
};

//*********************************************************************************
template <class T>
void CArray2D<T>::clear()
{
	_Array.clear();
	_Width = _Height = 0;
}

//*********************************************************************************
template <class T>
void CArray2D<T>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(_Array);
	uint32 width = _Width;
	uint32 height = _Height;
	f.serial(width);
	f.serial(height);
	_Width = width;
	_Height = height;
}

//*********************************************************************************
template <class T>
void CArray2D<T>::getUpdateRects(sint moveOffsetX, sint moveOffsetY, std::vector<NLMISC::CRect> &rectsToUpdate)
{
	rectsToUpdate.clear();	
	if (moveOffsetX < 0) // moved right ?
	{
		// the width to update
		uint width = std::min((uint) moveOffsetX, _Width);	
		// the the grid moved top or bottom, exclude this part
		sint height = _Height - abs(moveOffsetY);
		if (height > 0)
		{				
			// complete column on the right
			rectsToUpdate.push_back(NLMISC::CRect((sint32) (_Width - width), (sint32) (std::max(- moveOffsetY, 0)), (uint32) width, (uint32) height));
			#ifdef NL_DEBUG
				checkRect(rectsToUpdate.back());
			#endif
		}		
	}
	else if (moveOffsetX > 0) // moved left ?
	{
		// the width to update
		uint width = std::min((uint) (- moveOffsetX), _Width);
		// the the grid moved top or bottom.
		sint height = _Height - abs(moveOffsetY);
		if (height > 0)
		{			
			// complete column on the right
			rectsToUpdate.push_back(NLMISC::CRect(0, (sint32) std::max(- moveOffsetY, 0), (uint32) width, (uint32) height));
			#ifdef NL_DEBUG
				checkRect(rectsToUpdate.back());
			#endif
		}
	}
	// update top or bottom part
	if (moveOffsetY < 0)
	{
		sint height = std::min((uint) moveOffsetY, _Height);		
		rectsToUpdate.push_back(NLMISC::CRect(0, _Height - height, _Width, height));
		#ifdef NL_DEBUG
			checkRect(rectsToUpdate.back());
		#endif
	}
	else
	if (moveOffsetY > 0)
	{
		sint height = std::min((uint) (- moveOffsetY), _Height);		
		rectsToUpdate.push_back(NLMISC::CRect(0, 0, _Width, height));
		#ifdef NL_DEBUG
			checkRect(rectsToUpdate.back());
		#endif
	}	
}

//*********************************************************************************
template <class T>
void CArray2D<T>::getDiscardRects(sint moveOffsetX, sint moveOffsetY,std::vector<NLMISC::CRect> &discardedRects)
{
	getUpdateRects(- moveOffsetX, - moveOffsetY, discardedRects);
}

//*********************************************************************************
template <class T>
void CArray2D<T>::moveSubArray(sint dstX, sint dstY, sint srcX, sint srcY, sint width, sint height)
{	
	if (srcX >= (sint) getWidth()) return;
	if (srcY >= (sint) getHeight()) return;
	if (dstX >= (sint) getWidth()) return;
	if (dstY >= (sint) getHeight()) return;
	if (srcX < 0)
	{
		width += srcX;
		if (width <= 0) return;
		srcX = 0;
	}
	if (srcY < 0)
	{
		height += srcY;
		if (height <= 0) return;
		srcY = 0;
	}
	if (srcX + width > (sint) getWidth())
	{
		width = getWidth() - srcX;
	}
	if (srcY + height > (sint) getHeight())
	{
		height = getHeight() - srcY;
	}
	if (dstX < 0)
	{
		width += dstX;		
		if (width < 0) return;
		srcX -= dstX;
		dstX = 0;
	}
	if (dstY < 0)
	{
		height += dstY;
		if (height < 0) return;
		srcY -= dstY;
		dstY = 0;
	}
	if (dstX + width > (sint) getWidth())
	{
		width =  getWidth() - dstX;		
	}
	if (dstY + height > (sint) getHeight())
	{
		height =  getHeight() - dstY;
	}
	#ifdef NL_DEBUG
		nlassert(width > 0);
		nlassert(height > 0);
		nlassert(srcX >= 0 && srcX < (sint) getWidth());
		nlassert(srcY >= 0 && srcY < (sint) getHeight());
	#endif
	if (dstY < srcY)
	{
		const_iterator src = getIteratorAt(srcX, srcY);
		iterator dst = getIteratorAt(dstX, dstY);
		do 
		{
			if (CTraits<T>::SupportRawCopy)
			{
				// type support fast copy
				::memcpy(&(*dst), &(*src), sizeof(T) * width);
			}
			else
			{
				std::copy(src, src + width, dst);
			}
			src += _Width;
			dst += _Width;
		} 
		while(--height);
	}
	else if (dstY > srcY)
	{
		// copy from top to bottom
		const_iterator src = getIteratorAt(srcX, srcY + height - 1);
		iterator dst = getIteratorAt(dstX, dstY + height - 1);
		do 
		{
			if (CTraits<T>::SupportRawCopy)
			{
				// type support fast copy
				::memcpy(&(*dst), &(*src), sizeof(T) * width);
			}
			else
			{			
				std::copy(src, src + width, dst);
			}
			src -= _Width;
			dst -= _Width;
		} 
		while(--height);
	}
	else
	{
		const_iterator src = getIteratorAt(srcX, srcY);
		iterator dst = getIteratorAt(dstX, dstY);		
		if (dstX < srcX)
		{
			do
			{
				if (CTraits<T>::SupportRawCopy)
				{
					// type support fast copy
					::memmove(&(*dst), &(*src), sizeof(T) * width);
				}
				else
				{				
					std::reverse_copy(src, src + width, dst);
				}
				src += _Width;
				dst += _Width;
			} 
			while(--height);	
		}
		else
		{
			do
			{
				if (CTraits<T>::SupportRawCopy)
				{
					// type support fast copy
					::memcpy(&(*dst), &(*src), sizeof(T) * width);
				}
				else
				{
					std::copy(src, src + width, dst);
				}				
				src += _Width;
				dst += _Width;
			} 
			while(--height);
		}
	}
}


//*********************************************************************************
template <class T>
void CArray2D<T>::move(sint offsetX, sint offsetY)
{
	moveSubArray(offsetX, offsetY, 0, 0, _Width, _Height);
}			

//*********************************************************************************
template <class T>
void CArray2D<T>::init(uint width, uint height)
{	
	_Array.resize(width * height);	
	_Width = width;	
	_Height = height;
}

//*********************************************************************************
template <class T>
void CArray2D<T>::init(uint width,uint height, const T &defaultValue)
{	
	_Array.resize(width * height, defaultValue);	
	_Width = width;	
	_Height = height;	
}


} // NLMISC



#endif