/** \file zone.cpp
 *
 * $Id: zone.cpp,v 1.3 2001/03/26 11:28:18 chafik Exp $
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

#include "nel/ai/character/zone.h"

namespace NLAICHARACTER
{		
	IZone::IZone(const IZone &c):ICharacter(c),_Character((ICharacter *)c._Character->clone()),_Attachment((NLAIAGENT::IBaseGroupType *)c._Attachment->clone())
	{
	}

	IZone::IZone(ICharacter *c):_Character(c),_Attachment(new NLAIAGENT::CGroupType())
	{
	}

	IZone::IZone(const std::string &name):_Character(new CCharacterChild(name)),_Attachment(new NLAIAGENT::CGroupType())
	{
	}

	IZone::~IZone()
	{
		_Character->release();
		_Attachment->release();
	}

	void IZone::addObject(const NLAIAGENT::IObjectIA *o)
	{
		_Attachment->push(o);
	}

	void IZone::removeObject(const NLAIAGENT::IObjectIA *o)
	{
		_Attachment->erase(o);
	}

	NLAIAGENT::CIteratorContener IZone::getAttachement() const
	{
		return _Attachment->getIterator();
	}

	const IZone *IZone::isInZone(const IZone &) const
	{
		throw;
	}
}
