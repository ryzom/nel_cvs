/** \file _u_form_elm.h
 * Georges form element interface
 *
 * $Id: u_form_elm.h,v 1.1 2002/05/17 06:34:14 corvazier Exp $
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

#ifndef _NLGEORGES_U_FORM_ELM_H
#define _NLGEORGES_U_FORM_ELM_H

#include "nel/misc/types_nl.h"

namespace NLGEORGES
{

class UFormElm
{
public:


	// ** Common methods


	/// Where a node has been found
	enum TWhereIsNode
	{
		NodeForm,			/// The node has been found in the form
		NodeParentForm,		/// The node has been found in the parent form
	};

	/**
	  * Return a node pointer with its name.
	  *
	  * \param result will be filled with the node pointer. Can be NULL if the node doesn't exist.
	  * \param name is the form node name
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with 
	  * the position where the node has been found. If result is NULL, where is undefined.
	  * \return true if the result has been filled, false if the node is not referenced.
	  *
	  * About the node existance
	  *
	  * An atom node exist if its value are defined.
	  * A struct node exist if one of its children exist.
	  * An array node exist if one of its children exist.
	  * If the node doesn't exist, you can't have a pointer on it with getNodeByName(). It returns NULL.
	  * But, you can evaluate the value of non-existant atom nodes with getValueByName().
	  *
	  * About the form name:
	  *
	  * Struct elements name must be separeted by '.'
	  * Struct indexes must be between '[' and ']'
	  *
	  * Exemple:
	  * "position.x"			:	get the element named x in the struct named position
	  * "entities[2].color"		:	get the node named color in the second element of the entities array
	  */
	virtual bool	getNodeByName (const UFormElm **result, const char *name, TWhereIsNode *where = NULL) const = 0;
	virtual bool	getNodeByName (UFormElm **result, const char *name, TWhereIsNode *where = NULL) = 0;

	
	/// Where a value has been found
	enum TWhereIsValue
	{
		ValueForm,				/// The value has been found in the form
		ValueParentForm,		/// The value has been found in the parent form
		ValueDefaultDfn,		/// The value has been found in the DFN default value
		ValueDefaultType,		/// The value has been found in the TYPE default value
		Dummy = 0xffffffff		/// By sure the size == sizeof(uint32)
	};

	/**
	  * Get a form value with its name.
	  * The numbers are clamped to the type limit values.
	  *
	  * \param result is a reference on the value to fill with the result.
	  * \param name is the form name of the value to found.
	  * \param evaluate must be true if you want to have an evaluated value, false if you want the formula value.
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with 
	  * the position where the value has been found.
	  * \return true if the result has been filled, false if the value has not been found or the cast has failed or the evaluation has failed.
	  * \see getNodeByName ()
	  */
	virtual bool	getValueByName (std::string &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (sint8 &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (uint8 &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (sint16 &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (uint16 &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (sint32 &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (uint32 &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (float &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (double &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getValueByName (bool &result, const char *name, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;

	/// Get the parent form element. Return NULL if the element is the root form element.
	virtual UFormElm	*getParent () const = 0;

	
	// ** Array element methods


	/// Return true if the element is an array
	virtual bool	isArray () const = 0;

	/// Return true if the element is an array and fill size with the array size
	virtual bool	getArraySize (uint &size) const = 0;

	/**
	  * Get a array sub element const pointer.
	  * If return true, fill result with the arrayIndex cell's element 
	  * Can be NULL if the node doesn't exist.
	  */
	virtual bool	getArrayElement (const UFormElm **result, uint arrayIndex) const = 0;

	/**
	  * Get a array sub element mutable pointer.
	  * If return true, fill result with the arrayIndex cell's element pointer.
	  * Can be NULL if the node doesn't exist.
	  */
	virtual bool	getArrayElement (UFormElm **result, uint arrayIndex) = 0;


	/**
	  * Get an array value. The node must be an array of atom element.
	  *
	  * \param result is a reference on the value to fill with the result.
	  * \param arrayIndex is the array index to evaluate.
	  * \param evaluate must be true if you want to have an evaluated value, false if you want the formula value.
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with 
	  * the position where the value has been found.
	  * \return true if the result has been filled, false if the value has not been found or the cast has failed or the evaluation has failed.
	  */
	virtual bool	getArrayValue (std::string &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (sint8 &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (uint8 &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (sint16 &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (uint16 &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (sint32 &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (uint32 &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (float &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (double &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (bool &result, uint arrayIndex, bool evaluate = true, TWhereIsValue *where = NULL) const = 0;


	// ** Struct element methods


	/// Return true if the element is a struct or a virtual struct
	virtual bool	isStruct () const = 0;

	/// Return true if the element is a virtual struct
	virtual bool	isVirtualStruct () const = 0;

	/// Return the struct size
	virtual bool	getStructSize (uint &size) const = 0;

	/// Return the element name
	virtual bool	getStructElementName (uint element, std::string &result) const = 0;

	/// Return a const element pointer. Can be NULL if the node doesn't exist.
	virtual bool	getStructElement (uint element, const UFormElm **result) const = 0;

	/// Return a mutable element pointer. Can be NULL if the node doesn't exist.
	virtual bool	getStructElement (uint element, UFormElm **result) = 0;


	// ** Atom element methods


	/// Return true if the element is an atom
	virtual bool	isAtom () const = 0;

	/**
	  * Return the atom value.
	  * The numbers are clamped to the type limit values.
	  *
	  * \param result is the reference on the value to fill with result
	  * \param evaluate must be true if you want to have an evaluated value, false if you want the formula value.
	  */
	virtual bool	getValue (std::string &result, bool evaluate = true) const = 0;
	virtual bool	getValue (sint8 &result, bool evaluate = true) const = 0;
	virtual bool	getValue (uint8 &result, bool evaluate = true) const = 0;
	virtual bool	getValue (sint16 &result, bool evaluate = true) const = 0;
	virtual bool	getValue (uint16 &result, bool evaluate = true) const = 0;
	virtual bool	getValue (sint32 &result, bool evaluate = true) const = 0;
	virtual bool	getValue (uint32 &result, bool evaluate = true) const = 0;
	virtual bool	getValue (float &result, bool evaluate = true) const = 0;
	virtual bool	getValue (double &result, bool evaluate = true) const = 0;
	virtual bool	getValue (bool &result, bool evaluate = true) const = 0;
};

} // NLGEORGES

#endif // _NLGEORGES_U_FORM_ELM_H

