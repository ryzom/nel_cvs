%{
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/script/lex.h"
using  namespace NLAISCRIPT;
using  namespace std;
%}

blancs                [ \t]+
LineNew               [\n]+
lettre [A-Za-z]
chiffre2 [01]
chiffre10 [0-9]
chiffre16 [0-9A-Fa-f]
ident {lettre}(_|{lettre}|{chiffre10})*
entier2               [0][Bb]{chiffre2}+
entier10 ([0][Ll])?{chiffre10}+
entier16 [0][Xx]{chiffre16}+
exposant [eE][+-]?{entier10}
reel {entier10}"."{entier10}
%%
[\t]	{
			/*On ignore les espasement*/
			yyColone += 5;
}
[ ]	{
			/*On ignore les espasement*/
			yyColone ++;
}

[\n]	{
			/*On ignore les espasement*/
			yyLine ++;
			yyColone = 0;
			setNewLine();
}

[Bb][01]+	{	
				strcpy(LastyyText[0], LastyyText[1]);
				strcpy(LastyyText[1], yytext);
				LastyyNum = NLAISCRIPT::GetNombre(LastyyText[1],2);
				yyColone += strlen(yytext);
				return NOMBRE;
}

[Ll]?[0-9]+	{								
				strcpy(LastyyText[0], LastyyText[1]);
				strcpy(LastyyText[1], yytext);
				LastyyNum = NLAISCRIPT::GetNombre(LastyyText[1],10);
				yyColone += strlen(yytext);
				return NOMBRE;
	}
[xX][0-9A-Fa-f]+	{
					strcpy(LastyyText[0], LastyyText[1]);
					strcpy(LastyyText[1], yytext);
					LastyyNum = NLAISCRIPT::GetNombre(LastyyText[1],16);
					yyColone += strlen(yytext);
					return NOMBRE;
}

[0-9]+"."[0-9]+	{					
					strcpy(LastyyText[0], LastyyText[1]);
					strcpy(LastyyText[1], yytext);
					LastyyNum = NLAISCRIPT::GetNombre(LastyyText[1],0);
					yyColone += strlen(yytext);
					return NOMBRE;
	}

([0-9]+"."[0-9]*)([Ee][+-]?[0-9]*)	{					
					strcpy(LastyyText[0], LastyyText[1]);
					strcpy(LastyyText[1], yytext);
					LastyyNum = NLAISCRIPT::GetNombre(LastyyText[1],0);
					yyColone += strlen(yytext);
					return NOMBRE;
	}

([0-9]+[0-9]*)([Ee][+-]?[0-9]*)	{					
					strcpy(LastyyText[0], LastyyText[1]);
					strcpy(LastyyText[1], yytext);
					LastyyNum = NLAISCRIPT::GetNombre(LastyyText[1],0);
					yyColone += strlen(yytext);
					return NOMBRE;
	}

([a-zA-Z]|_)(_|[0-9]|[a-zA-Z])*	{
						strcpy(LastyyText[0], LastyyText[1]);
						strcpy(LastyyText[1], yytext);
						yyColone += strlen(yytext);					
						int i = NLAISCRIPT::GetIdentType(yytext);
						return i;
					
	}

"."	{		
		yyColone ++;
		return LEPOINT;
	}

"("	{	
		yyColone ++;
		return PAR_G;
	}

")"	{		
		yyColone ++;
		return PAR_D;
	}

"="	{		
		yyColone ++;
		int C = yyinput();
		if(C == '=')
		{ 
			yyColone ++;
			return EG_LOG;
		}
		else  
		{
			unput(C);			
			yytext[1] = 0;
		}
		return EG_MATH;
	}

">" {
		yyColone ++;
		int C = yyinput();
		if(C == '=')
		{ 
			yyColone ++;
			return SUP_EG;
		}
		else  
		{
			unput(C);			
			yytext[1] = 0;
		}
		return SUP;
	}

"<" {
		yyColone ++;
		int C = yyinput();
		if(C == '=')
		{ 
			yyColone ++;
			return INF_EG;
		}
		else  
		{
			unput(C);			
			yytext[1] = 0;
		}
		return INF;
	}


"!" {
		yyColone ++;
		int C = yyinput();
		if(C == '=')
		{ 
			yyColone ++;
			return DIFF;
		}
		else  
		{
			unput(C);
			yytext[1] = 0;
		}
		return NON_BIN;
	} 

"|" {
		yyColone ++;
		int C = yyinput();
		if(C == '|')
		{
			yyColone ++;
			return OR_LOG;
		}
		else  
		{
			unput(C);
			yytext[1] = 0;
		}
		return OR_BIN;
	}

"&" {
		yyColone ++;
		int C = yyinput();
		if(C == '&')
		{
			yyColone ++;
			return AND_LOG;
		}
		else  
		{
			unput(C);
			yytext[1] = 0;
		}
		return AND_BIN;
	}


"+"	{
		yyColone ++;
		return PLUS;
	}

"-"	{
		yyColone ++;
		return MOINS;
	}

"*"	{
		yyColone ++;
		return FOIS;
	}

"/"	{
		yyColone ++;

		int C = yyinput();
		if(C == '*')
		{
			yyColone ++;			
			int NbrCom = 1;
			int BeginDebut = 0;
			int BeginFin = 0;		
			#ifdef _DEBUG
				char DebugStr[2];
				DebugStr[1] = 0;
			#endif
			while(1)
			{
				C = yyinput();
				#ifdef _DEBUG
					DebugStr[0] = C;
				#endif
				yyColone ++;
				if(C == '\n') 
				{
					yyLine ++;
					yyColone = 0;
					setNewLine();
					continue;
				}
				if(C == '*' )
				{			
					yyColone ++;					
					C = yyinput();
					#ifdef _DEBUG
						DebugStr[0] = C;
					#endif
					if(C == '/')
					{
						yyColone ++;			
						yytext[1] = 0;
						NbrCom --;
						if(!NbrCom) break;
					}
					else
					{
						unput(C);
					}
				}
				else
				if(C == '/')
				{
					yyColone ++;				
					C = yyinput();
					#ifdef _DEBUG
						DebugStr[0] = C;
					#endif
					if(C == '*')
					{
						yyColone ++;
						NbrCom ++;
					}
					else
					{
						unput(C);

					}					
				}								
				
			}
		}
		else  
		if(C == '/')
		{			
			yyColone ++;
			#ifdef _DEBUG
				char DebugStr[2];
				DebugStr[1] = 0;
			#endif
			while(1)
			{
				C = yyinput();
				#ifdef _DEBUG
					DebugStr[0] = C;
				#endif
				yyColone ++;
				if(C == '\n')
				{										
					yyLine ++;
					yyColone = 0;
					setNewLine();
					yytext[1] = 0;
					break;
				}
			}
		}
		else
		{
			unput(C);
			yytext[1] = 0;
			return DIV;
		}		
				
	}

","	{
		yyColone ++;
		return VIRGULE;
	}

"°"	{
		yyColone ++;
		return POWER;
	}

";"	{
		yyColone ++;
		return POINT_VI;
	}

":"	{
		yyColone ++;
		int C = yyinput();
		if(C == ':')
		{
			yyColone ++;
			return SCOP;
		}
		else  
		{
			unput(C);
			yytext[1] = 0;
		}		
		return POINT_DEUX;
	}

"{"	{
		yyColone ++;
		return ACCOL_G;
	}
"}"	{
		yyColone ++;
		return ACCOL_D;
	}
"?"	{
		yyColone ++;
		return INTERROGATION;
	}
"'"	{
		yyColone ++;		
		char c = 0;
		int i = 0;
		strcpy(LastyyText[0], LastyyText[1]);
		while(c != 39)
		{
			c = yyinput();
			yyColone ++;			
			switch(c)
			{
			
			case 39:	
				c = yyinput();
				if(c == 39)
				{
					LastyyText[1][i] = c;
					i++;
					c = 0;
				}
				else
				{
					LastyyText[1][i] = 0;
					unput(c);
					c = 39;
				}				
				break;

			case '\n':
			case '\r':
				yyLine ++;
				yyColone = 0;
				break;

			case '\t':
				yyColone +=5;				
				break;
			case EOF:
				return 0;

			default:
				LastyyText[1][i] = c;
				i ++;		
				break;
			}
			
		}				
		return CHAINE;
	}
"]"	{
		yyColone ++;
		return CROCHER_D;
	}
"["	{
		yyColone ++;²
		return CROCHER_G;
	}

%%
