/** \file mold_elt_define.h
 * Georges system files
 *
 * $Id: mold_elt_define.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_MOLD_ELT_DEFINE_H
#define NLGEORGES_MOLD_ELT_DEFINE_H

#include "nel/georges/mold_elt.h"

namespace NLGEORGES
{

class CLoader;

class CMoldEltDefine : public CMoldElt  
{
protected:
	std::vector< std::pair< CStringEx, CMoldElt* > > vpair;

public:
	CMoldEltDefine( CLoader* const _pl );
	virtual ~CMoldEltDefine();

	virtual CMoldElt* GetMold(); 
	virtual void Load( const CStringEx _sxfilename );
	virtual void Load( const CStringEx _sxfilename, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
};

} // NLGEORGES

#endif // NLGEORGES_MOLD_ELT_DEFINE_H
