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
#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"

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
	bool _evaluated;
	CField(std::string name,bool eval) { _name=name; _evaluated=eval; }
};
std::vector<CField> fields;
std::vector<std::string> files;

vector<string>		inputScriptFiles;
vector<string>		inputCsvFiles;
string				inputSheetPath;
bool				inputSheetPathLoaded = false;
map<string, string>	inputSheetPathContent;


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
	fields.push_back(CField(name,true));
}

void addSource(char *name)
{
	fields.push_back(CField(name,false));
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
		fprintf(Outf,",%s",fields[i]);
	fprintf(Outf,"\n");

	NLGEORGES::UFormLoader *formLoader = NULL;
	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<NLGEORGES::UForm> form;


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
			formLoader = NLGEORGES::UFormLoader::createLoader ();
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
				NLGEORGES::UFormElm::TWhereIsValue where;
				bool result=form->getRootNode ().getValueByName(s,fields[i]._name.c_str(),fields[i]._evaluated,&where);
				if (!result)
				{
					if (fields[i]._evaluated)
						s="ERR";
					else switch(where)
					{
						case NLGEORGES::UFormElm::ValueForm: s="ValueForm"; break;
						case NLGEORGES::UFormElm::ValueParentForm: s="ValueParentForm"; break;
						case NLGEORGES::UFormElm::ValueDefaultDfn: s="ValueDefaultDfn"; break;
						case NLGEORGES::UFormElm::ValueDefaultType: s="ValueDefaultType"; break;
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
				fprintf(Outf,",%s",s);
			}
			fprintf(Outf,"\n");

		}
	}

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
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

void	loadSheetPath()
{
	if (inputSheetPathLoaded)
		return;

	CPath::addSearchPath(inputSheetPath, true, false);

	vector<string>	files;
	CPath::getPathContent (inputSheetPath, true, false, true, files);

	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		string	filename = files[i];
		string	filebase = CFile::getFilenameWithoutExtension(filename);
		inputSheetPathContent[filebase] = filename;
	}

	inputSheetPathLoaded = true;
}

void	convertCsvFile(const string &file)
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

	NLGEORGES::UFormLoader				*formLoader = NLGEORGES::UFormLoader::createLoader ();
	NLMISC::CSmartPtr<NLGEORGES::UForm> form;


	fgets(lineBuffer, 2048, s);
	explode(lineBuffer, ",", fields);

	nldebug("Updating modifications (only modified fields are updated)");

	while (!feof(s))
	{
		fgets(lineBuffer, 2048, s);
		explode(lineBuffer, ",", args);

		if (args.size() < 1)
			continue;

		string			&filebase = args[0];
		map<string, string>::iterator	it = inputSheetPathContent.find(filebase);
		if (it == inputSheetPathContent.end())
			continue;

		string			filename = (*it).second;
		form = formLoader->loadForm (filename.c_str());

		if (form == NULL)
			continue;

		bool	displayed = false;
		uint	i;
		for (i=1; i<args.size() && i<fields.size(); ++i)
		{
			const string	&var = fields[i];
			string			&val = args[i];

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

			string	test;
			if (form->getRootNode().getValueByName(test, var.c_str()) &&
				test == val)
				continue;

			form->getRootNode().setValueByName(val.c_str(), var.c_str());

			if (!displayed)
				nldebug("in %s:", filename.c_str());
			displayed = true;
			nldebug("%s = %s", var.c_str(), val.c_str());
		}

		if (displayed)
		{
			COFile	output(filename);
			form->write(output, true);
		}
	}

	NLGEORGES::UFormLoader::releaseLoader (formLoader);
}

//
void	usage(char *argv0, FILE *out)
{
	fprintf(out, "\n");
	fprintf(out, "Syntax: %s [-p <sheet path>] [<script file name> | <csv file name>]", argv0);
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
		convertCsvFile(inputCsvFiles[i]);

	fprintf(stderr,"\nDone.\n");
	getch();
	return 0;
}

