/** \file shape_bank_user.h
 * Implementation of the user interface managing shape bank.
 *
 * $Id: shape_bank_user.h,v 1.2 2002/08/21 09:39:54 lecroart Exp $
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

#ifndef NL_SHAPE_BANK_USER_H
#define NL_SHAPE_BANK_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_shape_bank.h"
#include "3d/shape_bank.h"

namespace NL3D 
{

/**
 * Implementation of the user interface managing shape bank
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CShapeBankUser : public UShapeBank
{
public:
	virtual ~CShapeBankUser() {}

	void addShapeCache(const std::string &shapeCacheName);
	void removeShapeCache(const std::string &shapeCacheName);
	void reset();
	void setShapeCacheSize(const std::string &shapeCacheName, sint32 maxSize);
	void linkShapeToShapeCache(const std::string &shapeName, const std::string &shapeCacheName);

	// The real shape bank
	CShapeBank _ShapeBank;
};


} // NL3D


#endif // NL_SHAPE_BANK_USER_H

/* End of shape_bank_user.h */
