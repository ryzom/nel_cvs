/** \file form.h
 * Georges form class
 *
 * $Id: form.h,v 1.2 2002/05/17 11:38:42 corvazier Exp $
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

#ifndef _NLGEORGES_FORM_H
#define _NLGEORGES_FORM_H

#include	"nel/misc/types_nl.h"
#include	"nel/georges/u_form.h"
#include	"form_elm.h"
#include	"header.h"

namespace NLGEORGES
{

class UFormElm;

/**
  * This class implement a georges form.
  */
class CForm : public UForm
{
	friend class CFormLoader;
public:

	// From UForm
	UFormElm&		getRootNode ();
	const UFormElm& getRootNode () const;

#pragma warning (disable : 4355)
	CForm () : Elements (this, NULL, NULL, 0xffffffff) {};

	// ** Types

	// ** Header
	CFileHeader			Header;

	// ** Body

	/// Vector of CFormElm*
	CFormElmStruct		Elements;

	// ** IO functions
	void				write (xmlDocPtr doc) const;

	// ** Parent access

	// Set parent
	bool				setParent (const char *filename, CForm *parent);

	// Get parent
	CForm *			getParent () const;
	const std::string	&getParentFilename () const;

private:

	/// Pointer on the parent
	std::string					ParentFilename;
	NLMISC::CSmartPtr<CForm>	Parent;

	// CFormLoader call it
	void				read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn);

};

} // NLGEORGES

#endif // _NLGEORGES_FORM_H

