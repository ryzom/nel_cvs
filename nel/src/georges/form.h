/** \file form.h
 * Georges form class
 *
 * $Id: form.h,v 1.6 2002/05/31 10:07:28 corvazier Exp $
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

extern bool convertFormFile (const char *oldFileName, const char *newFileName);

namespace NLGEORGES
{

class UFormElm;

/**
  * This class implement a georges form.
  */
class CForm : public UForm
{
	friend class CFormLoader;
	friend bool convertFormFile (const char *oldFileName, const char *newFileName);
public:

	// From UForm
	UFormElm&		getRootNode ();
	const UFormElm& getRootNode () const;
	const std::string &getComment () const;


#pragma warning (disable : 4355)
	CForm () : Elements (this, NULL, NULL, 0xffffffff) {};

	// Clean the form. Erase parents.
	void				clean ();

	// ** Types

	// ** Header
	CFileHeader			Header;

	// ** Body

	/// Vector of CFormElm*
	CFormElmStruct		Elements;

	// ** IO functions
	void				write (xmlDocPtr doc) const;

	// ** Parent access

	// Insert parent before parent indexed "before".
	bool				insertParent (uint before, const char *filename, CForm *parent);

	// Remove a parent from parent list
	void				removeParent (uint parent);

	// Get a parent
	CForm *				getParent (uint parent) const;
	const std::string	&getParentFilename (uint parent) const;

	// Get parent count
	uint				getParentCount () const;

	// Clear parents
	void				clearParents ();

private:

	// A parent structure
	class CParent
	{
	public:
		std::string					ParentFilename;
		NLMISC::CSmartPtr<CForm>	Parent;
	};

	/// Pointer on the parent
	std::vector<CParent>			ParentList;

	// CFormLoader call it
	void				read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn);

	// Called by read
	void				readParent (const char *parent, CFormLoader &loader);
};

} // NLGEORGES

#endif // _NLGEORGES_FORM_H

