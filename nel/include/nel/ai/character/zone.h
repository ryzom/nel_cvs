/** \file zone.h
 *
 * $Id: zone.h,v 1.2 2001/03/23 09:58:05 chafik Exp $
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

#ifndef NL_ZONE_H
#define NL_ZONE_H
#include "nel/ai/character/character.h"

namespace NLAICHARACTER
{		
	class IZone: public ICharacter
	{
	private:
		ICharacter *_Character;
		NLAIAGENT::IBaseGroupType *_Attachment;

	public:
		IZone(const IZone &);
		IZone(ICharacter *);
		///Built a new IZone with an CCharacterChild Character.
		IZone(const std::string &name);
		virtual ~IZone();

		virtual const IZone *isInZone(const IZone &) const = 0
		{
			throw;
		}		

		virtual const std::string &getName() const
		{			
			return _Character->getName();
		}
		virtual const ICharacter *haveCharacter(const std::string &s) const
		{
			return _Character->haveCharacter(s);
		}		
		virtual bool addCharacter(const ICharacter &c)
		{
			return _Character->addCharacter(c);
		}

		virtual bool canAddCharacter() const
		{
			return _Character->canAddCharacter();
		}

		virtual void addObject(const NLAIAGENT::IObjectIA *);
		virtual void removeObject(const NLAIAGENT::IObjectIA *);
		virtual NLAIAGENT::CIteratorContener getAttachement() const;
	};	
	
}
#endif
