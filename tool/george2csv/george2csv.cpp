// sadge_test.cpp : Defines the entry point for the console application.
//
/*

	Script commands:
		OUTPUT <output file name>
		FIELD <field name>
		SOURCE <field name>
		SCANFILES <extension>


*/


#include "stdafx.h"

// Misc
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"
// Georges, bypassing interface
#include "georges/stdgeorges.h"
#include "georges/form.h"

// Basic C++
#include <iostream.h>
#include <conio.h>
#include <stdio.h>
#include <io.h>

// stl
#include <map>

using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;


/*
	some handy prototypes
*/
void setOutputFile(char *);
void addField(char *);
void addSource(char *);
void scanFiles(std::string extension);
void executeScriptFile(const char *);

/*
	Some globals
*/
FILE *Outf;
class CField
{
public:
	std::string _name;
//	bool _evaluated;
	UFormElm::TEval	_evaluated;
	CField(std::string name,UFormElm::TEval eval) { _name=name; _evaluated=eval; }
};
std::vector<CField> fields;
std::vector<std::string> files;

vector<string>		inputScriptFiles;
vector<string>		inputCsvFiles;
string				inputSheetPath;
bool				inputSheetPathLoaded = false;
map<string, string>	inputSheetPathContent;

const char				*SEPARATOR = ";";


/*
	Some routines for dealing with script input
*/
void setOutputFile(char *filename)
{
	if (Outf!=NULL)
		fclose(Outf);
	Outf=fopen(filename,"wt");
}

void addField(char *name)
{
//	fields.push_back(CField(name,true));
	fields.push_back(CField(name,UFormElm::Eval));
}

void addSource(char *name)
{
//	fields.push_back(CField(name,false));
	fields.push_back(CField(name,UFormElm::NoEval));
}

void buildFileVector(std::vector<std::string> &filenames,std::string filespec)
{
	uint i,j;
	// split up the filespec into chains
	std::vector<std::string> in, out;
	for (i=0;i<filespec.size();)
	{
		for (j=i;j<filespec.size() && filespec[j]!=' ' && filespec[j]!='\t';j++) {}
		switch(filespec[i])
		{
		case '+': in.push_back(filespec.substr(i+1,j-i-1)); break;
		case '-': out.push_back(filespec.substr(i+1,j-i-1)); break;
		default: fprintf(stderr,"Filter must start with '+' or '-'\n",&(filespec[i])); getch(); exit(1);
		}
		i=j;
		while (i<filespec.size() && (filespec[i]==' ' || filespec[i]=='\t')) i++; // skip white space
	}

	// use the filespec as a filter while we build the sheet file vector
	for (i=0;i<files.size();i++)
	{
		bool ok=true;

		// make sure the filename includes all of the include strings
		for (j=0;j<in.size() && ok;j++)
			if (files[i].find(in[j])==-1)
			{
				ok=false;
			}

		// make sure the filename includes none of the exclude strings
		for (j=0;j<out.size() && ok;j++)
			if (files[i].find(out[j])!=-1)
			{
				ok=false;
			}

		// if the filename matched all of the above criteria then add it to the list
		if (ok)
		{
			printf("Added: %s\n",CFile::getFilename(files[i]).c_str());
			filenames.push_back(files[i]);
		}
	}
	printf("Found: %d matching files (from %d)\n",filenames.size(),files.size());

}


/*
	Scanning the files ... this is the business!!
*/
void scanFiles(std::string filespec)
{
	// make sure the CSheetId singleton has been properly initialised
//	NLMISC::CSheetId::init();

	// build a vector of the sheetFilters sheet ids (".item")
//	std::vector<NLMISC::CSheetId> sheetIds;
	std::vector<std::string> filenames;

//	NLMISC::CSheetId::buildIdVector(sheetIds, filenames, extension);
	buildFileVector(filenames,filespec);

	// if there s no file, nothing to do
//	if (sheetIds.empty())
	if (filenames.empty())
		return;

	// display the table header line
	fprintf(Outf,"FILE");
	for (unsigned i=0;i<fields.size();i++)
		fprintf(Outf,"%s%s",SEPARATOR, fields[i]._name.c_str());
	fprintf(Outf,"\n");

	UFormLoader *formLoader = NULL;
	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<UForm> form;


	for (uint j = 0; j < filenames.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/filenames.size(),j,filenames.size(), (filenames.size()-j)*(last-start)/j/1000);
		}

		//std::string p = NLMISC::CPath::lookup (filenames[j], false, false);
		std::string p = filenames[j];
		if (p.empty()) continue;

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = UFormLoader::createLoader ();
		}

		// Load the form with given sheet id
//		form = formLoader->loadForm (sheetIds[j].toString().c_str ());
		form = formLoader->loadForm (filenames[j].c_str ());
		if (form)
		{
			// the form was found so read the true values from George
			std::string s;
			fprintf(Outf,"%s",CFile::getFilenameWithoutExtension(filenames[j]));
			for (unsigned i=0;i<fields.size();i++)
			{
				UFormElm::TWhereIsValue where;
				bool result=form->getRootNode ().getValueByName(s,fields[i]._name.c_str(),fields[i]._evaluated,&where);
				if (!result)
				{
					if (fields[i]._evaluated)
						s="ERR";
					else switch(where)
					{
						case UFormElm::ValueForm: s="ValueForm"; break;
						case UFormElm::ValueParentForm: s="ValueParentForm"; break;
						case UFormElm::ValueDefaultDfn: s="ValueDefaultDfn"; break;
						case UFormElm::ValueDefaultType: s="ValueDefaultType"; break;
						default: s="ERR";
					}
				}
				else
				{
					// add quotes round strings
					std::string hold=s;
					s.erase();
					s='\"';
					for (unsigned i=0;i<hold.size();i++)
					{
						if (hold[i]=='\"')
							s+="\"\"";
						else
							s+=hold[i];
					}
					s+='\"';
				}
				fprintf(Outf,"%s%s", SEPARATOR, s);
			}
			fprintf(Outf,"\n");

		}
	}

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		UFormLoader::releaseLoader (formLoader);
		WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// housekeeping
//	sheetIds.clear ();
	filenames.clear ();

	fields.clear();
}


void executeScriptBuf(char *txt)
{
	/*
		note: the text buffer is assumed to end with 2 consecutive ASCIIZ terminators (0x00)
	*/
	char *command, *commandend;
	char *args;
	char *ptr=txt;

	while (*ptr)
	{
		// skip white space
		while (*ptr==' ' || *ptr=='\t' || *ptr=='\r' || *ptr=='\n') ptr++;

		// drop anchor at start of text and look for the next blank
		command=ptr;
		while (*ptr && *ptr!=' ' && *ptr!='\t' && *ptr!='\r' && *ptr !='\n') ptr++;

		// drop a bookmark and skip to the start of the args
		commandend=ptr;
		while (*ptr==' ' || *ptr=='\t') ptr++;

		// drop a bookmark and look for the end of line
		args=ptr;
		while (*ptr && *ptr!='\n' && *ptr!='\r') ptr++;
		while (ptr[-1]==' ' || ptr[-1]=='\t') ptr--;

		// add terminators to the args and command (note that this is destructive!!
		*commandend=0;
		*ptr=0;
		ptr++;

		if (command[0]=='/' || command[0]==0)
		{
			// this is a comment or the end of file line so ignore it
		}
		else if (strcmpi(command,"DFNPATH")==0)
		{
			//CPath::getPathContent(args,true,false,true,files);
			CPath::addSearchPath(args, true, false); // for the dfn files
		}
		else if (strcmpi(command,"PATH")==0)
		{
			files.clear();
			CPath::getPathContent(args,true,false,true,files);
			CPath::addSearchPath(args, true, false); // for the dfn files
		}
		else if (strcmpi(command,"OUTPUT")==0)
		{
			setOutputFile(args);
		}
		else if (strcmpi(command,"FIELD")==0)
		{
			addField(args);
		}
		else if (strcmpi(command,"SOURCE")==0)
		{
			addSource(args);
		}
		else if (strcmpi(command,"SCANFILES")==0)
		{
			scanFiles(args);
		}
		else if (strcmpi(command,"SCRIPT")==0)
		{
			executeScriptFile(args);
		}
		else
		{
			fprintf(stderr,"Unknown command: %s %s",command,args);
		}
	}
}

void executeScriptFile(const char *filename)
{
	FILE *inf=fopen(filename,"rb");
	if (inf==0)
	{
		fprintf(stderr,"script file not found: %s\n",filename);
		return;
	}
	int filelen=_filelength(fileno(inf));
	int result;
	char *buf=(char *)malloc(filelen+2);
	if (buf==0)
	{
		fclose(inf);
		fprintf(stderr,"failed to allocate memory buffer for script file: %s\n",filename);
		return;
	}
	result=fread(buf,1,filelen,inf);
	fclose(inf);
	if (result==filelen)
	{
		buf[filelen]=0;
		buf[filelen+1]=0;
		executeScriptBuf(buf);
	}
	else
		fprintf(stderr,"unknown error while reading file: %s\n",filename);

	free(buf);
}
//
/*
void	explode(const string &input, const string &separators, vector<string> &fields)
{
	fields.clear();
	uint	pos = 0;
	string	sep = separators+"\n\r";

	while (pos < input.size() && pos != string::npos)
	{
		uint	next = input.find_first_of(sep, pos);
		if (next == string::npos)
		{
			fields.push_back(input.substr(pos));
			return;
		}
		fields.push_back(input.substr(pos, next-pos));
		pos = next+1;
		if (input[next] == '\r' || input[next] == '\n')
			return;
	}
}
*/
void	loadSheetPath()
{
	if (inputSheetPathLoaded)
		return;

	NLMISC::createDebug();
	NLMISC::WarningLog->addNegativeFilter( "CPath::insertFileInMap" );

	CPath::addSearchPath(inputSheetPath, true, false);

	vector<string>	files;
	CPath::getPathContent (inputSheetPath, true, false, true, files);

	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		string& filename = files[i];
		string& filebase = CFile::getFilenameWithoutExtension(filename);
		inputSheetPathContent[filebase] = filename;
	}

	inputSheetPathLoaded = true;
}


/*
 *
 */
void fillFromDFN( UFormLoader *formLoader, set<string>& dfnFields, UFormDfn *formDfn, const string& rootName, const string& dfnFilename )
{
	uint i;
	for ( i=0; i!=formDfn->getNumEntry(); ++i )
	{
		string entryName, rootBase;
		formDfn->getEntryName( i, entryName );
		rootBase = rootName.empty() ? "" : (rootName+".");

		UFormDfn::TEntryType entryType;
		bool array;
		formDfn->getEntryType( i, entryType, array );
		switch ( entryType )
		{
			case UFormDfn::EntryVirtualDfn:
			{
				CSmartPtr<UFormDfn> subFormDfn = formLoader->loadFormDfn( (entryName + ".dfn").c_str() );
				if ( ! subFormDfn )
					nlwarning( "Can't load virtual DFN %s", entryName.c_str() );
				else
					fillFromDFN( formLoader, dfnFields, subFormDfn, rootBase + entryName, entryName + ".dfn" );
				break;
			}
			case UFormDfn::EntryDfn:
			{
				UFormDfn *subFormDfn;
				if ( formDfn->getEntryDfn( i, &subFormDfn) )
				{
					string filename;
					formDfn->getEntryFilename( i, filename );
					fillFromDFN( formLoader, dfnFields, subFormDfn, rootBase + entryName, filename ); // recurse
				}
				break;
			}
			case UFormDfn::EntryType:
			{
				dfnFields.insert( rootBase + entryName );
				//nlinfo( "DFN entry: %s (in %s)", (rootBase + entryName).c_str(), dfnFilename.c_str() );
				break;
			}
		}
	}
}


/*
 * Clear the form to reuse it (and all contents below node)
 */
void clearSheet( CForm *form, UFormElm* node )
{
	((CFormElm*)node)->clean();
	form->clean();
}


/*
 * - Remove CSV carriage returns.
 * - Ensure there is no non-ascii char (such as Excel's special blank crap), set them to ' '.
 */
void eraseCarriageReturnsAndMakeBlankNonAsciiChars( string& s )
{
	const char CR = '\n';
	string::size_type p = s.find( CR );
	while ( (p=s.find( CR )) != string::npos )
		s.erase( p, 1 );
	for ( p=0; p!=s.size(); ++p )
	{
		uint8& c = (uint8&)s[p]; // ensure the test is unsigned
		if ( c > 127 )
		{
			//nldebug( "Blanking bad char %u in '%s'", c, s.c_str() );
			s[p] = ' ';
		}
	}
}


string OutputPath;


/*
 * CSV -> Georges
 */
void	convertCsvFile( const string &file, bool generate, const string& sheetType )
{
	char			lineBuffer[2048];
	FILE			*s;

	vector<string>	fields;
	vector<string>	args;

	if ((s = fopen(file.c_str(), "r")) == NULL)
	{
		fprintf(stderr, "Can't find file %s to convert", file.c_str());
		return;
	}

	loadSheetPath();

	UFormLoader *formLoader = UFormLoader::createLoader ();
	NLMISC::CSmartPtr<CForm> form;
	NLMISC::CSmartPtr<UFormDfn> formDfn;


	fgets(lineBuffer, 2048, s);
	explode(lineBuffer, SEPARATOR, fields);

	vector<bool> activeFields( fields.size(), true );

	// Load DFN (generation only)
	set<string> dfnFields;
	if ( generate )
	{
		formDfn = formLoader->loadFormDfn( (sheetType + ".dfn").c_str() );
		if ( ! formDfn )
			nlerror( "Can't find DFN for %s", sheetType.c_str() );
		fillFromDFN( formLoader, dfnFields, formDfn, "", sheetType );

		// Display missing fields and check fields against DFN
		uint i;
		for ( i=1; i!=fields.size(); ++i )
		{
			eraseCarriageReturnsAndMakeBlankNonAsciiChars( fields[i] );
			if ( fields[i].empty() )
			{
				nlinfo( "Skipping field #%u (empty)", i );
				activeFields[i] = false;
			}
			else if ( nlstricmp( fields[i], "parent" ) == 0 )
			{
				strlwr( fields[i] ); // non-const version
			}
			else
			{
				set<string>::iterator ist = dfnFields.find( fields[i] );
				if ( ist == dfnFields.end() )
				{
					nlinfo( "Skipping field #%u (%s, not found in %s DFN)", i, fields[i].c_str(), sheetType.c_str() );
					activeFields[i] = false;
				}
			}
		}
		for ( i=1; i!=fields.size(); ++i )
		{
			if ( activeFields[i] )
				nlinfo( "Selected field: %s", fields[i].c_str() );
		}
	}

	string addExtension = "." + sheetType;
	uint dirmapLetterIndex = ~0;
	bool dirmapLetterBackward = false;
	vector<string> dirmapDirs;
	string dirmapSheetCode;
	bool WriteEmptyProperties = false, WriteSheetsToDisk = true;
	bool ForceInsertParents = false;

	if ( generate )
	{
		// Get the directory mapping
		try
		{
			CConfigFile dirmapcfg;
			dirmapcfg.load( sheetType + "_dirmap.cfg" );

			if ( OutputPath.empty() )
			{
				CConfigFile::CVar *path = dirmapcfg.getVarPtr( "OutputPath" );
				if ( path )
					OutputPath = path->asString();
				if ( ! OutputPath.empty() )
				{
					if ( OutputPath[OutputPath.size()-1] != '/' )
						OutputPath += '/';
					else if ( ! CFile::isDirectory( OutputPath ) )
						nlwarning( "Output path does not exist" );
				}
			}

			CConfigFile::CVar *letterIndex1 = dirmapcfg.getVarPtr( "LetterIndex" );
			if ( letterIndex1 && letterIndex1->asInt() > 0 )
			{
				dirmapLetterIndex = letterIndex1->asInt() - 1;

				CConfigFile::CVar *letterWay = dirmapcfg.getVarPtr( "LetterWay" );
				dirmapLetterBackward = (letterWay && (letterWay->asInt() == 1));
				
				CConfigFile::CVar dirs = dirmapcfg.getVar( "Directories" );
				for ( sint idm=0; idm!=dirs.size(); ++idm )
				{
					dirmapDirs.push_back( dirs.asString( idm ) );
					nlinfo( "Directory: %s", dirmapDirs.back().c_str() );
					if ( ! CFile::isExists( OutputPath + dirmapDirs.back() ) )
					{
						CFile::createDirectory( OutputPath + dirmapDirs.back() );
					}
					else
					{
						if ( ! CFile::isDirectory( OutputPath + dirmapDirs.back() ) )
						{
							nlwarning( "Already existing but not a directory!" );
						}
					}
				}

				nlinfo( "Mapping letter #%u (%s) of sheet name to directory", dirmapLetterIndex + 1, dirmapLetterBackward?"backward":"forward" );
			}
			
			CConfigFile::CVar *sheetCode = dirmapcfg.getVarPtr( "AddSheetCode" );
			if ( sheetCode )
				dirmapSheetCode = sheetCode->asString();
			nlinfo( "Sheet code: %s", dirmapSheetCode.c_str() );

			if ( ! dirmapLetterBackward )
				dirmapLetterIndex += dirmapSheetCode.size();

			CConfigFile::CVar *wep = dirmapcfg.getVarPtr( "WriteEmptyProperties" );
			if ( wep )
				WriteEmptyProperties = (wep->asInt() == 1);
			nlinfo( "Write empty properties mode: %s", WriteEmptyProperties ? "ON" : "OFF" );

			CConfigFile::CVar *wstd = dirmapcfg.getVarPtr( "WriteSheetsToDisk" );
			if ( wstd )
				WriteSheetsToDisk = (wstd->asInt() == 1);
			nlinfo( "Write sheets to disk mode: %s", WriteSheetsToDisk ? "ON" : "OFF" );

			CConfigFile::CVar *fiparents = dirmapcfg.getVarPtr( "ForceInsertParents" );
			if ( fiparents )
				ForceInsertParents = (fiparents->asInt() == 1);
			nlinfo( "Force insert parents mode: %s", ForceInsertParents ? "ON" : "OFF" );
		}
		catch ( EConfigFile& e )
		{
			nlwarning( "Problem in directory mapping: %s", e.what() );
		}
		

		nlinfo( "Using output path: %s", OutputPath.c_str() );
		nlinfo( "Press a key to generate *.%s", sheetType.c_str() );
		getch();
		nlinfo( "Generating...." );

	}
	else
		nlinfo("Updating modifications (only modified fields are updated)");

	set<string> newSheets;
	uint nbNewSheets = 0, nbModifiedSheets = 0, nbUnchangedSheets = 0, nbWritten = 0;
	while (!feof(s))
	{
		lineBuffer[0] = '\0';
		fgets(lineBuffer, 2048, s);
		explode(lineBuffer, SEPARATOR, args);

		if (args.size() < 1)
			continue;

		eraseCarriageReturnsAndMakeBlankNonAsciiChars( args[0] );

		// Skip empty lines
		if ( args[0].empty() || (args[0] == string(".")+sheetType) )
			continue;

		//nldebug( "%s: %u", args[0].c_str(), args.size() );
		string& filebase = dirmapSheetCode + args[0];
		strlwr( filebase );
		string filename, dirbase;
		bool isNewSheet;

		// Locate existing sheet
		map<string, string>::iterator it = inputSheetPathContent.find( CFile::getFilenameWithoutExtension( filebase ) );
		if ( it == inputSheetPathContent.end() )
		{
			// Not found
			if ( ! generate )
			{
				if ( ! filebase.empty() )
				{
					nlwarning( "Sheet %s not found", filebase.c_str( )); 
					continue;
				}
			}			
			else
			{
				// Load template sheet
				filename = strlwr( static_cast<const string&>(filebase) );
				form = (CForm*)formLoader->loadForm( (string("_empty.") + sheetType).c_str() );
				if (form == NULL)
				{
					nlerror( "Can't load sheet _empty.%s", sheetType.c_str() );
				}

				// Deduce directory from sheet name
				if ( dirmapLetterIndex != ~0 )
				{
					if ( dirmapLetterIndex < filebase.size() )
					{
						uint letterIndex;
						char c;
						if ( dirmapLetterBackward )
							letterIndex = filebase.size() - 1 - (CFile::getExtension( filebase ).size()+1) - dirmapLetterIndex;
						else
							letterIndex = dirmapLetterIndex;
						c = tolower( filebase[letterIndex] );
						vector<string>::const_iterator idm;
						for ( idm=dirmapDirs.begin(); idm!=dirmapDirs.end(); ++idm )
						{
							if ( (! (*idm).empty()) && (tolower((*idm)[0]) == c) )
							{
								dirbase = (*idm) + "/";
								break;
							}
						}
						if ( idm==dirmapDirs.end() )
						{
							nlinfo( "Directory mapping not found for %s (index %u)", filebase.c_str(), letterIndex );
							dirbase = ""; // put into root
						}
					}
					else
					{
						nlerror( "Can't map directory with letter #%u, greater than size of %s + code", dirmapLetterIndex, filebase.c_str() );
					}
				}

				nlinfo( "New sheet: %s", filebase.c_str() );
				++nbNewSheets;
				if ( ! newSheets.insert( filebase ).second )
					nlwarning( "Found duplicate sheet: %s", filebase.c_str() );
				isNewSheet = true;
			}
		}
		else // an existing sheet was found
		{

			// Load sheet (skip if failed)
			dirbase = "";
			filename = (*it).second; // whole path
			form = (CForm*)formLoader->loadForm( filename.c_str() );
			if (form == NULL)
			{
				nlwarning( "Can't load sheet %s", filename.c_str() );
				continue;
			}

			isNewSheet = false;
		}

		bool displayed = false;
		bool isModified = false;
		uint i;
		for ( i=1; i<args.size() && i<fields.size(); ++i )
		{
			const string	&var = fields[i];
			string			&val = args[i];

			eraseCarriageReturnsAndMakeBlankNonAsciiChars( val );

			// Skip column with inactive field (empty or not in DFN)
			if ( (! activeFields[i]) )
				continue;

			// Skip setting of empty cell except if required
			if ( (! WriteEmptyProperties) && val.empty() )
				continue;

			// Special case for parent sheet
			if ( var == "parent" ) // already case-lowered
			{
				vector<string> parentVals;
				explode( val, ":", parentVals );
				if ( (parentVals.size() == 1) && (parentVals[0].empty()) )
					parentVals.clear();
				if ( (isNewSheet || ForceInsertParents) && (! parentVals.empty()) )
				{
					// This is slow. Opti: insertParent() should have an option to do it without loading the form
					CSmartPtr<CForm> parentForm = (CForm*)formLoader->loadForm( val.c_str() );
					if ( ! parentForm )
						nlwarning( "Can't load parent form %s", val.c_str() );
					else
					{
						for ( uint p=0; p!=parentVals.size(); ++p )
							form->insertParent( p, val.c_str(), parentForm );
						nldebug( "Inserted %u parent(s)", parentVals.size() );
					}
				}
				// NOTE: Changing the parent is not currently implemented!
				continue;
			}

			if ( ! val.empty() )
			{
				if (val[0] == '"')
					val.erase(0, 1);
				if (val.size()>0 && val[val.size()-1] == '"')
					val.resize(val.size()-1);

				if (val == "ValueForm" ||
					val == "ValueParentForm" ||
					val == "ValueDefaultDfn" ||
					val == "ValueDefaultType" ||
					val == "ERR")
					continue;
			}

			if ( ! isNewSheet )
			{
				string	test;
				if ( form->getRootNode().getValueByName(test, var.c_str()) && (test == val) )
				{
					continue;
				}
			}

			//nldebug( "%s: %s '%s'", args[0].c_str(), var.c_str(), val.c_str() );
			form->getRootNode().setValueByName(val.c_str(), var.c_str());

			if ( ! isNewSheet )
			{
				isModified = true;
				if (!displayed)
					nldebug("in %s:", filename.c_str());
				displayed = true;
				nldebug("%s = %s", var.c_str(), val.c_str());
			}
		}

		if ( ! isNewSheet )
		{
			if ( isModified )
				++nbModifiedSheets;
			else
				++nbUnchangedSheets;
		}

		// Write sheet
		if ( isNewSheet || displayed )
		{
			if ( WriteSheetsToDisk )
			{
				++nbWritten;
				string path = isNewSheet ? OutputPath : "";
				string ext = (filename.find( addExtension ) == string::npos) ? addExtension : "";
				COFile	output( path + dirbase + filename + ext );
				form->write(output, true);
			}
			clearSheet( form, &form->getRootNode() );
		}
	}

	nlinfo( "%u sheets processed (%u new, %u modified, %u unchanged - %u written)", nbNewSheets+nbModifiedSheets+nbUnchangedSheets, nbNewSheets, nbModifiedSheets, nbUnchangedSheets, nbWritten );
	UFormLoader::releaseLoader (formLoader);
}

//
void	usage(char *argv0, FILE *out)
{
	fprintf(out, "\n");
	fprintf(out, "Syntax: %s [-p <sheet path>] [-s <field_separator>] [-g <sheet type>] [-o <output path>] [<script file name> | <csv file name>]", argv0);
	fprintf(out, "(-g = generate sheet files, needs template sheet _empty.<sheet type> and <sheet type>_dirmap.cfg in the current folder");
	fprintf(out, "\n");
	fprintf(out, "Script commands:\n");
	fprintf(out, "\tDFNPATH\t\t<search path for george dfn files>\n");
	fprintf(out, "\tPATH\t\t<search path for files to scan>\n");
	fprintf(out, "\tOUTPUT\t\t<output file>\n");
	fprintf(out, "\tFIELD\t\t<field in george file>\n");
	fprintf(out, "\tSOURCE\t\t<field in george file>\n");
	fprintf(out, "\tSCANFILES\t[+<text>|-<text>[...]]\n");
	fprintf(out, "\tSCRIPT\t\t<script file to execute>\n");
	fprintf(out, "\n");
}

int main(int argc, char* argv[])
{
	bool generate = false;
	string sheetType;

	// parse command line
	uint	i;
	for (i=1; (sint)i<argc; i++)
	{
		const char	*arg = argv[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
			case 'p':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <sheet path> after -p option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				inputSheetPath = argv[i];
				break;
			case 's':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <field_separator> after -s option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				SEPARATOR = argv[i];
				break;
			case 'g':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <sheetType> after -g option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				generate = true;
				sheetType = string(argv[i]);
				break;
			case 'o':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <output path> after -o option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				OutputPath = string(argv[i]);
				break;
			default:
				fprintf(stderr, "Unrecognized option '%c'\n", arg[1]);
				usage(argv[0], stderr);
				exit(0);
				break;
			}
		}
		else
		{
			if (CFile::getExtension(arg) == "csv")
			{
				inputCsvFiles.push_back(arg);
			}
			else
			{
				inputScriptFiles.push_back(arg);
			}
		}
	}

	if (inputScriptFiles.empty() && inputCsvFiles.empty())
	{
		fprintf(stderr, "Missing input script file or csv file\n");
		usage(argv[0], stderr);
		exit(0);
	}

	for (i=0; i<inputScriptFiles.size(); ++i)
		executeScriptFile(inputScriptFiles[i].c_str());

	for (i=0; i<inputCsvFiles.size(); ++i)
		convertCsvFile(inputCsvFiles[i], generate, sheetType);

	fprintf(stderr,"\nDone.\n");
	getch();
	return 0;
}

