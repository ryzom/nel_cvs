/** \file texture_mem.cpp
 * <File description>
 *
 * $Id: texture_mem.cpp,v 1.5 2002/02/04 10:38:22 vizerie Exp $
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

#include "3d/texture_mem.h"
#include "nel/misc/mem_stream.h"


namespace NL3D 
{


/*==================================================================*\
							CTEXTUREMEM
\*==================================================================*/

/*------------------------------------------------------------------*\
							doGenerate()
\*------------------------------------------------------------------*/
void CTextureMem::doGenerate()
{	
	if (_Data)
	{
		if (_IsFile)
		{
			NLMISC::CMemStream m (true);
			m.fill (_Data, _Length);
			load (m);
		}
		else
		{
			resize(_TexWidth, _TexHeight, NLMISC::CBitmap::RGBA);			
			std::copy(_Data, _Data + _Length, &	getPixels(0)[0]);
			buildMipMaps();
		}
	}
	else
	{
		makeDummy();
	}
}


} // NL3D
