/** \file panoply_maker.cpp
 * Panoply maker
 *
 * $Id: panoply_maker.cpp,v 1.7 2002/05/31 08:53:01 vizerie Exp $
 */

/* Copyright, 2000, 2001, 2002 Nevrax Ltd.
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


#include "color_modifier.h"
#include "color_mask.h"


#include <nel/misc/types_nl.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>



// ========================================================================================================
// This tool is for creating various colored texture from a base texture.
// Parts of a base texture can have hue, contrast, luminosity shifting etc.
// Each part is defined by a mask. The red component of it is considered as an alpha value (not the alpha, because it is more fast to create a grey texture with photoshop..)
// The result is serialized in tga files.
// ========================================================================================================
// why this tool ? : it is useful to create various colored cloth and skin textures
// Not all hardware allow it to manage that at runtime (lack for palettized textures or pixel shaders...)
//=========================================================================================================


/// describes the building infos
struct CBuildInfo
{
	std::string					 InputPath;
	std::string					 OutputPath;
	std::vector<std::string>     BitmapExtensions; // the supported extension for bitmaps
	std::string					 DefaultSeparator;
	TColorMaskVect				 ColorMasks;
};



/** Build the infos we need from a config file
  * It build a list of masks infos
  */
static void BuildMasksFromConfigFile(NLMISC::CConfigFile &cf,
									 TColorMaskVect &colorMasks);

/// Build the colored versions
static void BuildColoredVersions(const CBuildInfo &bi);

///
static void BuildColoredVersionForOneBitmap(const CBuildInfo &bi, const std::string &fileNameWithExtension);
											


///=====================================================
int main(int argc, char* argv[])
{	

	if (argc != 2)
	{
		nlinfo("usage : %s [config_file name]", argv[0]);
		exit(-1);
	}

	CBuildInfo bi;	

	/////////////////////////////////////////
	// reads infos from the config files   //
	/////////////////////////////////////////
			
		NLMISC::CConfigFile cf;
		try
		{
			/// load the config file
			cf.load(argv[1]);

			/// colors masks
			BuildMasksFromConfigFile(cf, bi.ColorMasks);

			/// look paths
			try
			{
				NLMISC::CConfigFile::CVar &additionnal_paths = cf.getVar ("additionnal_paths");
				for (uint k = 0; k < (uint) additionnal_paths.size(); ++k)
				{
					NLMISC::CPath::addSearchPath(NLMISC::CPath::standardizePath(additionnal_paths.asString(k)));
				}
			}
			catch (NLMISC::EUnknownVar &)
			{
			}
			
			/// input
			try
			{
				bi.InputPath = NLMISC::CPath::standardizePath(cf.getVar ("input_path").asString());
			}
			catch (NLMISC::EUnknownVar &)
			{
			}

			/// output
			try
			{
				bi.OutputPath = NLMISC::CPath::standardizePath(cf.getVar ("output_path").asString());
			}
			catch (NLMISC::EUnknownVar &)
			{
			}

			/// default ascii character for unused masks
			try
			{
				bi.DefaultSeparator = cf.getVar ("default_separator").asString();								
			}
			catch (NLMISC::EUnknownVar &)
			{
				bi.DefaultSeparator = '_';
			}

			/// extension for bitmaps
			try
			{
				NLMISC::CConfigFile::CVar &bitmap_extensions = cf.getVar ("bitmap_extensions");
				bi.BitmapExtensions.resize(bitmap_extensions.size());
				for (uint k = 0; k < (uint) bitmap_extensions.size(); ++k)
				{
					bi.BitmapExtensions[k] =  "." + bitmap_extensions.asString(k);					
					bi.BitmapExtensions[k] = NLMISC::strupr(bi.BitmapExtensions[k]);
				}				
			}
			catch (NLMISC::EUnknownVar &)
			{
				bi.BitmapExtensions[0].resize(1);
				bi.BitmapExtensions[0] = ".tga";
			}
		}
		catch (std::exception &e)
		{
			nlerror("Panoply building failed.");
			nlerror(e.what());
		}

	////////////////////////////////
	// Build the colored versions //
	////////////////////////////////
	try
	{
		BuildColoredVersions(bi);
	}
	catch (std::exception &e)
	{
		nlinfo("Something went wrong while building bitmap : %s", e.what());
		return -1;
	}
	return 0;
}



///======================================================
static void BuildMasksFromConfigFile(NLMISC::CConfigFile &cf,
									 TColorMaskVect &colorMasks)
									 
{
	/// get a list of the alpha mask extensions	
	NLMISC::CConfigFile::CVar &mask_extensions = cf.getVar ("mask_extensions");
	colorMasks.resize(mask_extensions.size());

	/// For each kind of mask, build a list of the color modifiers
	for (uint k = 0; k < (uint) mask_extensions.size(); ++k)
	{			
		colorMasks[k].MaskExt = mask_extensions.asString(k);
		NLMISC::CConfigFile::CVar &luminosities    = cf.getVar (colorMasks[k].MaskExt + "_luminosities");
		NLMISC::CConfigFile::CVar &contrasts	   = cf.getVar (colorMasks[k].MaskExt + "_constrasts");
		NLMISC::CConfigFile::CVar &hues			   = cf.getVar (colorMasks[k].MaskExt + "_hues");
		NLMISC::CConfigFile::CVar &lightness	   = cf.getVar (colorMasks[k].MaskExt + "_lightness");
		NLMISC::CConfigFile::CVar &saturation	   = cf.getVar (colorMasks[k].MaskExt + "_saturations");
		NLMISC::CConfigFile::CVar &colorIDs		   = cf.getVar (colorMasks[k].MaskExt + "_color_id");


		if (luminosities.size() != contrasts.size()
			|| luminosities.size() != hues.size()	
			|| luminosities.size() != lightness.size()
			|| luminosities.size() != saturation.size()
			|| luminosities.size() != colorIDs.size()
			)

		{
			throw NLMISC::Exception("All color descriptors must have the same number of arguments");
		}
		colorMasks[k].CMs.resize(luminosities.size());
		for (uint l = 0; l < (uint) luminosities.size(); ++l)
		{
			CColorModifier &cm = colorMasks[k].CMs[l];
			cm.Contrast		   = contrasts.asFloat(l);
			cm.Luminosity      = luminosities.asFloat(l);
			cm.Hue		       = hues.asFloat(l);
			cm.Lightness       = lightness.asFloat(l);
			cm.Saturation      = saturation.asFloat(l);

			cm.ColID = colorIDs.asString(l);
		}
	}
}

///======================================================
static void BuildColoredVersions(const CBuildInfo &bi)
{
	std::vector<std::string> tgaFiles;
	if (!NLMISC::CFile::isExists(bi.InputPath))
	{
		nlwarning(("Path not found : " + bi.InputPath).c_str());
		return;
	}
	NLMISC::CPath::getPathContent (bi.InputPath, false, false, true, tgaFiles);
	for (uint k = 0;  k < tgaFiles.size(); ++k)
	{
		for (uint l = 0; l < bi.BitmapExtensions.size(); ++l)
		{
			std::string fileExt = "." + NLMISC::strupr(NLMISC::CFile::getExtension(tgaFiles[k]));						
			if (fileExt == bi.BitmapExtensions[l])
			{
				nlinfo("Processing : %s ", tgaFiles[k].c_str());				
				try
				{
					BuildColoredVersionForOneBitmap(bi,											
													NLMISC::CFile::getFilename(tgaFiles[k])
												   );
				}
				catch (std::exception &e)
				{
					nlinfo("Processing of %s failed : %s \n", tgaFiles[k].c_str(), e.what());					
				}
			}
		}
	}
}


/// used to loop throiugh the process, avoiding unused masks 
struct CLoopInfo
{
	NLMISC::CBitmap		Mask;
	uint        Counter;
	uint        MaskID;
};


///======================================================
static void BuildColoredVersionForOneBitmap(const CBuildInfo &bi, const std::string &fileNameWithExtension)
{	
	uint32 depth;
	NLMISC::CBitmap srcBitmap;
	NLMISC::CBitmap resultBitmap;
	/// load the bitmap
	{
		NLMISC::CIFile is;
		try
		{
			is.open(bi.InputPath + fileNameWithExtension);
			depth = srcBitmap.load(is);
			if (srcBitmap.PixelFormat != NLMISC::CBitmap::RGBA)
			{
				srcBitmap.convertToType(NLMISC::CBitmap::RGBA);
			}
			if (srcBitmap.getPixels().empty())
			{
				throw NLMISC::Exception(std::string("Failed to load bitmap ") + bi.InputPath + fileNameWithExtension);
			}
		}
		catch (NLMISC::Exception &)
		{
			nlinfo("File or format error with : %s. Processing next...", fileNameWithExtension.c_str());
			return;
		}
	}



	static std::vector<CLoopInfo> masks;
	/// check the needed masks
	masks.clear();

	std::string fileName = NLMISC::CFile::getFilenameWithoutExtension(fileNameWithExtension);
	std::string fileExt  = NLMISC::strupr(NLMISC::CFile::getExtension(fileNameWithExtension));

	for (uint k = 0; k < bi.ColorMasks.size(); ++k)
	{
		std::string maskName = fileName + "_" + bi.ColorMasks[k].MaskExt + "." + fileExt;
		std::string maskFileName = NLMISC::CPath::lookup(maskName,
														 false, false);
		if (!maskFileName.empty()) // found the mask ?
		{
			CLoopInfo li;
			li.Counter = 0;
			li.MaskID = k;

			/// try to load the bitmap
			NLMISC::CIFile is;
			try
			{
				is.open(maskFileName);
				li.Mask.load(is);
				if (li.Mask.getPixels().empty())
				{
					throw NLMISC::Exception(std::string("Failed to load mask ") + maskFileName);
				}

				if (li.Mask.PixelFormat != NLMISC::CBitmap::RGBA)
				{
					li.Mask.convertToType(NLMISC::CBitmap::RGBA);
				}

				/// make sure the mask has the same size
				if (li.Mask.getWidth() != srcBitmap.getWidth()
					|| li.Mask.getHeight() != srcBitmap.getHeight())
				{
					throw NLMISC::Exception("Bitmap and mask do not have the same size");
				}

				masks.push_back(li);	
			}
			catch (std::exception &e)
			{
				nlinfo("Error with : %s : %s. Aborting this bitmap processing", maskFileName.c_str(), e.what());				
				return;
			}
		}
	}

	if (masks.size() == 0) 
	{
		nlinfo("no masks found, processing next");
		return; // perhaps it was a mask ?
	}

	/// generate each texture 
	for(;;)
	{	
		resultBitmap = srcBitmap;
		uint l;
		std::string outputFileName = fileName;
		/// build current tex
		for (l  = 0; l < masks.size(); ++l)
		{
			uint maskID = masks[l].MaskID;
			uint colorID = masks[l].Counter;

			/// get the color modifier
			const CColorModifier &cm = bi.ColorMasks[maskID].CMs[colorID];

			/// apply the mask
			cm.convertBitmap(resultBitmap, resultBitmap, masks[l].Mask);

			/// complete the file name
			outputFileName += bi.DefaultSeparator + bi.ColorMasks[maskID].CMs[colorID].ColID;

			/// fill the gap with default character (for unused masks)
			uint nextMaskID = (l == (masks.size() - 1))	? 
							  bi.ColorMasks.size()  :
							  masks[l + 1].MaskID;
			/* for (uint m = masks[l].MaskID + 1; m < nextMaskID; ++m)
			{
				outputFileName += bi.DefaultChar;
			}*/								   
		}
		
		nlinfo("--- writing %s", outputFileName.c_str());
		/// Save the result. We let propagate exceptions (if there's no more space disk it useless to continue...)
		{
			try
			{			
				NLMISC::COFile os;
				os.open(bi.OutputPath + outputFileName + ".tga");
				resultBitmap.writeTGA(os, depth);
			}
			catch(NLMISC::EStream &e)
			{
				nlwarning(("Couldn't write " + bi.OutputPath + outputFileName + ".tga" + " : " + e.what()).c_str());
			}
		}

		/// increment counters		
		for (l  = 0; l < (uint) masks.size(); ++l)
		{
			++ (masks[l].Counter);

			/// check if we have done all colors for this mask
			if (masks[l].Counter == bi.ColorMasks[masks[l].MaskID].CMs.size())
			{
				masks[l].Counter = 0;
			}
			else
			{
				break;
			}
		}
		if (l == masks.size()) break; // all cases dones
	}
}

