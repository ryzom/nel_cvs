/** \file item_elt_struct.h
 * Georges system files
 *
 * $Id: item_elt_struct.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_ITEM_ELT_STRUCT_H
#define NLGEORGES_ITEM_ELT_STRUCT_H

#include "nel/georges/item_elt.h"

namespace NLGEORGES
{

class CMoldElt;
class CMoldEltDefine;
class CFormBodyEltStruct;

class CItemEltStruct : public CItemElt  
{
protected:
	CMoldEltDefine*	pmed;
	std::vector< CItemElt* > vpie;
	void Clear();

public:
	CItemEltStruct( CLoader* const _pl );
	virtual ~CItemEltStruct();
	
	void BuildItem( CMoldElt* const _pme );
	
	virtual CStringEx GetFormula() const;
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	void BuildForm( CFormBodyEltStruct* const _pfbes );
	void BuildForm( CFormBodyEltStruct* const _pfbes, const std::vector< std::pair< CStringEx, CStringEx > >& _vsxparents );
	virtual unsigned int GetNbElt() const;
	virtual CItemElt* GetElt( const unsigned int _index ) const;
	virtual CItemElt* GetElt( const CStringEx sxname ) const;
	virtual bool SetModified( const unsigned int _index );
	virtual void SetModified( const bool _b );
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_ELT_STRUCT_H
