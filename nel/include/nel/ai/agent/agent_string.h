/** \file agent_string.h
 * class for string manipulation.
 *
 * $Id: agent_string.h,v 1.8 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_AGENT_STRING_H
#define NL_AGENT_STRING_H
#include <list>

#include "nel/ai/c/abstract_interface.h"

namespace NLAIAGENT
{	
	/**	
	This is an abstract class for string in NLAIC::IBasicInterface format.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/
	class IVarName: public NLAIC::IBasicInterface
	{
	public:

		IVarName()
		{
		}

		///Get the string stored in the instance.
		virtual const char *getString() const = 0;		

		/// \name Compare methode. 
		//@{
		bool operator < (const IVarName &v) const
		{			
			const char *name1 = getString();
			const char *name2 = v.getString();
			if(name1[0] < name2[0]) return true;
			else return strcmp(name1,name2) < 0;			
		}

		bool operator > (const IVarName &v) const
		{			
			const char *name1 = getString();
			const char *name2 = v.getString();
			if(name1[0] > name2[0]) return true;
			else return strcmp(name1,name2) > 0;
		}


		bool operator == (const IVarName &v) const
		{
			const char *name1 = getString();
			const char *name2 = v.getString();
			if(name1[0] == name2[0]) return strcmp(name1,name2) == 0;
			
			return false;
		}
		//@}

		///Adding two string.
		const char *addString(const IVarName &s) const 
		{
			char *nameTmp,*str = (char *)s.getString();		
			nameTmp = (char *)getString();	
			char *name = new char [strlen(str) + strlen(nameTmp) + 2];
			sprintf(name,"%s%s",nameTmp,str);			
			return name;
		}

		///Extract an string from an ather.
		const char *subString(const IVarName &s) const 
		{
			char *nameTmp,*str = (char *)s.getString();		
			nameTmp = (char *)getString();	

			sint32 k = strlen(nameTmp);

			for(sint32 i = 0; i < k; i ++)
			{
				if(!strcmp(&nameTmp[i],str))
				{
					sint32 l,n = strlen(str);
					char *name = new char [(l = (k - n + 1))];
					memcpy(name,nameTmp,i);
					memcpy(&name[i],nameTmp + n,l);
					return name;
				}
			}
			return NULL;
		}

		/// \name Usually operation. 
		//@{
		virtual IVarName &operator += (const IVarName &s) = 0;
		virtual IVarName &operator -= (const IVarName &s) = 0;
		virtual IVarName &operator = (const IVarName &v) = 0;		
		//@}

		virtual ~IVarName()
		{
		}	
		//virtual typeVarName getEnumType() const = 0;
		
	};		

	/**	
	Basic concret class for string manipulation with NLAIC::IBasicInterface format.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/
	class CStringVarName : public IVarName
	{
	public:
		static const NLAIC::CIdentType IdStringVarName;
	private:
		char *_Name;
	public:
		CStringVarName(const char *name)
		{
			sint32 i = strlen(name) + 1;
			_Name =  new char [i];
			if(i == 1)
			{
				_Name[0] = 0;
			}			
			else memcpy(_Name,name,i);
		}

		CStringVarName(const CStringVarName &s)
		{
			sint32 i = strlen(s._Name) + 1;
			_Name =  new char [i];
			if(i == 1)
			{
				_Name[0] = 0;
			}			
			else memcpy(_Name,s._Name,i);
		}

		CStringVarName(NLMISC::IStream &is):_Name(NULL)
		{
			load(is);
		}
		

		virtual ~CStringVarName()
		{
			delete []_Name;
		}	

		virtual  const char *getString() const
		{
			return _Name;
		}		

		const NLAIC::CIdentType &getType() const;
		

		void save(NLMISC::IStream &os)
		{			
			uint32 size = strlen(_Name) + 1;
			os.serial( size );
			std::string x = std::string(_Name);
			os.serial( x );
		}

		void load(NLMISC::IStream &is)
		{
			uint32 i;		
			is.serial(i);
			if(_Name != NULL) delete []_Name;
			_Name =  new char [i];
			std::string name;
			is.serial( name );	
			strcpy(_Name, name.c_str() );
		}		

		IVarName &operator += (const IVarName &s)
		{
			char *nameTmp = (char *)addString(s);
			delete _Name;
			_Name = nameTmp;			
			return *this;
		}		
		
		IVarName &operator -= (const IVarName &s)
		{			
			char *n = (char *)subString(s);
			if(n)
			{
				delete _Name;
				_Name = n;
			}				
			return *this;
		}
		
		IVarName &operator = (const IVarName &v)
		{
			delete _Name;
			sint32 i = strlen(v.getString()) + 1;
			_Name =  new char [i];
			memcpy(_Name,v.getString(),i);
			return *this;
		}

		CStringVarName &operator = (const CStringVarName &v)
		{
			delete _Name;
			sint32 i = strlen(v.getString()) + 1;
			_Name =  new char [i];
			memcpy(_Name,v.getString(),i);
			return *this;
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CStringVarName(_Name);
			return m;
		}

		const NLAIC::IBasicType *newInstance() const
		{			
			return clone();
		}

		void getDebugString(std::string &text) const
		{
			text = NLAIC::stringGetBuild("'%s'",_Name);			
		}
	};	

	/**	
	This class is an IVarName concret class. Its share string in a static map, when we constuct a new instance if string is not stored in the std::map 
	then its add in it else it references it from string stored in th std::map.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/
	class CIndexedVarName : public IVarName
	{

	public:
		static const NLAIC::CIdentType IdIndexedVarName;
	private:
		///Structure for store string in a std::map.
		struct CNameStruc
		{
			///Number of reference.
			sint32 Count;
			///unique index for the string
			sint32 Index;
			///String stored in the map.
			CStringVarName *Name;
			

			CNameStruc():Count(0),Index(0),Name(NULL) {}
			CNameStruc(CStringVarName *name,sint32 index):Count(0),Index(index),Name(name){}
			///add new reference. if Count is null we have to remove this structur from thje map.
			const sint32  &inc()
			{
				return ++Count;
			}
			///dec one reference. if ref
			const sint32  &dec()
			{
				return --Count;
			}

			virtual void save(NLMISC::IStream &)
			{
				// TODO
/*				os.serial( Count );
				os.serial( Index );
				os.serial( *Name );*/
			}

			virtual void load(NLMISC::IStream &)
			{
/*				is.serial(Count);
				is.serial(Index);				
				if(Name == NULL) 
					Name = new CStringVarName(is);
				is.serial(Name);*/
			}
		};		
		
		///Definition of the static std::map.
		typedef std::map<CStringVarName ,CNameStruc *> TMapName;

	private:

		///Static std::map for storing string.
		static TMapName *_Map;
		///Table of string, this the contents of the map.
		static CNameStruc *_TableName;
		/**
		Actual nomber of allocation resolution in the Table.
		Note that when we superseded table resolution we realloc a new table with the old size add by _Bank.
		*/
		static const sint32 _Bank;

		///Number of string allocated. This is the table size.
		static sint32 _Count;
		/**
		Number of cell free in the table, not that this is very usefull because thy prevent us to realloc all time a new table.
		*/
		static std::list<CNameStruc *> *_Empty;

	private:
		///the index of the declared string in the table.
		sint32 _Index;

	public:
		CIndexedVarName(const char *name);
		CIndexedVarName(const CIndexedVarName &name);
		CIndexedVarName(NLMISC::IStream &is);


		sint32 newIndex();
		
		const char *getString() const;

		const NLAIC::CIdentType &getType() const;	

		const sint32 &getIndex() const
		{
			return _Index;
		}

		IVarName &operator += (const IVarName &s);		

		IVarName &operator -= (const IVarName &s);

		IVarName &operator = (const IVarName &v);		

		void save(NLMISC::IStream &os);
		void load(NLMISC::IStream &is);
		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		void getDebugString(std::string &text) const;

		virtual ~CIndexedVarName();

	private:
		///Insert new string.
		void insert(const CStringVarName &name);
		///Clear string from map.
		void clear();

	public:
		static void initClass();
		static void saveClass(NLMISC::IStream &is);
		static void loadClass(NLMISC::IStream &is);
		static void releaseClass();
	};
}
#endif
