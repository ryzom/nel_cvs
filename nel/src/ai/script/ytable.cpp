
/*  A Bison parser, made from grammar.yacc
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	FIN	258
#define	NOMBRE	259
#define	IDENT	260
#define	PAR_G	261
#define	PAR_D	262
#define	ACCOL_G	263
#define	ACCOL_D	264
#define	CROCHER_G	265
#define	CROCHER_D	266
#define	EG_MATH	267
#define	VIRGULE	268
#define	EG_LOG	269
#define	SUP	270
#define	INF	271
#define	SUP_EG	272
#define	INF_EG	273
#define	DIFF	274
#define	OR_LOG	275
#define	AND_LOG	276
#define	NON_BIN	277
#define	OR_BIN	278
#define	AND_BIN	279
#define	XOR_BIN	280
#define	SCOP	281
#define	PLUS	282
#define	MOINS	283
#define	FOIS	284
#define	DIV	285
#define	POWER	286
#define	POINT_VI	287
#define	LEPOINT	288
#define	POINT_DEUX	289
#define	INTERROGATION	290
#define	CHAINE	291
#define	NILL	292
#define	EXEC	293
#define	ACHIEVE	294
#define	ASK	295
#define	BREAK	296
#define	TELL	297
#define	KILL	298
#define	ERROR	299
#define	BEGIN_GRAMMAR	300
#define	FROM	301
#define	DEFINE	302
#define	GROUP	303
#define	COMPONENT	304
#define	CONSTRUCTION	305
#define	DESTRUCTION	306
#define	MESSAGE_MANAGER	307
#define	MAESSAGELOOP	308
#define	TRIGGER	309
#define	PRECONDITION	310
#define	POSTCONDITION	311
#define	GOAL	312
#define	RETURN	313
#define	COS	314
#define	SIN	315
#define	TAN	316
#define	POW	317
#define	LN	318
#define	LOG	319
#define	FACT	320
#define	AS	321
#define	DIGITAL	322
#define	COLLECTOR	323
#define	WITH	324
#define	DO	325
#define	END	326
#define	IF	327
#define	THEN	328
#define	BEGINING	329
#define	END_GRAMMAR	330
#define	LOGICVAR	331
#define	RULE	332
#define	IA_ASSERT	333
#define	FUZZYRULE	334
#define	FUZZYRULESET	335
#define	SETS	336
#define	FUZZYVAR	337
#define	FIS	338
#define	OR	339
#define	NEW	340
#define	AND	341
#define	LOCAL	342

#line 1 "grammar.yacc"

#include <malloc.h>
#include <list>
#define PARSER_NO_REDEF_YYTABLE_H_
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/constraint.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/logic/logic.h"
#include "nel/ai/fuzzy/fuzzy.h"


using  namespace NLAISCRIPT;
using  namespace NLAILOGIC;
using  namespace NLAIFUZZY;
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		329
#define	YYFLAG		-32768
#define	YYNTBASE	88

#define YYTRANSLATE(x) ((unsigned)(x) <= 342 ? yytranslate[x] : 210)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,    10,    14,    16,    21,    22,    29,    31,
    34,    35,    40,    43,    46,    47,    55,    57,    61,    63,
    65,    67,    70,    72,    74,    77,    82,    86,    90,    91,
    96,    97,   102,   103,   108,   110,   113,   114,   120,   124,
   126,   128,   130,   132,   133,   137,   138,   143,   145,   149,
   151,   155,   158,   160,   161,   162,   169,   171,   175,   177,
   179,   181,   184,   187,   189,   192,   195,   198,   201,   202,
   203,   213,   214,   219,   221,   222,   227,   228,   234,   235,
   240,   242,   244,   246,   250,   253,   255,   257,   258,   264,
   265,   266,   276,   278,   281,   284,   288,   290,   292,   295,
   296,   302,   303,   308,   313,   320,   322,   326,   328,   330,
   335,   337,   339,   342,   343,   348,   349,   354,   357,   358,
   363,   364,   369,   370,   375,   376,   381,   382,   387,   388,
   393,   395,   396,   401,   402,   407,   409,   411,   413,   415,
   417,   419,   421,   423,   425,   427,   429,   433,   435,   437,
   439,   441,   443,   445,   447,   449,   453,   457,   461,   462,
   467,   469,   471,   474,   475,   476,   483,   486,   488,   490,
   491,   492,   499,   501,   502,   506,   508,   509,   514,   518,
   521,   524,   525,   532,   534,   535,   539,   542,   543,   548,
   549,   553,   554,   558,   559,   560,   567,   568,   572,   574,
   575,   580,   581,   582,   590,   591,   592,   593,   601,   603,
   604,   608
};

static const short yyrhs[] = {    89,
     0,    88,    89,     0,    93,     8,   100,     9,     0,    93,
     8,     9,     0,    90,     0,    92,     6,     7,    71,     0,
     0,    92,     6,     7,    91,   127,    71,     0,    52,     0,
    94,    96,     0,     0,    46,     5,    95,    34,     0,    47,
    97,     0,    47,     5,     0,     0,    48,    16,   159,    98,
    13,    99,    15,     0,     5,     0,    99,    13,     5,     0,
   101,     0,   100,     0,   102,     0,   102,   111,     0,   111,
     0,   103,     0,   103,   104,     0,    49,    34,   149,    71,
     0,    49,    34,    71,     0,   105,   107,   109,     0,     0,
    56,   106,    34,    71,     0,     0,    55,   108,    34,    71,
     0,     0,    57,   110,    34,    71,     0,   112,     0,   111,
   112,     0,     0,   114,   115,   127,   113,    71,     0,   114,
   115,    71,     0,     5,     0,    51,     0,   116,     0,   118,
     0,     0,     6,   117,     7,     0,     0,     6,   119,   120,
     7,     0,   121,     0,   120,    13,    99,     0,    99,     0,
   121,    32,    99,     0,   122,   125,     0,     5,     0,     0,
     0,     5,   123,    16,     5,   124,    15,     0,   126,     0,
   125,    13,   126,     0,     5,     0,   128,     0,   143,     0,
   127,   128,     0,   127,   143,     0,    32,     0,   132,    32,
     0,   129,    32,     0,   134,    32,     0,   142,    32,     0,
     0,     0,     5,   130,    28,    15,    66,     6,     5,   131,
     7,     0,     0,   173,   133,    12,   160,     0,   136,     0,
     0,   134,    33,   135,   136,     0,     0,   139,     6,   137,
   141,   140,     0,     0,   139,     6,   138,   140,     0,   173,
     0,     7,     0,   160,     0,   141,    13,   160,     0,    58,
   160,     0,   146,     0,   144,     0,     0,   160,    35,   145,
   127,    71,     0,     0,     0,    72,   160,    35,   147,   127,
    34,   148,   127,    71,     0,    32,     0,   150,    32,     0,
   149,    32,     0,   149,   150,    32,     0,   151,     0,   155,
     0,   153,    15,     0,     0,   153,    13,    87,   152,    15,
     0,     0,     5,   154,    16,    36,     0,    68,    16,   159,
    15,     0,    68,    16,   159,    13,   156,    15,     0,   157,
     0,   156,    13,   157,     0,     5,     0,   158,     0,   172,
    33,    33,   172,     0,    36,     0,   169,     0,    28,   169,
     0,     0,   160,   161,    27,   169,     0,     0,   160,   162,
    28,   169,     0,    22,   169,     0,     0,   160,   163,    19,
   169,     0,     0,   160,   164,    16,   169,     0,     0,   160,
   165,    15,   169,     0,     0,   160,   166,    17,   169,     0,
     0,   160,   167,    18,   169,     0,     0,   160,   168,    14,
   169,     0,   172,     0,     0,   169,   170,    29,   172,     0,
     0,   169,   171,    30,   172,     0,    38,     0,    39,     0,
    40,     0,    41,     0,    44,     0,    42,     0,    43,     0,
    37,     0,     4,     0,   173,     0,   134,     0,     6,   160,
     7,     0,   174,     0,   178,     0,   176,     0,   194,     0,
   183,     0,   200,     0,   195,     0,     5,     0,   134,    33,
     5,     0,   172,    33,     5,     0,   173,    33,     5,     0,
     0,    10,   175,   177,    11,     0,    36,     0,   160,     0,
   177,   160,     0,     0,     0,    85,   179,   182,     6,   180,
   181,     0,   141,     7,     0,     7,     0,     5,     0,     0,
     0,    77,   186,   184,    73,   185,   188,     0,   186,     0,
     0,    72,   187,   188,     0,   190,     0,     0,   190,   189,
    86,   188,     0,   190,    84,   188,     0,    78,   190,     0,
    31,     5,     0,     0,    35,     6,     5,   191,   192,     7,
     0,   160,     0,     0,   160,   193,   192,     0,    35,     5,
     0,     0,    80,     8,   196,   197,     0,     0,   200,   198,
     9,     0,     0,   200,   199,   197,     0,     0,     0,    79,
   203,   201,    73,   202,   205,     0,     0,    72,   204,   205,
     0,   207,     0,     0,   207,   206,    86,   205,     0,     0,
     0,     6,     5,   208,    83,     5,   209,     7,     0,     0,
     0,     0,    82,     0,     5,     0,    81,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   160,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    50,    54,    57,    65,    71,    74,    78,    84,    90,   100,
   103,   119,   121,   122,   141,   144,   147,   148,   150,   151,
   156,   157,   158,   161,   162,   165,   166,   169,   174,   179,
   182,   187,   190,   195,   198,   199,   202,   261,   261,   322,
   330,   343,   344,   347,   352,   357,   361,   367,   368,   371,
   372,   375,   379,   384,   389,   396,   399,   400,   403,   410,
   411,   412,   413,   416,   417,   418,   419,   426,   433,   438,
   446,   448,   458,   470,   471,   476,   479,   486,   487,   495,
   497,   504,   519,   523,   531,   538,   539,   542,   548,   554,
   561,   566,   572,   573,   574,   575,   578,   579,   583,   584,
   590,   592,   598,   613,   614,   617,   618,   621,   622,   625,
   627,   630,   638,   644,   656,   661,   672,   677,   688,   698,
   704,   715,   720,   730,   735,   745,   750,   760,   765,   770,
   778,   782,   793,   799,   811,   819,   825,   830,   835,   840,
   845,   850,   855,   860,   866,   876,   881,   885,   889,   893,
   897,   901,   905,   909,   916,   925,   937,   945,   963,   968,
   971,   977,   981,   987,   992,   997,   998,  1002,  1009,  1016,
  1022,  1026,  1037,  1040,  1045,  1056,  1060,  1064,  1068,  1075,
  1081,  1093,  1102,  1112,  1118,  1125,  1127,  1137,  1142,  1144,
  1149,  1152,  1157,  1162,  1168,  1172,  1179,  1184,  1189,  1193,
  1197,  1200,  1218,  1223,  1229,  1234,  1239,  1243,  1249,  1253,
  1258,  1260
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","FIN","NOMBRE",
"IDENT","PAR_G","PAR_D","ACCOL_G","ACCOL_D","CROCHER_G","CROCHER_D","EG_MATH",
"VIRGULE","EG_LOG","SUP","INF","SUP_EG","INF_EG","DIFF","OR_LOG","AND_LOG","NON_BIN",
"OR_BIN","AND_BIN","XOR_BIN","SCOP","PLUS","MOINS","FOIS","DIV","POWER","POINT_VI",
"LEPOINT","POINT_DEUX","INTERROGATION","CHAINE","NILL","EXEC","ACHIEVE","ASK",
"BREAK","TELL","KILL","ERROR","BEGIN_GRAMMAR","FROM","DEFINE","GROUP","COMPONENT",
"CONSTRUCTION","DESTRUCTION","MESSAGE_MANAGER","MAESSAGELOOP","TRIGGER","PRECONDITION",
"POSTCONDITION","GOAL","RETURN","COS","SIN","TAN","POW","LN","LOG","FACT","AS",
"DIGITAL","COLLECTOR","WITH","DO","END","IF","THEN","BEGINING","END_GRAMMAR",
"LOGICVAR","RULE","IA_ASSERT","FUZZYRULE","FUZZYRULESET","SETS","FUZZYVAR","FIS",
"OR","NEW","AND","LOCAL","program","DefinitionClass","MessageRun","@1","MessageManager",
"CorpDeDefinition","HeritageDeType","@2","EnteteDeDefinition","DefinitionDeGroup",
"@3","CParam","DefinitionDeProgram","BlocDeDefinition","Register","RegistDesAttributs",
"RegisterOperator","PostCondition","@4","PreCondition","@5","Goal","@6","BlocPourLesCode",
"BlocAvecCode","@7","Methode","Argument","ArgumentVide","@8","ArgumentListe",
"@9","ListesDeVariables","DesParams","TypeOfParam","@10","@11","DeclarationVariables",
"NonDeVariable","DuCode","Code","StaticCast","@12","@13","Affectation","@14",
"AppelleDeFonction","@15","AppelleDeFoncDirect","@16","@17","NonDeFonction",
"CallFunction","Prametre","RetourDeFonction","Evaluation","EvaluationSimpleState",
"@18","EvaluationState","@19","@20","TypeDeDeclaration","RegisterAnyVar","RegisterTypeDef",
"@21","TypeDeComp","@22","RegisterCollector","MessageType","DefMessage","Borne",
"Nom","Expression","@23","@24","@25","@26","@27","@28","@29","@30","Term","@31",
"@32","Facteur","Variable","List","@33","ChaineDeCaractaire","ElementList","NewObject",
"@34","@35","PrametreNew","NewObjectName","Rule","@36","@37","RuleCondition",
"@38","Clause","@39","FactPattern","@40","LogicVarSet","@41","LogicVar","FuzzyRuleSet",
"@42","ListFuzzyRule","@43","@44","FuzzyRule","@45","@46","FuzzyCondition","@47",
"FuzzyClause","@48","FuzzyFactPattern","@49","@50", NULL
};
#endif

static const short yyr1[] = {     0,
    88,    88,    89,    89,    89,    90,    91,    90,    92,    93,
    95,    94,    96,    96,    98,    97,    99,    99,   100,   100,
   101,   101,   101,   102,   102,   103,   103,   104,   106,   105,
   108,   107,   110,   109,   111,   111,   113,   112,   112,   114,
   114,   115,   115,   117,   116,   119,   118,   120,   120,   121,
   121,    99,   122,   123,   124,   122,   125,   125,   126,   127,
   127,   127,   127,   128,   128,   128,   128,   128,   130,   131,
   129,   133,   132,   134,   135,   134,   137,   136,   138,   136,
   139,   140,   141,   141,   142,   143,   143,   145,   144,   147,
   148,   146,   149,   149,   149,   149,   150,   150,   151,   152,
   151,   154,   153,   155,   155,   156,   156,   157,   157,   158,
   159,   160,   160,   161,   160,   162,   160,   160,   163,   160,
   164,   160,   165,   160,   166,   160,   167,   160,   168,   160,
   169,   170,   169,   171,   169,   172,   172,   172,   172,   172,
   172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
   172,   172,   172,   172,   173,   173,   173,   173,   175,   174,
   176,   177,   177,   179,   180,   178,   181,   181,   182,   184,
   185,   183,    -1,   187,   186,   188,   189,   188,   188,    -1,
    -1,   191,   190,   192,   193,   192,   194,   196,   195,   198,
   197,   199,   197,   201,   202,   200,   204,   203,   205,   206,
   205,   208,   209,   207,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1
};

static const short yyr2[] = {     0,
     1,     2,     4,     3,     1,     4,     0,     6,     1,     2,
     0,     4,     2,     2,     0,     7,     1,     3,     1,     1,
     1,     2,     1,     1,     2,     4,     3,     3,     0,     4,
     0,     4,     0,     4,     1,     2,     0,     5,     3,     1,
     1,     1,     1,     0,     3,     0,     4,     1,     3,     1,
     3,     2,     1,     0,     0,     6,     1,     3,     1,     1,
     1,     2,     2,     1,     2,     2,     2,     2,     0,     0,
     9,     0,     4,     1,     0,     4,     0,     5,     0,     4,
     1,     1,     1,     3,     2,     1,     1,     0,     5,     0,
     0,     9,     1,     2,     2,     3,     1,     1,     2,     0,
     5,     0,     4,     4,     6,     1,     3,     1,     1,     4,
     1,     1,     2,     0,     4,     0,     4,     2,     0,     4,
     0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
     1,     0,     4,     0,     4,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     3,     3,     0,     4,
     1,     1,     2,     0,     0,     6,     2,     1,     1,     0,
     0,     6,     1,     0,     3,     1,     0,     4,     3,     2,
     2,     0,     6,     1,     0,     3,     2,     0,     4,     0,
     3,     0,     3,     0,     0,     6,     0,     3,     1,     0,
     4,     0,     0,     7,     0,     0,     0,     7,     1,     0,
     3,     1
};

static const short yydefact[] = {     0,
     0,     9,     0,     1,     5,     0,     0,     0,    11,     2,
     0,     0,     0,    10,     0,     7,    40,     4,     0,    41,
     0,    19,    21,    24,    23,    35,     0,    14,     0,    13,
    12,     6,     0,     0,     3,    22,    29,    25,     0,    36,
    44,     0,    42,    43,     0,   144,   155,     0,   159,     0,
     0,    64,     0,   161,   143,   136,   137,   138,   139,   141,
   142,   140,     0,     0,     0,     0,     0,   164,     0,    60,
     0,     0,   146,    74,     0,     0,    61,    87,    86,   114,
   112,   131,   145,   148,   150,   149,   152,   151,   154,   153,
   102,    93,     0,    27,     0,     0,    97,     0,    98,     0,
    31,     0,     0,     0,    39,    37,   111,    15,     0,   155,
   146,   114,   145,     0,   118,   113,   187,    85,   114,   174,
   170,   197,   194,   188,     0,     8,    62,    63,    66,    65,
    67,    75,    77,    68,    88,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    95,    26,     0,    94,     0,    99,     0,     0,    33,    28,
    45,    17,    50,     0,    48,     0,     0,     0,     0,   147,
   162,     0,    90,     0,     0,     0,     0,     0,   169,     0,
   156,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   157,   158,     0,     0,     0,
    96,   100,    30,     0,     0,     0,     0,    47,     0,     0,
    59,    52,    57,    38,     0,     0,   160,   163,     0,     0,
   175,   176,   171,     0,   198,   199,   195,   189,   190,   165,
    76,     0,     0,    83,    82,    80,     0,   115,   117,   120,
   122,   124,   126,   128,   130,   133,   135,    73,   103,     0,
   104,     0,    32,     0,     0,    18,    49,    51,     0,     0,
     0,     0,     0,     0,     0,     0,   202,     0,     0,     0,
     0,     0,     0,    78,    89,   108,     0,   106,   109,     0,
   101,    34,    55,    58,    16,     0,    91,   182,   179,     0,
   172,     0,     0,   196,   191,   193,   168,     0,   166,    84,
     0,   105,     0,     0,    70,     0,     0,   178,     0,   201,
   167,   107,     0,    56,     0,     0,   185,     0,   203,   110,
    71,    92,     0,   183,     0,   186,   204,     0,     0
};

static const short yydefgoto[] = {     3,
     4,     5,    33,     6,     7,     8,    15,    14,    30,   168,
   163,    21,    22,    23,    24,    38,    39,   100,   102,   158,
   160,   205,    25,    26,   167,    27,    42,    43,   103,    44,
   104,   164,   165,   166,   206,   304,   212,   213,    69,    70,
    71,   109,   315,    72,   148,   111,   182,    74,   183,   184,
    75,   236,   233,    76,    77,    78,   185,    79,   219,   306,
    95,    96,    97,   252,    98,   149,    99,   277,   278,   279,
   108,    80,   136,   137,   138,   139,   140,   141,   142,   143,
    81,   144,   145,    82,   113,    84,   114,    85,   172,    86,
   125,   272,   299,   180,    87,   175,   266,   121,   174,   221,
   265,   222,   307,   318,   323,    88,    89,   178,   228,   270,
   271,    90,   177,   269,   123,   176,   225,   268,   226,   292,
   325
};

static const short yypact[] = {   -24,
    30,-32768,    44,-32768,-32768,    39,    72,    46,-32768,-32768,
    99,     8,     1,-32768,    24,    40,-32768,-32768,   109,-32768,
   135,-32768,     3,    77,     3,-32768,   141,-32768,   133,-32768,
-32768,-32768,   462,    10,-32768,     3,-32768,-32768,   100,-32768,
   145,    28,-32768,-32768,   115,-32768,   126,   624,-32768,    79,
    79,-32768,   152,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   624,   624,    90,    91,   157,-32768,   217,-32768,
   128,   134,    29,-32768,   161,   136,-32768,-32768,-32768,   113,
    95,   137,    18,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   153,-32768,    11,   142,-32768,    89,-32768,   139,
-32768,   118,   169,   172,-32768,   462,-32768,-32768,   150,-32768,
   147,   262,     6,   624,    95,    95,-32768,   291,   198,-32768,
-32768,-32768,-32768,-32768,   176,-32768,-32768,-32768,-32768,-32768,
-32768,   177,   178,-32768,-32768,   156,   170,   165,   184,   186,
   185,   200,   189,   195,   201,   220,   223,   222,   216,   115,
-32768,-32768,   205,-32768,   151,-32768,   171,   206,-32768,-32768,
-32768,     9,   228,    81,   211,   239,   175,   234,   233,-32768,
   291,   518,-32768,   227,   190,   258,   194,   193,-32768,   259,
-32768,    79,   624,   261,   462,    79,    79,    79,    79,    79,
    79,    79,    79,    79,    79,-32768,-32768,   624,   235,    97,
-32768,-32768,-32768,   214,   248,   271,   286,-32768,   172,   172,
-32768,   279,-32768,-32768,   172,   229,-32768,   291,   462,   287,
-32768,    50,-32768,   296,-32768,   225,-32768,-32768,   224,-32768,
   280,   137,    88,   291,-32768,-32768,   294,    95,    95,    95,
    95,    95,    95,    95,    95,   137,   137,   291,-32768,   634,
-32768,   297,-32768,   243,   310,-32768,   228,   228,   239,   124,
   311,   350,   315,   227,   237,   227,-32768,   238,   258,   312,
   193,   571,   624,-32768,-32768,    15,   127,-32768,-32768,   292,
-32768,-32768,-32768,-32768,-32768,   322,-32768,-32768,-32768,   227,
-32768,   245,   258,-32768,-32768,-32768,-32768,    96,-32768,   291,
   634,-32768,    13,   324,-32768,   462,   624,-32768,   335,-32768,
-32768,-32768,    79,-32768,   334,   406,   192,   336,-32768,   137,
-32768,-32768,   624,-32768,   337,-32768,-32768,   342,-32768
};

static const short yypgoto[] = {-32768,
   343,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -118,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   325,    51,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    86,   -40,   -66,
-32768,-32768,-32768,-32768,-32768,   -33,-32768,   167,-32768,-32768,
-32768,   114,    78,-32768,   -65,-32768,-32768,-32768,-32768,-32768,
-32768,   256,-32768,-32768,-32768,-32768,-32768,-32768,    52,-32768,
   207,   -37,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
     2,-32768,-32768,  -175,   -32,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -235,
-32768,-32768,-32768,    35,-32768,-32768,-32768,-32768,    92,-32768,
-32768,  -173,-32768,-32768,-32768,-32768,  -246,-32768,-32768,-32768,
-32768
};


#define	YYLAST		719


static const short yytable[] = {    73,
    83,   106,   127,   128,   229,    28,   232,    17,    73,    83,
   112,   -81,    17,   -53,    91,    91,    18,   196,   246,   247,
  -155,     1,   294,   -81,   -54,   118,   119,     2,   289,   -72,
   291,    46,    47,    48,     9,    73,    83,    49,   147,   127,
   128,    92,   151,   328,    11,   313,   310,  -155,    29,    50,
   147,   115,   116,    20,   308,    51,    19,    31,    20,    52,
   131,   132,    53,    54,    55,    56,    57,    58,    59,    60,
    61,    62,    73,    83,   280,    40,   171,    93,    93,    12,
    94,   152,    46,   110,    48,    63,    40,   208,    49,     1,
   257,   258,    13,   209,   235,     2,   260,   229,   105,    64,
   273,   155,   311,   156,    65,    16,    66,    67,   273,   250,
    32,   251,    68,    53,    54,    55,    56,    57,    58,    59,
    60,    61,    62,  -132,  -134,   280,  -129,  -123,  -121,  -125,
  -127,  -119,    37,   264,   218,  -177,   207,   320,   285,   301,
  -116,   302,    34,    35,   237,   234,    41,   135,    45,   -46,
   107,    73,    83,   -69,   101,    65,   117,    66,    67,   129,
   248,   120,   122,    68,   124,   130,   133,   134,   150,   146,
   127,   128,   157,   154,   159,   161,   162,   169,   262,   132,
   179,   181,   186,   188,   -79,    73,    83,   238,   239,   240,
   241,   242,   243,   244,   245,   127,   128,   187,  -184,   189,
   190,   191,   193,    73,    83,  -129,  -123,  -121,  -125,  -127,
  -119,  -129,  -123,  -121,  -125,  -127,  -119,   192,  -114,  -116,
    46,    47,    48,   194,   196,  -116,    49,   197,    73,    83,
   195,   199,   173,   198,   234,   300,   201,   202,    50,   204,
   207,   203,   210,   211,    51,   214,   215,   216,    52,   127,
   128,    53,    54,    55,    56,    57,    58,    59,    60,    61,
    62,   220,   223,   224,   230,   316,   227,   235,   170,   317,
   249,    66,    73,    83,    63,  -129,  -123,  -121,  -125,  -127,
  -119,   254,    73,    83,   253,   317,   255,   126,    64,  -116,
   256,   259,   263,    65,   261,    66,    67,    46,    47,    48,
   267,    68,  -192,    49,  -129,  -123,  -121,  -125,  -127,  -119,
  -200,   281,   -74,   282,   283,    50,   286,  -114,  -116,   288,
   295,    51,   290,   293,   303,    52,   305,   309,    53,    54,
    55,    56,    57,    58,    59,    60,    61,    62,   314,   319,
   321,   329,   324,   327,   284,    10,   274,    36,   231,   298,
   153,    63,   312,    46,    47,    48,   200,   326,     0,    49,
     0,     0,   296,     0,   275,    64,     0,     0,     0,     0,
    65,    50,    66,    67,     0,     0,     0,    51,    68,     0,
     0,    52,     0,   287,    53,    54,    55,    56,    57,    58,
    59,    60,    61,    62,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    63,     0,    46,
    47,    48,     0,     0,     0,    49,     0,     0,     0,     0,
     0,    64,     0,     0,     0,     0,    65,    50,    66,    67,
     0,     0,     0,    51,    68,     0,     0,    52,     0,     0,
    53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    63,     0,    46,    47,    48,     0,     0,
     0,    49,     0,     0,     0,     0,   322,    64,     0,     0,
     0,     0,    65,    50,    66,    67,     0,     0,     0,    51,
    68,     0,     0,    52,     0,     0,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    63,
     0,    46,   110,    48,     0,     0,     0,    49,   217,     0,
     0,     0,     0,    64,     0,     0,     0,     0,    65,    50,
    66,    67,     0,     0,     0,    51,    68,     0,     0,     0,
     0,     0,    53,    54,    55,    56,    57,    58,    59,    60,
    61,    62,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    46,   110,    48,   297,     0,     0,
    49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    50,     0,    65,     0,    66,    67,    51,     0,
     0,     0,    68,     0,     0,    53,    54,    55,    56,    57,
    58,    59,    60,    61,    62,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    46,   110,    48,
     0,     0,     0,    49,     0,     0,     0,    46,   276,    48,
     0,     0,     0,    49,     0,    50,     0,    65,     0,    66,
    67,    51,     0,     0,     0,    68,     0,     0,    53,    54,
    55,    56,    57,    58,    59,    60,    61,    62,    53,    54,
    55,    56,    57,    58,    59,    60,    61,    62,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    65,     0,    66,    67,     0,     0,     0,     0,    68,     0,
    65,     0,    66,    67,     0,     0,     0,     0,    68
};

static const short yycheck[] = {    33,
    33,    42,    69,    69,   178,     5,   182,     5,    42,    42,
    48,     6,     5,     5,     5,     5,     9,     5,   194,   195,
     6,    46,   269,     6,    16,    63,    64,    52,   264,    12,
   266,     4,     5,     6,     5,    69,    69,    10,    33,   106,
   106,    32,    32,     0,     6,    33,   293,    33,    48,    22,
    33,    50,    51,    51,   290,    28,    49,    34,    51,    32,
    32,    33,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,   106,   106,   250,    25,   114,    68,    68,     8,
    71,    71,     4,     5,     6,    58,    36,     7,    10,    46,
   209,   210,    47,    13,     7,    52,   215,   271,    71,    72,
    13,    13,     7,    15,    77,     7,    79,    80,    13,    13,
    71,    15,    85,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    44,    29,    30,   301,    14,    15,    16,    17,
    18,    19,    56,    84,   172,    86,    13,   313,    15,    13,
    28,    15,    34,     9,   185,   183,     6,    35,    16,     5,
    36,   185,   185,    28,    55,    77,     5,    79,    80,    32,
   198,    72,    72,    85,     8,    32,     6,    32,    16,    33,
   237,   237,    34,    32,    57,     7,     5,    28,   219,    33,
     5,     5,    27,    19,     7,   219,   219,   186,   187,   188,
   189,   190,   191,   192,   193,   262,   262,    28,     7,    16,
    15,    17,    14,   237,   237,    14,    15,    16,    17,    18,
    19,    14,    15,    16,    17,    18,    19,    18,    27,    28,
     4,     5,     6,    29,     5,    28,    10,     5,   262,   262,
    30,    16,    35,    12,   272,   273,    32,    87,    22,    34,
    13,    71,    32,     5,    28,    71,    13,    15,    32,   316,
   316,    35,    36,    37,    38,    39,    40,    41,    42,    43,
    44,    35,    73,     6,     6,   306,    73,     7,     7,   307,
    36,    79,   306,   306,    58,    14,    15,    16,    17,    18,
    19,    34,   316,   316,    71,   323,    16,    71,    72,    28,
     5,    13,     6,    77,    66,    79,    80,     4,     5,     6,
     5,    85,    79,    10,    14,    15,    16,    17,    18,    19,
    86,    15,    33,    71,     5,    22,     6,    27,    28,     5,
     9,    28,    86,    86,    33,    32,     5,    83,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    15,     5,
     7,     0,     7,     7,   259,     3,   233,    23,   182,   272,
    95,    58,   301,     4,     5,     6,   150,   323,    -1,    10,
    -1,    -1,   271,    -1,    71,    72,    -1,    -1,    -1,    -1,
    77,    22,    79,    80,    -1,    -1,    -1,    28,    85,    -1,
    -1,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,     4,
     5,     6,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,
    -1,    72,    -1,    -1,    -1,    -1,    77,    22,    79,    80,
    -1,    -1,    -1,    28,    85,    -1,    -1,    32,    -1,    -1,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    58,    -1,     4,     5,     6,    -1,    -1,
    -1,    10,    -1,    -1,    -1,    -1,    71,    72,    -1,    -1,
    -1,    -1,    77,    22,    79,    80,    -1,    -1,    -1,    28,
    85,    -1,    -1,    32,    -1,    -1,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,
    -1,     4,     5,     6,    -1,    -1,    -1,    10,    11,    -1,
    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    77,    22,
    79,    80,    -1,    -1,    -1,    28,    85,    -1,    -1,    -1,
    -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,     4,     5,     6,     7,    -1,    -1,
    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    22,    -1,    77,    -1,    79,    80,    28,    -1,
    -1,    -1,    85,    -1,    -1,    35,    36,    37,    38,    39,
    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,
    -1,    -1,    -1,    10,    -1,    -1,    -1,     4,     5,     6,
    -1,    -1,    -1,    10,    -1,    22,    -1,    77,    -1,    79,
    80,    28,    -1,    -1,    -1,    85,    -1,    -1,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    77,    -1,    79,    80,    -1,    -1,    -1,    -1,    85,    -1,
    77,    -1,    79,    80,    -1,    -1,    -1,    -1,    85
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
CCompilateur::yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 51 "grammar.yacc"
{
								
							;
    break;}
case 3:
#line 61 "grammar.yacc"
{						
								if(!computContraint()) return false;
								((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
							;
    break;}
case 4:
#line 68 "grammar.yacc"
{
								((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
							;
    break;}
case 7:
#line 80 "grammar.yacc"
{
									initMessageManager();
							;
    break;}
case 8:
#line 85 "grammar.yacc"
{
								if(!endMessageManager()) return 0;
							;
    break;}
case 9:
#line 91 "grammar.yacc"
{
								if(_ResultCompile != NULL) 
								{
									yyerror("an MessageManager block had all ready declared");
									return 0;
								}								
							;
    break;}
case 11:
#line 104 "grammar.yacc"
{
								NLAIAGENT::CStringVarName interfName(LastyyText[1]);																																
								try
								{	
									NLAIC::CIdentType id = getTypeOfClass(interfName);
									_SelfClass.push((IClassInterpret *)(((CClassInterpretFactory *)id.getFactory())->getClass()->newInstance()));
									((IClassInterpret *)_SelfClass.get())->setInheritanceName(interfName);
								}
								catch(NLAIE::IException &a)
								{									
									yyerror((char *)a.what());
									return 0;
								}
							;
    break;}
case 14:
#line 123 "grammar.yacc"
{
								try
								{
									NLAIC::CIdentType id(LastyyText[1]);
									char text[1024*4];
									sprintf(text,"class '%s' all ready exist",LastyyText[1]);
									yyerror(text);
									return 0;
								}
								catch(NLAIE::IException &)
								{
									((IClassInterpret *)_SelfClass.get())->setClassName(NLAIAGENT::CStringVarName(LastyyText[1]));
									((IClassInterpret *)_SelfClass.get())->buildVTable();
									RegisterClass();
								}
							;
    break;}
case 15:
#line 142 "grammar.yacc"
{
								((IClassInterpret *)_SelfClass.get())->setClassName(NLAIAGENT::CStringVarName(LastyyText[1]));
							;
    break;}
case 29:
#line 175 "grammar.yacc"
{
								if(!classIsAnOperator()) return 0;
							;
    break;}
case 31:
#line 183 "grammar.yacc"
{
								if(!classIsAnOperator()) return 0;
							;
    break;}
case 33:
#line 191 "grammar.yacc"
{
								if(!classIsAnOperator()) return 0;
							;
    break;}
case 37:
#line 204 "grammar.yacc"
{
								if(_LastBloc != NULL && !_LastBloc->isCodeMonted())
								{
									_VarState.popMark();

									IOpCode *x;
									if(!_InLineParse)
									{										
										if (_Debug)
										{
											x = new CFreeAllocDebug();
										}
										else
										{
											x = new CFreeAlloc();
										}
										_LastBloc->addCode(x);
									}
									x = new CHaltOpCode();									
									_LastBloc->addCode(x);

									CCodeBrancheRun* listCode;
									if (_Debug)
									{
										listCode = _LastBloc->getCodeDebug(_SourceFileName);
									}
									else
									{
										listCode = _LastBloc->getCode();
									}

									if(listCode != NULL)
									{
										((IClassInterpret *)_SelfClass.get())->getBrancheCode().setCode((IOpCode *)listCode);										
										_Heap -= (sint32)_Heap;
										if(_Heap.restoreStackState()) _Heap.restoreStack();
										if(_Heap.restoreShiftState()) _Heap.restoreShift();
										_Heap -= (sint32)_Heap;									
										//listCode->release();
									}
												
									_DecalageHeap = 0;
									CVarPStackParam::_Shift = 0;
								}

								if(_IsVoid)
								{
									IOpType *x = new COperandVoid();									
									((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(x);
								}
								else
								{
									_IsVoid = true;
								}								
								if(!caseRunMsg()) return false;
							;
    break;}
case 39:
#line 263 "grammar.yacc"
{	
								if(_LastBloc != NULL && !_LastBloc->isCodeMonted())
								{
									_VarState.popMark();

									IOpCode *x;
									if(!_InLineParse)
									{
										if (_Debug)
										{
											x = new CFreeAllocDebug();
										}
										else
										{
											x = new CFreeAlloc();
										}
										_LastBloc->addCode(x);
									}
									x = new CHaltOpCode();									
									_LastBloc->addCode(x);

									CCodeBrancheRun* listCode;
									if (_Debug)
									{
										listCode = _LastBloc->getCodeDebug(_SourceFileName);
									}
									else
									{
										listCode = _LastBloc->getCode();
									}

									if(listCode != NULL)
									{
										((IClassInterpret *)_SelfClass.get())->getBrancheCode().setCode((IOpCode *)listCode);										
										_Heap -= (sint32)_Heap;
										if(_Heap.restoreStackState()) _Heap.restoreStack();
										if(_Heap.restoreShiftState()) _Heap.restoreShift();
										_Heap -= (sint32)_Heap;									
										//listCode->release();
									}
												
									_DecalageHeap = 0;
									CVarPStackParam::_Shift = 0;
								}
								if(_IsVoid)
								{
									IOpType *x = new COperandVoid();									
									((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(x);
								}
								else
								{
									_IsVoid = true;
								}
								if(!caseRunMsg()) return false;

							;
    break;}
case 40:
#line 323 "grammar.yacc"
{				
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							;
    break;}
case 41:
#line 331 "grammar.yacc"
{
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							;
    break;}
case 44:
#line 348 "grammar.yacc"
{								
								initParam();
							;
    break;}
case 45:
#line 352 "grammar.yacc"
{
								if(!registerMethod()) return false;
							;
    break;}
case 46:
#line 358 "grammar.yacc"
{
								initParam();
							;
    break;}
case 47:
#line 362 "grammar.yacc"
{
								registerMethod();
							;
    break;}
case 53:
#line 380 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[0]);
								_BaseObjectDef = false;
							;
    break;}
case 54:
#line 385 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							;
    break;}
case 55:
#line 390 "grammar.yacc"
{
								_LastBaseObjectDef = NLAIAGENT::CStringVarName(LastyyText[1]);
								_BaseObjectDef = true;

							;
    break;}
case 59:
#line 404 "grammar.yacc"
{
								if(!setParamVarName()) return 0;
							;
    break;}
case 67:
#line 420 "grammar.yacc"
{
								_LastStringParam.back()->release();
								_LastStringParam.pop_back();
								_Param.back()->release();
								_Param.pop_back();								
							;
    break;}
case 68:
#line 428 "grammar.yacc"
{
								_LastBloc->addCode((new CHaltOpCode));
							;
    break;}
case 69:
#line 434 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							;
    break;}
case 70:
#line 442 "grammar.yacc"
{					
								NLAIAGENT::CStringVarName x(LastyyText[1]);			
								if(!castVariable(_LastString,x)) return false;
							;
    break;}
case 72:
#line 449 "grammar.yacc"
{								
								std::list<NLAISCRIPT::CStringType>::iterator i = _LasVarStr.begin();
								_LasAffectationVarStr.clear();
								while(i != _LasVarStr.end())
								{
									_LasAffectationVarStr.push_back(*i++);
								}												
							;
    break;}
case 73:
#line 459 "grammar.yacc"
{
								if(!affectation()) 
								{
									_FromStack.pop_back();
									return false;
								}
								_FromStack.pop_back();
							;
    break;}
case 75:
#line 472 "grammar.yacc"
{
								
							;
    break;}
case 77:
#line 480 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 79:
#line 488 "grammar.yacc"
{								
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 81:
#line 498 "grammar.yacc"
{
								nameMethodeProcessing();
							;
    break;}
case 82:
#line 505 "grammar.yacc"
{	
								_ExpressionType = _ExpressionTypeTmp.back();
								_ExpressionTypeTmp.pop_back();								
								if(!callFunction())
								{
									_FromStack.pop_back();
									return 0;	
								}
								_FromStack.pop_back();
							;
    break;}
case 83:
#line 520 "grammar.yacc"
{
								pushParamExpression();								
							;
    break;}
case 84:
#line 525 "grammar.yacc"
{
								pushParamExpression();
							;
    break;}
case 85:
#line 533 "grammar.yacc"
{
								if(!typeOfMethod()) return false;
							;
    break;}
case 88:
#line 544 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 89:
#line 549 "grammar.yacc"
{
								interrogationEnd();
							;
    break;}
case 90:
#line 557 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 91:
#line 562 "grammar.yacc"
{								
								ifInterrogationPoint()
							;
    break;}
case 92:
#line 567 "grammar.yacc"
{
								ifInterrogationEnd();
							;
    break;}
case 100:
#line 585 "grammar.yacc"
{
								CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);								
								if(c != NULL) c->Local = true;								
							;
    break;}
case 102:
#line 593 "grammar.yacc"
{								
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
								_LastRegistered = ((IClassInterpret *)_SelfClass.get())->registerComponent(_LastString);
							;
    break;}
case 103:
#line 599 "grammar.yacc"
{
								if(((IClassInterpret *)_SelfClass.get())->getComponent(NLAIAGENT::CStringVarName(LastyyText[1])) == NULL)
								{
									CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);
									c->ObjectName = new NLAIAGENT::CStringVarName(LastyyText[1]);									
								}
								else
								{
									yyerror("variable all ready declared in the Register components");
									return 0;			
								}
							;
    break;}
case 112:
#line 631 "grammar.yacc"
{	
								if(_FacteurEval)
								{								
									allocExpression(NULL);
									setTypeExpression();
								}
							;
    break;}
case 113:
#line 639 "grammar.yacc"
{								
								allocExpression(new CNegOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opAdd,"(-)");
								
							;
    break;}
case 114:
#line 645 "grammar.yacc"
{	
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opAdd))
								{
									yyerror("erreur sémantique l'operateur + n'est pas supporté par cette expression");
									return 0;
								}*/								
								setTypeExpressionG();
								allocExpression(NULL);
								
								
							;
    break;}
case 115:
#line 657 "grammar.yacc"
{									
								allocExpression(new CAddOpCode,true);		
								setTypeExpressionD(NLAIC::CTypeOfOperator::opAdd,"+");	
							;
    break;}
case 116:
#line 662 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opSub))
								{
									yyerror("erreur sémantique l'operateur - n'est pas supporté par cette expression");
									return 0;
								}*/				
								setTypeExpressionG();				
								allocExpression(NULL);
								
							;
    break;}
case 117:
#line 673 "grammar.yacc"
{								
								allocExpression(new CSubOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSub,"-");							
							;
    break;}
case 118:
#line 678 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opNot))
								{
									yyerror("erreur sémantique l'operateur ! n'est pas supporté par cette expression");
									return 0;
								}*/								
								allocExpression(new CNotOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opNot,"!");
								
							;
    break;}
case 119:
#line 689 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opDiff))
								{
									yyerror("erreur sémantique l'operateur != n'est pas supporté par cette expression");
									return 0;
								}*/					
								setTypeExpressionG();			
								allocExpression(NULL);								
							;
    break;}
case 120:
#line 699 "grammar.yacc"
{								
								allocExpression(new CDiffOpCode,true);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiff,"!=");
								
							;
    break;}
case 121:
#line 705 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opInf))
								{
									yyerror("erreur sémantique l'operateur < n'est pas supporté par cette expression");
									return 0;
								}*/					
								setTypeExpressionG();			
								allocExpression(NULL);
								
							;
    break;}
case 122:
#line 716 "grammar.yacc"
{								
								allocExpression(new CInfOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInf,"<");							
							;
    break;}
case 123:
#line 721 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opSup))
								{
									yyerror("erreur sémantique l'operateur > n'est pas supporté par cette expression");
									return 0;
								}*/				
								setTypeExpressionG();				
								allocExpression(NULL);								
							;
    break;}
case 124:
#line 731 "grammar.yacc"
{								
								allocExpression(new CSupOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSup,">");							
							;
    break;}
case 125:
#line 736 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opSupEq))
								{
									yyerror("erreur sémantique l'operateur >= n'est pas supporté par cette expression");
									return 0;
								}*/					
								setTypeExpressionG();			
								allocExpression(NULL);
							;
    break;}
case 126:
#line 746 "grammar.yacc"
{								
								allocExpression(new CSupEqOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSupEq,">=");
							;
    break;}
case 127:
#line 751 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opInfEq))
								{
									yyerror("erreur sémantique l'operateur <= n'est pas supporté par cette expression");
									return 0;
								}*/
								setTypeExpressionG();			
								allocExpression(NULL);								
							;
    break;}
case 128:
#line 761 "grammar.yacc"
{								
								allocExpression(new CInfEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInfEq,"<=");
							;
    break;}
case 129:
#line 766 "grammar.yacc"
{
								setTypeExpressionG();	
								allocExpression(NULL);														
							;
    break;}
case 130:
#line 771 "grammar.yacc"
{								
								allocExpression(new CEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opEq,"==");

							;
    break;}
case 131:
#line 779 "grammar.yacc"
{
								_FacteurEval = true;	
							;
    break;}
case 132:
#line 783 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opMul))
								{
									yyerror("erreur sémantique l'operateur * n'est pas supporté par cette expression");
									return 0;
								}*/
								setTypeExpressionG();								
								allocExpression(NULL);
								_FacteurEval = false;								
							;
    break;}
case 133:
#line 794 "grammar.yacc"
{									
								setTypeExpressionD(NLAIC::CTypeOfOperator::opMul,"+");
								allocExpression(new CMulOpCode,false);
								_FacteurEval = false;
							;
    break;}
case 134:
#line 800 "grammar.yacc"
{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opDiv))
								{
									yyerror("erreur sémantique l'operateur / n'est pas supporté par cette expression");
									return 0;
								}*/
											
								setTypeExpressionG();
								allocExpression(NULL);
								_FacteurEval = false;								
							;
    break;}
case 135:
#line 812 "grammar.yacc"
{																
								allocExpression(new CDivOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiv,"/");
								_FacteurEval = false;	
							;
    break;}
case 136:
#line 821 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PExec);
							;
    break;}
case 137:
#line 826 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAchieve);
							;
    break;}
case 138:
#line 831 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAsk);
							;
    break;}
case 139:
#line 836 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PBreak);
							;
    break;}
case 140:
#line 841 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PError);
							;
    break;}
case 141:
#line 846 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PTell);
							;
    break;}
case 142:
#line 851 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PKill);
							;
    break;}
case 143:
#line 856 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVarNill();
							;
    break;}
case 144:
#line 862 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVar();
							;
    break;}
case 145:
#line 867 "grammar.yacc"
{								
								_IsFacteurIsExpression = false;
								if(!processingVar())
								{
									_FromStack.pop_back();
									return false;
								}
								_FromStack.pop_back();
							;
    break;}
case 146:
#line 877 "grammar.yacc"
{								
								_IsFacteurIsExpression = true;								
								setMethodVar();
							;
    break;}
case 147:
#line 882 "grammar.yacc"
{							
								_IsFacteurIsExpression = true;
							;
    break;}
case 148:
#line 886 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 149:
#line 890 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 150:
#line 894 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 151:
#line 898 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 152:
#line 902 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 153:
#line 906 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 154:
#line 910 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 155:
#line 917 "grammar.yacc"
{
								_LasVarStr.clear();
								_LasVarStr.push_back(NLAISCRIPT::CStringType(LastyyText[1]));
								_LastFact.VarType = varTypeUndef;
								_IsFacteurIsExpression = false;	
								_FromStack.push_back(false);							
							;
    break;}
case 156:
#line 926 "grammar.yacc"
{
								_LasVarStr.clear();
								cleanTypeList();
								_LasVarStr.push_back(LastyyText[1]);
								setMethodVar();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();
								_FromStack.push_back(true);

							;
    break;}
case 157:
#line 938 "grammar.yacc"
{	
								cleanTypeList();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();
								_FromStack.push_back(true);
							;
    break;}
case 158:
#line 947 "grammar.yacc"
{									
								if(_IsFacteurIsExpression)
								{
									IOpType *c = _TypeList.back();
									_TypeList.pop_back();
									c->release();
									_TypeList.push_back(_FlotingExpressionType);
									_FlotingExpressionType->incRef();
									_LasVarStr.clear();
									_FromStack.pop_back();
								}
								_LasVarStr.push_back(LastyyText[1]);
							;
    break;}
case 159:
#line 964 "grammar.yacc"
{									
								setListVar();
							;
    break;}
case 161:
#line 972 "grammar.yacc"
{									
								setChaineVar();
							;
    break;}
case 162:
#line 978 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));								
							;
    break;}
case 163:
#line 982 "grammar.yacc"
{								
								_LastBloc->addCode((new CAddOpCode));
							;
    break;}
case 164:
#line 988 "grammar.yacc"
{
								_LastStringParam.push_back(new NLAIAGENT::CGroupType());								

							;
    break;}
case 165:
#line 993 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
							;
    break;}
case 167:
#line 999 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 168:
#line 1003 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 169:
#line 1010 "grammar.yacc"
{							
								_LastStringParam.back()->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(LastyyText[1])));
								_Param.push_back(new CParam);								
							;
    break;}
case 170:
#line 1017 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );				
						;
    break;}
case 171:
#line 1022 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 172:
#line 1026 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
//							_LastBloc->addCode( new CLdbNewOpCode(CRule()) );		
							setStackVar( CVar::IdVar );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tUndef);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::Rule::idRule;
						;
    break;}
case 174:
#line 1041 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 175:
#line 1045 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbNewOpCode( CClause() ) );
							//_LastBloc->addCode( new CAddOpCode() );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::CClause::idCClause;
						;
    break;}
case 176:
#line 1057 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 177:
#line 1061 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 178:
#line 1065 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 179:
#line 1070 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );													
						;
    break;}
case 180:
#line 1076 "grammar.yacc"
{
							_LastBloc->addCode( new CTellOpCode() );
						;
    break;}
case 181:
#line 1082 "grammar.yacc"
{
							/*char buf[256];
							strcpy(buf, LastyyText[1]);
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CBoolType( NLAIAGENT::CStringVarName(LastyyText[1]) ) ) );
							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
							_LastFact.varType = varForFunc;
							_LastFact.isUsed = false;
							_lastIdentType = NLAIAGENT::CVar::idBoolType;
							*/
						;
    break;}
case 182:
#line 1095 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
							char *txt = LastyyText[1];
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode(new CAddOpCode() );
							_LastAssert = NLAIAGENT::CStringVarName(LastyyText[1]);
						;
    break;}
case 183:
#line 1103 "grammar.yacc"
{
							setStackVar( CFactPattern::IdFactPattern );
							IBaseAssert *my_assert = _FactBase.addAssert( _LastAssert, _NbLogicParams );
							_NbLogicParams = 0;
							_LastBloc->addCode(new CLdbNewOpCode( CFactPattern( my_assert ) ) );
						;
    break;}
case 184:
#line 1113 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 185:
#line 1119 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 187:
#line 1128 "grammar.yacc"
{
							char buf[256];
							strcpy(buf, LastyyText[1]);
							setStackVar( CVar::IdVar );
							_LastBloc->addCode( new CLdbOpCode(CVar( LastyyText[1] )) );
						;
    break;}
case 188:
#line 1138 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
						;
    break;}
case 190:
#line 1145 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 191:
#line 1149 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 192:
#line 1153 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 193:
#line 1157 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 194:
#line 1163 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );
						;
    break;}
case 195:
#line 1168 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 196:
#line 1172 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyRule() ) );		
							setStackVar(CFuzzyRule::IdFuzzyRule);
						;
    break;}
case 197:
#line 1180 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 198:
#line 1184 "grammar.yacc"
{
							
						;
    break;}
case 199:
#line 1190 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 200:
#line 1194 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 202:
#line 1201 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
							// Composant?
							char *txt = LastyyText[0];
/*							_lastFVarIndex = ((IClassInterpret *)_selfClass.get())->getComponentIndex(NLAIAGENT::CStringVarName(LastyyText[1]));
							if (_lastFVarIndex != -1 )
							{
								_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CDigitalType(_lastFVarIndex) ) );
								_LastBloc->addCode( new CAddOpCode() );
							}
							else
							{
								// Variable locale?
							}
							*/

						;
    break;}
case 203:
#line 1219 "grammar.yacc"
{
							_LastString = NLAIAGENT::CStringVarName( LastyyText[0] );
						;
    break;}
case 204:
#line 1223 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++ );
							_LastBloc->addCode(new CLdbNewOpCode( CSimpleFuzzyCond(NULL, NULL) ) );
						;
    break;}
case 205:
#line 1230 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 206:
#line 1234 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 207:
#line 1239 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 208:
#line 1243 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyVar(NLAIAGENT::CStringVarName("Inst"),0,1) ) );
						;
    break;}
case 209:
#line 1250 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 210:
#line 1254 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 212:
#line 1261 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 1265 "grammar.yacc"

