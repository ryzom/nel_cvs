/** \file msg_stream.h
 * 
 *	Using a NLAIAGENT::IMessageBase or in general NLAIAGENT::IBaseGroupType as a NLMISC::IStream
 *
 * $Id: msg_stream.h,v 1.4 2003/01/27 16:54:43 chafik Exp $
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
#ifndef NL_MSG_STREAM_H
#define NL_MSG_STREAM_H

#include "nel/misc/file.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/msg.h"

namespace NLAIAGENT
{
	/**
	Input Stream.
	*/
	class CMsgIStream: public NLMISC::IStream
	{
	private:
		const IBaseGroupType &_List;
		CConstIteratorContener _It;
		sint32 _Len;
		sint32 _Pos;

	public:
		CMsgIStream(const IBaseGroupType &l):NLMISC::IStream(true), _List(l), _It(_List.getConstIterator()), _Len(_List.size()), _Pos(0)
		{			
		}

		CMsgIStream(const CMsgIStream &ist) : NLMISC::IStream(ist), _List(ist._List), _It(ist._It), _Len(ist._Len), _Pos(ist._Len)
		{
		}

		virtual ~CMsgIStream()
		{
		}

		virtual void serial(uint8 &b) throw(NLMISC::EStream);
		virtual void serial(sint8 &b) throw(NLMISC::EStream);
		virtual void serial(uint16 &b) throw(NLMISC::EStream);
		virtual void serial(sint16 &b) throw(NLMISC::EStream);
		virtual void serial(uint32 &b) throw(NLMISC::EStream);
		virtual void serial(sint32 &b) throw(NLMISC::EStream);
		virtual void serial(uint64 &b) throw(NLMISC::EStream);
		virtual void serial(sint64 &b) throw(NLMISC::EStream);
		virtual void serial(float &b) throw(NLMISC::EStream);
		virtual void serial(double &b) throw(NLMISC::EStream);
		virtual void serial(bool &b) throw(NLMISC::EStream);

		virtual void serial(IObjectIA &b) throw(NLMISC::EStream);
		virtual void serial(IObjectIA* &b) throw(NLMISC::EStream);

#ifndef NL_OS_CYGWIN
		virtual void serial(char &b) throw(NLMISC::EStream);
#endif
		virtual void serial(std::string &b) throw(NLMISC::EStream);
		virtual void serial(ucstring &b) throw(NLMISC::EStream);
		virtual void serialBuffer(uint8 *buf, uint len)throw(NLMISC::EReadError)
		{
			throw NLMISC::EReadError((const char *)_List.getType());
		}
		virtual void serialBit(bool &bit) throw(NLMISC::EReadError)
		{
			throw NLMISC::EReadError((const char *)_List.getType());
		}
		virtual sint32 getPos ()
		{
			return _Pos;
		}
		bool isEnd() const
		{
			return _It.isInEnd();
		}
	};

	/**
	Output Stream.
	*/
	class CMsgOStream: public NLMISC::IStream
	{
	private:
		IBaseGroupType &_List;
	public:
		CMsgOStream(IBaseGroupType &msg):NLMISC::IStream(false), _List(msg)
		{			
		}

		CMsgOStream(const CMsgOStream &ost):NLMISC::IStream(ost), _List(ost._List)
		{			
		}

		virtual ~CMsgOStream()
		{
		}

		virtual void serial(uint8 &b) throw(NLMISC::EStream);
		virtual void serial(sint8 &b) throw(NLMISC::EStream);
		virtual void serial(uint16 &b) throw(NLMISC::EStream);
		virtual void serial(sint16 &b) throw(NLMISC::EStream);
		virtual void serial(uint32 &b) throw(NLMISC::EStream);
		virtual void serial(sint32 &b) throw(NLMISC::EStream);
		virtual void serial(uint64 &b) throw(NLMISC::EStream);
		virtual void serial(sint64 &b) throw(NLMISC::EStream);
		virtual void serial(float &b) throw(NLMISC::EStream);
		virtual void serial(double &b) throw(NLMISC::EStream);
		virtual void serial(bool &b) throw(NLMISC::EStream);

		virtual void serial(IObjectIA &b) throw(NLMISC::EStream);
		virtual void serial(IObjectIA* &b) throw(NLMISC::EStream);

#ifndef NL_OS_CYGWIN
		virtual void serial(char &b) throw(NLMISC::EStream);
#endif
		virtual void serial(std::string &b) throw(NLMISC::EStream);
		virtual void serial(ucstring &b) throw(NLMISC::EStream);
		virtual void serialBuffer(uint8 *buf, uint len)throw(NLMISC::EReadError)
		{
			throw NLMISC::EReadError((const char *)_List.getType());
		}
		virtual void serialBit(bool &bit) throw(NLMISC::EReadError)
		{
			throw NLMISC::EReadError((const char *)_List.getType());
		}
	};
}

#endif
