/** \file item_elt_list.h
 * Georges system files
 *
 * $Id: item_elt_list.h,v 1.3 2002/03/12 09:23:28 besson Exp $
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

#ifndef NLGEORGES_ITEM_ELT_LIST_H
#define NLGEORGES_ITEM_ELT_LIST_H

#include "nel/georges/item_elt.h"

namespace NLGEORGES
{

class CItemEltAtom;
class CItemEltStruct;

class CItemEltList : public CItemElt  
{
protected:
	CItemElt* piemodel;
	std::vector< CItemElt* > vpie;

public:
	CItemEltList( CLoader* const _pl );
	virtual ~CItemEltList();

	void BuildItem( CItemElt* const _pie );

	void Clear();

	virtual CStringEx GetFormula() const;
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	virtual unsigned int GetNbElt() const;
	virtual CItemElt* GetElt( const unsigned int _index ) const;
	virtual CItemElt* GetElt( const CStringEx sxname ) const;

	void NewElt();
	void AddElt( const CItemElt* const _pie );
	void DelElt( CItemElt* const _pie );
	void VerifyName();
	virtual bool SetModified( const unsigned int _index );
	virtual void SetModified( const bool _b );

	virtual unsigned int GetNbChild ();
	virtual CItemElt* GetChild (unsigned int _index);
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_ELT_LIST_H
