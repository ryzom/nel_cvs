/** \file lexsupport.cpp
 *
 * $Id: lexsupport.cpp,v 1.23 2002/08/21 13:58:33 lecroart Exp $
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

#include "nel/ai/script/ytable.h"
#include "nel/ai/script/lexsupport.h"
#include <math.h>
#include <list>
#include <map>
#include <string>

namespace NLAISCRIPT
{
	

	struct BaseDico
	{	
		int			Ident;
		CStringType	IdentName;


			BaseDico(CStringType S, int Id = IDENT)
			{
				Ident = Id;
				IdentName = S;							
			}
			int operator    <(const BaseDico & a) const
			{
				return IdentName.compare(a.IdentName) < 0;
			}

			int operator    >(const BaseDico & a) const
			{
				return IdentName.compare(a.IdentName) > 0;
			}

	};

/*%token	FROM 
%token	DEFINE	GROUP
%token	MESSAGERIE PROPERTY AGENT
%token	REGISTER  CONSTRUCTION DESTRUCTION MAESSAGELOOP
%token	WITH DO 
%token	END*/

	/*#define _MAX_CLES_DICO_	END_GRAMMAR - BEGIN_GRAMMAR
	BaseDico Table[_MAX_CLES_DICO_] = 
	{
		BaseDico("From",FROM),
		BaseDico("Define",DEFINE),
		BaseDico("Group",GROUP),
		BaseDico("IMessageBase",MESSAGERIE),
		BaseDico("Property",PROPERTY),
		BaseDico("Agent",AGENT),
		BaseDico("Register",REGISTER),
		BaseDico("Construction",CONSTRUCTION),
		BaseDico("Destruction",DESTRUCTION),
		BaseDico("MessageLoop",MAESSAGELOOP),
		
		BaseDico("With",WITH),
		BaseDico("Do",DO),
		BaseDico("End",END)
	};*/			

	typedef std::map<CStringType , long,std::less<CStringType> > Arbre;
	static Arbre DicoDeNom;

	void AddWordDico(const char *T, int tag)
	{
		DicoDeNom.insert(Arbre::value_type(T,tag) );
	};

	void InitDico()
	{		
		DicoDeNom.insert(Arbre::value_type("From",FROM) );
		DicoDeNom.insert(Arbre::value_type("Define",DEFINE) );
		DicoDeNom.insert(Arbre::value_type("Group",GROUP) );
		//DicoDeNom.insert(Arbre::value_type("IMessageBase",MESSAGERIE) );
		//DicoDeNom.insert(Arbre::value_type("Property",PROPERTY) );
		//DicoDeNom.insert(Arbre::value_type("Agent",AGENT) );
		DicoDeNom.insert(Arbre::value_type("Component",COMPONENT) );
		//DicoDeNom.insert(Arbre::value_type(_CONSTRUCTOR_,CONSTRUCTION) );
		DicoDeNom.insert(Arbre::value_type(_DESTRUCTOR_,DESTRUCTION) );
		//DicoDeNom.insert(Arbre::value_type(_SEND_,SEND) );
		//DicoDeNom.insert(Arbre::value_type(_RUN_,RUN) );
		DicoDeNom.insert(Arbre::value_type("MessageLoop",MAESSAGELOOP) );
		DicoDeNom.insert(Arbre::value_type("With",WITH) );
		DicoDeNom.insert(Arbre::value_type("Do",DO) );
		DicoDeNom.insert(Arbre::value_type("End",END) );
		DicoDeNom.insert(Arbre::value_type("If",IF) );
		DicoDeNom.insert(Arbre::value_type("MessageManager",MESSAGE_MANAGER) );		

		DicoDeNom.insert(Arbre::value_type("Local", LOCAL) );
		DicoDeNom.insert(Arbre::value_type("Static", STATIC) );
		DicoDeNom.insert(Arbre::value_type("Static", STATIC) );
		
		DicoDeNom.insert(Arbre::value_type("Begin",BEGINING) );
		
		DicoDeNom.insert(Arbre::value_type("As",AS) );
				
		DicoDeNom.insert(Arbre::value_type("Trigger",TRIGGER) );
		DicoDeNom.insert(Arbre::value_type("MessageCond",MSG) );
		DicoDeNom.insert(Arbre::value_type("PreCondition",PRECONDITION) );
		DicoDeNom.insert(Arbre::value_type("PostCondition",POSTCONDITION) );
		DicoDeNom.insert(Arbre::value_type("Goal",GOAL) );
		DicoDeNom.insert(Arbre::value_type("Steps",STEPS) );
		DicoDeNom.insert(Arbre::value_type("Comment",COMMENT) );
		DicoDeNom.insert(Arbre::value_type("UpdateEvery", UPDATEEVERY) );
		DicoDeNom.insert(Arbre::value_type("Priority", PRIORITY) );

		DicoDeNom.insert(Arbre::value_type("Return",RETURN) );
		DicoDeNom.insert(Arbre::value_type("MessageCollector",COLLECTOR) );
		DicoDeNom.insert(Arbre::value_type("defrule", RULE) );
		DicoDeNom.insert(Arbre::value_type("and", AND) );
		DicoDeNom.insert(Arbre::value_type("then",THEN) );
		DicoDeNom.insert(Arbre::value_type("new", NEW) );
		DicoDeNom.insert(Arbre::value_type("assert", IA_ASSERT) );
		DicoDeNom.insert(Arbre::value_type("deffuzzyrule",FUZZYRULE) );
		DicoDeNom.insert(Arbre::value_type("fuzzyruleset",FUZZYRULESET) );
		DicoDeNom.insert(Arbre::value_type("FuzzyVar", FUZZYVAR) );
		DicoDeNom.insert(Arbre::value_type("Fuzzy", FUZZY) );
		DicoDeNom.insert(Arbre::value_type("is", FIS) );
		DicoDeNom.insert(Arbre::value_type("sets", SETS) );
		DicoDeNom.insert(Arbre::value_type("Null",NILL) );
		     
		DicoDeNom.insert(Arbre::value_type("EXEC",EXEC) );
		DicoDeNom.insert(Arbre::value_type("ACHIEVE",ACHIEVE) );
		DicoDeNom.insert(Arbre::value_type("ASK",ASK) );
		DicoDeNom.insert(Arbre::value_type("BREAK",BREAK) );
		DicoDeNom.insert(Arbre::value_type("TELL",TELL) );
		DicoDeNom.insert(Arbre::value_type("KILL",KILL) );
		DicoDeNom.insert(Arbre::value_type("ERROR",PERROR) );
		DicoDeNom.insert(Arbre::value_type("EVEN",EVEN) );

		
		//DicoDeNom.insert(Arbre::value_type("Digital",DIGITAL) );		
		//DicoDeNom.insert(Arbre::value_type("Agent",AGENT) );
		//DicoDeNom.insert(Arbre::value_type("Property",PROPERTY) );
		      
		/*		
		DicoDeNom.insert(Arbre::value_type("Cos",COS) );
		DicoDeNom.insert(Arbre::value_type("Sin",SIN) );
		DicoDeNom.insert(Arbre::value_type("Tan",TAN) );
		DicoDeNom.insert(Arbre::value_type("Pow",POW) );
		DicoDeNom.insert(Arbre::value_type("Ln",LN) );
		DicoDeNom.insert(Arbre::value_type("Log",LOG) );
		DicoDeNom.insert(Arbre::value_type("CFact",FACT) );
		DicoDeNom.insert(Arbre::value_type("Print",PRINT) );*/


	}

	int GetIdentType(char *Str)
	{
		CStringType S = Str;

		if(DicoDeNom.size())
		{		
			Arbre::iterator Itr = DicoDeNom.find(S);

			if(Itr != DicoDeNom.end())
			{
				return (*Itr).second;
			}
		}
		return IDENT;
	}

	void EraseDico()
	{
	        //Arbre::iterator Itr = DicoDeNom.begin();
		DicoDeNom.clear();
		/*while(Itr != DicoDeNom.end())
		{
			DicoDeNom.erase(Itr);
		}*/
	}

	double GetNombre(char *CStringType,int Base)
	{
		
		double V = 0.0,n,B = (double)Base;


		if(!Base)
		{
			return (float)atof(CStringType);		
		}
		else
		{
			if(Base <= 10)
			{
				int i,j = 0;
				for(i = strlen(CStringType); i >= 1; i--)
				{			
					n = (double)(CStringType[i - 1] - '0'); 
					V += n*(int)pow(B,(double)j);
					j ++;
				} 
			}
			else
			{
				int i,j = 0;
				for(i = strlen(CStringType) - 1; i >= 1; i--)
				{			
					n = (double)(CStringType[i] - '0'); 
					if(CStringType[i] >= '0' && CStringType[i] <'9') n = (double)(CStringType[i] - '0'); 
					else
					if(CStringType[i] >= 'A' && CStringType[i] <'Z') n = 10.0 + (double)(CStringType[i] - 'A');						
					else 
					if(CStringType[i] >= 'a' && CStringType[i] <'z') n = n = 10.0 + (double)(CStringType[i] - 'a');
					if(CStringType[i] == '1') V += n*(int)pow(B,(double)j);
					j ++;
				} 
			}
		}

		return V;
	}	
}
