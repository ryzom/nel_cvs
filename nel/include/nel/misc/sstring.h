/** \file sstring.h
 *
 * This file contains a string class derived from the STL string
 * The string compare functions of the class are case insensitive 
 *
 * The coding style is not CPU efficent - the routines are not designed for performance
 *
 * $Id: sstring.h,v 1.9 2004/04/28 18:50:44 boucher Exp $
 */


#ifndef NL_SSTRING_H
#define NL_SSTRING_H

#include <string>
#include <vector>
#include <stdio.h>

#include "nel/misc/stream.h"

namespace	NLMISC
{

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
	CSString()
	{
	}

	CSString(const char *s)
	{
		*(std::string *)this=s;
	}

	CSString(const std::string &s)
	{
		*(std::string *)this=s;
	}

	CSString(char c)
	{
		*(std::string *)this=c;
	}

	CSString(int i,const char *fmt="%d")
	{
		char buf[1024];
		sprintf(buf,fmt,i);
		*this=buf;
	}

	CSString(unsigned u,const char *fmt="%u")
	{
		char buf[1024];
		sprintf(buf,fmt,u);
		*this=buf;
	}

	CSString(double d,const char *fmt="%f")
	{
		char buf[1024];
		sprintf(buf,fmt,d);
		*this=buf;
	}

	CSString(const char *s,const char *fmt)
	{
		char buf[1024];
		sprintf(buf,fmt,s);
		*this=buf;
	}

	CSString(const std::string &s,const char *fmt)
	{
		char buf[1024];
		sprintf(buf,fmt,s.c_str());
		*this=buf;
	}

	/// Return the first character, or '\0' is the string is empty
	char operator*()
	{
		if (empty())
			return 0;
		return (*this)[0];
	}

	/// Return the n right hand most characters of a string
	char back() const
	{
		return (*this)[size()-1];
	}

	/// Return the n right hand most characters of a string
	CSString right(unsigned count) const
	{
		if (count>=size())
			return *this;
		return substr(size()-count);
	}

	/// Return the string minus the n right hand most characters of a string
	CSString rightCrop(unsigned count) const
	{
		if (count>=size())
			return CSString();
		return substr(0,size()-count);
	}

	/// Return the n left hand most characters of a string
	CSString left(unsigned count) const
	{
		return substr(0,count);
	}

	/// Return the string minus the n left hand most characters of a string
	CSString leftCrop(unsigned count) const
	{
		if (count>=size())
			return CSString();
		return substr(count);
	}

	/// Return sub string up to but not including first instance of given character
	CSString splitTo(char c) const
	{
		unsigned i;
		CSString result;
		for (i=0;i<size() && (*this)[i]!=c;++i)
			result+=(*this)[i];
		return result;
	}

	/// Return sub string up to but not including first instance of given character
	CSString splitTo(char c,bool truncateThis=false)
	{
		unsigned i;
		CSString result;
		for (i=0;i<size() && (*this)[i]!=c;++i)
			result+=(*this)[i];

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (i<size()-1)
				(*this)=substr(i+1);	// +1 to skip the separator character
			else
				clear();
		}

		return result;
	}

	/// Return sub string up to but not including first instance of given character
	CSString splitTo(const char *s,bool truncateThis=false)
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
					if (i<size()-1)
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

	/// Return sub string from character following first instance of given character on
	CSString splitFrom(char c) const
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

	/// Return sub string from character following first instance of given character on
	CSString splitFrom(const char *s) const
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

	/// Behave like a s strtok() routine, returning the sun string extracted from (and removed from) *this
	CSString strtok(const char *separators)
	{
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

	/// Return first word (blank separated)
	CSString firstWord(bool truncateThis=false)
	{
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

	CSString firstWordConst() const
	{
		return const_cast<CSString *>(this)->firstWord();
	}

	/// Return sub string up to but not including first instance of given character
	CSString tailFromFirstWord() const
	{
		CSString hold=*this;
		hold.firstWord(true);
		return hold;
	}

	/// Count the number of words (or quote delimited sub-strings) in a string
	unsigned countWords() const
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

	/// Count the number of words (or quote delimited sub-strings) in a string
	CSString word(unsigned idx) const
	{
		CSString hold=strip();

		for (unsigned count=0;count<idx;++count)
			hold=hold.tailFromFirstWord().strip();

		return hold.firstWord();
	}

	/// Return first word or quote-encompassed sub-string
	CSString firstWordOrWords(bool truncateThis=false)
	{
		CSString hold=strip();
		static const CSString emptyString;

		if (hold.empty())
			return emptyString;
		
		if (hold[0]!='\"')
			return firstWord(truncateThis);

		// the string is quote enclosed
		CSString result;
		unsigned i=1; // skip leading quote
		// copy from character following opening quote to char preceding closing quote (or end of string)
		while (i<hold.size() && hold[i]!='\"')
		{
			result+=hold[i];
			++i;
		}

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (i<size()-1)
				(*this)=substr(i+1);	// +1 to skip the closing quote
			else
				clear();
		}

		return result;
	}

	CSString firstWordOrWordsConst() const
	{
		return const_cast<CSString *>(this)->firstWordOrWords();
	}

	/// Return sub string up to but not including first instance of given character
	CSString tailFromFirstWordOrWords() const
	{
		CSString hold=*this;
		hold.firstWordOrWords(true);
		return hold;
	}

	/// Count the number of words (or quote delimited sub-strings) in a string
	unsigned countWordOrWords() const
	{
		unsigned count=0;
		CSString hold=strip();
		while (!hold.empty())
		{
			hold=hold.tailFromFirstWordOrWords().strip();
			++count;
		}
		return count;
	}

	/// Count the number of words (or quote delimited sub-strings) in a string
	CSString wordOrWords(unsigned idx) const
	{
		CSString hold=strip();
		
		for (unsigned count=0;count<idx;++count)
			hold=hold.tailFromFirstWordOrWords().strip();

		return hold.firstWordOrWords();
	}

	/// A handy utility routine for knowing if a character is a white space character or not
	static bool isWhiteSpace(char c) { return c==' ' || c=='\t' || c=='\n' || c=='\r' || c==26; }

	/// Return a copy of the string with leading and trainling spaces rmoved
	CSString strip() const
	{
		CSString result;
		int i,j;
		for (j=size()-1; j>=0 && isWhiteSpace((*this)[j]); --j) {}
		for (i=0;		 i<j  && isWhiteSpace((*this)[i]); ++i) {}
		for (; i<=j; ++i)
			result+=(*this)[i];
		return result;
	}

	/// Making an upper case copy of a string
	CSString toUpper() const
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

	/// Making a lower case copy of a string
	CSString toLower() const
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

	/// Replacing all occurences of one string with another
	CSString replace(const char *toFind,const char *replacement) const
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

	/// Find index at which a sub-string starts (case sensitive) - if sub-string not found then returns string::npos
	unsigned find(const char *toFind,unsigned startLocation=0) const
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
	unsigned findNS(const char *toFind,unsigned startLocation=0) const
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

	/// Return true if this contains given sub string
	bool contains(const char *toFind) const
	{
		return find(toFind)!=std::string::npos;
	}

	/// Return true if this contains given sub string
	bool contains(int character) const
	{
		for (const_iterator it=begin();it!=end();++it)
			if ((*it)==character)
				return true;

		return false;
	}

	/// A couple of handy atoi routines...
	template <class C> bool atoi(C& result) const
	{
		result=::atoi(c_str());
		return (result!=0 || *this=="0");
	}
	unsigned atoi() const
	{
		return ::atoi(c_str());
	}

	/// A couple of handy atof routines...
	template <class C> bool atof(C& result) const
	{
		result=::atof(c_str());
		return (result!=0 || *this=="0");
	}
	double atof() const
	{
		return ::atof(c_str());
	}

	/// Case insensitive string compare
	bool operator==(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())==0;
	}

	/// Case insensitive string compare
	bool operator==(const char* other) const
	{
		return stricmp(c_str(),other)==0;
	}

	/// Case insensitive string compare
	bool operator!=(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())!=0;
	}

	/// Case insensitive string compare
	bool operator!=(const char* other) const
	{
		return stricmp(c_str(),other)!=0;
	}

	/// Case insensitive string compare
	bool operator<=(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())<=0;
	}

	/// Case insensitive string compare
	bool operator<=(const char* other) const
	{
		return stricmp(c_str(),other)<=0;
	}

	/// Case insensitive string compare
	bool operator>=(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())>=0;
	}

	/// Case insensitive string compare
	bool operator>=(const char* other) const
	{
		return stricmp(c_str(),other)>=0;
	}

	/// Case insensitive string compare
	bool operator>(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())>0;
	}

	/// Case insensitive string compare
	bool operator>(const char* other) const
	{
		return stricmp(c_str(),other)>0;
	}

	/// Case insensitive string compare
	bool operator<(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())>0;
	}

	/// Case insensitive string compare
	bool operator<(const char* other) const
	{
		return stricmp(c_str(),other)>0;
	}

	/// Case insensitive string compare (useful for use as map keys, see less<CSString> below)
	bool icompare(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())<0;
	}
	
	/// Serial
	void serial( NLMISC::IStream& s )
	{
		s.serial( static_cast<std::string&>( *this ) );
	}
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


} // NLMISC


/*
 * The following code does not compile well with STLport/Linux, because it needs to
 * remap std to a different name. _STLP_BEGIN_NAMESPACE should do the trick,
 * however the symbol is not defined in public STLport headers.
 */
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
	bool operator()(const NLMISC::CSString& x, const NLMISC::CSString& y) const { return x.icompare(y); }
};


#endif // NL_SSTRING_H

/* End of sstring.h */
