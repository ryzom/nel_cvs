/** \file object_type.h
 * class encaputation a type of object.
 *
 * $Id: object_type.h,v 1.7 2001/05/22 16:08:01 chafik Exp $
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
#ifndef NL_OBJECT_TYPE_H
#define NL_OBJECT_TYPE_H


namespace NLAIAGENT
{	
	/**	
	  This class store an NLAIC::CIdentType as an IObjectIA. This usefull to transimt type object to the script.

	  * \author Portier Pierre	
	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class CObjectType: public IObjectIA	
	{
	
	private:
		///Type to store.
		NLAIC::CIdentType *_Type;		

	public:
		CObjectType(NLAIC::CIdentType *type):_Type(type)
		{

		}

		const NLAIC::CIdentType &getType() const/// throw (NLAIE::CExceptionUnReference)
		{
			if(_Type == NULL) throw NLAIE::CExceptionUnReference("object type is unreference");
			return *_Type;
		}

		const NLAIC::IBasicType *clone() const
		{			
			return new CObjectType(new NLAIC::CIdentType (*_Type));
		}

		const NLAIC::IBasicType *newInstance() const
		{
			return clone();			
		}		

		void getDebugString(std::string &txt) const
		{			
			txt += NLAIC::stringGetBuild("CObjectType< _Type < %s > >", (const char *)*_Type);
		}

		void save(NLMISC::IStream &os)
		{			
		}

		void load(NLMISC::IStream &is)
		{
		}

		bool isEqual(const IBasicObjectIA &a) const 
		{
			return *_Type == *((const CObjectType &)a)._Type;
		}
		
		const IObjectIA::CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}
		

		virtual ~CObjectType()
		{
			delete _Type;
		}
	};
}


#endif
