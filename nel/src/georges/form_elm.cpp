/** \file form_elt.h
 * Georges form element implementation class
 *
 * $Id: form_elm.cpp,v 1.6 2002/05/22 12:09:45 cado Exp $
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


#include "stdgeorges.h"

#include "nel/misc/i_xml.h"

#include "form_elm.h"
#include "form_loader.h"
#include "type.h"

using namespace NLMISC;
using namespace std;

namespace NLGEORGES
{

// ***************************************************************************
// class CFormElm
// ***************************************************************************

// ***************************************************************************

bool CFormElm::isArray () const 
{ 
	return false; 
};

// ***************************************************************************

bool CFormElm::getArraySize (uint &size) const 
{
	nlwarning ("Georges (CFormElm::getArraySize) : this node is not an array"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getArrayNode (const UFormElm **result, uint arrayIndex) const
{ 
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getArrayNode (UFormElm **result, uint arrayIndex) 
{ 
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getArrayValue (std::string &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (sint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (uint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (sint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (uint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (sint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (uint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (float &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (double &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (bool &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormElm::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormElm::isStruct () const 
{ 
	return false; 
};

// ***************************************************************************

bool CFormElm::isVirtualStruct () const 
{ 
	return false; 
};

// ***************************************************************************

bool CFormElm::getStructSize (uint &size) const 
{ 
	nlwarning ("Georges (CFormElm::getStructSize) : this node is not a struct"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getStructNodeName (uint element, string &result) const 
{ 
	nlwarning ("Georges (CFormElm::getStructNodeName) : this node is not a struct"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getStructNode (uint element, const UFormElm **result) const 
{ 
	nlwarning ("Georges (CFormElm::getStructNode) : this node is not a struct"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getStructNode (uint element, UFormElm **result) 
{ 
	nlwarning ("Georges (CFormElm::getStructNode) : this node is not a struct"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::isAtom () const 
{ 
	return false; 
};

// ***************************************************************************

bool CFormElm::getValue (string &result, bool evaluate) const 
{ 
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
};

// ***************************************************************************

bool CFormElm::getValue (sint8 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (uint8 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (sint16 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (uint16 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (sint32 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (uint32 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (float &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (double &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

bool CFormElm::getValue (bool &result, bool evaluate) const
{
	nlwarning ("Georges (CFormElm::getValue) : this node is not an atom"); 
	return false; 
}

// ***************************************************************************

CFormElm::CFormElm (CForm *form, CFormElm *parentNode, CFormDfn *parentDfn, uint parentIndex)
{
	Form = form;
	ParentNode = parentNode;
	ParentDfn = parentDfn;
	ParentIndex = parentIndex;
}

// ***************************************************************************

CFormElm::~CFormElm ()
{
}

// ***************************************************************************

bool CFormElm::isUsed (const CForm *form) const
{
	return form == Form;
}

// ***************************************************************************

CForm *CFormElm::getForm () const
{
	return Form;
}

// ***************************************************************************

bool CFormElm::getNodeByName (UFormElm **result, const char *name, TWhereIsNode *where)
{
	const UFormElm *resultConst = NULL;
	if (((const UFormElm*)this)->getNodeByName (&resultConst, name, where))
	{
		*result = const_cast<UFormElm*> (resultConst);
		return true;
	}
	return false;
}

// ***************************************************************************

bool CFormElm::getNodeByName (const UFormElm **result, const char *name, TWhereIsNode *where) const
{
	// The parent Dfn
	const CFormDfn *parentDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	const CFormElm *node;
	uint lastElement;
	bool array;
	CFormDfn::CEntry::TType type;

	// Search for the node
	if (getNodeByName (name, &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array))
	{
		// Set the result
		*result = node;

		// Where ?
		if (where && node)
		{
			*where = (node->getForm () == Form) ? NodeForm : NodeParentForm;
		}

		// Ok 
		return true;
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (string& result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	// The parent Dfn
	const CFormDfn *parentDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	const CFormElm *node;
	uint parentIndex;
	bool array;
	CFormDfn::CEntry::TType type;

	// Search for the node
	if (getNodeByName (name, &parentDfn, parentIndex, &nodeDfn, &nodeType, &node, type, array))
	{
		// End, return the current index
		if (type == CFormDfn::CEntry::EntryType)
		{
			// The atom
			const CFormElmAtom *atom = node ? safe_cast<const CFormElmAtom*> (node) : NULL;

			// Evale
			nlassert (nodeType);
			return (nodeType->getValue (result, Form, atom, *parentDfn, parentIndex, evaluate, (uint32*)where));
		}
		else
		{
			// Error message
			nlwarning ("Georges (CFormElm::getValueByName) : The node %s is not an atom element. Can't return a value.", name);
		}
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::getValueByName) : Can't find the node %s.", name);
	}

	// Error
	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (sint8 &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (uint8 &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (sint16 &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (uint16 &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (sint32 &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (uint32 &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (float &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (double &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (bool &result,	const char *name, bool evaluate, TWhereIsValue *where) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

UFormElm *CFormElm::getParent () const
{
	return ParentNode;
}

// ***************************************************************************

bool CFormElm::getNodeByName (const char *name, const CFormDfn **parentDfn, uint &lastElement, const CFormDfn **nodeDfn, const CType **nodeType, const CFormElm **node, CFormDfn::CEntry::TType &type, bool &array) const
{
	// *** Init output variables
	
	// Current node
	(*node) = this;

	// Get the parent form
	const UFormElm *parent = getParent ();
	const UFormElm *child = this;
	const CFormElmStruct *parentStruct = NULL;

	// Look for a parent struct 
	while (parent && !parent->isStruct ())
	{
		// Look for the good index
		child = parent;
		parent = parent->getParent ();
	}

	// Parent exist ?
	*parentDfn = ParentDfn;
	if (ParentDfn)
	{
		// Get the entry
		const CFormDfn::CEntry &theEntry = ParentDfn->getEntry (ParentIndex);

		// Get the type
		type = theEntry.getType ();
		*nodeType = theEntry.getTypePtr ();
		*nodeDfn = theEntry.getDfnPtr ();
		lastElement = ParentIndex;
	}
	else
	{
		nlassert (!isArray ());
		lastElement = 0xffffffff;
		*nodeType = isAtom () ? safe_cast<const CFormElmAtom*>(this)->Type : NULL;
		*nodeDfn = isStruct () ? (const CFormDfn *)(safe_cast<const CFormElmStruct*>(this)->FormDfn) : NULL;
		type = isAtom () ? CFormDfn::CEntry::EntryType : isVirtualStruct () ? CFormDfn::CEntry::EntryDfnPointer : CFormDfn::CEntry::EntryDfn;
	}

	// Is this node an array ?
	array = isArray ();

	// *** Parsing variables

	// Current token start and end
	const char *startToken = name;
	const char *endToken;

	// Current token start
	string token;

	// Current form name
	string currentName;

	// Error
	uint errorIndex;

	// Token code
	uint code;

	// Are we parsing an array ?
	bool inArrayIndex = false;

	// Index in the array
	uint arrayIndex;

	// While there is tokens
	while (endToken = tokenize (startToken, token, errorIndex, code))
	{
		// Ready an array index ?
		if (!inArrayIndex)
		{
			// For each code
			switch (code)
			{
			case TokenString:
				{
					// Are we a struct ?
					if ( (type == CFormDfn::CEntry::EntryDfn) || (type == CFormDfn::CEntry::EntryDfnPointer))
					{
						nlassert (*nodeDfn);

						// Look for the element
						uint elementCount = (*nodeDfn)->getNumEntry ();

						// Get the parents
						vector<const CFormDfn*> arrayDfn;
						arrayDfn.reserve ((*nodeDfn)->countParentDfn ());
						(*nodeDfn)->getParentDfn (arrayDfn);

						// For each parent
						uint i;
						uint formElm = 0;
						for (i=0; i<arrayDfn.size(); i++)
						{
							// The dfn
							const CFormDfn &dfn = *(arrayDfn[i]);

							// For each elements
							uint element;
							for (element=0; element<dfn.Entries.size(); element++)
							{
								// Good name ?
								if (dfn.Entries[element].Name == token)
								{
									// Good one.
									*parentDfn = &dfn;
									lastElement = element;
									*nodeDfn = dfn.Entries[element].Dfn;
									*nodeType = dfn.Entries[element].Type;
									type = dfn.Entries[element].TypeElement;
									array = dfn.Entries[element].Array;

									// Next node
									if (*node)
									{
										// Get next node
										const CFormElmStruct *nodeStruct = safe_cast<const CFormElmStruct*> (*node);
										CFormElm *nextElt = nodeStruct->Elements[formElm].Element;
										
										// If no next node, watch for parent node
										if ((nextElt == NULL) && (!array) && nodeStruct->Parent)
											nextElt = nodeStruct->Parent->Elements[formElm].Element;
										*node = nextElt;

										// Is a virtual DFN ?
										if ((*node) && (*node)->isVirtualStruct ())
										{
											// Should be NULL
											nlassert (*nodeDfn == NULL);

											// Set the current dfn
											*nodeDfn = safe_cast<const CFormElmVirtualStruct*> (*node)->FormDfn;
										}
									}
									else
										*node = NULL;

									break;
								}
								formElm++;
							}

							// Breaked ?
							if (element!=dfn.Entries.size())
								break;
						}

						// Breaked ?
						if (i==arrayDfn.size())
						{
							// Not found
							nlwarning ("Georges (CFormElm::getNodeByName) : %s struct does not comptain element named %s", currentName.c_str(), token.c_str());
							return false;
						}
					}
					else
					{
						// Error message
						nlwarning ("Georges (CFormElm::getNodeByName) : %s is not a struct element. Can't open the node %s", currentName.c_str(), name);
						return false;
					}
				}
				break;
			case TokenPoint:
				{
					// Are we a struct ?
					if ((type != CFormDfn::CEntry::EntryDfn) && (type != CFormDfn::CEntry::EntryDfnPointer))
					{
						// Error message
						nlwarning ("Georges (CFormElm::getNodeByName) : %s is not a struct element. Can't open the node %s", currentName.c_str(), name);
						return false;
					}
				}
				break;
			case TokenArrayBegin:
				{
					// Are we an array ?
					if (!array)
					{
						// Error message
						nlwarning ("Georges (CFormElm::getNodeByName) : %s is not an array element. Can't open the node %s", currentName.c_str(), name);
						return false;
					}
					inArrayIndex = true;
					arrayIndex = 0xffffffff;
				}
				break;
			default:
				{
					// Error message
					nlwarning ("Georges (CFormElm::getNodeByName) : syntax error in %s. Can't open the node %s", currentName.c_str(), name);
				}
				break;
			}
		}
		else
		{
			switch (code)
			{
			case TokenString:
				{
					// To int
					if (sscanf (token.c_str(), "%d", &arrayIndex)!=1)
					{
						// Error message
						nlwarning ("Georges (CFormElm::getNodeByName) : %s is not an array index. Can't open the node %s", token.c_str(), name);
						return false;
					}

					// Should have an array defined
					if (*node)
					{
						// Check index
						uint arraySize;
						nlverify ((*node)->getArraySize (arraySize));
						if (arrayIndex>=arraySize)
						{
							// Error message
							nlwarning ("Georges (CFormElm::getNodeByName) : Out of array bounds (size of %s is %d). Can't open the node %s", currentName.c_str(), arraySize, name);
							return false;
						}
					}
					else
					{
						// Error message
						nlwarning ("Georges (CFormElm::getNodeByName) : The array %s is not defined.", currentName.c_str());
						return false;
					}
				}
				break;
			case TokenArrayEnd:
				{
					// Index found ?
					if (arrayIndex == 0xffffffff)
					{
						// Error message
						nlwarning ("Georges (CFormElm::getNodeByName) : array index is missing in %s. Can't open the node %s", currentName.c_str(), name);
					}
					else
					{
						// Let the parent DFN
						// *parentDfn = ;
						nlassert (*parentDfn)

						// New current node
						const UFormElm *elt;
						nlverify ((*node)->getArrayNode(&elt, arrayIndex));
						*node = (const CFormElm*)elt; 

						// Is a dfn ?
						*nodeDfn = (*parentDfn)->getEntry (lastElement).getDfnPtr ();

						// Is a type ?
						*nodeType = (*parentDfn)->getEntry (lastElement).getTypePtr ();

						// Type ?
						type = (*parentDfn)->getEntry (lastElement).getType ();
						
						// Can't be an array of array
						array = false;

						// Not any more in index
						inArrayIndex = false;

						// Is a virtual DFN ?
						if ((*node) && (*node)->isVirtualStruct ())
						{
							// Should be NULL
							nlassert (*nodeDfn == NULL);

							// Set the current dfn
							*nodeDfn = safe_cast<const CFormElmVirtualStruct*> (*node)->FormDfn;
						}
					}
				}
				break;
			default:
				{
					// Error message
					nlwarning ("Georges (CFormElm::getNodeByName) : %s is not an array index. Can't open the node %s", currentName.c_str(), name);
					return false;
				}
			}
		}

		// Concat current adress
		currentName += token;
		startToken = endToken;
	}

	return true;
}

// ***************************************************************************

const char* CFormElm::tokenize (const char *name, string &str, uint &errorIndex, uint &code)
{
	if (*name == 0)
	{
		return NULL;
	}

	if (*name == '[')
	{
		code = TokenArrayBegin;
		str = "[";
		return name+1;
	}

	if (*name == ']')
	{
		code = TokenArrayEnd;
		str = "]";
		return name+1;
	}

	if (*name == '.')
	{
		code = TokenPoint;
		str = ".";
		return name+1;
	}

	str = "";
	while ( (*name != '.') && (*name != '[') && (*name != ']') && (*name != 0) )
	{
		// Add a char
		str += *name;
		name++;
	}

	code = TokenString;
	return name;
}

// ***************************************************************************
// class CFormElmStruct
// ***************************************************************************

CFormElmStruct::CFormElmStruct (CForm *form, CFormElm *parentNode, CFormDfn *parentDfn, uint parentIndex) : CFormElm (form, parentNode, parentDfn, parentIndex)
{
	FormDfn = NULL;
	Parent = NULL;
}

// ***************************************************************************

CFormElmStruct::~CFormElmStruct ()
{
	clean ();
}

// ***************************************************************************

void CFormElmStruct::clean ()
{
	// For each element of the array
	uint elm;
	for (elm =0; elm<Elements.size(); elm++)
	{
		if (Elements[elm].Element)
			delete Elements[elm].Element;
		Elements[elm].Element = NULL;
	}
}

// ***************************************************************************

bool CFormElmStruct::isStruct () const
{
	return true;
};

// ***************************************************************************

bool CFormElmStruct::getStructSize (uint &size) const 
{
	size = Elements.size();
	return true;
};

// ***************************************************************************

bool CFormElmStruct::getStructNodeName (uint element, string &result) const 
{
	if (element<Elements.size())
	{
		result = Elements[element].Name;
		return true;
	}
	else
	{
		nlwarning ("Georges (CFormElmStruct::getStructNodeName) : Index (%d) out of bound (%d)", element, Elements.size() );
		return false;
	}
};

// ***************************************************************************

bool CFormElmStruct::getStructNode (uint element, const UFormElm **result) const 
{
	if (element<Elements.size())
	{
		*result = Elements[element].Element;
		return true; 
	}
	else
	{
		nlwarning ("Georges (CFormElmStruct::getStructNode) : Index (%d) out of bound (%d)", element, Elements.size() );
		return false;
	}
};

// ***************************************************************************

bool CFormElmStruct::getStructNode (uint element, UFormElm **result) 
{
	if (element<Elements.size())
	{
		*result = Elements[element].Element;
		return true; 
	}
	else
	{
		nlwarning ("Georges (CFormElmStruct::getStructNode) : Index (%d) out of bound (%d)", element, Elements.size() );
		return false;
	}
};

// ***************************************************************************

xmlNodePtr  CFormElmStruct::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{
	// Is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"STRUCT", NULL);

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// For each elements of the structure
		uint elm;
		for (elm=0; elm<Elements.size(); elm++)
		{
			// Create a node if it exist
			if (Elements[elm].Element)
				Elements[elm].Element->write (node, form, Elements[elm].Name.c_str());
		}

		// Return the new node
		return node;
	}
	return NULL;
}

// ***************************************************************************

void CFormElmStruct::read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn, CForm *form)
{
	// Get the smart pointer on the dfn
	FormDfn = dfn;
	Parent = NULL;

	// Build the Form
	build (dfn);

	// Count parent
	uint dfnCount = dfn->countParentDfn ();

	// Array of Dfn
	std::vector<CFormDfn*> dfnArray;
	dfnArray.reserve (dfnCount);
	dfn->getParentDfn (dfnArray);

	// For each Dfn
	uint dfnId;
	uint elmIndex=0;
	for (dfnId=0; dfnId<dfnCount; dfnId++)
	{
		// Lookup for the name in the DFN
		uint elm;
		for (elm=0; elm<dfnArray[dfnId]->Entries.size(); elm++)
		{
			// Found ?
			bool found = false;

			// Read the struct
			xmlNodePtr child = NULL;

			// Node can be NULL
			if (node)
				child = node->children;

			while (child)
			{
				// Good node ?
				const char *name = (const char*)xmlGetProp (child, (xmlChar*)"Name");
				if (name && (dfnArray[dfnId]->Entries[elm].getName () == name) )
				{
					// Type
					bool atom=false;
					bool array=false;
					bool _struct=false;
					bool vStruct=false;

					// Is an atom ?
					if (strcmp ((const char*)child->name, "ATOM") == 0)
					{
						atom = true;
					}
					// Is a struct ?
					else if (strcmp ((const char*)child->name, "STRUCT") == 0)
					{
						_struct = true;
					}
					// Is a struct ?
					else if (strcmp ((const char*)child->name, "VSTRUCT") == 0)
					{
						vStruct = true;
					}
					// Is an array ?
					else if (strcmp ((const char*)child->name, "ARRAY") == 0)
					{
						array = true;
					}

					// Continue ?
					if (atom || _struct || vStruct || array)
					{
						// Same type ?
						if ( 
							(atom && (dfnArray[dfnId]->Entries[elm].getType ()==CFormDfn::CEntry::EntryType) && (!dfnArray[dfnId]->Entries[elm].getArrayFlag ()) ) || 
							(array && dfnArray[dfnId]->Entries[elm].getArrayFlag () && ( (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryType) || (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfn) ) ) || 
							(_struct && (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfn) && (!dfnArray[dfnId]->Entries[elm].getArrayFlag ()) ) ||
							(vStruct && (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfnPointer) && (!dfnArray[dfnId]->Entries[elm].getArrayFlag ()) )
							)
						{
							// Ok keep it
							break;
						}
						else
						{
							// Make a warning message
							nlwarning ("Georges FORM XML in block line %d, node %s type in DFN have changed.", 
								(int)child->content, child->name);
						}
					}
					else
					{
						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "Georges FORM XML Syntax error in block line %d, node %s name should be STRUCT, ATOM or ARRAY", 
							(int)child->content, child->name);

						if (name)
						{
							// Delete the value
							xmlFree ((void*)name);
						}

						throw EXmlParsingError (tmp);
					}
				}

				if (name)
				{
					// Delete the value
					xmlFree ((void*)name);
				}

				// Next child
				child = child->next;
			}

			// Found ?
			if (child)
			{
				// Create a new element
				if (dfnArray[dfnId]->Entries[elm].getArrayFlag ())
				{
					// Array of type
					CFormElmArray *newElm = NULL;
					if (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryType)
					{
						// Load the new element
						newElm = new CFormElmArray (form, NULL, dfnArray[dfnId]->Entries[elm].getTypePtr (), this, dfnArray[dfnId], elm);
					}
					// Array of struct
					else if (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfn)
					{
						newElm = new CFormElmArray (form, dfnArray[dfnId]->Entries[elm].getDfnPtr (), NULL, this, dfnArray[dfnId], elm);
					}

					// Should be created
					nlassert (newElm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, form);
				}
				else if (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryType)
				{
					// Load the new element
					CFormElmAtom *newElm = new CFormElmAtom (form, this, dfnArray[dfnId], elm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, dfnArray[dfnId]->Entries[elm].getTypePtr (), form);
				}
				else if (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfn)
				{
					// Load the new element
					CFormElmStruct *newElm = new CFormElmStruct (form, this, dfnArray[dfnId], elm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, dfnArray[dfnId]->Entries[elm].getDfnPtr (), form);
				}
				else // if dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfnPointer)
				{
					// Should be a struct
					nlassert (dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryDfnPointer);

					// Load the new element
					CFormElmVirtualStruct *newElm = new CFormElmVirtualStruct (form, this, dfnArray[dfnId], elm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, form);
				}
			}
			else
				Elements[elmIndex].Element = NULL;

			elmIndex++;
		}
	}
}

// ***************************************************************************

bool CFormElmStruct::isUsed (const CForm *form) const
{
	for (uint i=0; i<Elements.size(); i++)
	{
		if (Elements[i].Element && Elements[i].Element->isUsed (form))
			return true;
	}
	return false;
}

// ***************************************************************************

void CFormElmStruct::build (CFormDfn *dfn)
{
	// Clean the form
	clean ();

	// Set the DFN
	FormDfn = dfn;
	
	// Get the parents
	vector<const CFormDfn*> arrayDfn;
	arrayDfn.reserve (dfn->countParentDfn ());
	dfn->getParentDfn (arrayDfn);

	// Count element
	uint elementCount = 0;
	uint dfnIndex;
	for (dfnIndex=0; dfnIndex<arrayDfn.size(); dfnIndex++)
	{
		elementCount += arrayDfn[dfnIndex]->getNumEntry();
	}

	// Resize the element array
	Elements.resize (elementCount);

	elementCount = 0;
	for (dfnIndex=0; dfnIndex<arrayDfn.size(); dfnIndex++)
	{
		// For each element
		for (uint elm=0; elm<arrayDfn[dfnIndex]->Entries.size(); elm++)
		{
			// Copy the name
			Elements[elementCount].Name = arrayDfn[dfnIndex]->Entries[elm].Name;
			elementCount++;
		}
	}
}

// ***************************************************************************

bool CFormElmStruct::setParent (CFormElm *parent)
{
	if (parent)
	{
		// Is struct
		if (parent->isStruct ())
		{
			// Parent struct
			CFormElmStruct *parentStruct = safe_cast<CFormElmStruct*>(parent);
			if (FormDfn == parentStruct->FormDfn)
			{
				// Set the parent
				Parent = parentStruct;

				// Elements has the same size
				if (Elements.size () == parentStruct->Elements.size ())
				{
					// Set children parent
					for (uint i=0; i<Elements.size (); i++)
					{
						if (Elements[i].Element)
							if (!Elements[i].Element->setParent (parentStruct->Elements[i].Element))
								return false;
					}

					// Ok
					return true;
				}
			}
		}
	}
	else
	{
		Parent = NULL;

		// Set children parent
		for (uint i=0; i<Elements.size (); i++)
		{
			if (Elements[i].Element)
				if (!Elements[i].Element->setParent (NULL))
					return false;
		}

		// Ok
		return true;
	}
	return false;
}

// ***************************************************************************

CFormDfn *CFormElmStruct::getSubDfn (uint index, uint &dfnIndex) const
{
	// Get the sub DFN
	vector<CFormDfn*> parentDfn;
	parentDfn.reserve (FormDfn->countParentDfn ());
	FormDfn->getParentDfn (parentDfn);

	// For each parent
	uint dfn;
	dfnIndex = index;
	uint parentSize = parentDfn.size();
	for (dfn=0; dfn<parentSize; dfn++)
	{
		// Good element ?
		uint size = parentDfn[dfn]->Entries.size ();
		if (dfnIndex<size)
			return parentDfn[dfn];
		dfnIndex -= size;
	}

	// Should be found..
	nlstop
	return NULL;
}

// ***************************************************************************
// class CFormElmVirtualStruct
// ***************************************************************************

CFormElmVirtualStruct::CFormElmVirtualStruct (CForm *form, CFormElm *parentNode, CFormDfn *parentDfn, uint parentIndex) : CFormElmStruct (form, parentNode, parentDfn, parentIndex)
{
}

// ***************************************************************************

xmlNodePtr  CFormElmVirtualStruct::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{
	// Is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"VSTRUCT", NULL);

		// Write the DFN filename in the node
		xmlSetProp (node, (const xmlChar*)"DfnName", (const xmlChar*)DfnFilename.c_str());

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// For each elements of the structure
		uint elm;
		for (elm=0; elm<Elements.size(); elm++)
		{
			// Create a node if it exist
			if (Elements[elm].Element)
				Elements[elm].Element->write (node, form, Elements[elm].Name.c_str());
		}

		// Return the new node
		return node;
	}
	return NULL;
}

// ***************************************************************************

void CFormElmVirtualStruct::read (xmlNodePtr node, CFormLoader &loader, CForm *form)
{
	// Get the DFN filename
	const char *filename = (const char*)xmlGetProp (node, (xmlChar*)"DfnName");
	if (filename)
	{
		// Set the name
		DfnFilename = filename;

		// Delete the value
		xmlFree ((void*)filename);

		// Load the dfn
		FormDfn = loader.loadFormDfn (DfnFilename.c_str ());
	}
	else
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Georges FORM XML Syntax error in virtual struct in block line %d, should have a DfnName property.", 
			(int)node->content, node->name);
		throw EXmlParsingError (tmp);
	}

	// Read the parent
	CFormElmStruct::read (node, loader, FormDfn, form);
}

// ***************************************************************************

bool CFormElmVirtualStruct::isVirtualStruct () const
{
	return true;
}

// ***************************************************************************

bool CFormElmVirtualStruct::isUsed (const CForm *form) const
{
	return true;
}

// ***************************************************************************

// Todo remove it, use memory serial instead
/*bool CFormElm::copy (const UFormElm& src)
{
	
}*/

// ***************************************************************************
// class CFormElmArray
// ***************************************************************************

CFormElmArray::CFormElmArray (CForm *form, CFormDfn *formDfn, CType *type, CFormElm *parentNode, CFormDfn *parentDfn, uint parentIndex) : CFormElm (form, parentNode, parentDfn, parentIndex)
{
	FormDfn = formDfn;
	Type = type;
}

// ***************************************************************************

CFormElmArray::~CFormElmArray ()
{
	clean ();
}

// ***************************************************************************

void CFormElmArray::clean ()
{
	// For each element of the array
	uint elm;
	for (elm =0; elm<Elements.size(); elm++)
	{
		if (Elements[elm])
			delete Elements[elm];
	}
	Elements.clear ();
}

// ***************************************************************************

bool CFormElmArray::isArray () const 
{
	return true;
};

// ***************************************************************************

bool CFormElmArray::getArraySize (uint &size) const 
{
	size = Elements.size ();
	return true;
};

// ***************************************************************************

bool CFormElmArray::getArrayNode (const UFormElm **result, uint arrayIndex) const 
{
	if (arrayIndex<Elements.size())
	{
		*result = Elements[arrayIndex];
		return true;
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayNode) : Index (%d) out of bound (%d)", arrayIndex, Elements.size() );
		return false;
	}
};

// ***************************************************************************

bool CFormElmArray::getArrayNode (UFormElm **result, uint arrayIndex) 
{
	if (arrayIndex<Elements.size())
	{
		*result = Elements[arrayIndex];
		return true;
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayNode) : Index (%d) out of bound (%d)", arrayIndex, Elements.size() );
		return false;
	}
};


// ***************************************************************************

bool CFormElmArray::getArrayValue (std::string &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		return (Type->getValue (result, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where));
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (sint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (uint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (sint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (uint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (sint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (uint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	return true;
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (float &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (double &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (bool &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex]), *ParentDfn, ParentIndex, evaluate, (uint32*)where))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		nlwarning ("Georges (CFormElmArray::getArrayValue) : This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

xmlNodePtr CFormElmArray::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{	
	// Arrau is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"ARRAY", NULL);

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// For each elements of the structure
		uint elm;
		for (elm=0; elm<Elements.size(); elm++)
		{
			// Create a node
			Elements[elm]->write (node, form, NULL, true);
		}

		// Return the new node
		return node;
	}

	return NULL;
}

// ***************************************************************************

void CFormElmArray::read (xmlNodePtr node, CFormLoader &loader, CForm *form)
{
	// Clean the form
	clean ();

	// Count child
	if (node)
	{
		// Type of DFN array
		if (Type)
		{
			nlassert (FormDfn == NULL);

			// Count children
			uint childCount = CIXml::countChildren (node, "ATOM");

			// Resize the table
			Elements.resize (childCount);

			// For each children
			uint childNum=0;
			xmlNodePtr child = CIXml::getFirstChildNode (node, "ATOM");
			while (child)
			{
				// Create a new node
				CFormElmAtom *newElt = new CFormElmAtom (form, this, ParentDfn, ParentIndex);
				Elements[childNum] = newElt;
				newElt->read (child, loader, Type, form);

				// Next child
				child = CIXml::getNextChildNode (child, "ATOM");
				childNum++;
			}
		}
		else
		{
			nlassert (FormDfn);
			nlassert (Type == NULL);

			// Count children
			uint childCount = CIXml::countChildren (node, "STRUCT");

			// Resize the table
			Elements.resize (childCount);

			// For each children
			uint childNum=0;
			xmlNodePtr child = CIXml::getFirstChildNode (node, "STRUCT");
			while (child)
			{
				// Create a new node
				CFormElmStruct *newElt = new CFormElmStruct (form, this, ParentDfn, ParentIndex);
				Elements[childNum] = newElt;
				newElt->read (child, loader, FormDfn, form);

				// Next child
				child = CIXml::getNextChildNode (child, "STRUCT");
				childNum++;
			}
		}
	}
}

// ***************************************************************************

bool CFormElmArray::setParent (CFormElm *parent)
{
	return true;
}

// ***************************************************************************

// Todo remove it, use memory serial instead
/*bool CFormElm::copy (const UFormElm& src)
{
	
}*/

// ***************************************************************************
// CFormElmAtom
// ***************************************************************************

CFormElmAtom::CFormElmAtom (CForm *form, CFormElm *parentNode, CFormDfn *parentDfn, uint parentIndex) : CFormElm (form, parentNode, parentDfn, parentIndex)
{
	Parent = NULL;
	Type = NULL;
}

// ***************************************************************************

bool CFormElmAtom::isAtom () const
{
	return true;
}

// ***************************************************************************

bool CFormElmAtom::getValue (string &result, bool evaluate) const
{
	// evaluate the value ?
	if (evaluate)
	{
		nlassert (Type);
		uint i;
		uint predefCount = Type->Definitions.size ();
		for (i=0; i<predefCount; i++)
		{
			// Ref on the value
			const CType::CDefinition &def = Type->Definitions[i];

			// This predefinition ?
			if (def.Label == result)
			{
				result = def.Value;
				break;
			}
		}

		// Not found ?
		if (i == predefCount)
			result = Value;
	}
	else
	{
		result = Value;
	}

	return true;
}

// ***************************************************************************

bool CFormElmAtom::getValue (sint8 &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (uint8 &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (sint16 &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (uint16 &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (sint32 &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (uint32 &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (float &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (double &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (bool &result, bool evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

xmlNodePtr  CFormElmAtom::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{	
	// Atom is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"ATOM", NULL);

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// The value
		if (!Value.empty ())
			xmlSetProp (node, (const xmlChar*)"Value", (const xmlChar*)Value.c_str());

		// Return the new node
		return node;
	}
	return NULL;
}

// ***************************************************************************

void CFormElmAtom::read (xmlNodePtr node, CFormLoader &loader, CType *type, CForm *form)
{
	// Set the type
	Type = type;

	// Set the parent
	Parent = NULL;

	// Set the value ?
	if (node)
	{
		// Get the value
		const char *value = (const char*)xmlGetProp (node, (xmlChar*)"Value");
		if (value)
		{
			// Active value
			setValue (value);

			// Delete the value
			xmlFree ((void*)value);
		}
	}
}

// ***************************************************************************

void CFormElmAtom::setValue (const char *value)
{
	Value = value;
}

// ***************************************************************************

bool CFormElmAtom::setParent (CFormElm *parent)
{
	if (parent)
	{
		// Is struct
		if (parent->isAtom ())
		{
			CFormElmAtom *parentAtom = safe_cast<CFormElmAtom*>(parent);
			if (parentAtom->Type == Type)
			{
				// Parent struct
				Parent = parentAtom;

				// Ok
				return true;
			}
		}
	}
	else
	{
		// No parent
		Parent = NULL;

		// Ok
		return true;
	}
	return false;
}

// ***************************************************************************

} // NLGEORGES
