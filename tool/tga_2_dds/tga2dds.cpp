
#include <iostream.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/3d/bitmap.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"

#include "s3_intrf.h"



const uint8 DDSURFACEDESC_HEADER_SZ = 124;







void main(int argc, char **argv)
{
	nlassert(argc>=4);

	std::string inputFileName(argv[1]);
	std::string outputFileName(argv[2]);
	std::string algorithm(argv[3]);

	// Reading TGA and converting to RGBA
	CIFile input;
	input.open(inputFileName);
	uint32 height = intput.getHeight();
	uint32 width= input.getWidth();
	input.convertToRGBA();

	// Checking if option "usercolor" has been used
	std::string userColorFileName;
	if(argc>4)
	{
		if(!strcmp("-usercolor"))
		{
			nlassert(argc==6);
			userColorFileName(argv[5]);
		}
	}

	// Vectors for RGBA data
	std::vector<uint8> ARGBSrc = input.getPixels();
	std::vector<uint8> ARGBSrc2;
	std::vector<uint8> ARGBDest;

	// Reading second Tga for user color
	if(userColorFileName.length())
	{
		CIFile input2;
		input2.open(userColorFileName);
		uint32 height2 = intput2.getHeight();
		uint32 width2 = input2.getWidth();
		nlassert(width2==width);
		nlassert(height2==height);
		input2.convertToRGBA();
		ARGBSrc2 = input2.getPixels();
	}


	// Filling DDSURFACEDESC structure
	uint32 DDSSurfaceDesc[DDSURFACEDESC_HEADER_SZ/sizeof(uint32)];
	DDSSurfaceDesc[0] = DDSURFACEDESC_HEADER_SZ;

	

	COFile output;
	output.open(outputFileName);

}