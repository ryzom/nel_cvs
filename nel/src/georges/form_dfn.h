/** \file _form_dfn.h
 * Georges form definition class
 *
 * $Id: form_dfn.h,v 1.4 2002/05/22 16:02:58 corvazier Exp $
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

#ifndef _NLGEORGES__FORM_DFN_H
#define _NLGEORGES__FORM_DFN_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form_elm.h"
#include "header.h"
#include "georges/type.h"

bool convertDfnFile (const char *oldFileName, const char *newFileName);

namespace NLGEORGES
{

class CFormLoader;

/**
  * This class is the defnition for a familly of from.
  */
class CFormDfn : public NLMISC::CRefCount, public UFormElm
{
	friend class CForm;
	friend class CType;
	friend class CFormElm;
	friend class CFormLoader;
	friend class CFormElmStruct;
	friend bool convertDfnFile (const char *oldFileName, const char *newFileName);
public:

	// A form defnition entry
	class CEntry
	{
		friend class CType;
		friend class CForm;
		friend class CFormElm;
		friend class CFormDfn;
		friend class CFormElmStruct;
		friend bool convertDfnFile (const char *oldFileName, const char *newFileName);
	public:

		CEntry ()
		{
			TypeElement = EntryType;
		}

		// Type of dfn element
		enum TType
		{
			EntryType,
			EntryDfn,
			EntryDfnPointer,
		};

		// Get the type
		class CType					*getTypePtr ();

		// Get the type
		const CType				*getTypePtr () const;

		// Get the dfn
		CFormDfn					*getDfnPtr ();

		// Get the dfn
		const CFormDfn				*getDfnPtr () const;

		// Get type flag
		TType						getType () const;

		// Set as a type
		void						setType (CFormLoader &loader, const char *filename);

		// Set as a dfn
		void						setDfn (CFormLoader &loader, const char *filename);

		// Set as a dfn pointer
		void						setDfnPointer ();

		// Get element Name
		const std::string			&getName () const;

		// Set element Name
		void						setName (const char *name);

		// Get the filename
		const std::string			&getFilename() const;

		// Set the filename
		void						setFilename (const char *def);

		// Get default value
		const std::string			&getDefault () const;

		// Set default value
		void						setDefault (const char *def);

		// Set array flag
		void						setArrayFlag (bool flag);

		// Get array flag
		bool						getArrayFlag () const;

	private:
		// Entry name
		std::string					Name;

		// What is the type of the element ?
		TType						TypeElement;

		// Is an array of this type ?
		bool						Array;

		// The filename
		std::string					Filename;

		// The default value for atom
		std::string					Default;

		// Smart ptr on the type or the dfn
		NLMISC::CSmartPtr<CType>	Type;

		// Smart ptr on the type or the dfn
		NLMISC::CSmartPtr<CFormDfn>	Dfn;
	};

	// Parent DFN
	class CParent
	{
	public:
		// The parent filename
		std::string						ParentFilename;

		// The parent smart
		NLMISC::CSmartPtr<CFormDfn>	Parent;
	};

	// ** IO functions
	void							write (xmlDocPtr root) const;

	// Count parent DFN
	uint							countParentDfn () const;

	// Get parent DFN
	void							getParentDfn (std::vector<CFormDfn*> &array);

	// Get parent DFN
	void							getParentDfn (std::vector<const CFormDfn*> &array) const;

	// Get num parent
	uint							getNumParent () const;

	// Get parent count
	void							setNumParent (uint size);

	// Set a parent
	void							setParent (uint parent, CFormLoader &loader, const char *filename);

	// Get a parent
	CFormDfn						*getParent (uint parent) const;

	// Get a parent string
	const std::string				&getParentFilename (uint parent) const;

	// Get num entry
	uint							getNumEntry () const;

	// Set num entry
	void							setNumEntry (uint size);

	// Get an entry
	const CEntry					&getEntry (uint entry) const;

	// Get an entry
	CEntry							&getEntry (uint entry);

	// From UFormElm
	bool							getNodeByName (const UFormElm **result, const char *name, TWhereIsNode *where) const;
	bool							getNodeByName (UFormElm **result, const char *name, TWhereIsNode *where);
	bool							getValueByName (std::string &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (sint8 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (uint8 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (sint16 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (uint16 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (sint32 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (uint32 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (float &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (double &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							getValueByName (bool &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	bool							isArray () const;
	bool							getArraySize (uint &size) const;
	bool							getArrayNode (const UFormElm **result, uint arrayIndex) const;
	bool							getArrayNode (UFormElm **result, uint arrayIndex);
	bool							getArrayValue (std::string &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (sint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (uint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (sint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (uint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (sint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (uint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (float &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (double &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							getArrayValue (bool &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool							isStruct () const;
	bool							isVirtualStruct () const;
	bool							getStructSize (uint &size) const;
	bool							getStructNodeName (uint element, std::string &result) const;
	bool							getStructNode (uint element, const UFormElm **result) const;
	bool							getStructNode (uint element, UFormElm **result);
	bool							isAtom () const;
	bool							getValue (std::string &result, bool evaluate) const;
	bool							getValue (sint8 &result, bool evaluate) const;
	bool							getValue (uint8 &result, bool evaluate) const;
	bool							getValue (sint16 &result, bool evaluate) const;
	bool							getValue (uint16 &result, bool evaluate) const;
	bool							getValue (sint32 &result, bool evaluate) const;
	bool							getValue (uint32 &result, bool evaluate) const;
	bool							getValue (float &result, bool evaluate) const;
	bool							getValue (double &result, bool evaluate) const;
	bool							getValue (bool &result, bool evaluate) const;

	// Header
	CFileHeader						Header;

private:
	// The parents array
	std::vector<CParent>			Parents;

	// A vector of entries
	std::vector<CEntry>				Entries;

private:
	// Read method called by the form loader
	void							read (xmlNodePtr doc, CFormLoader &loader, bool forceLoad = false);

};

} // NLGEORGES

#endif // _NLGEORGES__FORM_DFN_H

