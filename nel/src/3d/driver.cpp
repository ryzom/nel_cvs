/** \file driver.cpp
 * Generic driver.
 * Low level HW classes : ITexture, Cmaterial, CVertexBuffer, CIndexBuffer, IDriver
 *
 * $Id: driver.cpp,v 1.84 2004/04/08 09:05:45 corvazier Exp $
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

#include <string>

#include "nel/misc/types_nl.h"
#include "3d/driver.h"
#include "3d/shader.h"
#include "3d/vertex_buffer.h"
#include "nel/misc/algo.h"

//#include <stdio.h>

using namespace NLMISC;
using namespace std;


namespace NL3D
{

// ***************************************************************************
const uint32 IDriver::InterfaceVersion = 0x51;

// ***************************************************************************
IDriver::IDriver() : _SyncTexDrvInfos( "IDriver::_SyncTexDrvInfos" )
{
	_PolygonMode= Filled;
	_StaticMemoryToVRAM=false;
	_ResetCounter=0;
}

// ***************************************************************************
IDriver::~IDriver()
{
	// Must clean up everything before closing driver.
	// Must doing this in release(), so assert here if not done...
	{		
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
		nlassert( rTexDrvInfos.size() == 0 );
	}

	nlassert(_TexDrvShares.size()==0);
	nlassert(_MatDrvInfos.size()==0);
	nlassert(_VBDrvInfos.size()==0);
	nlassert(_IBDrvInfos.size()==0);
	nlassert(_VtxPrgDrvInfos.size()==0);
}


// ***************************************************************************
bool		IDriver::release(void)
{
	// Called by derived classes.

	// DO THIS FIRST => to auto kill real textures (by smartptr).
	// First, Because must not kill a pointer owned by a CSmartPtr.
	// Release Textures drv.
	ItTexDrvSharePtrList		ittex;
	while( (ittex = _TexDrvShares.begin()) !=_TexDrvShares.end() )
	{
		// NB: at CTextureDrvShare deletion, this->_TexDrvShares is updated (entry deleted);
		delete *ittex;
	}


	// Release refptr of TextureDrvInfos. Should be all null (because of precedent pass).
	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

		// must be empty, because precedent pass should have deleted all.
		nlassert(rTexDrvInfos.empty());
	}

	// Release material drv.
	ItMatDrvInfoPtrList		itmat;
	while( (itmat = _MatDrvInfos.begin()) != _MatDrvInfos.end() )
	{
		// NB: at IShader deletion, this->_MatDrvInfos is updated (entry deleted);
		delete *itmat;
	}

	// Release Shader drv.
	ItShaderDrvInfoPtrList		itshd;
	while( (itshd = _ShaderDrvInfos.begin()) != _ShaderDrvInfos.end() )
	{
		// NB: at IShader deletion, this->_MatDrvInfos is updated (entry deleted);
		delete *itshd;
	}

	// Release VBs drv.
	ItVBDrvInfoPtrList		itvb;
	while( (itvb = _VBDrvInfos.begin()) != _VBDrvInfos.end() )
	{
		// NB: at IVBDrvInfo deletion, this->_VBDrvInfos is updated (entry deleted);
		delete *itvb;
	}

	// Release IBs drv.
	ItIBDrvInfoPtrList		itib;
	while( (itib = _IBDrvInfos.begin()) != _IBDrvInfos.end() )
	{
		// NB: at IIBDrvInfo deletion, this->_IBDrvInfos is updated (entry deleted);
		delete *itib;
	}

	// Release VtxPrg drv.
	ItVtxPrgDrvInfoPtrList		itVtxPrg;
	while( (itVtxPrg = _VtxPrgDrvInfos.begin()) != _VtxPrgDrvInfos.end() )
	{
		// NB: at IVertexProgramDrvInfos deletion, this->_VtxPrgDrvInfos is updated (entry deleted);
		delete *itVtxPrg;
	}

	return true;
}


// ***************************************************************************
GfxMode::GfxMode(uint16 w, uint16 h, uint8 d, bool windowed, bool offscreen, uint frequency)
{
	Windowed= windowed;
	Width= w;
	Height= h;
	Depth= d;
	OffScreen= offscreen;
	Frequency= frequency;
}

// ***************************************************************************
IDriver::TMessageBoxId IDriver::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, IDriver::TMessageBoxIcon icon)
{
	static const char* icons[iconCount]=
	{
		"",
		"WAIT:\n",
		"QUESTION:\n",
		"HEY!\n",
		"",
		"WARNING!\n",
		"ERROR!\n",
		"INFORMATION:\n",
		"STOP:\n"
	};
	static const char* messages[typeCount]=
	{
		"Press any key...",
		"(O)k or (C)ancel ?",
		"(Y)es or (N)o ?",
		"(A)bort (R)etry (I)gnore ?",
		"(Y)es (N)o (C)ancel ?",
		"(R)etry (C)ancel ?"
	};
	printf ("%s%s\n%s", icons[icon], title, message);
	while (1)
	{
		printf ("\n%s", messages[type]);
		int c=getchar();
		if (type==okType)
			return okId;
		switch (c)
		{
		case 'O':
		case 'o':
			if ((type==okType)||(type==okCancelType))
				return okId;
			break;
		case 'C':
		case 'c':
			if ((type==yesNoCancelType)||(type==okCancelType)||(type==retryCancelType))
				return cancelId;
			break;
		case 'Y':
		case 'y':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return yesId;
			break;
		case 'N':
		case 'n':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return noId;
			break;
		case 'A':
		case 'a':
			if (type==abortRetryIgnoreType)
				return abortId;
			break;
		case 'R':
		case 'r':
			if (type==abortRetryIgnoreType)
				return retryId;
			break;
		case 'I':
		case 'i':
			if (type==abortRetryIgnoreType)
				return ignoreId;
			break;
		}
	}
	nlassert (0);		// no!
	return okId;
}




// ***************************************************************************
void			IDriver::removeVBDrvInfoPtr(ItVBDrvInfoPtrList  vbDrvInfoIt)
{
	_VBDrvInfos.erase(vbDrvInfoIt);
}
// ***************************************************************************
void			IDriver::removeIBDrvInfoPtr(ItIBDrvInfoPtrList  ibDrvInfoIt)
{
	_IBDrvInfos.erase(ibDrvInfoIt);
}
// ***************************************************************************
void			IDriver::removeTextureDrvInfoPtr(ItTexDrvInfoPtrMap texDrvInfoIt)
{
	CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
	TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

	rTexDrvInfos.erase(texDrvInfoIt);
}
// ***************************************************************************
void			IDriver::removeTextureDrvSharePtr(ItTexDrvSharePtrList texDrvShareIt)
{
	_TexDrvShares.erase(texDrvShareIt);
}
// ***************************************************************************
void			IDriver::removeMatDrvInfoPtr(ItMatDrvInfoPtrList shaderIt)
{
	_MatDrvInfos.erase(shaderIt);
}
// ***************************************************************************
void			IDriver::removeShaderDrvInfoPtr(ItShaderDrvInfoPtrList shaderIt)
{
	_ShaderDrvInfos.erase(shaderIt);
}
// ***************************************************************************
void			IDriver::removeVtxPrgDrvInfoPtr(ItVtxPrgDrvInfoPtrList vtxPrgDrvInfoIt)
{
	_VtxPrgDrvInfos.erase(vtxPrgDrvInfoIt);
}

// ***************************************************************************
bool			IDriver::invalidateShareTexture (ITexture &texture)
{
	// Create the shared Name.
	std::string	name;
	getTextureShareName (texture, name);

	// Look for the driver info for this share name
	CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
	TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
	TTexDrvInfoPtrMap::iterator iteDrvInfo = rTexDrvInfos.find (name);
	if (iteDrvInfo != rTexDrvInfos.end())
	{
		// Now parse all shared info
		TTexDrvSharePtrList::iterator shareIte = _TexDrvShares.begin ();
		while (shareIte != _TexDrvShares.end ())
		{
			// Good one ?
			if ((*shareIte)->DrvTexture == iteDrvInfo->second)
			{
				// Remove this one
				TTexDrvSharePtrList::iterator toRemove = shareIte;
				shareIte++;
				delete (*toRemove);
			}
			else 
				shareIte++;
		}

		// Ok
		return true;
	}
	return false;
}
// ***************************************************************************
void			IDriver::getTextureShareName (const ITexture& tex, string &output)
{
	// Create the shared Name.
	output= strlwr(tex.getShareName());

	// append format Id of the texture.
	static char	fmt[256];
	smprintf(fmt, 256, "@Fmt:%d", (uint32)tex.getUploadFormat());
	output+= fmt;

	// append mipmap info
	if(tex.mipMapOn())
		output+= "@MMp:On";
	else
		output+= "@MMp:Off";
}

// ***************************************************************************

void			IDriver::setStaticMemoryToVRAM (bool staticMemoryToVRAM)
{ 
	_StaticMemoryToVRAM=staticMemoryToVRAM; 
}

// ***************************************************************************
class CTextureDebugInfo 
{
public:
	uint	MemoryCost;
	string	Line;
	
	bool	operator<(const CTextureDebugInfo &o) const {return Line<o.Line;}
};

// ***************************************************************************
void IDriver::profileTextureUsage(std::vector<std::string> &result)
{
	std::set<ITextureDrvInfos	*>		texSet;
	uint	i;
	
	// reserve result, sort by UploadFormat
	vector<CTextureDebugInfo>	tempInfo[ITexture::UploadFormatCount];
	for(i=0;i<ITexture::UploadFormatCount;i++)
		tempInfo[i].reserve(_TexDrvShares.size());
	
	// Parse all the DrvShare list
	uint	totalSize= 0;
	ItTexDrvSharePtrList	it= _TexDrvShares.begin();
	for(;it!=_TexDrvShares.end();it++)
	{
		// get TexDrvInfos and owner
		ITextureDrvInfos	*gltext= (ITextureDrvInfos*)(ITextureDrvInfos*)(*it)->DrvTexture;
		ITexture			*text= (*it)->getOwnerTexture();
		nlassert(gltext && text);
		
		// sort by upload format
		uint	upFmt= (uint32)text->getUploadFormat();
		nlassert(upFmt<ITexture::UploadFormatCount);

		// get the shareName
		string	shareName;
		if(text->supportSharing())
			shareName= strlwr(text->getShareName());
		else
			shareName= "Not Shared";
		
		// only if not already append to the set
		if(texSet.insert(gltext).second)
		{
			uint	memCost= gltext->getTextureMemoryUsed();
			totalSize+= memCost;
			string	typeStr= typeid(*text).name();
			strFindReplace(typeStr, "class NL3D::", string());
			tempInfo[upFmt].push_back(CTextureDebugInfo());
			tempInfo[upFmt].back().Line= toString("Type: %15s. ShareName: %s. Size: %d Ko", 
				typeStr.c_str(),
				shareName.c_str(),
				memCost/1024);
			tempInfo[upFmt].back().MemoryCost= memCost;
		}
	}
	
	// For convenience, sort
	for(i=0;i<ITexture::UploadFormatCount;i++)
		sort(tempInfo[i].begin(), tempInfo[i].end());
	
	// Store into result, appending Tag for each Mo reached. +10* is for extra lines and security
	result.clear();
	result.reserve(texSet.size() + 10*ITexture::UploadFormatCount + totalSize/(1024*1024));

	// copy and add tags
	for(i=0;i<ITexture::UploadFormatCount;i++)
	{
		switch(i)
		{
		case	ITexture::Auto: result.push_back("**** Format: Auto ****"); break;
		case	ITexture::RGBA8888: result.push_back("**** Format: RGBA8888 ****"); break;
		case	ITexture::RGBA4444: result.push_back("**** Format: RGBA4444 ****"); break;
		case	ITexture::RGBA5551: result.push_back("**** Format: RGBA5551 ****"); break;
		case	ITexture::RGB888: result.push_back("**** Format: RGB888 ****"); break;
		case	ITexture::RGB565: result.push_back("**** Format: RGB565 ****"); break;
		case	ITexture::DXTC1: result.push_back("**** Format: DXTC1 ****"); break;
		case	ITexture::DXTC1Alpha: result.push_back("**** Format: DXTC1Alpha ****"); break;
		case	ITexture::DXTC3: result.push_back("**** Format: DXTC3 ****"); break;
		case	ITexture::DXTC5: result.push_back("**** Format: DXTC5 ****"); break;
		case	ITexture::Luminance: result.push_back("**** Format: Luminance ****"); break;
		case	ITexture::Alpha: result.push_back("**** Format: Alpha ****"); break;
		case	ITexture::AlphaLuminance: result.push_back("**** Format: AlphaLuminance ****"); break;
		case	ITexture::DsDt: result.push_back("**** Format: DsDt ****"); break;
		default: result.push_back(toString("**** Format??: %d ****", i)); break;
		}
		
		// display stats for this format
		uint	tagTotal= 0;
		uint	curTotal= 0;
		for(uint j=0;j<tempInfo[i].size();j++)
		{
			result.push_back(tempInfo[i][j].Line);
			tagTotal+= tempInfo[i][j].MemoryCost;
			curTotal+= tempInfo[i][j].MemoryCost;
			if(tagTotal>=1024*1024)
			{
				result.push_back(toString("---- %.1f Mo", float(curTotal)/(1024*1024)));
				tagTotal= 0;
			}
		}
		// append last line?
		if(tagTotal!=0)
			result.push_back(toString("---- %.1f Mo", float(curTotal)/(1024*1024)));
	}
	
	// append the total
	result.push_back(toString("**** Total ****"));
	result.push_back(toString("Total: %d Ko", totalSize/1024));
}

// ***************************************************************************

}
