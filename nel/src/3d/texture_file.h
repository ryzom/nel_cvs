/** \file texture_file.h
 * <File description>
 *
 * $Id: texture_file.h,v 1.1 2001/06/15 16:24:45 corvazier Exp $
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

#ifndef NL_TEXTURE_FILE_H
#define NL_TEXTURE_FILE_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"


namespace NL3D
{


//****************************************************************************
/**
 * CTextureFile
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextureFile : public ITexture
{
	std::string _FileName;

public:

	/** 
	 * Default constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFile() { }


	/** 
	 * constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFile(std::string s) { touch(); _FileName = s; } 


	/** 
	 * Set the name of the file containing the texture
	 * \param name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setFileName(std::string s) { touch(); _FileName = s; }


	/** 
	 * get the name of the file containing the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	std::string getFileName() const { return _FileName; } 


	/** 
	 * sharing system.
	 * \author Lionel Berenguier
	 * \date 2000
	 */	
	virtual bool			supportSharing() const {return true;}
	virtual std::string		getShareName() const {return getFileName();}


	/** 
	 * Generate the texture, looking in CPath if necessary.
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void doGenerate();

	/// Save the texture file name.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(_FileName);
		if(f.isReading())
			touch();
	}
	NLMISC_DECLARE_CLASS(CTextureFile);

};


} // NL3D


#endif // NL_TEXTURE_FILE_H

/* End of texture_file.h */
