/** \file email.h
 * send email
 *
 * $Id: alarms.h,v 1.1 2003/01/08 18:06:40 lecroart Exp $
 */

/* Copyright, 2003 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_ALARMS_H
#define NL_ALARMS_H

#include <string>
#include <vector>


namespace NLNET {


struct CAlarm
{
	CAlarm (std::string &n, sint l, bool gt) : Name(n), Limit(l), GT(gt), Activated(false) { }
	
	std::string Name;		// variable name
	int	 Limit;				// limit value where the alarm is setted
	bool GT;				// true if the error is produce when var is greater than bound
	
	bool Activated;			// true if the limit is exceeded (mail is send everytimes the actived bool change from false to true)
};

extern std::vector<CAlarm> Alarms;
	
void initAlarms ();
void updateAlarms ();

void setAlarms (const std::vector<std::string> &alarms);

} // NLNET


#endif // NL_ALARMS_H

/* End of email.h */
