/** \file interface.h
 * 
 *
 * $Id: interface.h,v 1.1 2001/07/17 13:57:48 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>

#include <nel/misc/rgba.h>

void	initInterface();
void	updateInterface();
void	releaseInterface();

// prompt -> 0=normal, 1=with_star 2=no_prompt
void	askString (const std::string &queryString, const std::string &defaultString="", sint prompt=0, const NLMISC::CRGBA &color=NLMISC::CRGBA(0,0,64,128));
bool	haveAnswer (std::string &answer);
bool	interfaceOpen ();

#endif // INTERFACE_H

/* End of interface.h */
