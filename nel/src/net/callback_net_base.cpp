/** \file callback_net_base.cpp
 * <File description>
 *
 * $Id: callback_net_base.cpp,v 1.1 2001/02/22 16:18:35 cado Exp $
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

#include "nel/net/callback_net_base.h"


namespace NLNET {


/*
 * Constructor
 */
CCallbackNetBase::CCallbackNetBase()
{
}


/*
 *	Append callback array with the specified array
 */
void CCallbackNetBase::addCallbackArray( const TCallbackItem *callbackarray, TTypeNum arraysize )
{

}


/*
 * Update the network (call this method evenly)
 */
void CCallbackNetBase::update()
{

}


/*
 * Sets callback for detecting a disconnection
 */
void CCallbackNetBase::setDisconnectionCallback( TNetCallback cb )
{

}




} // NLNET
