#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include "nel/misc/types_nl.h"

#include <vector>
#include <string>

using namespace std;

struct BNPFile
{
	string Name;
	uint32 Size;
	uint32 Pos;
};

struct BNPHeader
{
	vector<BNPFile>			Files;
	uint32					OffsetFromBeginning;

	// Append the header to the big file
	bool append (const string &filename)
	{
		FILE *f = fopen(filename.c_str(), "ab");
		if (f == NULL) return false;

		uint32 nNbFile = Files.size();
		fwrite (&nNbFile, sizeof(uint32), 1, f);
		for (uint32 i = 0; i < nNbFile; ++i)
		{
			uint8 nStringSize = Files[i].Name.size();
			fwrite (&nStringSize, 1, 1, f);
			fwrite (Files[i].Name.c_str(), 1, nStringSize, f);
			fwrite (&Files[i].Size, sizeof(uint32), 1, f);
			fwrite (&Files[i].Pos, sizeof(uint32), 1, f);
		}
		fwrite (&OffsetFromBeginning, sizeof(uint32), 1, f);

		fclose (f);
		return true;

	}
};

string gDestBNPFile;
BNPHeader gBNPHeader;

// ---------------------------------------------------------------------------
void append(const string &filename1, const string &filename2, uint32 sizeToRead)
{
	FILE *f1 = fopen(filename1.c_str(), "ab");
	FILE *f2 = fopen(filename2.c_str(), "rb");
	if (f1 == NULL) return;
	if (f2 == NULL) { fclose(f1); return; }
	
	uint8 *ptr = new uint8[sizeToRead];
	fread (ptr, sizeToRead, 1, f2);
	fwrite (ptr, sizeToRead, 1, f1);
	delete ptr;
	
	fclose(f2);
	fclose(f1);
}
#define MAX_PATH 260
// ---------------------------------------------------------------------------
void packSubRecurse ()
{
	_finddata_t findData;
	long hFind;
	char sCurDir[MAX_PATH];

	getcwd (sCurDir, MAX_PATH);
	hFind = _findfirst ("*.*", &findData);	
	while (hFind != -1)
	{
		if (!(findData.attrib&_A_SUBDIR))
		{
			BNPFile ftmp;
			ftmp.Name = findData.name;
			ftmp.Size = findData.size;
			ftmp.Pos = gBNPHeader.OffsetFromBeginning;
			gBNPHeader.Files.push_back(ftmp);
			gBNPHeader.OffsetFromBeginning += ftmp.Size;
			append(gDestBNPFile, ftmp.Name, ftmp.Size);
			printf("adding %s\n", findData.name);
		}
		else if ((strcmp(findData.name, ".") != 0) && (strcmp(findData.name, "..") != 0))
		{
			chdir (findData.name);
			packSubRecurse ();
			chdir (sCurDir);
		}
		if (_findnext (hFind, &findData) == -1)
			break;
	}
	_findclose (hFind);
}

// ---------------------------------------------------------------------------
void usage()
{
	printf ("USAGE : \n");
	printf ("   bnp_make /p directory_name\n");
	printf (" Pack the directory to a bnp file\n");
	printf ("   bnp_make /u bnp_file\n");
	printf (" Unpack the bnp file to a directory\n");
}

// ---------------------------------------------------------------------------
int main (int nNbArg, char **ppArgs)
{
	if (nNbArg != 3)
	{
		usage();
		return -1;
	}

	if ((strcmp(ppArgs[1], "/p") == 0) || (strcmp(ppArgs[1], "/P") == 0) ||
		(strcmp(ppArgs[1], "-p") == 0) || (strcmp(ppArgs[1], "-P") == 0))
	{
		// Pack a directory
		char sCurDir[MAX_PATH];
		chdir (ppArgs[2]);
		getcwd (sCurDir, MAX_PATH);
		gDestBNPFile = string(sCurDir) + string(".bnp");
		remove (gDestBNPFile.c_str());
		gBNPHeader.OffsetFromBeginning = 0;	
		packSubRecurse();
		gBNPHeader.append (gDestBNPFile);
		return 1;
	}

	if ((strcmp(ppArgs[1], "/u") == 0) || (strcmp(ppArgs[1], "/U") == 0) ||
		(strcmp(ppArgs[1], "-u") == 0) || (strcmp(ppArgs[1], "-U") == 0))
	{
		// Unpack a bnp file
		return 1;
	}

	usage ();
	return -1;
}