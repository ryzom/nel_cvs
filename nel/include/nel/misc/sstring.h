/** \file sstring.h
 *
 * This file contains a string class derived from the STL string
 * The string compare functions of the class are case insensitive 
 *
 * The coding style is not CPU efficent - the routines are not designed for performance
 *
 * $Id: sstring.h,v 1.15 2004/06/09 09:47:20 fleury Exp $
 */


#ifndef NL_SSTRING_H
#define NL_SSTRING_H

#include <string>
#include <vector>
#include <stdio.h>

#include "nel/misc/stream.h"

namespace	NLMISC
{

// advanced class declaration...
class CVectorSString;

/**
 * CSString : std::string with more functionalities and case insensitive compare
 *
 * \author Daniel Miller
 * \author Nevrax
 * \date 2003
 */
class CSString: public std::string
{
public:
	///	ctor
	CSString();
	///	ctor
	CSString(const char *s);
	/// ctor
	CSString(const std::string &s);
	///	ctor
	CSString(char c);
	///	ctor
	CSString(int i,const char *fmt="%d");
	///	ctor
	CSString(unsigned u,const char *fmt="%u");
	/// ctor
	CSString(double d,const char *fmt="%f");
	///	ctor
	CSString(const char *s,const char *fmt);
	///	ctor
	CSString(const std::string &s,const char *fmt);
	///	ctor
	CSString(const std::vector<NLMISC::CSString>& v,const std::string& separator="\n");

	/// Const [] operator
	std::string::const_reference operator[](std::string::size_type idx) const;
	/// Non-Const [] operator
	std::string::reference operator[](std::string::size_type idx);

	/// Return the first character, or '\0' is the string is empty
	char operator*();
	/// Return the n right hand most characters of a string
	char back() const;

	/// Return the n left hand most characters of a string
	CSString left(unsigned count) const;
	/// Return the n right hand most characters of a string
	CSString right(unsigned count) const;

	/// Return the string minus the n left hand most characters of a string
	CSString leftCrop(unsigned count) const;
	/// Return the string minus the n right hand most characters of a string
	CSString rightCrop(unsigned count) const;

	/// Return sub string up to but not including first instance of given character
	CSString splitTo(char c) const;
	/// Return sub string up to but not including first instance of given character
	CSString splitTo(char c,bool truncateThis=false,bool absorbSeparator=true);
	/// Return sub string up to but not including first instance of given character
	CSString splitTo(const char *s,bool truncateThis=false);

	/// Return sub string from character following first instance of given character on
	CSString splitFrom(char c) const;
	/// Return sub string from character following first instance of given character on
	CSString splitFrom(const char *s) const;

	/// Behave like a s strtok() routine, returning the sun string extracted from (and removed from) *this
	CSString strtok(const char *separators,
					bool useSmartExtensions=false,			// if true then match brackets etc (and refine with following args)
					bool useAngleBrace=false,				// - treat '<' and '>' as brackets
					bool useSlashStringEscape=true,			// - treat '\' as escape char so "\"" == '"'
					bool useRepeatQuoteStringEscape=true);	// - treat """" as '"')

	/// Return first word (blank separated) - can remove extracted word from source string
	CSString firstWord(bool truncateThis=false);
	///	Return first word (blank separated)
	CSString firstWordConst() const;
	/// Return sub string remaining after the first word
	CSString tailFromFirstWord() const;
	/// Count the number of words in a string
	unsigned countWords() const;
	/// Extract the given word 
	CSString word(unsigned idx) const;

	/// Return first word or quote-encompassed sub-string - can remove extracted sub-string from source string
	CSString firstWordOrWords(bool truncateThis=false,bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true);
	///	Return first word or quote-encompassed sub-string
	CSString firstWordOrWordsConst(bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;
	/// Return sub string following first word (or quote-encompassed sub-string)
	CSString tailFromFirstWordOrWords(bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;
	/// Count the number of words (or quote delimited sub-strings) in a string
	unsigned countWordOrWords(bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;
	/// Extract the given words (or quote delimited sub-strings)
	CSString wordOrWords(unsigned idx,bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;

	/// Return first line - can remove extracted line from source string
	CSString firstLine(bool truncateThis=false);
	///	Return first line
	CSString firstLineConst() const;
	/// Return sub string remaining after the first line
	CSString tailFromFirstLine() const;
	/// Count the number of lines in a string
	unsigned countLines() const;
	/// Extract the given line
	CSString line(unsigned idx) const;

	/// A handy utility routine for knowing if a character is a white space character or not
	static bool isWhiteSpace(char c); 
	///	Test whether character matches '{', '(','[' or '<' (the '<' test depends on the useAngleBrace parameter
	static bool isOpeningDelimiter(char c,bool useAngleBrace=false);
	///	Test whether character matches '}', ')',']' or '>' (the '>' test depends on the useAngleBrace parameter
	static bool isClosingDelimiter(char c,bool useAngleBrace=false);
	///	Test whether character matches '\'' or '\"'
	static bool isStringDelimiter(char c);
	///	Tests whether the character 'b' is the closing delimiter or string delimiter corresponding to character 'a'
	static bool isMatchingDelimiter(char a,char b); 

	/// A handy utility routine for knowing if a character is a valid component of a file name
	static bool isValidFileNameChar(char c); 
	/// A handy utility routine for knowing if a character is a valid first char for a keyword (a..z, '_')
	static bool isValidKeywordFirstChar(char c); 
	/// A handy utility routine for knowing if a character is a valid subsequent char for a keyword (a..z, '_', '0'..'9')
	static bool isValidKeywordChar(char c); 

	// a handy routine that tests whether a given string is a valid file name or not
	// "\"hello there\\bla\""	is valid 
	// "hello there\\bla"		is not valid - missing quotes
	// "\"hello there\"\\bla"	is not valid - text after quotes
	bool isValidFileName() const;
	// a second handy routine that tests whether a given string is a valid file name or not
	// equivalent to ('\"'+*this+'\"').isValidFileName()
	// "\"hello there\\bla\""	is not valid  - too many quotes
	// "hello there\\bla"		is valid
	bool isValidUnquotedFileName() const;
	// a handy routine that tests whether or not a given string is a valid keyword
	bool isValidKeyword() const;

	///	Search for the closing delimiter matching the opening delimiter at position 'startPos' in the 'this' string
	/// on error returns startPos
	uint32 findMatchingDelimiterPos(bool useAngleBrace,bool useSlashStringEscape,bool useRepeatQuoteStringEscape,uint32 startPos=0) const;

	///	Extract a chunk from the 'this' string
	/// if first non-blank character is a string delimiter or an opening delimiter then extracts up to the matching closing delimiter
	/// in all other cases an empty string is returned
	/// the return string includes the opening blank characters (it isn't stripped)
	CSString matchDelimiters(bool truncateThis=false,
							 bool useAngleBrace=false,				// treat '<' and '>' as brackets
							 bool useSlashStringEscape=true,		// treat '\' as escape char so "\"" == '"'
							 bool useRepeatQuoteStringEscape=true);	// treat """" as '"'

	/// copy out section of string up to separator character, respecting quotes (but not brackets etc)
	/// on error tail after returned string doesn't begin with valid separator character
	CSString splitToStringSeparator(	char separator,
										bool truncateThis=false,
										bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
										bool truncateSeparatorCharacter=false);	// if true tail begins after separator char

	/// copy out section of string up to separator character, respecting quotes, brackets, etc
	/// on error tail after returned string doesn't begin with valid separator character
	/// eg: splitToSeparator(','); - this might be used to split some sort of ',' separated input 
	CSString splitToSeparator(	char separator,
								bool truncateThis=false,
								bool useAngleBrace=false,				// treat '<' and '>' as brackets
								bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
								bool truncateSeparatorCharacter=false);	// if true tail begins after separator char

	/// copy out section of string up to any of a given set of separator characters, respecting quotes, brackets, etc
	/// on error tail after returned string doesn't begin with valid separator character
	/// eg: splitToOneOfSeparators(",;",true,false,false,true); - this might be used to split a string read from a CSV file
	CSString splitToOneOfSeparators(	const CSString& separators,
										bool truncateThis=false,
										bool useAngleBrace=false,				// treat '<' and '>' as brackets
										bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape=true,	// treat """" as '"'
										bool truncateSeparatorCharacter=false);	// if true tail begins after separator char

	/// Return true if the string is a single block encompassed by a pair of delimiters
	/// eg: "((a)(b)(c))" or "(abc)" return true wheras "(a)(b)(c)" or "abc" return false
	bool isDelimitedMonoBlock(	bool useAngleBrace=false,				// treat '<' and '>' as brackets
								bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
								bool useRepeatQuoteStringEscape=true	// treat """" as '"'
							 ) const;

	/// Return the sub string with leading and trailing delimiters ( such as '(' and ')' or '[' and ']' ) removed
	/// if the string isn't a delimited monoblock then the complete string is returned
	/// eg "((a)b(c))" returns "(a)b(c)" whereas "(a)b(c)" returns the identical "(a)b(c)"
	CSString stripBlockDelimiters(	bool useAngleBrace=false,				// treat '<' and '>' as brackets
									bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
									bool useRepeatQuoteStringEscape=true	// treat """" as '"'
								 ) const;

	/// Append the individual words in the string to the result vector
	/// retuns true on success
	bool splitWords(CVectorSString& result) const;

	/// Append the individual "wordOrWords" elements in the string to the result vector
	/// retuns true on success
	bool splitWordOrWords(CVectorSString& result,bool useSlashStringEscape=true,bool useRepeatQuoteStringEscape=true) const;

	/// Append the individual lines in the string to the result vector
	/// retuns true on success
	bool splitLines(CVectorSString& result) const;

	/// Append the separator-separated elements in the string to the result vector
	/// retuns true on success
	bool splitBySeparator(	char separator, CVectorSString& result,
							bool useAngleBrace=false,				// treat '<' and '>' as brackets
							bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
							bool useRepeatQuoteStringEscape=true	// treat """" as '"'
						 ) const;

	/// Append the separator-separated elements in the string to the result vector
	/// retuns true on success
	bool splitByOneOfSeparators(	const CSString& separators, CVectorSString& result,
									bool useAngleBrace=false,				// treat '<' and '>' as brackets
									bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
									bool useRepeatQuoteStringEscape=true	// treat """" as '"'
								 ) const;

	/// Return a copy of the string with leading and trainling spaces removed
	CSString strip() const;
	/// Return a copy of the string with leading spaces removed
	CSString CSString::leftStrip() const;
	/// Return a copy of the string with trainling spaces removed
	CSString CSString::rightStrip() const;

	/// Making an upper case copy of a string
	CSString toUpper() const;

	/// Making a lower case copy of a string
	CSString toLower() const;

	/// encapsulate string in quotes, adding escape characters as necessary
	CSString quote(	bool useSlashStringEscape=true,			// treat '\' as escape char so "\"" == '"'
					bool useRepeatQuoteStringEscape=true	// treat """" as '"'
					) const;

	/// strip delimiting quotes and clear through escape characters as necessary
	CSString unquote(bool useSlashStringEscape=true,		// treat '\' as escape char so "\"" == '"'
					 bool useRepeatQuoteStringEscape=true	// treat """" as '"'
					) const;

	/// Replacing all occurences of one string with another
	CSString replace(const char *toFind,const char *replacement) const;

	/// Find index at which a sub-string starts (case not sensitive) - if sub-string not found then returns string::npos
	unsigned find(const char *toFind,unsigned startLocation=0) const;

	/// Find index at which a sub-string starts (case NOT sensitive) - if sub-string not found then returns string::npos
	unsigned findNS(const char *toFind,unsigned startLocation=0) const;

	/// Return true if this contains given sub string
	bool contains(const char *toFind) const;

	/// Return true if this contains given sub string
	bool contains(int character) const;

	/// A handy atoi routine...
	int atoi() const;

	/// A handy atof routine...
	double atof() const;

	/// assignment operator
	CSString& operator=(const char *s);
	/// assignment operator
	CSString& operator=(const std::string &s);
	/// assignment operator
	CSString& operator=(char c);

	/// Case insensitive string compare
	bool operator==(const CSString &other) const;
	/// Case insensitive string compare
	bool operator==(const std::string &other) const;
	/// Case insensitive string compare
	bool operator==(const char* other) const;

	/// Case insensitive string compare
	bool operator!=(const CSString &other) const;
	/// Case insensitive string compare
	bool operator!=(const std::string &other) const;
	/// Case insensitive string compare
	bool operator!=(const char* other) const;

	/// Case insensitive string compare
	bool operator<=(const CSString &other) const;
	/// Case insensitive string compare
	bool operator<=(const std::string &other) const;
	/// Case insensitive string compare
	bool operator<=(const char* other) const;

	/// Case insensitive string compare
	bool operator>=(const CSString &other) const;
	/// Case insensitive string compare
	bool operator>=(const std::string &other) const;
	/// Case insensitive string compare
	bool operator>=(const char* other) const;

	/// Case insensitive string compare
	bool operator>(const CSString &other) const;
	/// Case insensitive string compare
	bool operator>(const std::string &other) const;
	/// Case insensitive string compare
	bool operator>(const char* other) const;

	/// Case insensitive string compare
	bool operator<(const CSString &other) const;
	/// Case insensitive string compare
	bool operator<(const std::string &other) const;
	/// Case insensitive string compare
	bool operator<(const char* other) const;

	/// Case insensitive string compare (useful for use as map keys, see less<CSString> below)
	bool icompare(const std::string &other) const;

	/// Serial
	void serial( NLMISC::IStream& s );
};


/*
 * Vector of CSString compatible with vector<string>
 */
class CVectorSString : public std::vector<CSString>
{
public:
	operator std::vector<std::string>& () { return reinterpret_cast<std::vector<std::string>&>(*this); }
	CVectorSString&	operator= ( const std::vector<std::string>& v ) { *this = reinterpret_cast<const CVectorSString&>(v); return *this; }
};


/*
 * Inlines
 */

inline CSString::CSString()
{
}

inline CSString::CSString(const char *s)
{
	*(std::string *)this=s;
}

inline CSString::CSString(const std::string &s)
{
	*(std::string *)this=s;
}

inline CSString::CSString(char c)
{
	*(std::string *)this=c;
}

inline CSString::CSString(int i,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,i);
	*this=buf;
}

inline CSString::CSString(unsigned u,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,u);
	*this=buf;
}

inline CSString::CSString(double d,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,d);
	*this=buf;
}

inline CSString::CSString(const char *s,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,s);
	*this=buf;
}

inline CSString::CSString(const std::string &s,const char *fmt)
{
	char buf[1024];
	sprintf(buf,fmt,s.c_str());
	*this=buf;
}

inline CSString::CSString(const std::vector<NLMISC::CSString>& v,const std::string& separator)
{
	for (uint32 i=0;i<v.size();++i)
	{
		if (i>0)
			*this+=separator;
		*this+=v[i];
	}
}

inline char CSString::operator*()
{
	if (empty())
		return 0;
	return (*this)[0];
}

inline char CSString::back() const
{
	return (*this)[size()-1];
}

inline CSString CSString::right(unsigned count) const
{
	if (count>=size())
		return *this;
	return substr(size()-count);
}

inline CSString CSString::rightCrop(unsigned count) const
{
	if (count>=size())
		return CSString();
	return substr(0,size()-count);
}

inline CSString CSString::left(unsigned count) const
{
	return substr(0,count);
}

inline CSString CSString::leftCrop(unsigned count) const
{
	if (count>=size())
		return CSString();
	return substr(count);
}

inline CSString CSString::splitTo(char c) const
{
	unsigned i;
	CSString result;
	for (i=0;i<size() && (*this)[i]!=c;++i)
		result+=(*this)[i];
	return result;
}

inline CSString CSString::splitTo(char c,bool truncateThis,bool absorbSeparator)
{
	unsigned i;
	CSString result;
	for (i=0;i<size() && (*this)[i]!=c;++i)
		result+=(*this)[i];

	// remove the result string from the input string if so desired
	if (truncateThis)
	{
		if (absorbSeparator)
			++i;
		if (i<size())
			(*this)=substr(i);
		else
			clear();
	}

	return result;
}

inline CSString CSString::splitTo(const char *s,bool truncateThis)
{
	unsigned i;
	CSString result;
	for (i=0;i<size();++i)
	{
		// perform a quick string compare
		int j;
		for (j=0;s[j]!=0 && s[j]==(&((*this)[i]))[j];++j)
		{
		}
		// if string compare matched then return result so far
		if (s[j]==0)
		{
			// remove the result string from the input string if so desired
			if (truncateThis)
			{
				if (i+1<size())
					(*this)=substr(i+1);	// +1 to skip the separator character
				else
					clear();
			}

			return result;
		}
		result+=(*this)[i];
	}
	// we didn't find the separator string so we're returning a copy of the whole string
	if (truncateThis)
		clear();
	return result;
}

inline CSString CSString::splitFrom(char c) const
{
	CSString result;
	std::string::const_iterator it;
	for (it=begin();it!=end() && *it!=c;++it)
	{}
	if (it!=end())
	{
		++it;
		for (;it!=end();++it)
			result+=*it;
	}
	return result;
}

inline CSString CSString::splitFrom(const char *s) const
{
	unsigned int i;
	CSString result;
	for (i=0;i<size();++i)
	{
		// perform a quick string compare
		unsigned int j;
		for (j=0;i+j<size() && s[j]!=0 && s[j]==(*this)[i+j];++j)
		{
		}
		// if string compare matched then build and return a result
		if (s[j]==0)
		{
			result=substr(i+j);
			return result;
		}
	}
	return result;
}

inline CSString CSString::strtok(	const char *separators,
									bool useSmartExtensions,			// if true then match brackets etc (and refine with following args)
									bool useAngleBrace,					// - treat '<' and '>' as brackets
									bool useSlashStringEscape,			// - treat '\' as escape char so "\"" == '"'
									bool useRepeatQuoteStringEscape)	// - treat """" as '"')
{
	if (useSmartExtensions)
	{
		return splitToOneOfSeparators(separators,true,useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,true);
	}

	unsigned int i;
	CSString result;

	// skip leading junk
	for (i=0;i<size();++i)
	{
		// look for the next character in the 'separator' character list supplied
		unsigned j;
		for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
		{}
		// if not found then we're at end of leading junk
		if (!separators[j])
			break;
	}

	// copy out everything up to the next separator character
	for (;i<size();++i)
	{
		// look for the next character in the 'separator' character list supplied
		unsigned j;
		for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
		{}
		// if not found then we're at end of leading junk
		if (separators[j])
			break;
		result+=(*this)[i];
	}

	// skip trailing junk
	for (;i<size();++i)
	{
		// look for the next character in the 'separator' character list supplied
		unsigned j;
		for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
		{}
		// if not found then we're at end of leading junk
		if (!separators[j])
			break;
	}

	// delete the treated bit from this string
	(*this)=substr(i);

	return result;
}

inline CSString CSString::firstWord(bool truncateThis)
{
	// idiot test to avoid accessing index 0 in empty strings
	if (empty())
		return CSString();

	CSString result;
	unsigned i=0;
	// skip white space
	for (i=0;i<size() && isWhiteSpace((*this)[i]);++i)
	{}

	if ( ((*this)[i]>='A' && (*this)[i]<='Z') || ((*this)[i]>='a' && (*this)[i]<='z') || 
		 ((*this)[i]>='0' && (*this)[i]<='9') || (*this)[i]=='_')
	{
		// copy out an alpha-numeric string
		for (;i<(*this).size() && 
			( ((*this)[i]>='A' && (*this)[i]<='Z') || ((*this)[i]>='a' && (*this)[i]<='z') || 
			  ((*this)[i]>='0' && (*this)[i]<='9') || (*this)[i]=='_')
			;++i)
			result+=(*this)[i];
	}
	else
	{
		// just take the first character of the input
		result=(*this)[i];
		++i;
	}

	// remove the result string from the input string if so desired
	if (truncateThis)
	{
		if (i<size())
			(*this)=substr(i);
		else
			clear();
	}

	return result;
}

inline CSString CSString::firstWordConst() const
{
	return const_cast<CSString *>(this)->firstWord();
}

inline CSString CSString::tailFromFirstWord() const
{
	CSString hold=*this;
	hold.firstWord(true);
	return hold;
}

inline unsigned CSString::countWords() const
{
	unsigned count=0;
	CSString hold=strip();
	while (!hold.empty())
	{
		hold=hold.tailFromFirstWord().strip();
		++count;
	}
	return count;
}

inline CSString CSString::word(unsigned idx) const
{
	CSString hold=strip();

	for (unsigned count=0;count<idx;++count)
		hold=hold.tailFromFirstWord().strip();

	return hold.firstWord();
}

inline CSString CSString::firstWordOrWords(bool truncateThis,bool useSlashStringEscape,bool useRepeatQuoteStringEscape)
{
	uint32 startPos;
	for (startPos=0;startPos<size();++startPos)
		if (!isWhiteSpace((*this)[startPos]))
			break;

	if (isStringDelimiter((*this)[startPos]))
	{
		uint32 endPos= findMatchingDelimiterPos(false,useSlashStringEscape,useRepeatQuoteStringEscape,startPos);
		CSString result=substr(startPos+1,endPos-startPos-1);
		if (truncateThis)
			*this=leftCrop(endPos+1);
		return result;
	}
	else
		return firstWord(truncateThis);
}

inline CSString CSString::firstWordOrWordsConst(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	return const_cast<CSString *>(this)->firstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape);
}

inline CSString CSString::tailFromFirstWordOrWords(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	CSString hold=*this;
	hold.firstWordOrWords(true,useSlashStringEscape,useRepeatQuoteStringEscape);
	return hold;
}

inline unsigned CSString::countWordOrWords(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	unsigned count=0;
	CSString hold=strip();
	while (!hold.empty())
	{
		hold=hold.tailFromFirstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape).strip();
		++count;
	}
	return count;
}

inline CSString CSString::wordOrWords(unsigned idx,bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	CSString hold=strip();
	
	for (unsigned count=0;count<idx;++count)
		hold=hold.tailFromFirstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape).strip();

	return hold.firstWordOrWords(useSlashStringEscape,useRepeatQuoteStringEscape);
}


inline CSString CSString::firstLine(bool truncateThis)
{
	return splitTo('\n',truncateThis);
}

inline CSString CSString::firstLineConst() const
{
	return const_cast<CSString *>(this)->firstLine();
}

inline CSString CSString::tailFromFirstLine() const
{
	CSString hold=*this;
	hold.firstLine(true);
	return hold;
}

inline unsigned CSString::countLines() const
{
	unsigned count=0;
	CSString hold=strip();
	while (!hold.empty())
	{
		hold=hold.tailFromFirstLine().strip();
		++count;
	}
	return count;
}

inline CSString CSString::line(unsigned idx) const
{
	CSString hold=strip();
	
	for (unsigned count=0;count<idx;++count)
		hold= hold.tailFromFirstLine().strip();

	return hold.firstLine();
}


inline bool CSString::isWhiteSpace(char c) 
{
	return c==' ' || c=='\t' || c=='\n' || c=='\r' || c==26; 
}

inline bool CSString::isOpeningDelimiter(char c,bool useAngleBrace)
{
	return c=='(' || c=='[' || c=='{' || (useAngleBrace && c=='<'); 
}

inline bool CSString::isClosingDelimiter(char c,bool useAngleBrace)
{
	return c==')' || c==']' || c=='}' || (useAngleBrace && c=='>'); 
}

inline bool CSString::isStringDelimiter(char c)
{
	return c=='\"' || c=='\''; 
}

inline bool CSString::isMatchingDelimiter(char a,char b) 
{
	return	(a=='(' && b==')') || (a=='[' && b==']') || 
			(a=='{' && b=='}') || (a=='<' && b=='>') || 
			(a=='\"' && b=='\"') || (a=='\'' && b=='\''); 
}

inline bool CSString::isValidFileNameChar(char c)
{
	if (c>='a' && c<='z') return true;
	if (c>='A' && c<='Z') return true;
	if (c>='0' && c<='9') return true;
	if (c=='_') return true;
	if (c==':') return true;
	if (c=='/') return true;
	if (c=='\\') return true;
	if (c=='.') return true;
	if (c=='#') return true;
	if (c=='-') return true;
	return false;
}

inline bool CSString::isValidKeywordFirstChar(char c)
{
	if (c>='a' && c<='z') return true;
	if (c>='A' && c<='Z') return true;
	if (c=='_') return true;
	return false;
}

inline bool CSString::isValidKeywordChar(char c)
{
	if (c>='a' && c<='z') return true;
	if (c>='A' && c<='Z') return true;
	if (c>='0' && c<='9') return true;
	if (c=='_') return true;
	return false;
}

inline bool CSString::isValidFileName() const
{
	if (empty())
		return false;

	if ((*this)[0]=='"')
	{
		if (!isDelimitedMonoBlock(false,false,false))
			return false;

		// iterate from size-2 to 1
		for (uint32 i=size()-1; --i;)
			if (!isValidFileNameChar((*this)[i]) && (*this)[i]!=' ')
				return false;
	}
	else
	{
		// iterate from size-1 to 0
		for (uint32 i=size(); i--;)
			if (!isValidFileNameChar((*this)[i]))
				return false;
	}
	return true;
}

inline bool CSString::isValidUnquotedFileName() const
{
	return (CSString('\"'+*this+'\"')).isValidFileName();
}

inline bool CSString::isValidKeyword() const
{
	if (empty())
		return false;

	if (!isValidKeywordFirstChar((*this)[0]))
		return false;

	// iterate from size-1 to 1
	for (uint32 i=size(); --i;)
		if (!isValidKeywordChar((*this)[i]))
			return false;

	return true;
}

inline uint32 CSString::findMatchingDelimiterPos(	bool useAngleBrace,
													bool useSlashStringEscape,
													bool useRepeatQuoteStringEscape,
													uint32 startPos ) const
{
	uint32 i=startPos;
	char openingDelimiter= (*this)[i];
	if (isOpeningDelimiter(openingDelimiter,useAngleBrace))
	{
		// deal with (), [], {} or <> type delimiters
		while (i<size())
		{
			++i;
			if(isMatchingDelimiter(openingDelimiter,(*this)[i]))
			{
				// this is it! we've found the matching quote so we're done
				break;
			}
			if (isOpeningDelimiter((*this)[i],useAngleBrace) || isStringDelimiter((*this)[i]))
			{
				uint32 j=i;
				i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
				if (j==i)
					return startPos;
				continue;
			}
			if (isClosingDelimiter((*this)[i],useAngleBrace))
			{
				// we've found a closing delimiter that doesn't match our opening delimiter so give up
				return startPos;
			}
		}
	}
	else if (isStringDelimiter(openingDelimiter))
	{
		// deal with "..." or '...' type delimiters
		while (i<size())
		{
			++i;
			if ((*this)[i]==openingDelimiter)
			{
				if (useRepeatQuoteStringEscape && (*this)[i+1]==openingDelimiter)
				{
					// we've found a "" pair and we're treating it as \" equivalent so skip an extra character
					++i;
					continue;
				}
				else
				{
					// this is it! we've found the matching quote so we're done
					break;
				}
			}
			if (useSlashStringEscape && (*this)[i]=='\\')
			{
				// we've found a '\' character so skip the next character, whatever it is
				++i;
				continue;
			}
		}
	}

	return i;
}

inline CSString CSString::matchDelimiters(	bool truncateThis,
											bool useAngleBrace,					// treat '<' and '>' as brackets
											bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
											bool useRepeatQuoteStringEscape)	// treat """" as '"'
{
	// skip white space
	uint32 startPos;
	for (startPos=0;startPos<size() && isWhiteSpace((*this)[startPos]);++startPos) {}

	// locate the matching brace
	uint32 matchPos=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,startPos);

	// if not found give up
	if (matchPos>=size())
	{
		return CSString();
	}

	// build the return string
	CSString result=left(matchPos+1);

	// if need be truncate '*this' before returning
	if (truncateThis)
	{
		*this=leftCrop(matchPos+1);
	}

	return result;
}

inline CSString CSString::splitToStringSeparator(	
											char separator,
											bool truncateThis,
											bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
											bool useRepeatQuoteStringEscape,	// treat """" as '"'
											bool truncateSeparatorCharacter)	// if true tail begins after separator char
{
	// iterate over our string
	uint32 i;
	for (i=0;i<size();++i)
	{
		char thisChar=(*this)[i];

		// if we've found the separator character then all's cool so break out of the loop
		if (thisChar==separator)
			break;

		// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
		if (isStringDelimiter(thisChar))
		{
			uint32 j=i;
			i=findMatchingDelimiterPos(false,useSlashStringEscape,useRepeatQuoteStringEscape,i);
			// if there was a problem then break here
			if (j==i)
				break;
			continue;
		}
	}

	// build the return string
	CSString result=left(i);

	// if need be truncate '*this' before returning
	if (truncateThis)
	{
		if (truncateSeparatorCharacter && separator==(*this)[i])
			++i;
		*this=leftCrop(i);
	}

	return result;
}

inline CSString CSString::splitToSeparator(	char separator,
											bool truncateThis,
											bool useAngleBrace,					// treat '<' and '>' as brackets
											bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
											bool useRepeatQuoteStringEscape,	// treat """" as '"'
											bool truncateSeparatorCharacter)	// if true tail begins after separator char
{
	// iterate over our string
	uint32 i;
	for (i=0;i<size();++i)
	{
		char thisChar=(*this)[i];

		// if we've found the separator character then all's cool so break out of the loop
		if (thisChar==separator)
			break;

		// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
		if (isOpeningDelimiter(thisChar,useAngleBrace) || isStringDelimiter(thisChar))
		{
			uint32 j=i;
			i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
			// if there was a problem then break here
			if (j==i)
				break;
			continue;
		}
	}

	// build the return string
	CSString result=left(i);

	// if need be truncate '*this' before returning
	if (truncateThis)
	{
		if (truncateSeparatorCharacter && separator==(*this)[i])
			++i;
		*this=leftCrop(i);
	}

	return result;
}

inline CSString CSString::splitToOneOfSeparators(	const CSString& separators,
													bool truncateThis,
													bool useAngleBrace,					// treat '<' and '>' as brackets
													bool useSlashStringEscape,			// treat '\' as escape char so "\"" == '"'
													bool useRepeatQuoteStringEscape,	// treat """" as '"'
													bool truncateSeparatorCharacter)	// if true tail begins after separator char
{
	// iterate over our string
	uint32 i;
	for (i=0;i<size();++i)
	{
		char thisChar=(*this)[i];

		// if we've found the separator character then all's cool so break out of the loop
		if (separators.contains(thisChar))
			break;

		// if we have a bracket or quote of any type then match to it's matching bracket, quote or whatever
		if (isOpeningDelimiter(thisChar,useAngleBrace) || isStringDelimiter(thisChar))
		{
			uint32 j=i;
			i=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape,i);
			// if there was a problem then break here
			if (j==i)
				break;
			continue;
		}
	}

	// build the return string
	CSString result=left(i);

	// if need be truncate '*this' before returning
	if (truncateThis)
	{
		if (truncateSeparatorCharacter && separators.contains((*this)[i]))
			++i;
		*this=leftCrop(i);
	}

	return result;
}

inline bool CSString::isDelimitedMonoBlock(	bool useAngleBrace,				// treat '<' and '>' as brackets
											bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
											bool useRepeatQuoteStringEscape	// treat """" as '"'
										 ) const
{
	if (empty())
		return false;
	uint32 matchPos=findMatchingDelimiterPos(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape);
	return (matchPos==size()-1 && isMatchingDelimiter((*this)[0],(*this)[matchPos]));
}

inline CSString CSString::stripBlockDelimiters(	bool useAngleBrace,				// treat '<' and '>' as brackets
												bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape	// treat """" as '"'
											 ) const
{
	if (isDelimitedMonoBlock(useAngleBrace,useSlashStringEscape,useRepeatQuoteStringEscape))
	{
		return substr(1,size()-2);
	}
	else
	{
		return *this;
	}
}

inline bool CSString::splitWords(CVectorSString& result) const
{
	CSString s=*this;
	while(!s.empty())
	{
		uint32 pre=s.size();
		result.push_back(s.firstWord(true));
		uint32 post=s.size();
		if (post>=pre)
			return false;
	}
	return true;
}

inline bool CSString::splitWordOrWords(CVectorSString& result,bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	CSString s=*this;
	while(!s.empty())
	{
		uint32 pre=s.size();
		result.push_back(s.firstWordOrWords(true,useSlashStringEscape,useRepeatQuoteStringEscape));
		uint32 post=s.size();
		if (post>=pre)
			return false;
	}
	return true;
}

inline bool CSString::splitLines(CVectorSString& result) const
{
	CSString s=*this;

	// make sure we deal with '\n\r' style carriage returns cleanly
	if (s.contains('\r'))
		s=s.replace("\r","");

	while(!s.empty())
	{
		uint32 pre=s.size();
		result.push_back(s.splitTo('\n',true));
		uint32 post=s.size();
		if (post>=pre)
			return false;
	}
	return true;
}

inline bool CSString::splitBySeparator(	char separator, CVectorSString& result,
										bool useAngleBrace,				// treat '<' and '>' as brackets
										bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
										bool useRepeatQuoteStringEscape	// treat """" as '"'
									 ) const
{
	CSString s=*this;
	while(!s.empty())
	{
		uint32 pre=s.size();
		result.push_back(s.splitToSeparator(separator,true,useAngleBrace,useSlashStringEscape,
											useRepeatQuoteStringEscape,true));
		uint32 post=s.size();
		if (post>=pre)
			return false;
	}
	return true;
}

inline bool CSString::splitByOneOfSeparators(	const CSString& separators, CVectorSString& result,
												bool useAngleBrace,				// treat '<' and '>' as brackets
												bool useSlashStringEscape,		// treat '\' as escape char so "\"" == '"'
												bool useRepeatQuoteStringEscape	// treat """" as '"'
											 ) const
{
	CSString s=*this;
	while(!s.empty())
	{
		uint32 pre=s.size();
		result.push_back(s.splitToOneOfSeparators(	separators,true,useAngleBrace,useSlashStringEscape,
													useRepeatQuoteStringEscape,true ));
		uint32 post=s.size();
		if (post>=pre)
			return false;
	}
	return true;
}

inline CSString CSString::strip() const
{
	CSString result;
	int i,j;
	for (j=size()-1; j>=0 && isWhiteSpace((*this)[j]); --j) {}
	for (i=0;		 i<j  && isWhiteSpace((*this)[i]); ++i) {}
	result=substr(i,j-i+1);
	return result;
}

inline CSString CSString::leftStrip() const
{
	CSString result;
	int i,j=size()-1;
	for (i=0; i<j  && isWhiteSpace((*this)[i]); ++i) {}
	result=substr(i,j-i+1);
	return result;
}

inline CSString CSString::rightStrip() const
{
	CSString result;
	int i=0,j;
	for (j=size()-1; j>=0 && isWhiteSpace((*this)[j]); --j) {}
	result=substr(i,j-i+1);
	return result;
}

inline CSString CSString::toUpper() const
{
	CSString result;
	std::string::const_iterator it;
	for (it=begin();it!=end();++it)
	{
		char c=(*it);
		if (c>='a' && c<='z')
			c^=('a'^'A');
		result+=c;
	}
	return result;
}

inline CSString CSString::toLower() const
{
	CSString result;
	std::string::const_iterator it;
	for (it=begin();it!=end();++it)
	{
		char c=(*it);
		if (c>='A' && c<='Z')
			c^=('a'^'A');
		result+=c;
	}
	return result;
}

inline CSString CSString::quote(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	CSString result;
	
	result+='\"';
	for (uint32 i=0;i<size();++i)
	{
		switch ((*this)[i])
		{
		case '\"': 
			if (useSlashStringEscape)
				result+="\\\"";
			else if (useRepeatQuoteStringEscape)
				result+="\"\"";
			else
				result+="\"";
			break;
		case '\\':
			if (useSlashStringEscape)
				result+="\\\\";
			else
				result+="\\";
			break;
		default:
			result+=(*this)[i];
		}
	}
	result+='\"';

	return result;
}

inline CSString CSString::unquote(bool useSlashStringEscape,bool useRepeatQuoteStringEscape) const
{
	CSString result=stripBlockDelimiters(false,useSlashStringEscape,useRepeatQuoteStringEscape);

	uint32 i,j;
	for (i=0,j=0;i<size();++i,++j)
	{
		if (useSlashStringEscape && result[i]=='\\')
			++i;
		else if (useRepeatQuoteStringEscape && (i+1<size()) && result[i]=='\"' && result[i+1]=='\"')
			++i;
		if (i<size())
			result[j]=result[i];
	}

	return result;
}

inline CSString CSString::replace(const char *toFind,const char *replacement) const
{
	// just bypass the problems that can cause a crash...
	if (toFind==NULL || *toFind==0)
		return *this;

	unsigned i,j;
	CSString result;
	for (i=0;i<size();)
	{
		// string compare toFind against (*this)+i ...
		for (j=0;toFind[j];++j)
			if ((*this)[i+j]!=toFind[j])
				break;
		// if strings were identical then j reffers to ASCIIZ terminator at end of 'toFind'
		if (toFind[j]==0)
		{
			if (replacement!=NULL)
				result+=replacement;
			i+=j;
		}
		else
		{
			result+=(*this)[i];
			++i;
		}
	}
	return result;
}

inline unsigned CSString::find(const char *toFind,unsigned startLocation) const
{
	const char *constStr = c_str();

	// just bypass the problems that can cause a crash...
	if (toFind==NULL || *toFind==0 || startLocation>=size())
		return std::string::npos;

	unsigned i,j;
	for (i=startLocation;i<size();++i)
	{
		// string compare toFind against (*this)+i ...
		for (j=0;toFind[j];++j)
			if ((i+j>=size()) || (*this)[i+j]!=toFind[j])
				break;
		// if strings were identical then we're done
		if (toFind[j]==0)
			return i;
	}
	return std::string::npos;
}

/// Find index at which a sub-string starts (case NOT sensitive) - if sub-string not found then returns string::npos
inline unsigned CSString::findNS(const char *toFind,unsigned startLocation) const
{
	const char *constStr = c_str();

	// just bypass the problems that can cause a crash...
	if (toFind==NULL || *toFind==0 || startLocation>=size())
		return std::string::npos;

	unsigned i,j;
	for (i=startLocation;i<size();++i)
	{
		// string compare toFind against (*this)+i ...
		for (j=0;toFind[j];++j)
			if ((i+j>=size()) || tolower(constStr[i+j])!=tolower(toFind[j]))
				break;
		// if strings were identical then we're done
		if (toFind[j]==0)
			return i;
	}
	return std::string::npos;
}

inline bool CSString::contains(const char *toFind) const
{
	return find(toFind)!=std::string::npos;
}

inline bool CSString::contains(int character) const
{
	for (const_iterator it=begin();it!=end();++it)
		if ((*it)==character)
			return true;

	return false;
}

inline int CSString::atoi() const
{
	return ::atoi(c_str());
}

inline double CSString::atof() const
{
	return ::atof(c_str());
}

inline CSString& CSString::operator=(const char *s)
{
	*(std::string *)this=s;
	return *this;
}

inline CSString& CSString::operator=(const std::string &s)
{
	*(std::string *)this=s;
	return *this;
}

inline CSString& CSString::operator=(char c)
{
	*(std::string *)this=c;
	return *this;
}

inline bool CSString::operator==(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())==0;
}

inline bool CSString::operator==(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())==0;
}

inline bool CSString::operator==(const char* other) const
{
	return stricmp(c_str(),other)==0;
}

inline bool CSString::operator!=(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())!=0;
}

inline bool CSString::operator!=(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())!=0;
}

inline bool CSString::operator!=(const char* other) const
{
	return stricmp(c_str(),other)!=0;
}

inline bool CSString::operator<=(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())<=0;
}

inline bool CSString::operator<=(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())<=0;
}

inline bool CSString::operator<=(const char* other) const
{
	return stricmp(c_str(),other)<=0;
}

inline bool CSString::operator>=(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())>=0;
}

inline bool CSString::operator>=(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())>=0;
}

inline bool CSString::operator>=(const char* other) const
{
	return stricmp(c_str(),other)>=0;
}

inline bool CSString::operator>(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())>0;
}

inline bool CSString::operator>(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())>0;
}

inline bool CSString::operator>(const char* other) const
{
	return stricmp(c_str(),other)>0;
}

inline bool CSString::operator<(const CSString &other) const
{
	return stricmp(c_str(),other.c_str())>0;
}

inline bool CSString::operator<(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())>0;
}

inline bool CSString::operator<(const char* other) const
{
	return stricmp(c_str(),other)>0;
}

inline std::string::const_reference CSString::operator[](std::string::size_type idx) const
{
	static char zero=0;
	if ((uint32)idx>size())
		return zero;
	return *(_M_start + idx);
}

inline std::string::reference CSString::operator[](std::string::size_type idx)
{
	static char zero=0;
	if ((uint32)idx>size())
		return zero;
	return *(_M_start + idx);
}

inline bool CSString::icompare(const std::string &other) const
{
	return stricmp(c_str(),other.c_str())<0;
}

inline void CSString::serial( NLMISC::IStream& s )
{
	s.serial( reinterpret_cast<std::string&>( *this ) );
}
/*
inline CSString operator+(const CSString& s0,char s1)
{
	return CSString(s0)+s1;
}

inline CSString operator+(const CSString& s0,const char* s1)
{
	return CSString(s0)+s1;
}

inline CSString operator+(const CSString& s0,const std::string& s1)
{
	return CSString(s0)+s1;
}
inline CSString operator+(const CSString& s0,const CSString& s1)
{
	return CSString(s0)+s1;
}
*/
inline CSString operator+(char s0,const CSString& s1)
{
	return CSString(s0)+s1;
}

inline CSString operator+(const char* s0,const CSString& s1)
{
	return CSString(s0)+s1;
}

inline CSString operator+(const std::string& s0,const CSString& s1)
{
	return CSString(s0)+s1;
}

} // NLMISC

// *** The following was commented out by Sadge because there were strange compilation/ link issues ***
// *** The '<' operator was implemented instead ***
//_STLP_BEGIN_NAMESPACE
//namespace std
//{
//	/*
//	 * less<CSString> is case insensitive
//	 */
//	template <>
//	struct less<NLMISC::CSString> : public std::binary_function<NLMISC::CSString, NLMISC::CSString, bool>
//	{
//		bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x.icompare(y); }
//	};
//} // std
//_STLP_END_NAMESPACE
//namespace std
//{

//	/*
//	 * less<CSString> is case insensitive
//	 */
//	template <>
//	struct less<NLMISC::CSString> : public std::binary_function<NLMISC::CSString, NLMISC::CSString, bool>
//	{
//		bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x.icompare(y); }
//	};
//} // std
//_STLP_END_NAMESPACE

/** 
  * Instead of overriding std::less, please use the following predicate. 
  * For example, declare your map as: 
  *   std::map<NLMISC::CSString, CMyDataClass, NLMISC::CUnsensitiveSStringLessPred> MyMap; 
  * Caution: a map declared without CUnsensitiveSStringLessPred will behave as a 
  * standard string map. 
  * 
  * \see also CUnsensitiveStrLessPred in misc/string_conversion.h 
  * for a similar predicate with std::string. 
  */ 
struct CUnsensitiveSStringLessPred : public std::less<NLMISC::CSString> 
{ 
	bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x < y; /*.icompare(y);*/ } 
}; 
 

#endif // NL_SSTRING_H

/* End of sstring.h */
