/** \file primitive_class.cpp
 * Ligo primitive class description. Give access at common properties for a primitive class. Properties are given in an XML file
 *
 * $Id: primitive_class.cpp,v 1.11 2004/01/23 13:39:13 corvazier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include <nel/ligo/primitive_class.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/path.h>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************

CPrimitiveClass::CPrimitiveClass()
{
}

// ***************************************************************************

bool ReadFloat (const char *propName, float &result, xmlNodePtr xmlNode)
{
	string value;
	if (CIXml::getPropertyString (value, xmlNode, propName))
	{
		result = (float)atof (value.c_str ());
		return true;
	}
	return false;
}

// ***************************************************************************

bool ReadInt (const char *propName, int &result, xmlNodePtr xmlNode)
{
	string value;
	if (CIXml::getPropertyString (value, xmlNode, propName))
	{
		result = atoi (value.c_str ());
		return true;
	}
	return false;
}

// ***************************************************************************

bool ReadBool (const char *propName, bool &result, xmlNodePtr xmlNode, const char *filename, CLigoConfig &config)
{
	string str;
	if (CIXml::getPropertyString (str, xmlNode, propName))
	{
		if (str == "true")
			result = true;
		else if (str == "false")
			result = false;
		else
		{
			config.syntaxError (filename, xmlNode, "Unknown (%s) parameter (%s), should be false or true", propName, str.c_str ());
			return false;
		}
		return true;
	}
	return false;
}

// ***************************************************************************

bool ReadColor (CRGBA &color, xmlNodePtr node)
{
	// Read the color
	float r = DEFAULT_PRIMITIVE_COLOR.R;
	float g = DEFAULT_PRIMITIVE_COLOR.G;
	float b = DEFAULT_PRIMITIVE_COLOR.B;
	float a = DEFAULT_PRIMITIVE_COLOR.A;

	// Read the value
	if (!ReadFloat ("R", r, node))
		return false;
	if (!ReadFloat ("G", g, node))
		return false;
	if (!ReadFloat ("B", b, node))
		return false;
	if (!ReadFloat ("A", a, node))
		a = 255;

	// Clamp
	clamp (r, 0.f, 255.f);
	clamp (g, 0.f, 255.f);
	clamp (b, 0.f, 255.f);
	clamp (a, 0.f, 255.f);

	// Set
	color.set((uint8)r, (uint8)g, (uint8)b, (uint8)a);
	return true;
}

// ***************************************************************************

bool ReadChild (CPrimitiveClass::CChild &child, xmlNodePtr childNode, const char *filename, bool _static, CLigoConfig &config)
{
	// Read the class name
	if (config.getPropertyString (child.ClassName, filename, childNode, "CLASS_NAME"))
	{
		// Read the name
		if (!_static || config.getPropertyString (child.Name, filename, childNode, "NAME"))
		{
			// Read the parameters
			xmlNodePtr childParamNode = CIXml::getFirstChildNode (childNode, "PARAMETER");
			child.Parameters.reserve (CIXml::countChildren (childNode, "PARAMETER"));
			if (childParamNode)
			{
				do
				{
					// Add a static child
					child.Parameters.push_back (CPrimitiveClass::CInitParameters ());
					
					// Child ref
					CPrimitiveClass::CInitParameters &childParam = child.Parameters.back ();

					// Read the class name
					if (config.getPropertyString (childParam.Name, filename, childParamNode, "NAME"))
					{
						// Read the parameters
						xmlNodePtr childParamValueNode = CIXml::getFirstChildNode (childParamNode, "DEFAULT_VALUE");
						childParam.DefaultValue.resize (CIXml::countChildren (childParamNode, "DEFAULT_VALUE"));
						uint defaultId = 0;
						if (childParamValueNode)
						{
							do
							{
								// Gen id flag
								childParam.DefaultValue[defaultId].GenID = false;

								// Read the gen id flag
								string value;
								if (CIXml::getPropertyString (value, childParamValueNode, "GEN_ID") && (value != "false"))
								{
									childParam.DefaultValue[defaultId].GenID = true;
								}
								else
								{
									if (config.getPropertyString (value, filename, childParamValueNode, "VALUE"))
									{
										childParam.DefaultValue[defaultId].Name = value;
									}
									else
										goto failed;
								}
								defaultId++;
							}
							while ((childParamValueNode = CIXml::getNextChildNode (childParamValueNode, "DEFAULT_VALUE")));
						}
					}
					else
						goto failed;
				}
				while ((childParamNode = CIXml::getNextChildNode (childParamNode, "PARAMETER")));
			}

			// Ok
			return true;
		}
	}
failed:
	return false;
}

// ***************************************************************************

bool CPrimitiveClass::read (xmlNodePtr primitiveNode, const char *filename, const char *className, std::set<std::string> &contextStrings, 
							std::map<std::string, std::string> &contextFilesLookup, CLigoConfig &config)
{
	// The name
	Name = className;

	// Read the type
	std::string type;
	if (config.getPropertyString (type, filename, primitiveNode, "TYPE"))
	{
		// Good type ?
		if (type == "node")
			Type = Node;
		else if (type == "point")
			Type = Point;
		else if (type == "path")
			Type = Path;
		else if (type == "zone")
			Type = Zone;
		else if (type == "bitmap")
			Type = Bitmap;
		else 
		{
			config.syntaxError (filename, primitiveNode, "Unknown primitive type (%s)", type.c_str ());
			goto failed;
		}

		// Read the color
		ReadColor (Color, primitiveNode);

		// Autoinit
		AutoInit = false;
		ReadBool ("AUTO_INIT", AutoInit, primitiveNode, filename, config);
		
		// Deletable
		Deletable = true;
		ReadBool ("DELETABLE", Deletable, primitiveNode, filename, config);

		// File extension
		FileExtension = "";
		CIXml::getPropertyString (FileExtension, primitiveNode, "FILE_EXTENSION");

		// File type
		FileType = "";
		CIXml::getPropertyString (FileType, primitiveNode, "FILE_TYPE");

		// Collision
		Collision = false;
		ReadBool ("COLLISION", Collision, primitiveNode, filename, config);

		// LinkBrothers
		LinkBrothers = false;
		ReadBool ("LINK_BROTHERS", LinkBrothers, primitiveNode, filename, config);

		// ShowArrow
		ShowArrow = true;
		ReadBool ("SHOW_ARROW", ShowArrow, primitiveNode, filename, config);
		
		// Numberize when copy the primitive
		Numberize = true;
		ReadBool ("NUMBERIZE", Numberize, primitiveNode, filename, config);

		// Read the parameters
		xmlNodePtr paramNode = CIXml::getFirstChildNode (primitiveNode, "PARAMETER");
		if (paramNode)
		{
			do
			{
				// Add a parameter
				Parameters.push_back (CParameter ());

				// The parameter ref
				CParameter &parameter = Parameters.back ();

				// Read the property name
				if (config.getPropertyString (type, filename, paramNode, "NAME"))
				{
					// Set the name
					parameter.Name = type;

					// Read the type
					if (config.getPropertyString (type, filename, paramNode, "TYPE"))
					{
						// Good type ?
						if (type == "boolean")
							parameter.Type = CParameter::Boolean;
						else if (type == "const_string")
							parameter.Type = CParameter::ConstString;
						else if (type == "string")
							parameter.Type = CParameter::String;
						else if (type == "string_array")
							parameter.Type = CParameter::StringArray;
						else if (type == "const_string_array")
							parameter.Type = CParameter::ConstStringArray;
						else
						{
							config.syntaxError (filename, paramNode, "Unknown primitive parameter type (%s)", type.c_str ());
							goto failed;
						}

						// Visible
						parameter.Visible = true;
						ReadBool ("VISIBLE", parameter.Visible, paramNode, filename, config);

						// Filename
						parameter.Filename = false;
						ReadBool ("FILENAME", parameter.Filename, paramNode, filename, config);

						// Lookup
						parameter.Lookup = false;
						ReadBool ("LOOKUP", parameter.Lookup, paramNode, filename, config);

						// Read only primitive
						parameter.ReadOnly = false;
						ReadBool ("READ_ONLY", parameter.ReadOnly, paramNode, filename, config);

						// sort combo box entries 
						parameter.SortEntries = false;
						ReadBool ("SORT_ENTRIES", parameter.SortEntries, paramNode, filename, config);

						// Display horizontal scoller in multiline edit box
						parameter.DisplayHS = false;
						ReadBool ("SHOW_HS", parameter.DisplayHS, paramNode, filename, config);

						// Lookup
						parameter.WidgetHeight = 100;
						int temp;
						if (ReadInt ("WIDGET_HEIGHT", temp, paramNode))
							parameter.WidgetHeight = (uint)temp;

						// Read the file extension
						parameter.FileExtension = "";
						CIXml::getPropertyString (parameter.FileExtension, paramNode, "FILE_EXTENSION");
						parameter.FileExtension = strlwr (parameter.FileExtension);

						// Autonaming preference
						parameter.Autoname = "";
						CIXml::getPropertyString (parameter.Autoname, paramNode, "AUTONAME");
						
						// Read the file extension
						parameter.Folder = "";
						CIXml::getPropertyString (parameter.Folder, paramNode, "FOLDER");
						parameter.Folder = strlwr (parameter.Folder);

						// Read the combo values
						xmlNodePtr comboValueNode = CIXml::getFirstChildNode (paramNode, "COMBO_VALUES");
						if (comboValueNode)
						{
							do
							{
								// Read the context
								if (config.getPropertyString (type, filename, comboValueNode, "CONTEXT_NAME"))
								{
									// Add this context
									contextStrings.insert (type);

									// Add a combo value
									pair<std::map<std::string, CParameter::CConstStringValue>::iterator, bool> insertResult =
										parameter.ComboValues.insert (std::map<std::string, CParameter::CConstStringValue>::value_type (type, CParameter::CConstStringValue ()));

									// The combo value ref
									CParameter::CConstStringValue &comboValue = insertResult.first->second;

									// Read the values
									xmlNodePtr comboValueValueNode = CIXml::getFirstChildNode (comboValueNode, "CONTEXT_VALUE");
									comboValue.Values.reserve (CIXml::countChildren (comboValueNode, "CONTEXT_VALUE"));
									if (comboValueValueNode)
									{
										do
										{
											// Read the value 
											if (config.getPropertyString (type, filename, comboValueValueNode, "VALUE"))
											{
												// Add a combo box
												comboValue.Values.push_back (type);
											}
											else
												goto failed;
										}
										while ((comboValueValueNode = CIXml::getFirstChildNode (comboValueValueNode, "CONTEXT_VALUE")));
									}
								}
								else
									goto failed;
							}
							while ((comboValueNode = CIXml::getNextChildNode (comboValueNode, "COMBO_VALUES")));
						}

						// Read the combo files
						comboValueNode = CIXml::getFirstChildNode (paramNode, "COMBO_FILES");
						if (comboValueNode)
						{
							do
							{
								// Read the context
								if (config.getPropertyString (type, filename, comboValueNode, "CONTEXT_NAME"))
								{
									// Read the path to search
									string path;
									if (config.getPropertyString (path, filename, comboValueNode, "PATH"))
									{
										// Look for files in the path
										std::vector<std::string> files;
										CPath::getPathContent (path, true, false, true, files);

										// Not empty ?
										if (!files.empty ())
										{
											// Add this context
											contextStrings.insert (type);

											// For each file
											for (uint i=0; i<files.size (); i++)
											{
												// Good extension ?
												if (strlwr (NLMISC::CFile::getExtension (files[i])) == parameter.FileExtension)
												{
													// Add a combo value
													pair<std::map<std::string, CParameter::CConstStringValue>::iterator, bool> insertResult =
														parameter.ComboValues.insert (std::map<std::string, CParameter::CConstStringValue>::value_type (type, CParameter::CConstStringValue ()));

													// The combo value ref
													CParameter::CConstStringValue &comboValue = insertResult.first->second;

													// Get the filename without extension
													string nameWithoutExt = strlwr (NLMISC::CFile::getFilenameWithoutExtension (files[i]));

													// Add the values
													comboValue.Values.push_back (nameWithoutExt);

													// Add the value for lookup
													contextFilesLookup.insert (map<string, string>::value_type (nameWithoutExt, files[i]));
												}
											}
										}
									}
									else
										goto failed;
								}
								else
									goto failed;
							}
							while ((comboValueNode = CIXml::getNextChildNode (comboValueNode, "COMBO_FILES")));
						}

						// Read parameters default values
						xmlNodePtr defaultValueNode = CIXml::getFirstChildNode (paramNode, "DEFAULT_VALUE");
						parameter.DefaultValue.resize (CIXml::countChildren (paramNode, "DEFAULT_VALUE"));
						uint defaultId = 0;
						if (defaultValueNode)
						{
							do
							{
								// Gen id flag
								parameter.DefaultValue[defaultId].GenID = false;

								// Read the gen id flag
								string value;
								if (CIXml::getPropertyString (value, defaultValueNode, "GEN_ID") && (value != "false"))
								{
									parameter.DefaultValue[defaultId].GenID = true;
								}
								else
								{
									if (config.getPropertyString (value, filename, defaultValueNode, "VALUE"))
									{
										parameter.DefaultValue[defaultId].Name = value;
									}
									else
										goto failed;
								}
								defaultId++;
							}
							while ((defaultValueNode = CIXml::getNextChildNode (defaultValueNode, "DEFAULT_VALUE")));
						}
					}
					else
						goto failed;
				}
				else
					goto failed;
			}
			while ((paramNode = CIXml::getNextChildNode (paramNode, "PARAMETER")));
		}

		// Read static children
		xmlNodePtr childrenNode = CIXml::getFirstChildNode (primitiveNode, "STATIC_CHILD");
		StaticChildren.reserve (CIXml::countChildren (primitiveNode, "STATIC_CHILD"));
		if (childrenNode)
		{
			do
			{
				// Add a static child
				StaticChildren.push_back (CChild ());
				
				// Child ref
				CChild &child = StaticChildren.back ();

				// Read the child
				if (!ReadChild (child, childrenNode, filename, true, config))
					goto failed;
			}
			while ((childrenNode = CIXml::getNextChildNode (childrenNode, "STATIC_CHILD")));
		}

		// Read dynamic children
		childrenNode = CIXml::getFirstChildNode (primitiveNode, "DYNAMIC_CHILD");
		DynamicChildren.reserve (CIXml::countChildren (primitiveNode, "DYNAMIC_CHILD"));
		if (childrenNode)
		{
			do
			{
				// Add a static child
				DynamicChildren.push_back (CChild ());
				
				// Child ref
				CChild &child = DynamicChildren.back ();

				// Read the child
				if (!ReadChild (child, childrenNode, filename, false, config))
					goto failed;
			}
			while ((childrenNode = CIXml::getNextChildNode (childrenNode, "DYNAMIC_CHILD")));
		}

		// Read generated children
		childrenNode = CIXml::getFirstChildNode (primitiveNode, "GENERATED_CHILD");
		GeneratedChildren.reserve (CIXml::countChildren (primitiveNode, "GENERATED_CHILD"));
		if (childrenNode)
		{
			do
			{
				// Add a static child
				GeneratedChildren.push_back (CChild ());
				
				// Child ref
				CChild &child = GeneratedChildren.back ();

				// Read the child
				if (!ReadChild (child, childrenNode, filename, false, config))
					goto failed;
			}
			while ((childrenNode = CIXml::getNextChildNode (childrenNode, "GENERATED_CHILD")));
		}

		return true;
	}
failed:
	return false;
}

// ***************************************************************************

CPrimitiveClass::CParameter::CParameter (const NLLIGO::IProperty &property, const char *propertyName)
{
	Name = propertyName;
	Filename = false;
	Visible = true;
	Type = (typeid (property) == typeid (CPropertyString)) ? CPrimitiveClass::CParameter::String : CPrimitiveClass::CParameter::StringArray;
}

// ***************************************************************************
// CPrimitiveClass::CParameter
// ***************************************************************************

bool CPrimitiveClass::CParameter::operator== (const CParameter &other) const
{
	return (Type == other.Type) &&
		(Name == other.Name) &&
		(Visible == other.Visible) &&
		(Filename == other.Filename) &&
		(ComboValues == other.ComboValues) &&
		(DefaultValue == other.DefaultValue);
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::operator< (const CParameter &other) const
{
	return (Name < other.Name) ? true : (Name > other.Name) ? false :
		(Type < other.Type) ? true : (Type > other.Type) ? false :
		(Visible < other.Visible) ? true : (Visible > other.Visible) ? false :
		(Filename < other.Filename) ? true : (Filename > other.Filename) ? false :
		(ComboValues < other.ComboValues) ? true : (ComboValues > other.ComboValues) ? false :
		(DefaultValue < other.DefaultValue) ? true : (DefaultValue > other.DefaultValue) ? false : 
		false;
}

// ***************************************************************************
// CPrimitiveClass::CParameter::CConstStringValue
// ***************************************************************************

bool CPrimitiveClass::CParameter::CConstStringValue::operator== (const CConstStringValue &other) const
{
	return Values == other.Values;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::CConstStringValue::operator< (const CConstStringValue &other) const
{
	return Values < other.Values;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::translateAutoname (std::string &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass) const
{
	result = "";
	uint strBegin = 0;
	uint strEnd = 0;
	while (strBegin != Autoname.size())
	{
		strEnd = Autoname.find ('$', strBegin);
		if (strEnd == string::npos)
		{
			strEnd = Autoname.size();
			result += Autoname.substr (strBegin, strEnd-strBegin);
		}
		else
		{
			// Copy the remaining string
			result += Autoname.substr (strBegin, strEnd-strBegin);
			if (strEnd != Autoname.size())
			{
				strBegin = strEnd+1;
				strEnd = Autoname.find ('$', strBegin);
				if (strEnd == string::npos)
					strEnd = Autoname.size();
				else
				{
					string keyWord = Autoname.substr (strBegin, strEnd-strBegin);

					// Loop for the parameter
					uint i;
					for (i=0; i<primitiveClass.Parameters.size (); i++)
					{
						if (primitiveClass.Parameters[i].Name == keyWord)
						{
							// Get its string value
							string str;
							const IProperty *prop;
							if (primitive.getPropertyByName (keyWord.c_str(), prop))
							{
								// The property has been found ?
								if (prop)
								{
									// Array or string ?
									const CPropertyString *_string = dynamic_cast<const CPropertyString *>(prop);									
							
									// Is a string ?
									if (_string)
									{
										if (!(_string->String.empty()))
										{
											result += _string->String;
											break;
										}
									}
									else
									{
										// Try an array
										const CPropertyStringArray *array = dynamic_cast<const CPropertyStringArray *>(prop);
										
										// Is an array ?
										if (array)
										{
											if (!(array->StringArray.empty()))
											{
												uint i;
												for (i=0; i<array->StringArray.size()-1; i++)
													result += array->StringArray[i] + "\n";
												result += array->StringArray[i];
												break;
											}
										}
									}
								}
							}

							// Get its default value
							std::string result2;
							if (primitiveClass.Parameters[i].getDefaultValue (result2, primitive, primitiveClass))
							{
								result += result2;
								break;
							}
						}
					}
					strEnd++;
				}
				
			}
		}
		strBegin = strEnd;
	}
	return true;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::getDefaultValue (std::string &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass, std::string *fromWhere) const
{
	result = "";
	if (!Autoname.empty())
	{
		if (fromWhere)
			*fromWhere = "Autoname value : "+Autoname;
		return translateAutoname (result, primitive, primitiveClass);
	}
	else
	{
		if (fromWhere)
			*fromWhere = "Default value";
		if (!DefaultValue.empty())
			result = DefaultValue[0].Name;
	}
	return true;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::getDefaultValue (std::vector<std::string> &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass, std::string *fromWhere) const
{
	if (!Autoname.empty())
	{
		string temp;
		if (translateAutoname (temp, primitive, primitiveClass))
		{
			result.clear ();
			if (!temp.empty())
			{
				string tmp;
				uint i;
				for (i=0; i<temp.size(); i++)
				{
					if (temp[i] == '\n')
					{
						result.push_back (tmp);
						tmp.clear();
					}
					else
					{
						tmp.push_back(temp[i]);
					}
				}				
				if (!tmp.empty())
					result.push_back (tmp);
			}
			return true;
		}
		else
			return false;
	}
	else
	{
		uint i;
		result.resize (DefaultValue.size());
		for (i=0; i<DefaultValue.size(); i++)
			result[i] = DefaultValue[i].Name;
	}
	return true;
}

// ***************************************************************************

