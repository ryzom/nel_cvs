/** \file agentexport.h
 *
 * $Id: agentexport.h,v 1.1 2001/01/05 10:50:22 chafik Exp $
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
#ifndef NL_AGENTEXPORT_H
#define NL_AGENTEXPORT_H


#ifdef NL_OS_WINDOWS

#pragma warning (disable: 4666)
#pragma warning (disable: 4666)
#pragma warning (disable: 4786) 
#pragma warning (disable: 4620) 
#pragma warning (disable: 4275)
#pragma warning (disable: 4251)
#pragma warning (disable: 4621)
#pragma warning (disable: 4514)

#endif

#include <list>
#include <map>
#include <string>
#include <istream.h>
#include <iostream.h>
#include <fstream.h>
#include "e/ia_exception.h" 
#include "c/abstract_interface.h"
#include "c/registry_type.h"
#include "agent/mailbox.h"
#endif
