/** \file agent_string.cpp
 *
 * $Id: agent_string.cpp,v 1.5 2002/11/05 14:07:50 chafik Exp $
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
#include "nel/ai/agent/agent.h"

namespace NLAIAGENT
{
		const NLAIC::CIdentType &CStringVarName::getType() const
	{		
		return IdStringVarName;
	}

	const NLAIC::CIdentType &CIndexedVarName::getType() const
	{
		
		return IdIndexedVarName;
	}

	void CIndexedVarName::initClass()
	{
		if(_Empty == NULL) 
		{
			_Empty = new std::list<CIndexedVarName::CNameStruc *>;
			
		}

		if(_Map == NULL) 
		{
			_Map = new CIndexedVarName::tMapName;
		}
		
		if(_TableName != NULL)
		{
			_Map->clear();
			for(sint32 i =0; i < _Count; i++)
			{
				if(_TableName[i].Name != NULL) delete _TableName[i].Name;
			}
			delete [] _TableName;
		}
		_TableName = new CNameStruc [_Bank];
		_Count = _Bank;
		
		for(sint32 i = 0; i < _Count; i++)
		{
			_TableName[i].Name = NULL;
			_TableName[i].Count = 0;
			_TableName[i].Index = i;
			_Empty->push_back(&_TableName[i]);
		}
		
	}
	
	void CIndexedVarName::saveClass(NLMISC::IStream &os)
	{
		sint32 count = (sint32) _Count;
		os.serial( count );
		for(sint32 i = 0; i < count; i++)
		{
			if(_TableName[i].Name != NULL)
			{
				sint32 s = 1;
				os.serial(  s  );
				_TableName[i].save(os);
			}
			else 
			{
				sint32 s = 0;
				os.serial(  s  );
			}
		}

	}

	void CIndexedVarName::loadClass(NLMISC::IStream &is)
	{
		sint32 i;
		is.serial( i );
		if(i != 0)
		{			
			releaseClass();
			_TableName = new CNameStruc [i];
			_Count = i;
			sint32 k;
			for(i = 0;i < _Count;i ++)
			{
				is.serial( k );
				if(k)
				{									
					_TableName[i].load(is);
					_Map->insert(tMapName::value_type(*_TableName[i].Name, &_TableName[i]));
				}
				else
				{
					_TableName[i].Count = 0;
					_TableName[i].Index = i;
					_TableName[i].Name = NULL;
					_Empty->push_back(&_TableName[i]);
				}
			}
		}

	}

	void CIndexedVarName::releaseClass()
	{
		return;

		if(_TableName != NULL)
		{
			_Map->clear();
			_Empty->clear();
			for(sint32 i =0; i < _Count; i++)
			{
				if(_TableName[i].Name != NULL) delete _TableName[i].Name;
			}
			delete [] _TableName;
			_TableName = NULL;
		}
		if(_Empty != NULL) 
		{
			delete _Empty;
			_Empty = NULL;
		}
		if(_Map != NULL) 
		{
			delete _Map;
			_Map = NULL;
		}
	}	

	CIndexedVarName::CIndexedVarName(const char *name)
	{
		insert(CStringVarName(name));
	}	

	CIndexedVarName::CIndexedVarName(const CIndexedVarName &name)
	{		
		tMapName::iterator i = _Map->find(*_TableName[name.getIndex()].Name);		
		if(i != _Map->end())
		{
			_Index = name.getIndex();
			(*i).second->Count ++;
		}
		else
		{
			_Index = -1;
			throw NLAIE::CExceptionIndexError();
		}
		
	}
	
	CIndexedVarName::CIndexedVarName(NLMISC::IStream &is)
	{
		CStringVarName str(is);
		insert(str);		
	}
	
	CIndexedVarName::~CIndexedVarName()
	{		
		clear();
	}

	IVarName &CIndexedVarName::operator = (const IVarName &s)
	{
		insert(CStringVarName(s.getString()));
		return *this;
	}

	void CIndexedVarName::clear()
	{		
		if(_TableName && !(--_TableName[_Index].Count) )
		{
			tMapName::iterator i = _Map->find(*_TableName[_Index].Name);
			if(i != _Map->end()) 
			{
				_Map->erase(i);
				delete _TableName[_Index].Name;
				_TableName[_Index].Name = NULL;
				_Empty->push_back(&_TableName[_Index]);
			}
		}
	}


	void CIndexedVarName::insert(const CStringVarName &str)
	{		
		tMapName::iterator i = _Map->find(str);		
		if(i != _Map->end())
		{
			_Index = (*i).second->Index;
			(*i).second->Count ++;
		}
		else 
		{
			_Index = newIndex();
			_TableName[_Index].Name = (CStringVarName *)str.clone();
			_Map->insert(tMapName::value_type(*_TableName[_Index].Name, &_TableName[_Index]));
		}
	}

	

	IVarName &CIndexedVarName::operator += (const IVarName &s)
	{
		char *n = (char *)addString(s);
		insert(n);
		return *this;
	}

	IVarName &CIndexedVarName::operator -= (const IVarName &s)
	{			
		char *n = (char *)subString(s);
		if(n != NULL) insert(n);
		return *this;
	}

	
	sint32 CIndexedVarName::newIndex()
	{

		if(_Empty->size())
		{
			CNameStruc *o = _Empty->front();
			_Empty->pop_front();
			o->Count++;
			return o->Index;
		}
		else
		{
			CNameStruc *tmp = _TableName;
			_TableName = new CNameStruc [_Count + _Bank];
			
			for(sint32 i = _Count; i < (_Count + _Bank); i++)
			{
				_TableName[i].Count = 0;
				_TableName[i].Index = i;
				_TableName[i].Name = NULL;
				_Empty->push_back(&_TableName[i]);
			}
			
			memcpy(_TableName, tmp, _Count*sizeof(CNameStruc));
			_Count += _Bank;
			return newIndex();
		}
	}

	void CIndexedVarName::save(NLMISC::IStream &os)
	{
		os.serial( *_TableName[getIndex()].Name );
	}
	
	void CIndexedVarName::load(NLMISC::IStream &is)
	{
		clear();
		CStringVarName str(is);
		insert(str);
	}
	
	const NLAIC::IBasicType *CIndexedVarName::clone() const
	{
		NLAIC::IBasicInterface *x = new CIndexedVarName(*this);
		return x;
	}

	const NLAIC::IBasicType *CIndexedVarName::newInstance() const
	{
		NLAIC::IBasicInterface *x = new CIndexedVarName("Inst");
		return x;
	}

	void CIndexedVarName::getDebugString(std::string &text) const
	{
		
	}

	const char *CIndexedVarName::getString() const
	{
		return (*(_TableName + _Index)).Name->getString();
		//return _TableName[_Index].Name->getString();
	}	
}
