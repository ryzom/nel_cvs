/** \file item_loader.cpp
 * Georges system files
 *
 * $Id: item_loader.cpp,v 1.1 2002/02/14 10:40:39 corvazier Exp $
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

#include "stdgeorges.h"
#include "nel/georges/item_loader.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemLoader::CItemLoader()
{
}

CItemLoader::~CItemLoader()
{
}

void CItemLoader::LoadItem( CItem& _item, const CStringEx& _sxfilename )
{
	_item.Load( _sxfilename );
}

void CItemLoader::LoadItem( CItem& _item, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	_item.Load( _sxfilename, _sxdate );
}

void CItemLoader::SaveItem( CItem& _item, const CStringEx& _sxfilename )
{
	_item.Save( _sxfilename );
}

}