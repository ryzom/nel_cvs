/** \file buf_net_base.cpp
 * Network engine, layer 1, base
 *
 * $Id: buf_net_base.cpp,v 1.7 2002/08/22 16:09:32 cado Exp $
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

#include "stdnet.h"

#include "nel/net/buf_net_base.h"

using namespace NLMISC;
using namespace std;


namespace NLNET {


/***************************************************************************************************
 * User main thread 
 **************************************************************************************************/

 
/*
 * Constructor
 */
CBufNetBase::CBufNetBase() :
	_RecvFifo("CBufNetBase::_RecvFifo"),
	_DisconnectionCallback( NULL ),
	_DisconnectionCbArg( NULL ),
	_MaxExpectedBlockSize( 0x7FFFFFF ),
	_MaxSentBlockSize( 0x7FFFFFF ),
	_RecvFifo("CBufNetBase::_RecvFifo"),
	_DataAvailable( false )
{
	// Debug info for mutexes
#ifdef MUTEX_DEBUG
	initAcquireTimeMap();
#endif
}


} // NLNET
