/** \file msg_group.h
 * class for define a group of message (a group of message is same as a performative.
 *
 * $Id: msg_group.h,v 1.3 2001/01/17 10:32:29 chafik Exp $
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
#ifndef NL_IA_MSG_GROUP_H
#define NL_IA_MSG_GROUP_H

namespace NLAIAGENT
{

	class IBasicMessageGroup : public NLAIC::IBasicInterface {

		protected:			
		public:

			IBasicMessageGroup()
			{
			}			
			
			virtual bool operator==(const IBasicMessageGroup *grp) const = 0;
			virtual bool operator==(const IBasicMessageGroup &grp) const = 0;
			
			virtual const sint32 &getId() const = 0;
	};

	class  CMessageGroup : public IBasicMessageGroup 
	{
	public:
		static CMessageGroup systemGroup;
		static CMessageGroup msgScriptingGroup;
	
	private:
		sint32 _Id;

	public:
		static const NLAIC::CIdentType IdMessageGroup;
	public:

		CMessageGroup(sint32 id = 0)
		{
			_Id = id;
		}

		CMessageGroup(const CMessageGroup &c)
		{
			_Id = c._Id;
		}

		virtual void save(NLMISC::IStream &os)
		{
			sint32 id = (sint32) _Id;
			os.serial( id );
		}

		virtual void load(NLMISC::IStream &is) 
		{
			sint32 id;
			is.serial( id );
			_Id = (sint32) id;
		}
		
		virtual const NLAIC::IBasicType *clone() const 
		{
			NLAIC::IBasicType *x = new CMessageGroup( *this );
			return x;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{
			NLAIC::IBasicType *x = new CMessageGroup( *this );
			return x;
		}

		virtual bool operator==(const IBasicMessageGroup *grp) const
		{
			return ( this->_Id == grp->getId() );
		}

		virtual bool operator==(const IBasicMessageGroup &grp) const
		{
			return ( this->_Id == grp.getId());
		}

		virtual void getDebugString(char *t) const
		{
			sprintf(t,"CMessageGroup<%d>",_Id);
		}

		virtual const NLAIC::CIdentType &getType() const;		

		virtual const sint32 &getId() const
		{
			return _Id;
		}
	};
}
#endif
