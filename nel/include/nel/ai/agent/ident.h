/** \file ident.h
 * Sevral class for identification an objects fonctionality.
 *
 * $Id: ident.h,v 1.21 2002/06/17 14:16:54 chafik Exp $
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


#ifndef NL_IDENTIFIANT_H
#define NL_IDENTIFIANT_H
#include <stdlib.h>
#include "nel/misc/file.h"
#include "nel/ai/e/ai_exception.h" 
#include "nel/ai/agent/agent_string.h"
#include "nel/ai/agent/identtype.h"

namespace NLAIAGENT
{	
	class IRefrence;



	/**	
	  This class is a basic hi nomber manipulation. An index variant is an maxIndex*64 bit number.
	
	  The resolution of the number is define by the template argument indexMax. By default the number is initialed with a 128 bits resolution.
	  * \author Chafik sameh
	  * \author Robert Gabriel
	  * \author Nevrax France
	  * \date 2000
	*/
	template<class T = sint64, sint32 indexMax = maxIndex, T maxT = (T)-1>
	class CIndexVariant
	{		
	protected:
		///_Id is a table were is store the number.
		T _Id[indexMax];
	private:

		/**
		This fonction increment the _Id number by one. It's call from the ++ operator.
		If the number id overflow the NLAIE::CExceptionIndexError is emited.
		\param n The first index to increment.
		*/		

		void inc(int n)/// throw (NLAIE::CExceptionIndexError)
		{
			if (n == getMaxIndex())
			{
				// The highest Number as been reach.
				throw NLAIE::CExceptionIndexError();
			}

			if (_Id[n] == maxT)
			{
				// If an index is max, we increment the next index.
				inc(n+1);
				_Id[n] = 0;
			}
			else _Id[n]++;
		}
	public:		


		///Copy contructor
		CIndexVariant(const CIndexVariant<T,indexMax,maxT> &a)
		{
			memcpy(_Id,a._Id,getMaxIndex()*sizeof(T));
		}

		///fill with a table.
		CIndexVariant(const T *id)
		{
			for(sint32 i = 0; i < getMaxIndex(); i ++) 
			{
				_Id[i] = id[i];
			}
		}

		///Initializ by an value.
		CIndexVariant(T n)
		{
			/*for(sint32 i = 0; i < getMaxIndex(); i ++) 
			{
				_Id[i] = n;
			}*/
			*this = n;
		}


		///fill from read stream.
		CIndexVariant(NLMISC::IStream &is)
		{
			load(is);
		}

		///increment the number.
		const CIndexVariant &operator ++(int)/// throw (NLAIE::CExceptionIndexError)
		{
			try
			{
				inc(0);
			}
			catch(NLAIE::IException &e)
			{
				throw e;
			}
			return *this;
		}

		///\name comparison of two CIndexVariant.
		//@{
		bool operator == (const CIndexVariant<T,indexMax,maxT> &a) const
		{			
			for(sint32 i = 0; i < getMaxIndex(); i ++)
			{
				if(_Id[i] != a._Id[i]) return false;
			}
			
			return true;
		}
				
		bool operator < (const CIndexVariant<T,indexMax,maxT> &a) const
		{			
			for(sint32 i = a.getMaxIndex()-1; i >= 0; i --)
			{
				if(_Id[i] < a._Id[i]) return true;
			}			
			return false;
		}

		bool operator > (const CIndexVariant<T,indexMax,maxT> &a) const
		{			
			for(sint32 i = a.getMaxIndex()-1; i >= 0; i --)
			{
				if(_Id[i] > a._Id[i]) return true;
			}
			
			return false;
		}
		//@}

		///\name Binary method.
		//@{
		const CIndexVariant<T,indexMax,maxT> &operator |= (const CIndexVariant<T,indexMax,maxT> &a)
		{
			for(sint32 i = 0; i < getMaxIndex(); i ++)
			{
				_Id[i] |= a._Id[i];
			}		
			return *this;
		}
		const CIndexVariant<T,indexMax,maxT> &operator &= (const CIndexVariant<T,indexMax,maxT> &a)
		{
			for(sint32 i = 0; i < getMaxIndex(); i ++)
			{
				_Id[i] &= a._Id[i];
			}		
			return *this;
		}

		const CIndexVariant<T,indexMax,maxT> &operator ^= (const CIndexVariant<T,indexMax,maxT> &a)
		{
			for(sint32 i = 0; i < getMaxIndex(); i ++)
			{
				_Id[i] ^= a._Id[i];
			}		
			return *this;
		}
		//@}

		///\name Assignment method.
		//@{
		const CIndexVariant<T,indexMax,maxT> &operator = (const CIndexVariant<T,indexMax,maxT> &a)
		{
			for(sint32 i = 0; i < getMaxIndex(); i ++)
			{
				_Id[i] = a._Id[i];
			}		
			return *this;
		}

		const CIndexVariant<T,indexMax,maxT> &operator = (T a)
		{
			memset(_Id,0,getMaxIndex()*sizeof(T));
			_Id[0] = a;
			return *this;
		}

		const CIndexVariant<T,indexMax,maxT> &operator >>= (sint a)
		{
			T bits = 1;
			T r;
			T bitlen = 8*sizeof(T) - a;
			sint i;			

			bits <<= a;
			bits -= 1;
			_Id[0] >>= a;
			for(i = 1; i < getMaxIndex(); i ++)
			{			
				r = _Id[i] & bits;
				_Id[i] >>= a;
				r <<= bitlen;
				_Id[i - 1] |= r;
			}		

			return *this;
		}

		const CIndexVariant<T,indexMax,maxT> &operator <<= (sint a)
		{
			T bits = (1 << a) - 1;
			T r;
			T bitlen = 8*sizeof(T) - a;
			sint i;

			bits <<= (bitlen);

			_Id[getMaxIndex() - 1] <<= a;
			for(i = getMaxIndex() - 2 ; i >= 0; i --)
			{			
				r = _Id[i] & bits;
				r >>= bitlen;
				_Id[i] <<= a;
				_Id[i+1] |= r;
			}		

			return *this;
		}

		//@}	

		
		///saving the nomber in an output stream.
		void save(NLMISC::IStream &os)
		{			
			sint32 i;
			i = getMaxIndex();
			os.serial(i);
			for(i = 0; i < getMaxIndex(); i ++)
			{
				T n = _Id[i];				
				os.serial(n);				
			}
		}

		///loading the nomber from an input stream.
		void load(NLMISC::IStream &is)
		{
			sint32 max,i;

			is.serial(max);
			/*if(max != getMaxIndex()) 
			{
				throw(NLAIE::CExceptionIndexError());
			}*/

			for(i = 0; i < getMaxIndex(); i ++)
			{		
				T num;
				is.serial(num);
				_Id[i] = num;
			}

		}

		
		///Have a debug string.
		void getDebugString(std::string &str) const 
		{												
			char b[sizeof(T)*8 + 1];
			b[sizeof(T)*8] = 0;
			sint i;
			for(i = getMaxIndex() - 1 ; i >= 0; i --)
			{
				memset(b,'0',sizeof(T)*8);
				T s = _Id[i];
				sint base = 0;
				sint count = 0;
				for(base = 0; base < sizeof(T)*8; base ++)
				{
					if(s & 1)
					{
						b[sizeof(T)*8 - base - 1] = '1'; 
					}
					s >>= 1;
				}							
				str += std::string(b);
			}
		}
		

		sint32 getMaxIndex() const
		{
			return indexMax;
		}
	};

	

	/**
	Generator of unique ident for objects.
	
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/
	class CNumericIndex : public NLMISC::IStreamable
	{
	/*public:		
		///_I is a static IndexVariant were he have 0 at the initial time.
		static CIndexVariant<uint64,maxIndex,maxResolutionNumer> _I;
		static CIndexVariant<uint64,maxIndex,maxResolutionNumer> LocalServerID;
		static sint ShiftLocalServerMask;

	private:
		CIndexVariant<uint64,maxIndex,maxResolutionNumer> _Id;*/
	
	public:
		static CAgentNumber _I;		
	private:
		CAgentNumber _Id;
	public:
		///The constructor creat a new number by increment the _I numbre.
		CNumericIndex():_Id (_I++)
		{			
		}

		CNumericIndex(const CNumericIndex &id): _Id(id._Id)
		{
		}

		///copy constructor.
		CNumericIndex(const CAgentNumber &i):_Id (i)
		{			
		}		

		///init with a string.
		CNumericIndex(const char *id):_Id(id)
		{
		}


		///construct from a stream.
		CNumericIndex(NLMISC::IStream &is): _Id(is)
		{
		}

		void setTypeAt(uint64 t)
		{
			_Id.setTypeAt(t);
		}

		///\name comparison of two CIndexVariant.
		//@{
		bool operator == (const CNumericIndex &a) const
		{
			return _Id == a._Id;
		}

		bool operator < (const CNumericIndex &a) const
		{
			return _Id < a._Id;
		}
	
		bool operator > (const CNumericIndex &a) const
		{
			return _Id > a._Id;
		}
		//@}

		///saving the nomber in an output stream.
		void save(NLMISC::IStream &os)
		{
			_Id.save(os);
		}

		///loading the nomber from an input stream.
		void load(NLMISC::IStream &is)
		{
			_Id.load(is);
		}		
		
		void getDebugString(std::string &str) const
		{
			_Id.getDebugString(str);
		}

		/// \name NLMISC::IStreamable method.
		//@{
		virtual std::string	getClassName()
		{
			return std::string("<CNumericIndex>");
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if(f.isReading())
			{
				load(f);
			}
			else
			{				
				save(f);
			}

		}
		//@}

		
		const CAgentNumber &getId() const
		{
			return _Id;
		}
	};

	/**
		This class is the ident use in the Agent implementation. Not that an agent have IRefrence class as our base class and this class encapsulat an 		This class is the ident use in the Agent implementation. Not that an agent have IRefrence class as our base class and 
		this class encapsulat an IWordNumRef as an ident reference. So an agent have a pointer to an unique id same as IWordNumRef and the IWordNumRef class have a pointer to its own agent.

		This implementation allow us to share the creation of agent in an map whos is contane all agent created. This technic make simple the save, 
		load or serial procedure, because we have a method to save the pointer reference between agent.

		* \author Chafik sameh
		* \author Nevrax France
		* \date 2000
	*/
	class IWordNumRef: public NLAIC::IBasicInterface
	{
	public:
		IWordNumRef()
		{
		}

		IWordNumRef(IWordNumRef &)
		{
		}
		
		virtual operator const IRefrence *() const = 0;
		virtual const CNumericIndex &getNumIdent() const = 0;

		virtual ~IWordNumRef()
		{
		}

	};	

	/**
		This is an local version of the IWordNumRef.

		This class share agent in a static map.

		* \author Chafik sameh
		* \author Nevrax France
		* \date 2000
	*/
	class CLocWordNumRef: public IWordNumRef
	{			
	private:		
		///The map use an CNumericIndex as a key for the internal tree.
		typedef std::map<CNumericIndex ,IRefrence *> tMapRef;
		static NLMISC::CSynchronized<tMapRef> _LocRefence;

	private:
		
		///The iden of the class.
		CNumericIndex _Id;

		///_Stock define the cross reference between agent and this class.
		IRefrence *_Stock;

	public:
		///The class factory ident of the class.
		static const NLAIC::CIdentType *IdLocWordNumRef;

	public:
		///Construct object for an IRefrence agents objects.
		CLocWordNumRef(IRefrence *ref):_Stock(ref)
		{
			NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
			a.value().insert(tMapRef::value_type(_Id,ref));
		}
		
		/**
		Construct object with an CNumericIndex objects.

		This constroctor allow to reload an IRefrence defined with the CNumericIndex &ref.

		Exception: throw (NLAIE::CExceptionIndexHandeledError)
		*/
		CLocWordNumRef(const CNumericIndex &ref) :_Id(ref),_Stock(NULL)
		{			
			NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
			tMapRef::iterator Itr = a.value().find(_Id);
			if(Itr != a.value().end())
			{				
				_Stock = (*Itr).second;
			}
			else throw NLAIE::CExceptionIndexHandeledError();
		}

		CLocWordNumRef(const CNumericIndex &id,IRefrence *ref) :_Id(id),_Stock(ref)
		{
			NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
			tMapRef::iterator itr = a.value().find(_Id);
			if(itr != a.value().end())
			{
				if((*itr).second != _Stock)
				{
					return;
				}
				else
				{
					throw NLAIE::CExceptionIndexHandeledError();
				}
				a.value().erase(itr);
			}
			a.value().insert(tMapRef::value_type(_Id,_Stock));			
		}

		/**
		Copy Constructor.
		Exception: throw (NLAIE::CExceptionIndexHandeledError)
		*/
		CLocWordNumRef(const CLocWordNumRef &l) :_Id(l._Id),_Stock(NULL)
		{						
			NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
			tMapRef::iterator itr = a.value().find(_Id);
			if(itr != a.value().end())
			{
				_Stock = (*itr).second;
			}
			else throw NLAIE::CExceptionIndexHandeledError();
		}
		
		/**
		Copy Constructor.
		Exception: throw (NLAIE::CExceptionIndexHandeledError)
		*/

		CLocWordNumRef(NLMISC::IStream &is) :_Stock(NULL)
		{						
			load(is);			
		}

		CLocWordNumRef():_Stock(NULL)
		{
		}
		
		void getDebugString(std::string &t) const
		{			
			std::string i;
			_Id.getDebugString(i);
			t += NLAIC::stringGetBuild("CLocWordNumRef<%d>: _Id <%s>",this,i.c_str());
		}

		/// \name IBasicInterface method.
		//@{
		virtual const NLAIC::CIdentType &getType() const;		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CLocWordNumRef(*this);
			return m;
		}		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			NLAIC::IBasicInterface *m = new CLocWordNumRef();
			return m;
		}
		virtual void save(NLMISC::IStream &os)
		{
			_Id.save(os);
		}
		///Exception: throw (NLAIE::CExceptionIndexHandeledError)
		virtual void load(NLMISC::IStream &is)
		{			
			NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
			_Id.load(is);
			tMapRef::iterator Itr = a.value().find(_Id);
			if(Itr != a.value().end())
			{
				_Stock = (*Itr).second;
			}
			else throw NLAIE::CExceptionIndexHandeledError();
		}
		//@}

		virtual operator const IRefrence *() const
		{
			return (const IRefrence *)_Stock;
		}

		virtual const CNumericIndex &getNumIdent() const
		{
			return _Id;
		}

		void setTypeAt(uint64 t)
		{
			_Id.setTypeAt(t);
		}

		virtual ~CLocWordNumRef();		

	public:
		
		/// \name Map fonctionality method.		
		//@{

		/// Clear the map.
		static void clear();		

		/// Init the map.
		static void Init();
		/**
		Saving the mapping objects in the word.

		Saving mapping is to save all CNumericIndex shared in the map and its IRefrence class factory ident type.
		*/
		static void saveMapping(ostream &);
		/**
		Load the mapping objects in the word
		*/
		static bool loadMapping(NLMISC::IStream &);

		static IRefrence *getRef(const CNumericIndex &);		
		//@}
	};
	
	///Global method proghrammer have to call this function to initialise the word map reference.
	void initAgentLib();

	///Global method proghrammer have to call this function to release the word map reference.
	void releaseAgentLib();
}


#endif

