/* smart_ptr.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: smart_ptr.cpp,v 1.1 2000/10/02 16:58:56 berenguier Exp $
 *
 * CSmartPtr and CRefPtr class.
 */


#include "nel/misc/smart_ptr.h"


namespace NLMISC
{

CRefCount::CPtrInfo		CRefCount::NullPtrInfo('a');

}