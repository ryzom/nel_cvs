/** \file patch.cpp
 *
 * $Id: patch.cpp,v 1.2 2002/10/21 14:52:35 lecroart Exp $
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

#include "stdafx.h"

#include "Wininet.h"
#include <process.h>
#include <direct.h>
#include <sys/utime.h>

#include <queue>

#include "zlib.h"

#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"

#include "patch.h"
//#include "nel_launcherDlg.h"

using namespace std;
using namespace NLMISC;

HINTERNET RootInternet = NULL;

static const string DirFilename = "dir.gz";

struct CEntry
{
	string Filename;
	uint32 Size;
	uint32 Date;
	CEntry(const string &fn, uint32 s, uint32 d) : Filename(fn), Size(s), Date(d) { }
};


void setVersion(const std::string &version)
{
	string fn = "VERSION";
	FILE *fp = fopen (fn.c_str(), "wb");
	if (fp == NULL)
		throw ("Can't open file '%s'", fn.c_str ());

	fputs (version.c_str (), fp);
	fclose (fp);
}

string getVersion()
{
	string fn = "VERSION";
	FILE *fp = fopen (fn.c_str (), "rb");
	if (fp!=NULL)
	{
		char ver[1000];
		if (fgets (ver, 1000, fp) != NULL)
		{
			return ver;
		}
		fclose (fp);
	}
	return "";
}

class CPatchThread : public IRunnable
{
public:
	
	CPatchThread(const string &sp, const string &sv, const std::string &urlOk, const std::string &urlFailed, const std::string &logSeparator) :
	  ServerPath (sp), ServerVersion(sv), UrlOk(urlOk), UrlFailed(urlFailed), Ended(false), StateChanged(true), LogSeparator(logSeparator)
	{
	}
	
	bool Ended;
	string Url;
	string State;
	string StateLog;
	bool StateChanged;

private:

	void run ()
	{
		try
		{
			CurrentFilesToGet = 0;
			CurrentBytesToGet = 0;
			TotalFilesToGet = 0;
			TotalBytesToGet = 0;

			bool needToExecuteAPatch = false;

			string ClientRootPath = "./";
			string ClientPatchPath = "./patch/";
			string ServerRootPath = CPath::standardizePath (ServerPath);

			setState(true, "Patching from '%s'", ServerRootPath.c_str());

			// create the patch directory if not exists
			if (!NLMISC::CFile::isExists ("patch"))
			{
				setState(true, "Creating patch directory");
				_mkdir ("patch");
			}

			// first, get the file that contains all files
			_unlink (DirFilename.c_str());
			downloadFile (ServerRootPath+DirFilename, DirFilename);

			// now parse the file
			gzFile gz = gzopen (DirFilename.c_str (), "rb");
			if (gz == NULL)
				throw Exception ("Can't open file '%s'", DirFilename.c_str());

			vector<CEntry> filesList;
			vector<CEntry> needToGetFilesList;

			setState(true, "Parsing %s...", DirFilename.c_str());

			char buffer[2000];
			gzgets (gz, buffer, 2000);

			if (string(buffer) != "FILESLIST\n")
			{
				throw Exception ("%s has not a valid content", DirFilename.c_str());
			}

			while (!gzeof(gz))
			{
				gzgets (gz, buffer, 2000);

				string b = buffer;
				uint pos1 = b.find ("/");
				uint pos2 = b.find ("/", pos1+1);

				if (pos1 != string::npos || pos2 != string::npos)
				{
					string filename = b.substr (0, pos1);
					uint32 size = atoi(b.substr (pos1+1, pos2-pos1).c_str());
					uint32 date = atoi(b.substr (pos2+1).c_str());

					string path = ClientRootPath+filename;
					if (!NLMISC::CFile::fileExists (path))
					{
						path = ClientPatchPath + filename;
					}

					if (NLMISC::CFile::getFileModificationDate (path) != date || size != NLMISC::CFile::getFileSize (path))
					{
						needToGetFilesList.push_back (CEntry(filename, size, date));
						TotalFilesToGet++;
						TotalBytesToGet += size;
					}
					filesList.push_back (CEntry(filename, size, date));
				}
			}
			gzclose (gz);

			// get file if necessary
			uint i;
			for (i = 0; i < needToGetFilesList.size (); i++)
			{
				nlSleep (1000);
				
				string path = ClientRootPath+needToGetFilesList[i].Filename;
				if (!NLMISC::CFile::fileExists (path))
				{
					path = ClientPatchPath + needToGetFilesList[i].Filename;
				}

				nlinfo ("Get the file from '%s' to '%s'", string(ServerRootPath+needToGetFilesList[i].Filename).c_str(), path.c_str());

				// get the new file
				downloadFile (ServerRootPath+needToGetFilesList[i].Filename+".gz", path+".gz");
				// decompress it
				decompressFile (path+".gz", needToGetFilesList[i].Date);

				// special case
				if (needToGetFilesList[i].Filename == "patch_execute.bat")
				{
					needToExecuteAPatch = true;
				}
			}

			if (RootInternet != NULL)
			{
				InternetCloseHandle(RootInternet);
				RootInternet = NULL;
			}

			// now, we have to delete files that are not in the server list
	
			setState(true, "Scanning patch directory");
			vector<string> res;
			CPath::getPathContent(ClientPatchPath, false, false, true, res);

			for (i = 0; i < res.size (); i++)
			{
				string fn = NLMISC::CFile::getFilename (res[i]);
				uint j;
				for (j = 0; j < filesList.size (); j++)
				{
					if (fn == filesList[j].Filename)
					{
						break;
					}
				}
				if (j == filesList.size ())
				{
					string file = ClientPatchPath+res[i];
					nlinfo ("Deleting %s", file.c_str());
					setState(true, "Deleting %s", res[i]);
					_unlink (file.c_str ());
				}
			}

			// remove the files list file
			nlinfo ("Deleting %s", DirFilename.c_str());
			_unlink (DirFilename.c_str());

			// now that all is ok, we set the new client version
			setState (true, "set client version to %s", ServerVersion.c_str ());
			setVersion (ServerVersion);
			
			if (needToExecuteAPatch)
			{
				setState (true, "Launching patch_execute.bat");
				nlinfo ("Need to execute patch_execute.bat");
				_chdir (ClientPatchPath.c_str());
				_execlp ("patch_execute.bat", "patch_execute.bat", NULL);
				exit(0);
			}

			nlinfo ("Patching completed");
			setState (true, "Patching completed");
		
			Url = UrlOk;
			Ended = true;
		}
		catch (Exception &e)
		{
			Url = UrlFailed;
			Url += e.what();
			Ended = true;
		}
	}

	void decompressFile (const string &filename, uint32 date)
	{
		setState(true, "Decompressing %s.gz...", NLMISC::CFile::getFilename(filename).c_str ());

		gzFile gz = gzopen (filename.c_str (), "rb");
		if (gz == NULL)
		{
			_unlink (filename.c_str ());
			throw Exception ("Can't open compressed file '%s'", filename.c_str());
		}

		string dest = filename.substr(0, filename.size ()-3);
		FILE *fp = fopen (dest.c_str(), "wb");
		if (fp == NULL)
		{
			gzclose(gz);
			_unlink (filename.c_str ());
			throw Exception ("Can't open file '%s'", dest.c_str());
		}
		
		uint8 buffer[10000];
		while (!gzeof(gz))
		{
			int res = gzread (gz, buffer, 10000);
			if (res == -1)
			{
				gzclose(gz);
				fclose(fp);
				_unlink (filename.c_str ());
				throw Exception ("Can't read compressed file '%s'", filename.c_str());
			}

			int res2 = fwrite (buffer, 1, res, fp);
			if (res2 != res)
			{
				gzclose(gz);
				fclose(fp);
				_unlink (filename.c_str ());
				throw Exception ("Can't write file '%s'", dest.c_str());
			}
		}

		gzclose(gz);
		fclose(fp);
		_unlink (filename.c_str ());

		// change the file time for having the same as the server side

		if(date != 0)
		{
			_utimbuf utb;
			utb.actime = utb.modtime = date;
			if (_utime (dest.c_str (), &utb) == -1)
			{
				nlwarning ("Can't change file time for %s", dest.c_str ());
			}
		}
	}


	void downloadFile (const string &source, const string &dest)
	{
		const uint32 bufferSize = 8000;
		uint8 buffer[bufferSize];

		if (RootInternet == NULL)
			RootInternet = InternetOpen("nel_launcher", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

		HINTERNET hUrlDump = InternetOpenUrl(RootInternet, source.c_str(), NULL, NULL, INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_RAW_DATA, 0);

		FILE *fp = fopen (dest.c_str(), "wb");
		if (fp == NULL)
			throw Exception ("Can't open file '%s'", dest.c_str());

		CurrentFilesToGet++;

		setState(true, "Getting %s", NLMISC::CFile::getFilename (source).c_str ());

		do
		{
			DWORD realSize;

			if(!InternetReadFile(hUrlDump,(LPVOID)buffer, bufferSize, &realSize))
			{
				  // error
				LPVOID lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf, 0, NULL);
				string error = (LPCTSTR)lpMsgBuf;
				LocalFree(lpMsgBuf);
				
				fclose (fp);
				InternetCloseHandle(hUrlDump);

				throw Exception ("InternetReadFile() failed on file '%s': %s", source.c_str (), error.c_str());
				
				break;
			}
			else
			{
				if (realSize == 0)
				{
					// download complete successfully
					break;
				}

				fwrite (buffer, realSize, 1, fp);

				CurrentBytesToGet += realSize;

				if (TotalBytesToGet == 0 && TotalFilesToGet == 0)
					setState(false, "Getting %s, %d bytes downloaded", NLMISC::CFile::getFilename (source).c_str (), CurrentBytesToGet);
				else
					setState(false, "Getting file %d on %d, %d bytes on %d bytes, filename %s", CurrentFilesToGet, TotalFilesToGet, CurrentBytesToGet, TotalBytesToGet, NLMISC::CFile::getFilename (source).c_str ());

			}
		}
		while (true);

		fclose (fp);
		InternetCloseHandle(hUrlDump);
	}


	void setState (bool log, const char *format, ...)
	{
		char *str;
		NLMISC_CONVERT_VARGS (str, format, 256);
		nlinfo (str);
		State = str;
		if(log)
		{
			StateLog += str;
			StateLog += LogSeparator;
		}
		StateChanged = true;
	}

	string LogSeparator;

	string ServerPath;
	string ServerVersion;

	string UrlOk;
	string UrlFailed;

	uint TotalFilesToGet;
	uint TotalBytesToGet;
	uint CurrentFilesToGet;
	uint CurrentBytesToGet;
};

CPatchThread *PatchThread = NULL;

void startPatchThread (const std::string &serverPath, const std::string &serverVersion, const std::string &urlOk, const std::string &urlFailed, const std::string &logSeparator)
{
	if (PatchThread != NULL)
	{
		nlwarning ("patch thread already running");
		return;
	}
	
	PatchThread = new CPatchThread (serverPath, serverVersion, urlOk, urlFailed, logSeparator);

	IThread *thread = IThread::create (PatchThread);
	thread->start ();
}

bool patchEnded (string &url)
{
	if (PatchThread == NULL)
		return true;

	bool end = PatchThread->Ended;
	if (end)
	{
		url = PatchThread->Url;

		delete PatchThread;
		PatchThread = NULL;
	}

	return end;
}

bool patchState (string &state, std::string &stateLog)
{
	if (PatchThread == NULL)
		return false;

	bool statechanged = PatchThread->StateChanged;
	if (statechanged)
	{
		state = PatchThread->State;
		stateLog = PatchThread->StateLog;
		PatchThread->StateChanged = false;
	}

	return statechanged;
}