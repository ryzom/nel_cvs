/* vectord.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: vectord.cpp,v 1.1 2000/09/14 16:29:57 berenguier Exp $
 *
 * CVectorD class
 */


#include	"nel/misc/vectord.h"


namespace	NLMISC
{

const CVectorD	CVectorD::Null(0,0,0);
const CVectorD	CVectorD::I(1,0,0);
const CVectorD	CVectorD::J(0,1,0);
const CVectorD	CVectorD::K(0,0,1);

}