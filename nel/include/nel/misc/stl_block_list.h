/** \file stl_block_list.h
 * <File description>
 *
 * $Id: stl_block_list.h,v 1.1 2001/12/27 14:31:47 berenguier Exp $
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

#ifndef NL_STL_BLOCK_LIST_H
#define NL_STL_BLOCK_LIST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stl_block_allocator.h"
#include <list>


namespace NLMISC {


// ***************************************************************************
/**
 * This class is a list<> which use CSTLBlockAllocator
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template <class T, size_t blockSize=16>
class CSTLBlockList : public std::list<T, CSTLBlockAllocator<T, blockSize> >
{
public:
    explicit CSTLBlockList(const allocator_type& a= CSTLBlockAllocator<T, blockSize>() ) :
		std::list<T,CSTLBlockAllocator<T, blockSize> >(a)
    {
    }

    explicit CSTLBlockList(size_type n, const T& v=T(), const allocator_type& a=CSTLBlockAllocator<T, blockSize>() ) :
		std::list<T,CSTLBlockAllocator<T, blockSize> >(n,v,a)
    {
    }

	explicit CSTLBlockList(size_type __n) :
		std::list<T, CSTLBlockAllocator<T, blockSize> >(n)
	{
	}


    CSTLBlockList(const_iterator first,const_iterator last, const allocator_type& a=CSTLBlockAllocator<T, blockSize>() ):
		std::list<T,CSTLBlockAllocator<T, blockSize> >(first,last,a)
    {
    }
};



} // NLMISC


#endif // NL_STL_BLOCK_LIST_H

/* End of stl_block_list.h */
