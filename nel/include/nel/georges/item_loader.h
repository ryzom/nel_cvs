/** \file item_loader.h
 * Georges system files
 *
 * $Id: item_loader.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_ITEM_LOADER_H
#define NLGEORGES_ITEM_LOADER_H

#include "nel/georges/item.h"

namespace NLGEORGES
{

class CItemLoader  
{
//	std::map< CStringEx, CFormFile* >;

public:
	CItemLoader();
	virtual ~CItemLoader();

	void LoadItem( CItem& _item, const CStringEx& _sxfilename );
	void LoadItem( CItem& _item, const CStringEx& _sxfilename, const CStringEx& _sxdate ); 
	void SaveItem( CItem& _item, const CStringEx& _sxfilename );
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_LOADER_H
