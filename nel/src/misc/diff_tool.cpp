/** \file diff_tool.cpp
 *
 * $Id: diff_tool.cpp,v 1.2 2003/10/20 16:10:17 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/diff_tool.h"
#include "nel/misc/path.h"

using namespace NLMISC;
using namespace std;

namespace STRING_MANAGER
{

uint64 makePhraseHash(const TPhrase &phrase)
{
	ucstring text;
	text = phrase.Parameters;
	for (uint i=0; i<phrase.Clauses.size(); ++i)
	{
		text += phrase.Clauses[i].Conditions;
		text += phrase.Clauses[i].Identifier;
		text += phrase.Clauses[i].Text;
	}

	return CI18N::makeHash(text);
}




bool parseHashFromComment(const ucstring &comments, uint64 &hashValue)
{
	string str = comments.toString();

	string::size_type pos = str.find("HASH_VALUE ");
	if (pos == string::npos)
		return false;

	string hashStr = str.substr(pos + 11, 16);

	hashValue = CI18N::stringToHash(hashStr);
	return true;
}




bool loadStringFile(const std::string filename, vector<TStringInfo> &stringInfos, bool forceRehash, ucchar openMark, ucchar closeMark, bool specialCase)
{
/*	uint8 *buffer = 0;
	uint	size;

	try
	{
		CIFile fp(filename);
		size = fp.getFileSize();
		buffer = new uint8[size];
		fp.serialBuffer(buffer, size);
	}
	catch(Exception &e)
	{
		nlinfo("Can't open file [%s] (%s)\n", filename.c_str(), e.what());
		return true;
	}
*/
/*	FILE *fp = fopen(filename.c_str(), "rb");

	if (fp == NULL)
	{
		nlinfo("Can't open file [%s]\n", filename.c_str());
		if (buffer != 0)
			delete [] buffer;
		return true;
	}

	// move to end of file
	fseek(fp, 0, SEEK_END);

	fpos_t	pos;
	fgetpos(fp, &pos);

	uint8 *buffer = new uint8[uint(pos)];

	rewind(fp);
	uint size = fread(buffer, 1, uint(pos), fp);
	fclose (fp);
*/
	ucstring text;

	CI18N::readTextFile(filename, text, false, false, true);
//	CI18N::readTextBuffer(buffer, size, text);
//	delete [] buffer;

	// ok, parse the file now.
	ucstring::const_iterator first(text.begin()), last(text.end());
	std::string lastLabel("nothing");

	while (first != last)
	{
		TStringInfo si;
		CI18N::skipWhiteSpace(first, last, &si.Comments);

		if (first == last)
		{
			// check if there is only swap command remaining in comment
			if (si.Comments.find(ucstring("// DIFF SWAP ")) != ucstring::npos)
			{
				stringInfos.push_back(si);
			}
			break;
		}

		if (!CI18N::parseLabel(first, last, si.Identifier))
		{
			nlwarning("DT: Fatal : Invalid label will reading %s after %s\n", filename.c_str(), lastLabel.c_str());
			return false;
		}
		lastLabel = si.Identifier;

		CI18N::skipWhiteSpace(first, last, &si.Comments);

		if (!CI18N::parseMarkedString(openMark, closeMark, first, last, si.Text))
		{
			nlwarning("DT: Fatal : Invalid text value reading %s for label %s\n", filename.c_str(), lastLabel.c_str());
			return false;
		}

		if (specialCase)
		{
			CI18N::skipWhiteSpace(first, last, &si.Comments);

			if (!CI18N::parseMarkedString(openMark, closeMark, first, last, si.Text2))
			{
				nlwarning("DT: Fatal : Invalid text2 value reading %s for label %s\n", filename.c_str(), lastLabel.c_str());
				return false;
			}

		}

		if (forceRehash || !parseHashFromComment(si.Comments, si.HashValue))
		{
			// compute the hash value from text.
			si.HashValue = CI18N::makeHash(si.Text);
//			nldebug("Generating hash for %s as %s", si.Identifier.c_str(), CI18N::hashToString(si.HashValue).c_str());
		}
		else
		{
//			nldebug("Comment = [%s]", si.Comments.toString().c_str());
//			nldebug("Retreiving hash for %s as %s", si.Identifier.c_str(), CI18N::hashToString(si.HashValue).c_str());
		}
		stringInfos.push_back(si);
	}


	// check identifier uniqueness
	{
		bool error = false;
		set<string>	unik;
		set<string>::iterator it;
		for (uint i=0; i<stringInfos.size(); ++i)
		{
			it = unik.find(stringInfos[i].Identifier); 
			if (it != unik.end())
			{
				nlwarning("DT: loadStringFile : identifier '%s' exist twice", stringInfos[i].Identifier.c_str() );
				error = true;
			}
			else
				unik.insert(stringInfos[i].Identifier);
				
		}
		if (error)
			return false;
	}

	return true;
}


ucstring prepareStringFile(const vector<TStringInfo> &strings, bool removeDiffComments)
{
	ucstring diff;

	vector<TStringInfo>::const_iterator first(strings.begin()), last(strings.end());
	for (; first != last; ++first)
	{
		ucstring str;
		const TStringInfo &si = *first;
		string comment = si.Comments.toString();
		vector<string>	lines;
		explode(comment, "\n", lines, true);

		uint i;
		for (i=0; i<lines.size(); ++i)
		{
			if (removeDiffComments)
			{
				if (lines[i].find("// DIFF ") != string::npos)
				{
					lines.erase(lines.begin()+i);
					--i;
					continue;
				}
			}
			if (lines[i].find("// INDEX ") != string::npos)
			{
				lines.erase(lines.begin()+i);
				--i;
			}
		}

		comment.erase();
		for (i=0; i<lines.size(); ++i)
		{
			comment += lines[i] + "\n";
		}
		si.Comments = ucstring(comment);

		str = si.Comments;
		if (!si.Identifier.empty() || !si.Text.empty())
		{
			// add hash value comment if needed
			if (si.Comments.find(ucstring("// HASH_VALUE ")) == ucstring::npos)
				str += ucstring("// HASH_VALUE ") + CI18N::hashToString(si.HashValue)+ nl;
			str += ucstring("// INDEX ") + NLMISC::toString("%u", first-strings.begin())+ nl;
			str += si.Identifier + '\t';
			str += CI18N::makeMarkedString('[', ']', si.Text) + nl + nl;
		}

//		nldebug("Adding string [%s]", str.toString().c_str());
		diff += str;
	}

	return diff;
}


bool readPhraseFile(const std::string &filename, vector<TPhrase> &phrases, bool forceRehash)
{
	ucstring doc;

	CI18N::readTextFile(filename, doc, false, false, true);

//	CI18N::writeTextFile("test.txt", doc, false);

/*	uint8 *buffer = 0;
	uint	size;
*/

/*	try
	{
		CIFile fp(filename);
		size = fp.getFileSize();
		buffer = new uint8[size];
		fp.serialBuffer(buffer, size);
	}
	catch(Exception &e)
	{
		nlinfo("Can't open file [%s] (%s)\n", filename.c_str(), e.what());
		return true;
	}
*/
/*	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp == 0)
	{
		nlinfo("Could't not open %s\n", filename.c_str());
		return true;
	}
	uint size = CFile::getFileSize(fp);
	uint8 *buffer = new uint8[size];
	fread(buffer, 1, size, fp);
	fclose(fp);
*/
/*	CI18N::readTextBuffer(buffer, size, doc);
	delete [] buffer;
*/
	std::string lastRead("nothing");

	ucstring::const_iterator first(doc.begin()), last(doc.end());
	while (first != last)
	{
		TPhrase	phrase;
		// parse the phrase
		CI18N::skipWhiteSpace(first, last, &phrase.Comments);

		if (first == last)
			break;
		if (!CI18N::parseLabel(first, last, phrase.Identifier))
		{
			nlwarning("DT: Error parsing phrase identifier after %s\n", lastRead.c_str());
			return false;
		}
		lastRead = phrase.Identifier;
		CI18N::skipWhiteSpace(first, last, &phrase.Comments);
		if (!CI18N::parseMarkedString('(', ')', first, last, phrase.Parameters))
		{
			nlwarning("DT: Error parsing parameter list for phrase %s\n", phrase.Identifier.c_str());
			return false;
		}
		CI18N::skipWhiteSpace(first, last, &phrase.Comments);
		if (first == last || *first != '{')
		{
			nlwarning("DT: Error parsing block opening '{' in phase %s\n", phrase.Identifier.c_str());
			return false;
		}
		++first;

		ucstring temp;

		while (first != last && *first != '}')
		{
			TClause	clause;
			// append the comment preread at previous pass
			clause.Comments = temp;
			temp.erase();
			// parse the clauses
			CI18N::skipWhiteSpace(first, last, &clause.Comments);
			if (first == last)
			{
				nlwarning("DT: Found end of file in non closed block for phrase %s\n", phrase.Identifier.c_str());
				return false;
			}

			if (*first == '}')
				break;

			// skip the conditionnal expression
			ucstring cond;
			while (first != last && *first == '(')
			{
				if (!CI18N::parseMarkedString('(', ')', first, last, cond))
				{
					nlwarning("DT: Error parsing conditionnal expression in phrase %s, clause %u\n", phrase.Identifier.c_str(), phrase.Clauses.size()+1);
					return false;
				}
				clause.Conditions += "(" + cond + ") ";
				CI18N::skipWhiteSpace(first, last, &clause.Comments);
			}
			if (first == last)
			{
				nlwarning("DT: Found end of file in non closed block for phrase %s\n", phrase.Identifier.c_str());
				return false;
			}
			// read the idnetifier (if any)
			CI18N::parseLabel(first, last, clause.Identifier);
			CI18N::skipWhiteSpace(first, last, &temp);
			// read the text
			if (CI18N::parseMarkedString('[', ']', first, last, clause.Text))
			{
				// the last read comment is for this clause.
				clause.Comments += temp;
				temp.erase();
			}
			else
			{
				nlwarning("DT: Error reading text for clause %u (%s) in  phrase %s\n", 
					phrase.Clauses.size()+1, 
					clause.Identifier.c_str(), 
					phrase.Identifier.c_str());
				return false;

			}

			phrase.Clauses.push_back(clause);
		}
		CI18N::skipWhiteSpace(first, last);
		if (first == last || *first != '}')
		{
			nlwarning("DT: Missing block clogin tag '}' in phrase %s\n", phrase.Identifier.c_str());
			return false;
		}
		++first;

		// handle hash value.
		if (forceRehash || !parseHashFromComment(phrase.Comments, phrase.HashValue))
		{
			// the hash is not in the comment, compute it.
			phrase.HashValue = makePhraseHash(phrase);
		}

		phrases.push_back(phrase);
	}

	// check identifier uniqueness
	{
		bool error = false;
		set<string>	unik;
		set<string>::iterator it;
		for (uint i=0; i<phrases.size(); ++i)
		{
			it = unik.find(phrases[i].Identifier); 
			if (it != unik.end())
			{
				nlwarning("DT: loadStringFile : identifier '%s' exist twice", phrases[i].Identifier.c_str() );
				error = true;
			}
			else
				unik.insert(phrases[i].Identifier);
		}
		if (error)
			return false;
	}

	return true;
}
ucstring tabLines(uint nbTab, const ucstring &str)
{
	ucstring ret;
	ucstring tabs;

	for (uint i =0; i<nbTab; ++i)
		tabs.push_back('\t');

	ret = tabs;
	ucstring::const_iterator first(str.begin()), last(str.end());
	for (; first != last; ++first)
	{
		ret += *first;
		if (*first == '\n')
			ret += tabs;
	}

	while (ret[ret.size()-1] == '\t')
		ret = ret.substr(0, ret.size()-1);

	return ret;
}

ucstring preparePhraseFile(const vector<TPhrase> &phrases, bool removeDiffComments)
{
	ucstring ret;
	vector<TPhrase>::const_iterator first(phrases.begin()), last(phrases.end());
	for (; first != last; ++first)
	{
		const TPhrase &p = *first;

		if (removeDiffComments)
		{
			string comment = p.Comments.toString();
			vector<string>	lines;
			explode(comment, "\n", lines, true);

			uint i;
			for (i=0; i<lines.size(); ++i)
			{
				if (lines[i].find("// DIFF ") != string::npos)
				{
					lines.erase(lines.begin()+i);
					--i;
				}
			}

			comment.erase();
			for (i=0; i<lines.size(); ++i)
			{
				comment += lines[i] + "\n";
			}
			p.Comments = ucstring(comment);
		}
		ret += p.Comments;

		if (!p.Identifier.empty() || !p.Clauses.empty())
		{
			if (p.Comments.find(ucstring("// HASH_VALUE ")) == ucstring::npos)
			{
				// add the hash value.
				ret += ucstring("// HASH_VALUE ")+CI18N::hashToString(p.HashValue) + nl;
			}
			ret += p.Identifier + " ("+p.Parameters + ")" + nl;
			ret += '{';
			ret += nl;
			for (uint i=0; i<p.Clauses.size(); ++i)
			{
				const TClause &c = p.Clauses[i];
				if (!c.Comments.empty())
				{
					ucstring comment = tabLines(1, c.Comments);
					ret += comment; // + '\r'+'\n';
				}
				if (!c.Conditions.empty())
				{
					ucstring cond = tabLines(1, c.Conditions);
					ret += cond + nl;
				}
				ret += '\t';
				ucstring text = CI18N::makeMarkedString('[', ']', c.Text);
				text = tabLines(3, text);
				// remove begin tabs
				text = text.substr(3);
				ret += '\t' + c.Identifier + '\t' + text + nl + nl;
			}
			ret += '}';
		}
		ret += nl + nl;
	}

	return ret;
}

bool loadExcelSheet(const string filename, TWorksheet &worksheet, bool checkUnique)
{
	// Yoyo: must test with CIFile because can be packed into a .bnp on client...
	CIFile	fp;
	if(!fp.open(filename))
	{
		nldebug("DT: Can't open file [%s]\n", filename.c_str());
		return true;
	}
	fp.close();

	ucstring str;
	CI18N::readTextFile(filename, str, false, false);

	if (!readExcelSheet(str, worksheet, checkUnique))
		return false;

	return true;
}

bool readExcelSheet(const ucstring &str, TWorksheet &worksheet, bool checkUnique)
{
	vector<ucstring> lines;
	ucstring::size_type pos = 0;
	ucstring::size_type lastPos = 0;
	while ((pos = str.find(nl, lastPos)) != ucstring::npos)
	{
		ucstring line = str.substr(lastPos, pos - lastPos);
		if (!line.empty())
		{
//			nldebug("Found line : [%s]", line.toString().c_str());
			lines.push_back(line);
		}
		lastPos = lastPos + 2 + line.size();
	}

	if (lastPos != str.size() -2)
	{
		ucstring line = str.substr(lastPos, str.size() - 2);
		if (!line.empty())
		{
//			nldebug("Found line : [%s]", line.toString().c_str());
			lines.push_back(line);
		}
	}

//	nldebug("Found %u lines", lines.size());
	for (uint i=0; i<lines.size(); ++i)
	{
		vector<ucstring>	cells;
		ucstring			cell;

		ucstring::iterator	first(lines[i].begin()), last(lines[i].end());
		for (; first != last; ++first)
		{
			if (*first == '\t')
			{
//				nldebug("Found cell [%s]", cell.toString().c_str());
				cells.push_back(cell);
				cell.erase();
			}
			else if (*first == '"' && cell.empty())
			{
				// read a quoted field.
				first++;
				while (first != last && *first != '"' && (first+1) != last && *(first+1) != '"')
				{
					cell += *first++;
					if (first != last && *first == '"')
					{
						// skip this
						first++;
					}
				}
			}
			else
			{
				cell += *first;
			}
		}
//		nldebug("Found cell [%s]", cell.toString().c_str());
		/// append last cell
		cells.push_back(cell);
//		nldebug("Found %u cells in line %u", cells.size(), i);
		while (worksheet.ColCount < cells.size())
			worksheet.insertColumn(worksheet.ColCount);
		cells.resize(worksheet.ColCount);
		worksheet.insertRow(worksheet.Data.size(), cells);
	}

	// identifier uniqueness checking.
	if (checkUnique)
	{
		if (worksheet.size() > 0)
		{
			// look for the first non '* tagged' or 'DIFF_CMD' column
			uint nameCol = 0;
			while (nameCol < worksheet.ColCount && (*worksheet.getData(0, nameCol).begin() == uint16('*') || worksheet.getData(0, nameCol) == ucstring("DIFF_CMD")))
				++nameCol;

			if (nameCol < worksheet.ColCount )
			{
				// ok we can check unikness
				bool error = false;
				set<ucstring>	unik;
				set<ucstring>::iterator it;
				for (uint i=0; i<worksheet.size(); ++i)
				{
					it = unik.find(worksheet.getData(i, nameCol)); 
					if (it != unik.end())
					{
						nlwarning("DT: loadStringFile : identifier '%s' exist twice", worksheet.getData(i, nameCol).toString().c_str() );
						error = true;
					}
					else
						unik.insert(worksheet.getData(i, nameCol));
				}
				if (error)
					return false;
			}
		}
	}
	return true;
}

void makeHashCode(TWorksheet &sheet, bool forceRehash)
{
	if (!sheet.Data.empty())
	{
		TWorksheet::TRow::iterator it = find(sheet.Data[0].begin(), sheet.Data[0].end(), ucstring("*HASH_VALUE"));
		if (forceRehash || it == sheet.Data[0].end())
		{
			// we need to generate HASH_VALUE column !
			if (it == sheet.Data[0].end())
			{
				sheet.insertColumn(0);
				sheet.Data[0][0] = ucstring("*HASH_VALUE");
			}
			for (uint j=1; j<sheet.Data.size(); ++j)
			{
				ucstring str;
				for (uint k=1; k<sheet.ColCount; ++k)
				{
					if (sheet.Data[0][k].find(ucstring("*")) != 0 && sheet.Data[0][k].find(ucstring("DIFF ")) != 0)
					{
						str += sheet.Data[j][k];
					}
				}
				uint64 hash = CI18N::makeHash(str);
				sheet.Data[j][0] = CI18N::hashToString(hash);
			}
		}
		else
		{
			uint index = it - sheet.Data[0].begin();
			for (uint j=1; j<sheet.Data.size(); ++j)
			{
				ucstring &field = sheet.Data[j][index];

				if (!field.empty() && field[0] == '_')
					field = field.substr(1);
			}
		}
	}
}

ucstring prepareExcelSheet(const TWorksheet &worksheet)
{
	ucstring text;

	for (uint i=0; i<worksheet.Data.size(); ++i)
	{
		for (uint j=0; j<worksheet.Data[i].size(); ++j)
		{
			if (i > 0 && worksheet.Data[0][j] == ucstring("*HASH_VALUE"))
				text += "_";
			text += worksheet.Data[i][j];
			if (j != worksheet.Data[i].size()-1)
				text += '\t';
		}
		text += nl;
	}

	return text;
}







}	// namespace STRING_MANAGER

