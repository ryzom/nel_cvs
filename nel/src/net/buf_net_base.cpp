/** \file buf_net_base.cpp
 * Network engine, layer 1, base
 *
 * $Id: buf_net_base.cpp,v 1.11.8.2 2003/08/20 10:30:16 cado Exp $
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

uint32 	NbNetworkTask = 0;

/***************************************************************************************************
 * User main thread 
 **************************************************************************************************/

 
/*
 * Constructor
 */
CBufNetBase::CBufNetBase() :
	_RecvFifo("CBufNetBase::_RecvFifo"),
	_DataAvailable( false ),
	_DisconnectionCallback( NULL ),
	_DisconnectionCbArg( NULL ),
	_MaxExpectedBlockSize( 10485760 ), // 10M
	_MaxSentBlockSize( 10485760 )
{
	// Debug info for mutexes
#ifdef MUTEX_DEBUG
	initAcquireTimeMap();
#endif
}

NLMISC_VARIABLE(uint32, NbNetworkTask, "Number of server and client thread");
	
} // NLNET
