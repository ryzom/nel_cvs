/** \file sheet_id.h
 * This class defines a sheet id
 *
 * $Id: sheet_id.h,v 1.6 2002/06/24 17:14:50 vizerie Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#ifndef NL_SHEET_ID_H
#define NL_SHEET_ID_H

// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

// std
#include <string>
#include <map>

namespace NLMISC {

/**
 * CSheetId
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CSheetId
{

public :
	/// Unknow CSheetId is similar as an NULL pointer.
	static const CSheetId Unknown;

	/**
	 *	Constructor
	 */
	explicit CSheetId( uint32 sheetRef = 0 ) { _Id.Id = sheetRef; }

	/**
	 *	Constructor
	 */
	explicit CSheetId( const std::string& sheetName );

	// build from a string and returns true if the build succeed
	bool	 build(const std::string& sheetName);

	/**
	 *	Load the association sheet ref / sheet name 
	 */
	static void init();
	
	/**
	 * Return the **whole** sheet id (id+type)
	 */
	uint32 asInt() const { return _Id.Id; }

	/**
	 * Return the sheet type (sub part of the sheetid)
	 */
	uint8 getType() const { return _Id.IdInfos.Type; }

	/**
	 * Return the sheet sub id (sub part of the sheetid)
	 */
	uint8 getShortId() const { return _Id.IdInfos.Id; }

	/**
	 *	Operator=
	 */
	CSheetId& operator=( const CSheetId& sheetId );

	/**
	 *	Operator=
	 */
	CSheetId& operator=( const std::string& sheetName );

	/**
	 *	Operator=
	 */
	CSheetId& operator=( uint32 sheetRef );

	/**
	 *	Operator<
	 */
	bool operator < (const CSheetId& sheetRef ) const;

	/**
	 *	Operator==
	 */
	inline bool operator == (const CSheetId& sheetRef ) const { return ( _Id.Id == sheetRef._Id.Id) ; }

	/**
	 *	Operator !=
	 */
	inline bool operator != (const CSheetId& sheetRef ) const { return (_Id.Id != sheetRef._Id.Id) ; }




	/**
	 * Return the sheet id as a string
	 */
	std::string toString() const;
	
	/**
	 *	Serial
	 */
	void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
	{
		f.serial( _Id.Id );
	}


	/**
	 *  Display the list of valid sheet ids with their associated file names
	 *  if (type != -1) then restrict list to given type
	 */
	static void display();
	static void display(uint8 type);

	/**
	 *  Generate a vector of all the sheet ids of a given type 
	 *  This operation is non-destructive, the new entries are appended to the result vector
	 *  note: fileExtension include the '.' eg ".bla"
	 **/
	static void buildIdVector(std::vector <CSheetId> &result);
	static void buildIdVector(std::vector <CSheetId> &result, uint8 type);
	static void buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames, uint8 type);
	static void buildIdVector(std::vector <CSheetId> &result, const std::string &fileExtension);
	static void buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames, const std::string &fileExtension);

	/**
	 *  Convert between file extensions and numeric sheet types
	 *  note: fileExtension include the '.' eg ".bla"
	 **/
	static const std::string &fileExtensionFromType(uint8 type);
	static uint32 typeFromFileExtension(const std::string &fileExtension);

private :

	/// sheet id
	union TSheetId
	{
		uint32		Id;
		
		struct
		{
			uint32	Type	: 8;
			uint32	Id		: 24;
		} IdInfos;
	};
	TSheetId _Id;

	/// associate sheet id and sheet name
	static std::map<uint32,std::string> _SheetIdToName;
	static std::map<std::string,uint32> _SheetNameToId;
	static std::vector<std::string> _FileExtensions;
	static bool _initialised;
};

inline std::stringstream &operator << (std::stringstream &__os, const CSheetId &__t)
{
	__os << __t.toString ();
	return __os;
}

} // NLMISC

#endif // NL_SHEET_ID_H

/* End of sheet_id.h */
