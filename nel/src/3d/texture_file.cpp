/** \file texture_file.cpp
 * <File description>
 *
 * $Id: texture_file.cpp,v 1.19 2002/11/13 17:53:21 berenguier Exp $
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

#include "std3d.h"

#include "3d/texture_file.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
using namespace std;
using namespace NLMISC;


namespace NL3D
{


///==================================================================
void CTextureFile::buildBitmapFromFile(NLMISC::CBitmap &dest, const std::string &fileName, bool asyncload, uint8 mipMapSkip)
{
	NLMISC::CIFile f;
	//nldebug(_FileName.c_str());
	try
	{
		string file = CPath::lookup(fileName);
		f.setAsyncLoading (asyncload);
		f.setCacheFileOnOpen (asyncload);

		// if mipmap skip, must not cache, because don't have to load all!!
		if(asyncload && mipMapSkip>0)
		{
			f.setCacheFileOnOpen (false);
			f.allowBNPCacheFileOnOpen(false);
		}

		// Load bitmap.
		if (f.open(file))
		{
			// skip DDS mipmap if wanted
			dest.load (f, mipMapSkip);
		}
		else throw EPathNotFound(fileName);

		// *** Need usercolor computing ?

		// Texture not compressed ?
		if (dest.PixelFormat == RGBA)
		{
			// Make a filename
			string path = CFile::getFilename(fileName);
			string ext = strrchr (fileName.c_str(), '.');
			path.resize (path.size () - ext.size());
			path += "_usercolor" + ext;

			// Loopup the texture
			string file2 = CPath::lookup( path, false, false);
			if (!file2.empty())
			{
				// The file2 exist, load and compute it
				CBitmap bitmap;
				bitmap.loadGrayscaleAsAlpha (true);

				// Open and read the file2
				NLMISC::CIFile f2;
				f2.setAsyncLoading (asyncload);
				f2.setCacheFileOnOpen (asyncload); // Same as async loading
				if (f2.open(file2))
				{
					bitmap.load(f2);
				}
				else throw EPathNotFound(file2);

				// Texture are the same size ?
				if ((dest.getWidth() == bitmap.getWidth()) && (dest.getHeight() == bitmap.getHeight()))
				{
					// Convert in Alpha
					if (bitmap.convertToType (CBitmap::Alpha))
					{
						// Compute it
						uint8 *userColor = (uint8 *)&(bitmap.getPixels ()[0]);
						CRGBA *color = (CRGBA *)&(dest.getPixels ()[0]);

						// For each pixel
						uint pixelCount = dest.getWidth()*dest.getHeight();
						uint pixel;
						for (pixel = 0; pixel<pixelCount; pixel++)
						{
							if (userColor[pixel]==0) 
							{
								// New code: use new restrictions from IDriver.
								float	Rt, Gt, Bt, At;
								float	Lt;
								float	Rtm, Gtm, Btm, Atm;

								// read 0-1 RGB pixel.
								Rt= (float)color[pixel].R/255;
								Gt= (float)color[pixel].G/255;
								Bt= (float)color[pixel].B/255;
								Lt= Rt*0.3f + Gt*0.56f + Bt*0.14f;

								// take Alpha from userColor src.
								At= (float)userColor[pixel]/255;
								Atm= 1-Lt*(1-At);

								// If normal case.
								if(Atm>0)
								{
									Rtm= Rt*At / Atm;
									Gtm= Gt*At / Atm;
									Btm= Bt*At / Atm;
								}
								// Else special case: At==0, and Lt==1.
								else
								{
									Rtm= Gtm= Btm= 0;
								}

								// copy to buffer.
								sint	r,g,b,a;
								r= (sint)(Rtm*255+0.5f);
								g= (sint)(Gtm*255+0.5f);
								b= (sint)(Btm*255+0.5f);
								a= (sint)(Atm*255+0.5f);
								clamp(r, 0,255);
								clamp(g, 0,255);
								clamp(b, 0,255);
								clamp(a, 0,255);
								color[pixel].R = (uint8)r;
								color[pixel].G = (uint8)g;
								color[pixel].B = (uint8)b;
								color[pixel].A = (uint8)a;
							}
						}
					}
					else
					{
						nlinfo ("Can't convert the usercolor texture %s in alpha mode", file2.c_str());
					}
				}
				else
				{
					// Error
					nlinfo ("User color texture is not the same size than the texture. (Tex : %s, Usercolor : %s)", file.c_str(), file2.c_str());
				}
			}
		}
	}
	catch(EPathNotFound &e)
	{
		// Not found...
		dest.makeDummy();
		nlwarning("Missing textureFile: %s (%s)", fileName.c_str(), e.what());
	}
}


/*==================================================================*\
							CTEXTUREFILE
\*==================================================================*/

/*------------------------------------------------------------------*\
							doGenerate()
\*------------------------------------------------------------------*/
void CTextureFile::doGenerate()
{
	buildBitmapFromFile(*this, _FileName, _AsyncLoading, _MipMapSkipAtLoad);
}

// ***************************************************************************
void CTextureFile::setAsyncLoading (bool isAsync)
{
	_AsyncLoading = isAsync;
}


// ***************************************************************************
void	CTextureFile::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 1:
		- AllowDegradation.
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(1);

	// serial the base part of ITexture.
	ITexture::serial(f);

	f.serial(_FileName);
	if(ver>=1)
		f.serial(_AllowDegradation);
	else if(f.isReading())
		_AllowDegradation= true;

	if(f.isReading())
		touch();
}


// ***************************************************************************
void	CTextureFile::setAllowDegradation(bool allow)
{
	_AllowDegradation= allow;
}

// ***************************************************************************
CTextureFile::CTextureFile(const CTextureFile &other) : ITexture(other)
{
	dupInfo(other);
}

// ***************************************************************************
CTextureFile &CTextureFile::operator = (const CTextureFile &other)
{
	// copy base infos
	(ITexture &) *this = (ITexture &) other;
	dupInfo(other);
	return *this;
}

// ***************************************************************************
void CTextureFile::dupInfo(const CTextureFile &other)
{
	_FileName         = other._FileName;
	_AsyncLoading     =	other._AsyncLoading;
	_AllowDegradation = other._AllowDegradation;
	_SupportSharing	  = other._SupportSharing;
	_MipMapSkipAtLoad = other._MipMapSkipAtLoad; 
}


// ***************************************************************************
void			CTextureFile::enableSharing(bool enable)
{
	_SupportSharing = enable;
}

// ***************************************************************************
void			CTextureFile::setMipMapSkipAtLoad(uint8 level)
{
	_MipMapSkipAtLoad= level;
}

// ***************************************************************************
std::string		CTextureFile::getShareName() const
{
	string	ret= _FileName;
	strlwr(ret);
	return ret;
}


} // NL3D
