/** \file stddirect3d.h
 * Direct 3d driver implementation
 *
 * $Id: stddirect3d.h,v 1.1 2004/03/19 10:11:36 corvazier Exp $
 */

/* Copyright, 2003 Nevrax Ltd.
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

// Default NeL includes
#include "nel/misc/types_nl.h"

// System includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <exception>
#include <utility>
#include <deque>

// Directx includes
#include <d3d9.h>
#include <d3dx9math.h>

#undef min
#undef max

// NeL includes
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"
