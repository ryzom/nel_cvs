/** \file form_head.h
 * Georges system files
 *
 * $Id: form_head.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_FORM_HEAD_H
#define NLGEORGES_FORM_HEAD_H

#include "nel/misc/stream.h"
#include "nel/georges/string_ex.h"

namespace NLGEORGES
{

// La classe CFormHead comprend trois strings: le nom de l'utilisateur, la date de dernière modif et la version.
class CFormHead  
{
protected:
	CStringEx	sxuser;	
	CStringEx	sxversion;	
	CStringEx	sxdate;	

public:
	CFormHead();
	CFormHead( const CFormHead& _fh );
	virtual ~CFormHead();
	void serial( NLMISC::IStream& s );
	CStringEx GetUser() const;
	CStringEx GetVersion() const;
	CStringEx GetDate() const;
	void SetUser( const CStringEx _sxuser );
	void SetVersion( const CStringEx _sxversion );
	void SetDate( const CStringEx _sxdate );

	CFormHead&	operator =( const CFormHead& _f );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_HEAD_H
