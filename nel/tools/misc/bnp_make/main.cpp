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
		FILE *f = fopen (filename.c_str(), "ab");
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

	// Read the header from a big file
	bool read (const string &filename)
	{
		FILE *f = fopen (filename.c_str(), "rb");
		if (f == NULL) return false;

		fseek (f, 0, SEEK_END);
		uint32 nFileSize = ftell (f);
		fseek (f, nFileSize-sizeof(uint32), SEEK_SET);
		uint32 nOffsetFromBegining;
		fread (&nOffsetFromBegining, sizeof(uint32), 1, f);
		if (fseek (f, nOffsetFromBegining, SEEK_SET) != 0)
			return false;
		
		uint32 nNbFile;
		if (fread (&nNbFile, sizeof(uint32), 1, f) != 1)
			return false;
		for (uint32 i = 0; i < nNbFile; ++i)
		{
			uint8 nStringSize;
			char sName[256];
			if (fread (&nStringSize, 1, 1, f) != 1)
				return false;
			if (fread (sName, 1, nStringSize, f) != nStringSize)
				return false;
			sName[nStringSize] = 0;
			BNPFile tmpBNPFile;
			tmpBNPFile.Name = sName;
			if (fread (&tmpBNPFile.Size, sizeof(uint32), 1, f) != 1)
				return false;
			if (fread (&tmpBNPFile.Pos, sizeof(uint32), 1, f) != 1)
				return false;
			Files.push_back (tmpBNPFile);
		}

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

			// Check if we can read the source file
			FILE *f = fopen (findData.name, "rb");
			if (f != NULL)
			{
				fclose (f);
				ftmp.Name = findData.name;
				ftmp.Size = findData.size;
				ftmp.Pos = gBNPHeader.OffsetFromBeginning;
				gBNPHeader.Files.push_back(ftmp);
				gBNPHeader.OffsetFromBeginning += ftmp.Size;
				append(gDestBNPFile, ftmp.Name, ftmp.Size);
				printf("adding %s\n", findData.name);
			}
			else
			{
				printf("error cannot open %s\n", findData.name);
			}
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
void unpack (const string &dirName)
{
	FILE *bnp = fopen (gDestBNPFile.c_str(), "rb");
	FILE *out;
	if (bnp == NULL)
		return;

	for (uint32 i = 0; i < gBNPHeader.Files.size(); ++i)
	{
		BNPFile &rBNPFile = gBNPHeader.Files[i];
		string filename = dirName + "\\" + rBNPFile.Name;
		out = fopen (filename.c_str(), "wb");
		if (out != NULL)
		{
			fseek (bnp, rBNPFile.Pos, SEEK_SET);
			uint8 *ptr = new uint8[rBNPFile.Size];
			fread (ptr, rBNPFile.Size, 1, bnp);
			fwrite (ptr, rBNPFile.Size, 1, out);
			fclose (out);
			delete ptr;
		}
	}
	fclose (bnp);
}

// ---------------------------------------------------------------------------
void usage()
{
	printf ("USAGE : \n");
	printf ("   bnp_make /p <directory_name> [<destination_path>] [<destination_filename>]\n");
	printf (" Pack the directory to a bnp file\n");
	printf ("   bnp_make /u <bnp_file>\n");
	printf (" Unpack the bnp file to a directory\n");
}

// ---------------------------------------------------------------------------
int main (int nNbArg, char **ppArgs)
{
	if (nNbArg < 3)
	{
		usage();
		return -1;
	}

	if ((strcmp(ppArgs[1], "/p") == 0) || (strcmp(ppArgs[1], "/P") == 0) ||
		(strcmp(ppArgs[1], "-p") == 0) || (strcmp(ppArgs[1], "-P") == 0))
	{
		// Pack a directory

		char sCurDir[MAX_PATH];

		if (nNbArg >= 4)
		{
			// store current path
			getcwd (sCurDir, MAX_PATH);
	
			// go to the dest path
			char sDestDir[MAX_PATH];
			chdir (ppArgs[3]);
			getcwd (sDestDir, MAX_PATH);
			
			// restore current path
			chdir (sCurDir);
			// go to the source dir
			chdir (ppArgs[2]);
			getcwd (sCurDir, MAX_PATH);
			
			gDestBNPFile = string(sDestDir) + '\\';

			if(nNbArg == 5)
			{
				gDestBNPFile += ppArgs[4];
				// add ext if necessary
				if (string(ppArgs[4]).find(".") == string::npos)
					gDestBNPFile += string(".bnp");
			}
			else
			{
				char *pos = strrchr (sCurDir, '\\');
				if (pos != NULL)
				{
					gDestBNPFile += string(pos+1);
				}
				// get the dest file name
				gDestBNPFile += string(".bnp");
			}
		}
		else
		{
			chdir (ppArgs[2]);
			getcwd (sCurDir, MAX_PATH);
			gDestBNPFile = string(sCurDir) + string(".bnp");
		}
		
		remove (gDestBNPFile.c_str());
		gBNPHeader.OffsetFromBeginning = 0;	
		packSubRecurse();
		gBNPHeader.append (gDestBNPFile);
		return 1;
	}

	if ((strcmp(ppArgs[1], "/u") == 0) || (strcmp(ppArgs[1], "/U") == 0) ||
		(strcmp(ppArgs[1], "-u") == 0) || (strcmp(ppArgs[1], "-U") == 0))
	{
		int i;
		string path;
		gDestBNPFile = ppArgs[2];
		if ((gDestBNPFile.rfind('\\') != string::npos) || (gDestBNPFile.rfind('/') != string::npos))
		{
			int pos = gDestBNPFile.rfind('\\');
			if (pos == string::npos)
				pos = gDestBNPFile.rfind('/');
			for (i = 0; i <= pos; ++i)
				path += gDestBNPFile[i];
			string wholeName = gDestBNPFile;
			gDestBNPFile = "";
			for (; i < (int)wholeName.size(); ++i)
				gDestBNPFile += wholeName[i];
			char sCurDir[MAX_PATH];
			chdir (path.c_str());
			getcwd (sCurDir, MAX_PATH);
			path = sCurDir;
		}
		if (stricmp (gDestBNPFile.c_str()+gDestBNPFile.size()-4, ".bnp") != 0)
		{
			gDestBNPFile += ".bnp";
		}
		string dirName;
		for (i = 0; i < (int)(gDestBNPFile.size()-4); ++i)
			dirName += gDestBNPFile[i];
		// Unpack a bnp file
		if (!gBNPHeader.read (gDestBNPFile))
			return -1;

		mkdir (dirName.c_str());

		unpack (dirName);

		return 1;
	}

	usage ();
	return -1;
}