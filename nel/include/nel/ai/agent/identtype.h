/** \file identtype.h
 * Sevral class for identification an objects fonctionality.
 *
 * $Id: identtype.h,v 1.24 2002/06/17 14:16:54 chafik Exp $
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


#ifndef NL_IDENTTYPE_H
#define NL_IDENTTYPE_H
#include <stdlib.h>
#include "nel/misc/file.h"
#include "nel/misc/entity_id.h"
#include "nel/ai/e/ai_exception.h"
#include "nel/ai/agent/agent_string.h"

namespace NLAIAGENT
{	
#ifndef _MAX__INDEX_DEFINED
		const sint32 maxIndex = 2;
		#define _MAX__INDEX_DEFINED

		const uint64 maxResolutionNumer = ((uint64)2 << 48) - (uint64)1;
#else
		extern const sint32 maxIndex;
		extern const uint64 maxResolutionNumer;
#endif

	struct CAgentNumber: public NLMISC::CEntityId
	{		
		///All agents id'S numbers have the 8th bit reserved on NLMISC::CEntityId::Type byte
		///The CEntityId::Type field is for AI an user bits field becarful we dont't touch at the 8e bits.
		static const uint8 AgentTypeBit;
		static const CAgentNumber Unknow;

		CAgentNumber()
		{
			Type = CAgentNumber::AgentTypeBit;
		}
		CAgentNumber(uint64 id,uint8 creator,uint8 dyn):NLMISC::CEntityId(AgentTypeBit,id,creator,dyn)
		{			
		}

		CAgentNumber(uint64 id, uint8 t,uint8 creator,uint8 dyn):NLMISC::CEntityId(t,id,creator,dyn)
		{		
		}

		CAgentNumber(uint64 p):NLMISC::CEntityId(p)
		{
		}

		CAgentNumber(const CAgentNumber &a):NLMISC::CEntityId(a)
		{
		}

		CAgentNumber(NLMISC::IStream &is):NLMISC::CEntityId(is)
		{
		}

		CAgentNumber(const char *id):NLMISC::CEntityId(id)
		{
		}

		bool isAgentId() const
		{
			return (getType() & AgentTypeBit) != 0;
		}

		virtual bool operator < (const NLMISC::CEntityId &a) const
		{
			
			if(isAgentId())
			{
				if (Id < a.Id)
				{
					return true;
				}
				else 
				if (Id == a.Id)
				{
					if(CreatorId < a.CreatorId)
					{
						return true;
					}
					else
						if(CreatorId == a.CreatorId)
						{
							return DynamicId < a.DynamicId;
						}
				}
				return false;
			}
			else
			{
				return CEntityId::operator < (a);
			}
		}

		///That function allow user to change the 7 bit of the type field in the sid agent number; We don't touche at the 8e bits.
		void setTypeAt(uint64 t)
		{
			CEntityId::Type |= (t & 0x7f);
		}

	};

	/*struct CAgentNumber: public NLMISC::IStreamable
	{
		static uint8 ServerID;

		///num of the service manager who is it.
		uint64	DynamicId   :  8;
		///num of the service manager whos created it.
		uint64	CreatorId   :  8;
		///Local agent number.
		uint64	AgentNumber : 48;

		CAgentNumber()
		{
			CreatorId = (uint64)ServerID;
			DynamicId = (uint64)ServerID;
			AgentNumber = 0;
		}		

		CAgentNumber(uint64 id,uint64 creator,uint64 dyn)
		{
			CreatorId = creator;
			DynamicId = dyn;
			AgentNumber = id;
		}

		CAgentNumber(uint64 p)
		{			
			DynamicId = (uint64)(p & 0xff);
			p >>= 8;
			CreatorId = (uint64)(p & 0xff);
			p >>= 8;
			AgentNumber = (uint64)(p);			
		}

		CAgentNumber(const CAgentNumber &a)
		{
			CreatorId = a.CreatorId;			
			DynamicId = a.DynamicId;
			AgentNumber = a.AgentNumber;
		}

		///fill from read stream.
		CAgentNumber(NLMISC::IStream &is)
		{
			uint64 p;
			is.serial(p);

			DynamicId = (uint64)(p & 0xff);
			p >>= 8;
			CreatorId = (uint64)(p & 0xff);
			p >>= 8;
			AgentNumber = (uint64)(p);			
		}

		CAgentNumber(const char *id);

		///\name comparison of two CIndexVariant.
		//@{
		bool operator == (const CAgentNumber &a) const
		{			
			return (AgentNumber == a.AgentNumber && CreatorId == a.CreatorId);			
		}
				
		bool operator < (const CAgentNumber &a) const
		{			
			if(AgentNumber < a.AgentNumber) return true; 
			else
			if(AgentNumber == a.AgentNumber) return (CreatorId < a.CreatorId);

			return false;
		}

		bool operator > (const CAgentNumber &a) const
		{			
			if(AgentNumber > a.AgentNumber) return true; 
			else
			if(AgentNumber == a.AgentNumber) return (CreatorId > a.CreatorId);

			return false;
		}
		//@}

		const CAgentNumber &operator ++(int)/// throw (NLAIE::CExceptionIndexError)
		{
			if(AgentNumber < maxResolutionNumer)
			{
				AgentNumber ++;
			}
			else
			{
				throw NLAIE::CExceptionIndexError();
			}
			return *this;
		}

		const CAgentNumber &operator = (const CAgentNumber &a)
		{
			CreatorId = a.CreatorId;			
			DynamicId = a.DynamicId;
			AgentNumber = a.AgentNumber;
			return *this;
		}

		const CAgentNumber &operator = (uint64 p)
		{			
			DynamicId = (uint64)(p & 0xff);
			p >>= 8;
			CreatorId = (uint64)(p & 0xff);
			p >>= 8;
			AgentNumber = (uint64)(p);
			
			return *this;
		}

		operator uint64 () const
		{
			uint64 p = AgentNumber;
			p <<= 8;
			p |= (uint64)CreatorId;
			p <<= 8;
			p |= (uint64)DynamicId;

			return p;
		}

		///saving the nomber in an output stream.
		virtual void save(NLMISC::IStream &os)
		{			
			uint64 p = AgentNumber;
			p <<= 8;
			p |= (uint64)CreatorId;
			p <<= 8;
			p |= (uint64)DynamicId;
			os.serial(p);			
		}

		///loading the nomber from an input stream.
		virtual void load(NLMISC::IStream &is)
		{
			uint64 p;
			is.serial(p);

			DynamicId = (uint64)(p & 0xff);
			p >>= 8;
			CreatorId = (uint64)(p & 0xff);
			p >>= 8;
			AgentNumber = (uint64)(p);			

		}

		
		///Have a debug string.
		virtual void getDebugString(std::string &str) const;		

		/// \name NLMISC::IStreamable method.
		//@{
		virtual std::string	getClassName()
		{
			return std::string("<CAgentNumber>");
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if(f.isReading())
			{
				load(f);
			}
			else
			{				
				save(f);
			}

		}
		//@}

	};	*/

	/**
	Complete reference of an agent. 
	Name of the class of the type of object.
	Ident of the agent.
	*/
	struct CIdent: public CAgentNumber
	{
		std::string TypeName;
		CIdent()
		{			
		}		

		CIdent(const std::string &typeName,uint64 id,uint8 creator = ServerId,uint8 dyn = ServerId):CAgentNumber(id,creator,dyn),TypeName(typeName)
		{			
		}

		CIdent(const std::string &typeName,const CAgentNumber &a):CAgentNumber(a),TypeName(typeName)
		{			
		}

		CIdent(const CIdent &a):CAgentNumber(a),TypeName(a.TypeName)
		{			
		}

		///fill from read stream.
		CIdent(NLMISC::IStream &is):CAgentNumber(is)
		{			
			std::string s;
			is.serial(s);
			TypeName = s;
		}

		bool operator == (const CIdent &a) const
		{
			if(TypeName == a.TypeName) 
			{
				return CAgentNumber::operator == (a);
			}
			return false;
		}
				
		bool operator < (const CIdent &a) const
		{				
			return TypeName < a.TypeName;
		}

		bool operator > (const CIdent &a) const
		{
			return !(TypeName < a.TypeName);
		}		
		

		const CIdent &operator = (const CIdent &a)
		{
			CAgentNumber::operator = (a);
			TypeName = a.TypeName;
			return *this;
		}

		///saving the nomber in an output stream.
		virtual void save(NLMISC::IStream &os)
		{	
			CAgentNumber::save(os);
			os.serial(TypeName);
		}

		///loading the nomber from an input stream.
		virtual void load(NLMISC::IStream &is)
		{
			CAgentNumber::load(is);
			std::string s;
			is.serial(s);
			TypeName = s;
		}

		

		///Have a debug string.
		virtual void getDebugString(std::string &str) const
		{
			CAgentNumber::getDebugString(str);
			str += NLAIC::stringGetBuild(" '%s'",TypeName.data());
		}

		const std::string &getTypeName() const
		{
			return TypeName;
		}
	};
}
#endif
