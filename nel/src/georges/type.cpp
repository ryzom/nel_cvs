/** \file _type.cpp
 * Georges type class
 *
 * $Id: type.cpp,v 1.5 2002/08/02 13:47:03 corvazier Exp $
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
#include "nel/misc/eval_num_expr.h"
#include "nel/georges/u_type.h"

#include "georges/form.h"
#include "georges/form_elm.h"

#include "type.h"

using namespace NLMISC;
using namespace std;

namespace NLGEORGES
{

// ***************************************************************************

CType::CType ()
{
	Type = UnsignedInt;
	UIType = Edit;
}

// ***************************************************************************

CType::~CType ()
{
	int toto = 0;
}

// ***************************************************************************

void CType::write (xmlDocPtr doc) const
{
	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"TYPE", NULL);
	xmlDocSetRootElement (doc, node);

	// Type
	xmlSetProp (node, (const xmlChar*)"Type", (const xmlChar*)TypeNames[Type]);
	xmlSetProp (node, (const xmlChar*)"UI", (const xmlChar*)UITypeNames[UIType]);

	// Default valid
	if (!Default.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Default", (const xmlChar*)Default.c_str());
	}

	// Min valid
	if (!Min.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Min", (const xmlChar*)Min.c_str());
	}

	// Max valid
	if (!Max.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Max", (const xmlChar*)Max.c_str());
	}

	// Increment valid
	if (!Increment.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Increment", (const xmlChar*)Increment.c_str());
	}

	// Definition 
	uint def = 0;
	for (def = 0; def<Definitions.size(); def++)
	{
		xmlNodePtr defNode = xmlNewChild ( node, NULL, (const xmlChar*)"DEFINITION", NULL);
		xmlSetProp (defNode, (const xmlChar*)"Label", (const xmlChar*)Definitions[def].Label.c_str());
		xmlSetProp (defNode, (const xmlChar*)"Value", (const xmlChar*)Definitions[def].Value.c_str());
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CType::read (xmlNodePtr root)
{
	// Check node name
	if ( ((const char*)root->name == NULL) || (strcmp ((const char*)root->name, "TYPE") != 0) )
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Georges TYPE XML Syntax error in block line %d, node %s should be TYPE", 
			(int)root->content, root->name);
		throw EXmlParsingError (tmp);
	}

	// Read the type
	const char *value = (const char*)xmlGetProp (root, (xmlChar*)"Type");
	if (value)
	{
		// Lookup type
		uint type;
		for (type=0; type<TypeCount; type++)
		{
			if (strcmp (value, TypeNames[type]) == 0)
				break;
		}

		// Type found ?
		if (type!=TypeCount)
			Type = (TType)type;
		else
		{
			// Make an error message
			char tmp[512];
			smprintf (tmp, 512, "Georges TYPE XML Syntax error in TYPE block line %d, the Type value is unknown (%s)", 
				(int)root->content, value);

			// Delete the value
			xmlFree ((void*)value);

			throw EXmlParsingError (tmp);
		}

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Georges TYPE XML Syntax error in TYPE block line %d, the Type argument was not found", 
			(int)root->content);
		throw EXmlParsingError (tmp);
	}

	// Read the UI
	value = (const char*)xmlGetProp (root, (xmlChar*)"UI");
	if (value)
	{
		// Lookup type
		uint type;
		for (type=0; type<UITypeCount; type++)
		{
			if (strcmp (value, UITypeNames[type]) == 0)
				break;
		}

		// Type found ?
		if (type!=UITypeCount)
			UIType = (TUI)type;
		else
			UIType = Edit;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		UIType = Edit;

	// Read Default
	value = (const char*)xmlGetProp (root, (xmlChar*)"Default");
	if (value)
	{
		Default = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Default = "";

	// Read Min
	value = (const char*)xmlGetProp (root, (xmlChar*)"Min");
	if (value)
	{
		Min = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Min = "";

	// Read Max
	value = (const char*)xmlGetProp (root, (xmlChar*)"Max");
	if (value)
	{
		Max = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Max = "";

	// Read Increment
	value = (const char*)xmlGetProp (root, (xmlChar*)"Increment");
	if (value)
	{
		Increment = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Increment = "";

	// Read the definitions
	uint childrenCount = CIXml::countChildren (root, "DEFINITION");

	// Resize the array
	Definitions.resize (childrenCount);
	uint child=0;
	xmlNodePtr childPtr = CIXml::getFirstChildNode (root, "DEFINITION");
	while (child < childrenCount)
	{
		// Should not be NULL
		nlassert (childPtr);

		// Read Default
		const char *label = (const char*)xmlGetProp (childPtr, (xmlChar*)"Label");
		if (label)
		{
			// Read Default
			value = (const char*)xmlGetProp (childPtr, (xmlChar*)"Value");
			if (value)
			{
				Definitions[child].Label = label;
				Definitions[child].Value = value;

				// Delete the value
				xmlFree ((void*)value);
			}
			else
			{
				// Delete the value
				xmlFree ((void*)label);

				// Make an error message
				char tmp[512];
				smprintf (tmp, 512, "Georges TYPE XML Syntax error in DEFINITION block line %d, the Value argument was not found", 
					(int)childPtr->content);
				throw EXmlParsingError (tmp);
			}			

			// Delete the value
			xmlFree ((void*)label);
		}
		else
		{
			// Make an error message
			char tmp[512];
			smprintf (tmp, 512, "Georges TYPE XML Syntax error in DEFINITION block line %d, the Label argument was not found", 
				(int)childPtr->content);
			throw EXmlParsingError (tmp);
		}

		// One more
		child++;

		childPtr = CIXml::getNextChildNode (childPtr, "DEFINITION");;
	}

	// Read the header
	Header.read (root);
}

// ***************************************************************************

const char *CType::TypeNames[TypeCount]=
{
	"UnsignedInt",
	"SignedInt",
	"Double",
	"String",
	"Color",
};

// ***************************************************************************

const char *CType::UITypeNames[UITypeCount]=
{
	"Edit",
	"EditSpin",
	"NonEditableCombo",
	"FileBrowser",
	"BigEdit",
	"ColorEdit",
};

// ***************************************************************************

const char *CType::getTypeName (TType type)
{
	return TypeNames[type];
}

// ***************************************************************************

const char *CType::getUIName (TUI type)
{
	return UITypeNames[type];
}

// ***************************************************************************

class CMyEvalNumExpr : public CEvalNumExpr
{
public:
	CMyEvalNumExpr (const CForm *form)
	{
		Form = form;
	}
	CEvalNumExpr::TReturnState evalValue (const char *value, double &result)
	{
		// If a form is available
		if (Form)
		{
			// Ask for the filename ?
			if (strcmp (value, "$filename") == 0)
			{
				// Get the filename
				const string &filename = Form->getFilename ();

				// While the filename as a number
				sint i;
				for (i=filename.size ()-1; i>=0; i--)
				{
					if ((filename[i]<'0') || (filename[i]>'9'))
						break;
				}

				// Number found..
				if ((i >= 0) && (i<((sint)filename.size ()-1)))
				{
					i++;
					// Set the result
					result = atof (filename.c_str () + i);
				}
				else
				{
					// If the filename doesn't contain a number, returns 0
					result = 0;
				}
				return CEvalNumExpr::NoError;
			}
			else
			{
				// try to get a Form value
				if (Form->getRootNode ().getValueByName (result, value))
					return CEvalNumExpr::NoError;
			}
		}
		return CEvalNumExpr::evalValue (value, result);
	}

	// The working form
	const CForm		*Form;
};

// ***************************************************************************

bool CType::getValue (string &result, const CForm *form, const CFormElmAtom *node, const CFormDfn &parentDfn, uint parentIndex, bool evaluate, uint32 *where) const
{
	// Node exist ?
	if (node && !node->Value.empty())
	{
		if (where)
			*where = (node->Form == form) ? CFormElm::ValueForm : CFormElm::ValueParentForm;
		result = node->Value;
	}
	// Have a default dfn value ?
	else
	{
		const string &defDfn = parentDfn.Entries[parentIndex].Default;
		if (!defDfn.empty ())
		{
			if (where)
				*where = CFormElm::ValueDefaultDfn;
			result = defDfn;
		}
		else
		{
			if (where)
				*where = CFormElm::ValueDefaultType;
			result = Default;
		}
	}

	// evaluate the value ?
	if (evaluate)
	{
		// Evaluate predefinition
		uint i;
		uint predefCount = Definitions.size ();
		for (i=0; i<predefCount; i++)
		{
			// Ref on the value
			const CType::CDefinition &def = Definitions[i];

			// This predefinition ?
			if (def.Label == result)
			{
				result = def.Value;
				break;
			}
		}

		// Evaluate numerical expression
		if ((Type == Double) || (Type == SignedInt) || (Type == UnsignedInt) || (Type == UnsignedInt))
		{
			double value;
			CMyEvalNumExpr expr (form);
			int offset;
			CEvalNumExpr::TReturnState error = expr.evalExpression (result.c_str (), value, &offset);
			if (error == CEvalNumExpr::NoError)
			{
				// To string
				result = toString (value);
			}
			else
			{
				// Build a nice error output in warning
				char msg[512] = {0};
				if (offset<512)
				{
					int i;
					for (i=0; i<offset; i++)
						msg[i] = '-';
					msg[i] = '^';
					msg[i+1] = 0;
				}
				nlwarning ("Syntax error in expression: %s\n%s\n%s\n", expr.getErrorString (error), result.c_str (), msg);
				return false;
			}
		}
		else
		{
			// Not empty command ?
			if (!result.empty ())
			{
				// Ref to a variable ?
				if (result[0] == '"')
				{
					uint i;
					for (i=1; i<result.size (); i++)
					{
						if (result[i]=='"')
						{
							break;
						}
					}
					if (i == result.size ())
					{
						nlwarning ("Geroges (CType::getValue) Missing double quote in value : %s", result.c_str ());
						return false;
					}
					string valueName = result.substr (1, i-1);

					// try to get a Form value
					if ((form) && (form->getRootNode ().getValueByName (result, valueName.c_str ())))
						return true;
					else
						return false;
				}
			}	
		}
	}

	// Ok
	return true;
}

// ***************************************************************************

UType::TType CType::getType () const
{
	return Type;
}

// ***************************************************************************

bool CType::uiCompatible (TType type, TUI ui)
{
	switch (type)
	{
	case UnsignedInt:
	case SignedInt:
	case Double:
		return (ui == Edit) || (ui == EditSpin) || (ui == NonEditableCombo);
	case String:
		return (ui == Edit) || (ui == NonEditableCombo) || (ui == FileBrowser) || (ui == BigEdit);
	case Color:
		return (ui == ColorEdit);
	}
	return false;
}

// ***************************************************************************

} // NLGEORGES