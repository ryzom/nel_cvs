/** \file agent_method_def.h
 * Sevral define name method manipulation.
 *
 * $Id: agent_method_def.h,v 1.5 2001/01/18 15:04:46 portier Exp $
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

#ifndef NL_AGENT_METHOD_DEF_H
#define NL_AGENT_METHOD_DEF_H


///define the send method name
#define _SEND_ "Send"
///define the getChild method name
#define _GETCHILD_ "GetChild"
///define the addChild method name
#define _ADDCHILD_ "AddChild"

#define	_CONSTRUCTOR_	"Constructor"
#define _DESTRUCTOR_	"Destructor"
#define _RUN_			"Run"
#define _SEND_			"Send"
#define _OPPLUS_		"operator+"
#define _OPMOINS_		"operator-"
#define _OPMUL_			"operator*"
#define _OPDIV_			"operator/"
#define _OPEQ_			"operator=="
#define _OPLESS_		"operator!"
#define _OPINFEQ_		"operator<="
#define _OPSUPEQ_		"operator>="
#define _OPDIFF_		"operator!="
#define _ADDSUBSET_		"addSubset"
#define _UNFUZIFY_		"unfuzify"
#define _FATHER_		"father"
#endif