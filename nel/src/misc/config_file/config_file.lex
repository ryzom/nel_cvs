%{

/* Includes */

#include <vector>
#include <string>

#include "nel/misc/debug.h"

using namespace std;
using namespace NLMISC;

/* Constantes */

//#define DEBUG_PRINTF	InfoLog->displayRaw
#define DEBUG_PRINTF	// InfoLog->displayRaw

#define YY_NEVER_INTERACTIVE 1

#ifdef WIN32
#define read _read
#endif

/* Types */

enum cf_type { T_UNKNOWN, T_INT, T_STRING, T_REAL };

struct cf_value
{
	cf_type	Type;
	int	Int;
	double	Real;
	char	String[1024];
};

/* special include, need to know cf_value */

#include "config_file.yacc.cpp.h"

/* Externals */

extern int cf_CurrentLine;

/* Variables */

bool cf_Ignore;

void comment ();

%}

alpha		[A-Za-z]
digit		[0-9]
variable	({alpha}|[_])({alpha}|{digit}|[_])*
num1		{digit}+\.([eE][-+]?{digit}+)?
num2		{digit}*\.{digit}+([eE][-+]?{digit}+)?
real		{num1}|{num2}
int			{digit}+
hex			0x[0-9a-fA-F]+
string		\"[^\"]*\"

%%

"+"			{ if (!cf_Ignore) return PLUS; }
"-"			{ if (!cf_Ignore) return MINUS; }
"*"			{ if (!cf_Ignore) return MULT; }
"/"			{ if (!cf_Ignore) return DIVIDE; }
")"			{ if (!cf_Ignore) return RPAREN; }
"("			{ if (!cf_Ignore) return LPAREN; }
"="			{ if (!cf_Ignore) return ASSIGN; }
"+="		{ if (!cf_Ignore) return ADD_ASSIGN; }
";"			{ if (!cf_Ignore) return SEMICOLON; }
"}"			{ if (!cf_Ignore) return RBRACE; }
"{"			{ if (!cf_Ignore) return LBRACE; }
","			{ if (!cf_Ignore) return COMMA; }

(\ |\t)		{ /* ignore tabulation and spaces */; }

"\n"		{ /* ignore new line but count them */ cf_CurrentLine++; DEBUG_PRINTF("*****line++ %d\n", cf_CurrentLine); }

"//"		{ comment(); }

\/\*		{ /* Start of a comment */ cf_Ignore = true; }

\*\/		{ /* End of a comment */ cf_Ignore = false; }

{string}	{ /* A string */
				if (!cf_Ignore)
				{
					cflval.Val.Type = T_STRING;
					strcpy (cflval.Val.String, yytext+1);
					cflval.Val.String[strlen(cflval.Val.String)-1] = '\0';
					DEBUG_PRINTF("lex: string '%s' '%s'\n", yytext, cflval.Val.String);
					return STRING;
				}
			}

{variable}	{ /* A variable */
				if (!cf_Ignore)
				{
					cflval.Val.Type = T_STRING;
					strcpy (cflval.Val.String, yytext);
					DEBUG_PRINTF("lex: variable '%s' '%s'\n", yytext, cflval.Val.String);
					return VARIABLE;
				}
			}

{real}		{ /* A real */
				if (!cf_Ignore)
				{
					cflval.Val.Type = T_REAL;
					cflval.Val.Real = atof (yytext);
					DEBUG_PRINTF("lex: real '%s' '%f\n", yytext, cflval.Val.Real);
					return REAL;
				}
			}

{int}		{ /* An int */
				if (!cf_Ignore)
				{
					cflval.Val.Type = T_INT;
					cflval.Val.Int = atoi (yytext);
					DEBUG_PRINTF("lex: int '%s' '%d'\n", yytext, cflval.Val.Int);
					return INT;
				}
			}

{hex}		{ /* An hex int */
				if (!cf_Ignore)
				{
					cflval.Val.Type = T_INT;
					sscanf (yytext, "%x", &(cflval.Val.Int));
					DEBUG_PRINTF("lex: hexa '%s' '0x%x' '%d'\n", yytext, cflval.Val.Int, cflval.Val.Int);
					return INT;
				}
			}

%%

int yywrap()
{
	return 1;
}

//"//".*\n	{ /* ignore one line comment but count the new line */ cf_CurrentLine++; DEBUG_PRINTF("*****line++ %d\n", cf_CurrentLine); }
void comment ()
{
	char c;

	do
	{
		c = yyinput ();
	}
	while (c != '\n' && c != -1);

	if (c != '\n')
		cf_CurrentLine++;
}
