/** \file primitive.h
 * <File description>
 *
 * $Id: primitive.h,v 1.13 2002/12/13 14:55:09 coutelas Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"

// Include from libxml2
#include <libxml/parser.h>

#include <vector>

namespace NLLIGO
{

// There are only x and y used in the vector
// LayerName is the Group to which the primitive belongs to
// Hide say if the primitive is hidden or not


// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 */
class IProperty
{
	// Force class to be polymorphic
	virtual void foo () const = 0;
};

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 * This is a simple string
 */
class CPropertyString : public IProperty
{
public:
	CPropertyString () {};
	CPropertyString (const char *str);
	~CPropertyString () {};
	std::string			String;

	// Force class to be polymorphic
	virtual void foo () const {};
};

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 * This is a string array
 */
class CPropertyStringArray : public IProperty
{
public:
	CPropertyStringArray () {};
	~CPropertyStringArray () {}
	CPropertyStringArray (const std::vector<std::string> &stringArray);
	std::vector<std::string>	StringArray;

	// Force class to be polymorphic
	virtual void foo () const {};
};

// ***************************************************************************

/*
 * This class is a property class for ligo primitive.
 * This is a string array
 */
class CPropertyColor : public IProperty
{
public:
	NLMISC::CRGBA		Color;

	// Force class to be polymorphic
	virtual void foo () const {};
};

// ***************************************************************************

class CPrimVector : public NLMISC::CVector
{
public:
	CPrimVector ()
	{
		Selected = false;
	}
	CPrimVector (const NLMISC::CVector &v)
	{
		CVector::operator= (v);
		Selected = false;
	}

	bool	Selected;
};

// ***************************************************************************

/*
 * This class is the base class for ligo primitive.
 *
 * Provide access to common properties.
 * Provide access to the primitive hierachy
 */
class IPrimitive
{
public:

	// Deprecated
	std::string						Layer;
	std::string						Name;

	// Expended in the tree view
	bool							Expanded;
	
	enum 
	{
		NotAnArray,
		AtTheEnd = 0xffffffff,
	};

	/// \name Hierarchy
	IPrimitive () 
	{
		_Parent = NULL;
	};
	virtual ~IPrimitive ();

	IPrimitive (const IPrimitive &node);

	// Make a copy of this primitive
	IPrimitive*	copy () const;

	virtual void operator= (const IPrimitive &node);

	/** Get the children primitive count */
	uint				getNumChildren () const
	{
		return _Children.size ();
	}

	/** Get a child primitive */
	bool				getChild (const IPrimitive *&result, uint childId) const;

	/** Get a child primitive */
	bool				getChild (IPrimitive *&result, uint childId);

	/** Get the parent primtive */
	IPrimitive			*getParent ()
	{
		return _Parent;
	}
	const IPrimitive	*getParent () const
	{
		return _Parent;
	}

	/** Get the id of the child, return 0xffffffff if not found */
	bool				getChildId (uint &childId, const IPrimitive *child) const;

	/** Remove a child primitive */
	bool				removeChild (uint childId);

	/** Remove all children primitives */
	void				removeChildren ();

	/** 
	  * Insert a child primitive before the index. 
	  * The pointer will be deleted by the parent primitive using the ::delete operator.
	  * return false if the index is invalid
	  */
	bool				insertChild (IPrimitive *primitive, uint index = AtTheEnd);

	/// \name Properties 

	/** 
	  * Get a num properties
	  **/
	uint				getNumProperty () const;

	/** 
	  * Get a properties by its index
	  * This is method (iterate a list) is slower than getPropertyByName (look up in a map).
	  **/
	bool				getProperty (uint index, std::string &property_name, const IProperty *&result) const;

	/** 
	  * Get a properties by its index
	  * This is method (iterate a list) is slower than getPropertyByName (look up in a map).
	  **/
	bool				getProperty (uint index, std::string &property_name, IProperty *&result);

	/** 
	  * Add a property 
	  * If the property already exist, the method does nothing and returns false.
	  * The pointer will be deleted by the primitive using the ::delete operator.
	  **/
	bool				addPropertyByName (const char *property_name, IProperty *result);

	/** 
	  * Get a property with its name
	  **/
	bool				getPropertyByName (const char *property_name, const IProperty *&result) const;

	/** 
	  * Get a property with its name
	  **/
	bool				getPropertyByName (const char *property_name, IProperty *&result);

	/** 
	  * Get a string property with its name. Return false if the property is not found or is not a string property.
	  **/
	bool				getPropertyByName (const char *property_name, std::string *&result);

	/** 
	  * Get a string array property with its name. Return false if the property is not found or is not a string array property.
	  **/
	bool				getPropertyByName (const char *property_name, std::vector<std::string> *&result);

	/** 
	  * Get a string property with its name. Return false if the property is not found or is not a string property.
	  **/
	bool				getPropertyByName (const char *property_name, std::string &result) const;

	/** 
	  * Get a string array property with its name. Return false if the property is not found or is not a string array property.
	  **/
	bool				getPropertyByName (const char *property_name, const std::vector<std::string> *&result) const;

	/** 
	  * Get a color property with its name. Return false if the property is not found or is not a string array property.
	  **/
	bool				getPropertyByName (const char *property_name, NLMISC::CRGBA &result) const;

	/** 
	  * Remove a property
	  * This is method (iterate a list) is slower than removePropertyByName (look up in a map).
	  **/
	bool				removeProperty (uint index);

	/** 
	  * Remove a property by its name
	  **/
	bool				removePropertyByName (const char *property_name);

	/** 
	  * Remove all the properties
	  **/
	void				removeProperties ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;

	// Get the vertices
	virtual uint				getNumVector () const = 0;
	virtual const CPrimVector	*getPrimVector () const = 0;
	virtual CPrimVector			*getPrimVector () = 0;

private:

	// Parent
	IPrimitive								*_Parent;

	// Children
	std::vector<IPrimitive*>				_Children;

	// Single properties
	std::map<std::string, IProperty*>		_Properties;
};

// ***************************************************************************

// Simple prmiitive node
class CPrimNode : public IPrimitive
{
private:

	// void operator= (const CPrimNode &node);
	
	// \name From IPrimitive

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
};

// ***************************************************************************

class CPrimPoint : public IPrimitive
{

public:

	CPrimPoint ()
	{
		Angle = 0;
	}


	CPrimVector				Point;
	float					Angle;	// Angle on OZ, CCW

public:

	// Deprecated
	void serial (NLMISC::IStream &f);

	// void operator= (const CPrimPoint &node);

private:
	
	// \name From IPrimitive

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
};


// ***************************************************************************
class CPrimPath : public IPrimitive
{

public:

	std::vector<CPrimVector>	VPoints;

public:

	// Deprecated
	void serial (NLMISC::IStream &f);

	// void operator= (const CPrimPath &node);

private:

	// \name From IPrimitive

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
};


// ***************************************************************************

class CPrimZone : public IPrimitive
{

public:

	std::vector<CPrimVector>	VPoints;

	static float getSegmentDist(const NLMISC::CVector v, const NLMISC::CVector &p1, const NLMISC::CVector &p2, NLMISC::CVector &nearPos);

public:

	bool contains (const NLMISC::CVector &v) const { return CPrimZone::contains(v, VPoints); }
	bool contains(const NLMISC::CVector &v, float &distance, NLMISC::CVector &nearPos, bool isPath) const { return CPrimZone::contains(v, VPoints, distance, nearPos, isPath); }

	// void operator= (const CPrimZone &node);

	// Deprecated
	void serial (NLMISC::IStream &f);

	// Returns true if the vector v is inside of the patatoid
	static bool contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points);
	// Returns true if the vector v is inside of the patatoid and set the distance of the nearest segement and the position of the nearsest point.
	static bool contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath);
	// Returns true if the vector v is inside of the patatoid
	static bool contains (const NLMISC::CVector &v, const std::vector<CPrimVector> &points);
	// Returns true if the vector v is inside of the patatoid and set the distance of the nearest segement and the position of the nearsest point.
	static bool contains (const NLMISC::CVector &v, const std::vector<CPrimVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath);

private:

	// \name From IPrimitive

	// Get the vertices
	virtual uint				getNumVector () const;
	virtual const CPrimVector	*getPrimVector () const;
	virtual CPrimVector			*getPrimVector ();

	// Read the primitive
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version);

	// Write the primitive
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
};

// ***************************************************************************

/*
This class is deprecated.
*/
class CPrimRegion
{

public:

	std::string				Name;
	std::vector<CPrimPoint> VPoints;
	std::vector<CPrimZone>	VZones;
	std::vector<CPrimPath>	VPaths;

	std::vector<bool>		VHidePoints;
	std::vector<bool>		VHideZones;
	std::vector<bool>		VHidePaths;

public:

	void serial (NLMISC::IStream &f);
};

// ***************************************************************************

/**
  * This class is a ligo primitives set
  */
class CPrimitives
{
public:

	// Convert from old format to the new one
	void			convert (const CPrimRegion &region);

	// Root primitive hierarchy
	CPrimNode		RootNode;

	// Read the primitive
	bool			read (xmlNodePtr xmlNode, const char *filename);

	// Write the primitive
	void			write (xmlDocPtr xmlNode, const char *filename) const;

private:
	// Conversion internal methods
	void			convertAddPrimitive (IPrimitive *child, const IPrimitive *prim, bool hidden);
	void			convertPrimitive (const IPrimitive *prim, bool hidden);

};

// ***************************************************************************


} // namespace NLLIGO

#endif // __PRIMITIVE_H__

