/** \file item_elt_atom.h
 * Georges system files
 *
 * $Id: item_elt_atom.h,v 1.2 2002/02/20 15:26:34 besson Exp $
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

#ifndef NLGEORGES_ITEM_ELT_ATOM_H
#define NLGEORGES_ITEM_ELT_ATOM_H

#include "nel/georges/item_elt.h"

namespace NLGEORGES
{

class CMoldElt;
class CMoldEltType;

class CItemEltAtom : public CItemElt  
{
protected:
	CMoldEltType*	pmet;

public:
	CItemEltAtom( CLoader* const _pl );
	virtual ~CItemEltAtom();
	
	void BuildItem( CMoldElt* const _pme );

	virtual CStringEx GetFormula() const;
	virtual void SetParentValue( const CStringEx _sxparentvalue );
	virtual void SetCurrentValue( const CStringEx _sxcurrentvalue );
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	virtual unsigned int GetNbElt() const;
	
	CMoldEltType* GetMoldType() const;

	virtual unsigned int GetNbChild ();
	virtual CItemElt* GetChild (unsigned int _index);
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_ELT_ATOM_H
