/** \file form.cpp
 * Georges form interface class
 *
 * $Id: u_form.h,v 1.1 2002/05/17 06:34:14 corvazier Exp $
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

#ifndef _NLGEORGES_U_FORM_H
#define _NLGEORGES_U_FORM_H

#include	"nel/misc/types_nl.h"
#include	"nel/misc/smart_ptr.h"

namespace NLGEORGES
{

class UFormElm;

/**
  * This class provide an interface to access Georges form
  */
class UForm : public NLMISC::CRefCount
{
public:
	
	virtual ~UForm ();

	/**
	  * Access form nodes
	  */

	/// Get a mutable pointer on the root element of the form. It is a struct node.
	virtual UFormElm&		getRootElement () = 0;

	/// Get a const pointer on the root element of the form. It is a struct node.
	virtual const UFormElm& getRootElement () const = 0;
};


} // NLGEORGES

#endif // _NLGEORGES_U_FORM_H
