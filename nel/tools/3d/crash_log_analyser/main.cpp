




#include "nel/misc/file.h"
#include "nel/misc/path.h"


using namespace std;
using namespace NLMISC;


#define myinfo NLMISC::createDebug (), NLMISC::InfoLog->setPosition( __LINE__, __FILE__ ), NLMISC::InfoLog->displayRawNL


// ***************************************************************************
void	filterRyzomBug(const char *dirSrc, const char *dirDst, uint patchVersionWanted)
{
	if(!CFile::isDirectory(dirDst))
	{
		myinfo("%s is not a directory", dirDst);
		return;
	}

	vector<string>	fileList;
	CPath::getPathContent(dirSrc, false, false, true, fileList, NULL, true);

	for(uint i=0;i<fileList.size();i++)
	{
		const string &fileFullPath= fileList[i];
		CIFile	f;
		if(f.open(fileFullPath, true))
		{
			// Parse all "UserId: ", this get the number of crash in this file
			const	string	userIdTok= "UserId: ";
			const	string	patchVersionTok= "PatchVersion: ";
			uint	numUserId= 0;
			uint	numPatchVersion= 0;
			bool	precUserId= false;
			bool	ok= true;
			while(!f.eof())
			{
				char	tmp[1000];
				f.getline(tmp, 1000);
				string	str= tmp;
				if(str.compare(0, userIdTok.size(), userIdTok)==0)
				{
					numUserId++;
					if(precUserId)
					{
						nlwarning("Don't find a PatchVersion for all UserId in %s", fileFullPath.c_str());
						ok= false; 
						break;
					}
					else
						precUserId= true;
				}
				if(str.compare(0, patchVersionTok.size(), patchVersionTok)==0)
				{
					numPatchVersion++;
					if(!precUserId)
					{
						nlwarning("Don't find a PatchVersion for all UserId in %s", fileFullPath.c_str());
						ok= false; 
						break;
					}
					else
						precUserId= false;

					// parse the version number
					sint	version;
					sscanf(tmp, "PatchVersion: %d", &version);
					if(version!=(sint)patchVersionWanted)
					{
						nlwarning("The Log %s contains a PatchVersion different: %d", fileFullPath.c_str(), version);
						ok= false;
						break;
					}
				}
			}
			f.close();
			if(ok && numUserId!=numPatchVersion)
			{
				nlwarning("Don't find a PatchVersion for all UserId in %s", fileFullPath.c_str());
				ok= false;
			}

			// if ok, copy the file and the associated .dmp dir
			if(ok)
			{
				//myinfo("Copy  %s", fileFullPath.c_str());

				// get the log size
				uint	size= CFile::getFileSize(fileFullPath);

				// copy the log
				string	fileNoDir= CFile::getFilename(fileFullPath);
				string	fileNoExt= CFile::getFilenameWithoutExtension(fileFullPath);
				string	dirDest= dirDst;
				dirDest+= "/" + toString("%05d_", size/1024) + fileNoExt;
				string	dmpDirSrc= string(dirSrc) + "/" + fileNoExt;

				CFile::createDirectory(dirDest);
				
				// copy near the dmp
				CFile::copyFile((dirDest + "/" + fileNoDir).c_str(), fileFullPath.c_str());


				// copy all the .dmp in a new dir
				static vector<string>	dmpList;
				dmpList.clear();
				CPath::getPathContent(dmpDirSrc, false, false, true, dmpList, NULL);
				for(uint j=0;j<dmpList.size();j++)
				{
					string	dmpNoDir= CFile::getFilename(dmpList[j]);
					CFile::copyFile((dirDest+ "/" + dmpNoDir).c_str(), dmpList[j].c_str());
				}
			}
		}
		else
			nlwarning("cannot open %s", fileFullPath.c_str());
	}
}


// ***************************************************************************
struct CStatVal
{
	uint	Val;

	CStatVal()
	{
		Val = 0;
	}
};

typedef	map<sint, CStatVal>			TStatMap;
typedef	map<string, CStatVal>		TStatStrMap;

void	statRyzomBug(const char *dirSrc)
{
	vector<string>	fileList;
	CPath::getPathContent(dirSrc, false, false, true, fileList, NULL, true);
	

	TStatStrMap		senderMap;
	TStatMap		shardMap;
	TStatMap		timeInGameMap;
	TStatMap		info3dMap;
	uint			totalCrash= 0;
	uint			totalCrashDuplicate= 0;
	

	// **** parse all files
	for(uint i=0;i<fileList.size();i++)
	{
		const string &fileFullPath= fileList[i];
		CIFile	f;
		if(f.open(fileFullPath, true))
		{
			const	string	senderIdTok= "Sender: ";
			const	string	shardIdTok= "ShardId: ";
			const	string	timeInGameIdTok= "Time in game: ";
			const	string	localTimeIdTok= "LocalTime: ";
			const	string	nel3dIdTok= "NeL3D: ";
			const	string	card3dIdTok= "3DCard: ";
			
			string	precSenderId;
			string	precSenderId2;
			sint	precShardId= -1;
			sint	precTimeInGame= -1;	// 0 means "never in game", 1 means < 10 min, 2 means more
			sint	precNel3DMode= -1;	// 0 OpenGL, 1 D3D, 2 ????
			sint	precCard3D= -1;		// 0 NVidia, 1 ATI, 2 ????
			sint64	precLocalTime= -1;	// local time in second
			sint64	precLocalTime2= -1;	// local time in second
			while(!f.eof())
			{
				char	tmp[1000];
				f.getline(tmp, 1000);
				string	str= tmp;
				if(str.compare(0, senderIdTok.size(), senderIdTok)==0)
				{
					precSenderId2= precSenderId;
					precSenderId= str.c_str()+senderIdTok.size();
				}
				else if(str.compare(0, shardIdTok.size(), shardIdTok)==0)
				{
					precShardId= atoi(str.c_str()+shardIdTok.size());
				}
				else if(str.compare(0, timeInGameIdTok.size(), timeInGameIdTok)==0)
				{
					string timeStr= str.substr(timeInGameIdTok.size(), string::npos);
					if(timeStr=="0h 0min 0sec" || timeStr.size()<12)
						precTimeInGame= 0;
					else
					{
						if(timeStr[1]=='h' && timeStr[4]=='m' && timeStr[3]<='5')
							precTimeInGame= 1;
						else
							precTimeInGame= 2;
					}
				}
				else if(str.compare(0, localTimeIdTok.size(), localTimeIdTok)==0)
				{
					precLocalTime2= precLocalTime;
					// 2004/09/17 04:21:16
					string timeStr= str.substr(localTimeIdTok.size(), string::npos);
					if(timeStr.size()<19)
						precLocalTime= 0;
					else
					{
						sint64	year= atoi(timeStr.substr(0,4).c_str());
						sint64	month= atoi(timeStr.substr(5,2).c_str());
						sint64	day= atoi(timeStr.substr(8,2).c_str());
						sint64	hour= atoi(timeStr.substr(11,2).c_str());
						sint64	minute= atoi(timeStr.substr(14,2).c_str());
						sint64	sec= atoi(timeStr.substr(17,2).c_str());
						year= max(year, (sint64)2004);
						year-=2004;
						precLocalTime= ((year*366+month)*12)+day;
						precLocalTime= (((precLocalTime*24)+hour)*60+minute)*60+sec;
					}
				}
				else if(str.compare(0, nel3dIdTok.size(), nel3dIdTok)==0)
				{
					string	tmp= str;
					strlwr(tmp);
					if(tmp.find("opengl")!=string::npos)
						precNel3DMode= 0;
					else if(tmp.find("direct3d")!=string::npos)
						precNel3DMode= 1;
					else
						precNel3DMode= 2;
				}
				else if(str.compare(0, card3dIdTok.size(), card3dIdTok)==0)
				{
					string	tmp= str;
					if(tmp.find("NVIDIA")!=string::npos)
						precCard3D= 0;
					else if(tmp.find("RADEON")!=string::npos)
						precCard3D= 1;
					else
						precCard3D= 2;

					// END a block, add info in map (only if not repetition)
					if( precSenderId!=precSenderId2 ||
						abs(precLocalTime-precLocalTime2)>(sint64)60 )
					{
						senderMap[precSenderId].Val++;
						shardMap[precShardId].Val++;
						timeInGameMap[precTimeInGame].Val++;
						if(precNel3DMode!=0 && precNel3DMode!=1)
							precNel3DMode= 2;
						if(precCard3D!=0 && precCard3D!=1)
							precCard3D= 2;
						info3dMap[precNel3DMode*256+precCard3D].Val++;
						totalCrash++;
					}
					totalCrashDuplicate++;
				}
			}
		}
	}

	// **** display Stats
	// general stats
	myinfo("**** Total: %d Crashs (%d with duplicates)", totalCrash, totalCrashDuplicate);
	myinfo("NB: 'duplicates' means: crashs that are removed because suppose the player click 'ignore' (same sender/same Localtime, within about 1 min)");
	
	// senderId
	TStatMap::iterator	it;
	TStatStrMap::iterator	itStr;
	myinfo("");
	myinfo("**** Stat Per Sender:");
	multimap<uint, string>	resortSender;
	for(itStr=senderMap.begin();itStr!=senderMap.end();itStr++)
	{
		resortSender.insert(make_pair(itStr->second.Val, itStr->first));
	}
	for(multimap<uint, string>::iterator it2=resortSender.begin();it2!=resortSender.end();it2++)
	{
		myinfo("**** %d Crashs for UserId %s", it2->first, it2->second.c_str());
	}
	// shardId
	myinfo("");
	myinfo("**** Stat Per ShardId:");
	for(it=shardMap.begin();it!=shardMap.end();it++)
	{
		myinfo("**** %d Crashs for ShardId %d", it->second.Val, it->first);
	}
	// timeInGame
	myinfo("");
	myinfo("**** Stat Per TimeInGame:");
	for(it=timeInGameMap.begin();it!=timeInGameMap.end();it++)
	{
		myinfo("**** %d Crashs for TimeInGame %s", it->second.Val, it->first==0?"0h 0min 0sec":
				(it->first==1?"<=5 min":
				 it->first==2?"> 5 min":"??? Bad parse ???"));
	}
	// info3d
	myinfo("");
	myinfo("**** Stat Per 3d Mode:");
	for(it=info3dMap.begin();it!=info3dMap.end();it++)
	{
		uint	card3d= it->first&255;
		uint	mode3d= it->first>>8;
		
		myinfo("**** %d Crashs for %s / Card %s", it->second.Val, 
				mode3d==0?"OpenGL":(mode3d==1?"Direct3D":"??? No Driver ???"),
				card3d==0?"NVIDIA":(card3d==1?"RADEON":"Misc"));
	}
}

// ***************************************************************************
int	main(int argc, char *argv[])
{
	bool	ok= false;
	bool	statMode= false;
	bool	filterMode= false;
	if(argc == 3 && argv[2]==string("-s"))
		ok=	true,statMode= true;
	if(argc == 5 && argv[2]==string("-p"))
		ok=	true,filterMode= true;
	
	if(!ok)
	{
		myinfo("Usage1 (stats):\n\t%s src_dir -s\n", CFile::getFilename(argv[0]).c_str());
		myinfo("Usage2 (patch filter):\n\t%s src_dir -p patch_version dst_dir\n", CFile::getFilename(argv[0]).c_str());
	}
	else
	{
		if(!CFile::isDirectory(argv[1]))
		{
			myinfo("%s is not a directory", argv[1]);
			return 1;
		}
		
		if(filterMode)
		{
			filterRyzomBug(argv[1], argv[3], atoi(argv[4]));
		}
		else if(statMode)
		{
			statRyzomBug(argv[1]);
		}
	}

	return 0;
}

