/** \file texture_mem.cpp
 * <File description>
 *
 * $Id: texture_mem.cpp,v 1.6 2002/02/21 17:38:16 vizerie Exp $
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

#include <memory>


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


static NLMISC::CRGBA WhitePix(255, 255, 255, 255); // the texture datas ... :)

///===========================================================================
ITexture *CTextureMem::Create1x1WhiteTex()
{
	std::auto_ptr<CTextureMem> tex(new CTextureMem((uint8 *) &WhitePix,
												   sizeof(WhitePix),
												   false, /* dont delete */
												   false, /* not a file */
												   1, 1)
								  );
	tex->setShareName("#WhitePix1x1");	
	return tex.release();
}


} // NL3D
