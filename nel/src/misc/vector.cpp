/* vector.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: vector.cpp,v 1.1 2000/09/13 17:58:56 berenguier Exp $
 *
 * CVector class
 */


#include	"nel/misc/vector.h"


namespace	NLMISC
{

const CVector	CVector::Null(0,0,0);
const CVector	CVector::I(1,0,0);
const CVector	CVector::J(0,1,0);
const CVector	CVector::K(0,0,1);

}