/** \file georges/common.h
 * Georges system files
 *
 * $Id: common.h,v 1.2 2002/02/20 18:05:28 lecroart Exp $
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

#ifndef NLGEORGES_COMMON_H
#define NLGEORGES_COMMON_H

namespace NLGEORGES
{

#define ITEM_HAVEPARENT 1
#define ITEM_HAVECURRENT 2
#define ITEM_ISATOM 4
#define ITEM_ISSTRUCT 8
#define ITEM_ISLIST 16
#define ITEM_ISENUM 32
#define ITEM_ISLISTCHILD 64

#define RESERVEDKEYWORD_PARENTS "Parents"
#define RESERVEDKEYWORD_COMMENTS "Comments"

}

#endif // NLGEORGES_COMMON_H
