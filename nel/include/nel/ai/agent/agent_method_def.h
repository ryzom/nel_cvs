/** \file agent_method_def.h
 * Sevral define name method manipulation.
 *
 * $Id: agent_method_def.h,v 1.20 2002/05/17 13:46:24 chafik Exp $
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
#define _SEND_			"Send"
///define the getChild method name
#define _GETCHILD_		"GetChild"
///define the addChild method name
#define _ADDCHILD_		"AddChild"
///define the addChild method name
#define _REMOVECHILD_	"RemoveChild"
///define the self mail boxe name
#define _MAILER_		"GetMailer"
#define _GETNAME_		"GetName"

#define	_RUNTEL_		"RunTell"
#define	_RUNASK_		"RunAsk"
#define _RUNACHIEVE_	"RunAchieve"

#define _SETSTATIC_		"SetStatic"

#define	_GETNUMID_		"GetNumId"

#define	_ATTACH_		"Attach"
#define	_SETCLOCK_		"SetClock"
#define	_GETCLOCK_		"GetClock"
#define	_GETTIMER_		"GetTimer"
#define	_ADDATTRIB_		"AddAttrib"

#define	_CONSTRUCTOR_	"Constructor"
#define _DESTRUCTOR_	"Destructor"
#define _RUN_			"Run"
#define _SELF_			"Self"
#define _OPPLUS_		"operator+"
#define _OPMOINS_		"operator-"
#define _OPMUL_			"operator*"
#define _OPDIV_			"operator/"
#define _OPEQ_			"operator=="
#define _OPLESS_		"operator!"
#define _OPINFEQ_		"operator<="
#define _OPSUPEQ_		"operator>="
#define _OPDIFF_		"operator!="
#define _ADDSUBSET_		"AddSubset"
#define _UNFUZIFY_		"Unfuzify"
#define _FATHER_		"Father"
#define _SENDER_		"Sender"
#define _RECEIVER_		"Receiver"
#define _CONTINUATION_	"Continuation"
#define _SETCONTINUATION_	"SetContinuation"
#define _FIRST_			"First"
#define _SECOND_		"Second"
#define _ISA_			"IsA"
#define _ADD_SET_		"AddSet"
#endif
