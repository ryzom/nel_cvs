/** \file ligo_config.h
 * Ligo config file 
 *
 * $Id: ligo_config.h,v 1.3 2003/11/17 14:26:37 distrib Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_LIGO_CONFIG_H
#define NL_LIGO_CONFIG_H

#include "nel/misc/types_nl.h"
#include "primitive_class.h"
#include "primitive_configuration.h"

#define DEFAULT_PRIMITIVE_COLOR (CRGBA (128, 0, 0, 128))

namespace NLLIGO
{

class IPrimitive;

/**
 *  Ligo config file 
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CLigoConfig
{
public:

	virtual ~CLigoConfig() { }

	/** Load the config file. Can throw some exception if file doesn't exist or is malformed.
	  * 
	  * This file will try to open the file ligo class description file (XML) using the LigoClass as file name.
	  * It will try first to load directly the file and then to lookup the file in NLMISC::CPath.
	  */
	bool read (const char *fileName);

	/**
	  * This file will read the file ligo class description file (XML) using the LigoClass as file name.
	  * It will try first to load directly the file and then to lookup the file in NLMISC::CPath.
	  */
	bool readPrimitiveClass (const char *fileName);

	/// \name Public value

	/// Size of a cell of the ligoscape in meter
	float	CellSize;

	/// Snap precision for vertex position checking in meter
	float	Snap;

	/// Zone snap shot resolution
	uint	ZoneSnapShotRes;

	/// The ligo class file
	std::string	PrimitiveClassFilename;

	/// \name Primitive class accessors

	// Get a primitive class
	const CPrimitiveClass					*getPrimitiveClass (const NLLIGO::IPrimitive &primitive) const;

	// Get a primitive class
	const CPrimitiveClass					*getPrimitiveClass (const char *className) const;

	// Get the primitive color
	NLMISC::CRGBA getPrimitiveColor (const NLLIGO::IPrimitive &primitive);

	// Is the primitive linked to it's brother primitive ?
	bool isPrimitiveLinked (const NLLIGO::IPrimitive &primitive);

	// Is the primitive deletable ?
	bool isPrimitiveDeletable (const NLLIGO::IPrimitive &primitive);

	// Is the child primitive can be a child of the parent primitive ?
	bool canBeChild (const NLLIGO::IPrimitive &child, const NLLIGO::IPrimitive &parent);

	// Is the primitive a root primitive ?
	bool canBeRoot (const NLLIGO::IPrimitive &primitive);

	// Read a property from an XML file
	bool getPropertyString (std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName);

	// Output error message
	void syntaxError (const char *filename, xmlNodePtr xmlNode, const char *format, ...);
	virtual void errorMessage (const char *format, ... );

	// Acces to the config string
	const std::vector<std::string> &getContextString () const;

	// Access the primitive configuration
	const std::vector<CPrimitiveConfigurations> &getPrimitiveConfiguration() const
	{
		return _PrimitiveConfigurations;
	}

private:

	// Init primitive class manager
	bool		initPrimitiveClass (const char *filename);

	// The primitive class manager
	std::map<std::string, CPrimitiveClass>	_PrimitiveClasses;

	// The context strings
	std::vector<std::string>	_Contexts;

	// The file context look up
	std::map<std::string, std::string>	_ContextFilesLookup;

	// The primitive configurations
	std::vector<CPrimitiveConfigurations>	_PrimitiveConfigurations;
};

}

#endif // NL_LIGO_CONFIG_H

/* End of ligo_config.h */
