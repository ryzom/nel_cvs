/** 
 *  Build a bank tile far
 */

#include <nel/misc/file.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/tile_far_bank.h>
#include <nel/misc/bitmap.h>

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#define VERSION "1.0"

#if defined (NL_OS_WINDOWS)
#define STAT _stat
#else // NL_OS_WINDOWS
#define STAT stat
#endif // NL_OS_WINDOWS

// Return true if f2 must be recomputed with f1, else return false.
// return true if date f1 > date f2
// If f1 doesn't exist, returns false
// If f2 doesn't exist, returns true
bool recompute (const char* f1, const char* f2)
{
	struct STAT buf1;
	struct STAT buf2;
	if (STAT (f1, &buf1)!=0)
		return false;
	if (STAT (f2, &buf2)!=0)
		return true;
	return buf1.st_mtime > buf2.st_mtime;
}

// Return true if the file exist
bool isFileExist (const char* sName)
{
	FILE *pFile=fopen (sName, "rb");
	if (!pFile)
		return false;
	fclose (pFile);
	return true;
}

// Fill tile far pixel with this bitmap
bool fillTileFar (uint tile, const char* sName, CTileFarBank::TFarType type, CTileFarBank& farBank)
{
	// Progress message
	printf ("Computing %s...\n", sName);

	// Create a stream
	CIFile inputBitmap;
	if (inputBitmap.open (sName))
	{
		try
		{
			// Load the texture
			CBitmap bitmap;
			bitmap.load (inputBitmap);

			// Convert to RGBA
			bitmap.convertToType (CBitmap::RGBA);

			// Get bitmap size
			uint width=bitmap.getWidth();
			uint height=bitmap.getHeight();

			// Check size..
			if ((width!=128 && width!=256) || (height!=128 && height!=256))
			{
				fprintf (stderr, "Error: invalid size. Only 128 or 256\n");
			}
			else
			{
				// Build mipmaps
				bitmap.buildMipMaps ();

				// Get the tile
				CTileFarBank::CTileFar* pFarTile=farBank.getTile (tile);

				// Copy arrays
				pFarTile->setPixels (type, CTileFarBank::order0, (CRGBA*)&bitmap.getPixels (5)[0], (width>>5)*(height>>5));
				pFarTile->setPixels (type, CTileFarBank::order1, (CRGBA*)&bitmap.getPixels (6)[0], (width>>6)*(height>>6));
				pFarTile->setPixels (type, CTileFarBank::order2, (CRGBA*)&bitmap.getPixels (7)[0], (width>>7)*(height>>7));

				// Ok.
				return true;
			}
		}
		catch (Exception& except)
		{
			fprintf (stderr, "Error: %s\n", except.what());
		}
	}
	else
	{
		fprintf (stderr, "Error: can't open bitmap %s for reading\n", sName);
	}
	return false;
}

// Go go go !
int main (int argc, char **argv)
{
	// Options
	bool useCurrentPath=false;
	bool forceRecomputation=false;
	bool outputVersion=false;
	bool outputHelp=false;

	// If no argu, show help
	if (argc==1)
		outputHelp=true;

	int nFirstGoodArgu=3;

	// Correct number of argu..
	// Check for options
	for (int n=1; n<argc; n++)
	{
		if (stricmp (argv[n], "-r")==0)
			useCurrentPath=true;
		if (stricmp (argv[n], "-f")==0)
			forceRecomputation=true;
		if (stricmp (argv[n], "-v")==0)
			outputVersion=true;
		if (stricmp (argv[n], "-?")==0)
			outputHelp=true;
	}

	// Output version?
	if (outputVersion)
		printf ("Version %s\n", VERSION);

	// Output help?
	if (outputHelp)
	{
		// Ok, some help if not enough argu
		printf (
			"build_far_bank [input.bank][output.farbank][-r][-f][-v][-?]\n"
			"options:\n"
			"\t-r: load the bitmaps from the current directory\n"
			"\t-f: force recomputation of all the tiles\n"
			"\t-v: print the version\n"
			"\t-?: print help\n"
			);
	}

	if (argc>=3)
	{

		// Count number of tiles
		int tileCount=0;
		int tileComputed=0;

		// Open input file
		CIFile inputFile;
		if (inputFile.open(argv[1]))
		{
			// Get some incoming exceptions
			try
			{
				// Ok, create a far tile bank
				CTileFarBank farBank;

				// Try to open input file
				CIFile inputFarBank;
				if (inputFarBank.open(argv[2]))
				{
					// Serial the bank in input
					farBank.serial (inputFarBank);
				}
				
				// Force recomputation ?
				if (recompute (argv[1], argv[2]))
				{
					// Progress message
					printf ("%s have been modified, recompute all the tiles...\n", argv[1]);
					forceRecomputation=true;
				}

				// Close this file
				inputFarBank.close ();

				// Progress message
				printf ("Reading the bank...\n");

				// Create a bank
				CTileBank bank;

				// Serialize the input bank
				bank.serial (inputFile);

				// Path relative
				if (useCurrentPath)
					bank.makeAllPathRelative ();

				// Resize far bank
				farBank.setNumTile (bank.getTileCount());

				// Scan each tiles...
				int tile;
				for (tile=0; tile<bank.getTileCount(); tile++)
				{
					// Get a pointer on this tile
					CTile *pTile=bank.getTile(tile);

					// Delete pixels
					bool bDeleteDiffuse=true;
					bool bDeleteAdditive=true;
					bool bDeleteAlpha=true;

					// Tile not free ?
					if (!pTile->isFree())
					{
						// Diffuse bitmap filled ?
						if (pTile->getRelativeFileName (CTile::diffuse)!="")
						{
							// File exist ?
							if (isFileExist ((bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse)).c_str()))
							{
								// Recompute it?
								if (recompute ((bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse)).c_str(), argv[2])||forceRecomputation)
								{
									// Fill infos
									if (fillTileFar (tile, (bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse)).c_str(), CTileFarBank::diffuse, farBank))
									{
										// One more tile
										tileCount++;

										tileComputed++;
										bDeleteDiffuse=false;
									}
								}
								else
								{
									// One more tile
									tileCount++;
	
									printf ("Skipping %s...\n", (bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse)).c_str());
									bDeleteDiffuse=false;
								}
							}
						}

						// Additive bitmap filled ?
						if (pTile->getRelativeFileName (CTile::additive)!="")
						{
							// File exist ?
							if (isFileExist ((bank.getAbsPath()+pTile->getRelativeFileName (CTile::additive)).c_str()))
							{
								// Recompute it?
								if (recompute ((bank.getAbsPath()+pTile->getRelativeFileName (CTile::additive)).c_str(), argv[2])||forceRecomputation)
								{
									// Fill infos
									if (fillTileFar (tile, (bank.getAbsPath()+pTile->getRelativeFileName (CTile::additive)).c_str(), CTileFarBank::additive, farBank))
									{
										// One more tile
										tileCount++;

										tileComputed++;
										bDeleteAdditive=false;
									}
								}
								else
								{
									// One more tile
									tileCount++;

									printf ("Skipping %s...\n", (bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse)).c_str());
									bDeleteAdditive=false;
								}
							}
						}

						// Alpha bitmap filled ?
						if (pTile->getRelativeFileName (CTile::alpha)!="")
						{
							// File exist ?
							if (isFileExist ((bank.getAbsPath()+pTile->getRelativeFileName (CTile::alpha)).c_str()))
							{
								// Recompute it?
								if (recompute ((bank.getAbsPath()+pTile->getRelativeFileName (CTile::alpha)).c_str(), argv[2])||forceRecomputation)
								{
									// Fill infos
									if (fillTileFar (tile, (bank.getAbsPath()+pTile->getRelativeFileName (CTile::alpha)).c_str(), CTileFarBank::alpha, farBank))
									{
										// One more tile
										tileCount++;

										tileComputed++;
										bDeleteAlpha=false;
									}
								}
								else
								{
									// One more tile
									tileCount++;

									printf ("Skipping %s...\n", (bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse)).c_str());
									bDeleteAlpha=false;
								}
							}
						}
					}

					// Delete diffuse pixels?
					if (bDeleteDiffuse)
						farBank.getTile (tile)->erasePixels (CTileFarBank::diffuse);

					// Delete additif pixels?
					if (bDeleteAdditive)
						farBank.getTile (tile)->erasePixels (CTileFarBank::additive);
				}

				// Open output file
				COFile outputFile;
				if (outputFile.open(argv[2]))
				{
					// Progress message
					printf ("Writing %s...\n", argv[2]);

					// Writing the bank
					farBank.serial (outputFile);

					// Progress message
					printf ("%d far tiles found.\n", tileCount);
					printf ("%d far tiles computed.\n", tileComputed);
				}
				else	// Open failed
				{
					fprintf (stderr, "Can't open file %s for writing\n", argv[2]);
				}
			}
			catch (Exception& except)
			{
				fprintf (stderr, "Error: %s\n", except.what());
			}
		}
		else	// Open failed
		{
			fprintf (stderr, "Can't open file %s for reading\n", argv[1]);
		}
	}		

	// exit
	return 0;
}
