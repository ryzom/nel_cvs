
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
#define	BEGIN_GRAMMAR	299
#define	FROM	300
#define	DEFINE	301
#define	GROUP	302
#define	COMPONENT	303
#define	CONSTRUCTION	304
#define	DESTRUCTION	305
#define	MESSAGE_MANAGER	306
#define	MAESSAGELOOP	307
#define	TRIGGER	308
#define	PRECONDITION	309
#define	POSTCONDITION	310
#define	GOAL	311
#define	RETURN	312
#define	COS	313
#define	SIN	314
#define	TAN	315
#define	POW	316
#define	LN	317
#define	LOG	318
#define	FACT	319
#define	AS	320
#define	DIGITAL	321
#define	COLLECTOR	322
#define	WITH	323
#define	DO	324
#define	END	325
#define	IF	326
#define	THEN	327
#define	BEGINING	328
#define	END_GRAMMAR	329
#define	LOGICVAR	330
#define	RULE	331
#define	IA_ASSERT	332
#define	FUZZYRULE	333
#define	FUZZYRULESET	334
#define	SETS	335
#define	FUZZYVAR	336
#define	FIS	337
#define	OR	338
#define	NEW	339
#define	AND	340
#define	LOCAL	341

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



#define	YYFINAL		328
#define	YYFLAG		-32768
#define	YYNTBASE	87

#define YYTRANSLATE(x) ((unsigned)(x) <= 341 ? yytranslate[x] : 209)

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
    86
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
   417,   419,   421,   423,   425,   427,   431,   433,   435,   437,
   439,   441,   443,   445,   447,   451,   455,   459,   460,   465,
   467,   469,   472,   473,   474,   481,   484,   486,   488,   489,
   490,   497,   499,   500,   504,   506,   507,   512,   516,   519,
   522,   523,   530,   532,   533,   537,   540,   541,   546,   547,
   551,   552,   556,   557,   558,   565,   566,   570,   572,   573,
   578,   579,   580,   588,   589,   590,   591,   599,   601,   602,
   606
};

static const short yyrhs[] = {    88,
     0,    87,    88,     0,    92,     8,    99,     9,     0,    92,
     8,     9,     0,    89,     0,    91,     6,     7,    70,     0,
     0,    91,     6,     7,    90,   126,    70,     0,    51,     0,
    93,    95,     0,     0,    45,     5,    94,    34,     0,    46,
    96,     0,    46,     5,     0,     0,    47,    16,   158,    97,
    13,    98,    15,     0,     5,     0,    98,    13,     5,     0,
   100,     0,    99,     0,   101,     0,   101,   110,     0,   110,
     0,   102,     0,   102,   103,     0,    48,    34,   148,    70,
     0,    48,    34,    70,     0,   104,   106,   108,     0,     0,
    55,   105,    34,    70,     0,     0,    54,   107,    34,    70,
     0,     0,    56,   109,    34,    70,     0,   111,     0,   110,
   111,     0,     0,   113,   114,   126,   112,    70,     0,   113,
   114,    70,     0,     5,     0,    50,     0,   115,     0,   117,
     0,     0,     6,   116,     7,     0,     0,     6,   118,   119,
     7,     0,   120,     0,   119,    13,    98,     0,    98,     0,
   120,    32,    98,     0,   121,   124,     0,     5,     0,     0,
     0,     5,   122,    16,     5,   123,    15,     0,   125,     0,
   124,    13,   125,     0,     5,     0,   127,     0,   142,     0,
   126,   127,     0,   126,   142,     0,    32,     0,   131,    32,
     0,   128,    32,     0,   133,    32,     0,   141,    32,     0,
     0,     0,     5,   129,    28,    15,    65,     6,     5,   130,
     7,     0,     0,   172,   132,    12,   159,     0,   135,     0,
     0,   133,    33,   134,   135,     0,     0,   138,     6,   136,
   140,   139,     0,     0,   138,     6,   137,   139,     0,   172,
     0,     7,     0,   159,     0,   140,    13,   159,     0,    57,
   159,     0,   145,     0,   143,     0,     0,   159,    35,   144,
   126,    70,     0,     0,     0,    71,   159,    35,   146,   126,
    34,   147,   126,    70,     0,    32,     0,   149,    32,     0,
   148,    32,     0,   148,   149,    32,     0,   150,     0,   154,
     0,   152,    15,     0,     0,   152,    13,    86,   151,    15,
     0,     0,     5,   153,    16,    36,     0,    67,    16,   158,
    15,     0,    67,    16,   158,    13,   155,    15,     0,   156,
     0,   155,    13,   156,     0,     5,     0,   157,     0,   171,
    33,    33,   171,     0,    36,     0,   168,     0,    28,   168,
     0,     0,   159,   160,    27,   168,     0,     0,   159,   161,
    28,   168,     0,    22,   168,     0,     0,   159,   162,    19,
   168,     0,     0,   159,   163,    16,   168,     0,     0,   159,
   164,    15,   168,     0,     0,   159,   165,    17,   168,     0,
     0,   159,   166,    18,   168,     0,     0,   159,   167,    14,
   168,     0,   171,     0,     0,   168,   169,    29,   171,     0,
     0,   168,   170,    30,   171,     0,    38,     0,    39,     0,
    40,     0,    41,     0,    42,     0,    43,     0,    37,     0,
     4,     0,   172,     0,   133,     0,     6,   159,     7,     0,
   173,     0,   177,     0,   175,     0,   193,     0,   182,     0,
   199,     0,   194,     0,     5,     0,   133,    33,     5,     0,
   171,    33,     5,     0,   172,    33,     5,     0,     0,    10,
   174,   176,    11,     0,    36,     0,   159,     0,   176,   159,
     0,     0,     0,    84,   178,   181,     6,   179,   180,     0,
   140,     7,     0,     7,     0,     5,     0,     0,     0,    76,
   185,   183,    72,   184,   187,     0,   185,     0,     0,    71,
   186,   187,     0,   189,     0,     0,   189,   188,    85,   187,
     0,   189,    83,   187,     0,    77,   189,     0,    31,     5,
     0,     0,    35,     6,     5,   190,   191,     7,     0,   159,
     0,     0,   159,   192,   191,     0,    35,     5,     0,     0,
    79,     8,   195,   196,     0,     0,   199,   197,     9,     0,
     0,   199,   198,   196,     0,     0,     0,    78,   202,   200,
    72,   201,   204,     0,     0,    71,   203,   204,     0,   206,
     0,     0,   206,   205,    85,   204,     0,     0,     0,     6,
     5,   207,    82,     5,   208,     7,     0,     0,     0,     0,
    81,     0,     5,     0,    80,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   159,     0
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
   446,   448,   458,   465,   466,   471,   474,   481,   482,   490,
   492,   499,   509,   513,   521,   528,   529,   532,   538,   544,
   551,   556,   562,   563,   564,   565,   568,   569,   573,   574,
   580,   582,   588,   603,   604,   607,   608,   611,   612,   615,
   617,   620,   628,   634,   646,   651,   662,   667,   678,   688,
   694,   705,   710,   720,   725,   735,   740,   750,   755,   760,
   768,   772,   783,   789,   801,   809,   815,   820,   825,   830,
   835,   840,   845,   851,   856,   861,   865,   869,   873,   877,
   881,   885,   889,   896,   904,   914,   921,   938,   943,   946,
   952,   956,   962,   967,   972,   973,   977,   984,   991,   997,
  1001,  1012,  1015,  1020,  1031,  1035,  1039,  1043,  1050,  1056,
  1068,  1077,  1087,  1093,  1100,  1102,  1112,  1117,  1119,  1124,
  1127,  1132,  1137,  1143,  1147,  1154,  1159,  1164,  1168,  1172,
  1175,  1193,  1198,  1204,  1209,  1214,  1218,  1224,  1228,  1233,
  1235
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","FIN","NOMBRE",
"IDENT","PAR_G","PAR_D","ACCOL_G","ACCOL_D","CROCHER_G","CROCHER_D","EG_MATH",
"VIRGULE","EG_LOG","SUP","INF","SUP_EG","INF_EG","DIFF","OR_LOG","AND_LOG","NON_BIN",
"OR_BIN","AND_BIN","XOR_BIN","SCOP","PLUS","MOINS","FOIS","DIV","POWER","POINT_VI",
"LEPOINT","POINT_DEUX","INTERROGATION","CHAINE","NILL","EXEC","ACHIEVE","ASK",
"BREAK","TELL","KILL","BEGIN_GRAMMAR","FROM","DEFINE","GROUP","COMPONENT","CONSTRUCTION",
"DESTRUCTION","MESSAGE_MANAGER","MAESSAGELOOP","TRIGGER","PRECONDITION","POSTCONDITION",
"GOAL","RETURN","COS","SIN","TAN","POW","LN","LOG","FACT","AS","DIGITAL","COLLECTOR",
"WITH","DO","END","IF","THEN","BEGINING","END_GRAMMAR","LOGICVAR","RULE","IA_ASSERT",
"FUZZYRULE","FUZZYRULESET","SETS","FUZZYVAR","FIS","OR","NEW","AND","LOCAL",
"program","DefinitionClass","MessageRun","@1","MessageManager","CorpDeDefinition",
"HeritageDeType","@2","EnteteDeDefinition","DefinitionDeGroup","@3","CParam",
"DefinitionDeProgram","BlocDeDefinition","Register","RegistDesAttributs","RegisterOperator",
"PostCondition","@4","PreCondition","@5","Goal","@6","BlocPourLesCode","BlocAvecCode",
"@7","Methode","Argument","ArgumentVide","@8","ArgumentListe","@9","ListesDeVariables",
"DesParams","TypeOfParam","@10","@11","DeclarationVariables","NonDeVariable",
"DuCode","Code","StaticCast","@12","@13","Affectation","@14","AppelleDeFonction",
"@15","AppelleDeFoncDirect","@16","@17","NonDeFonction","CallFunction","Prametre",
"RetourDeFonction","Evaluation","EvaluationSimpleState","@18","EvaluationState",
"@19","@20","TypeDeDeclaration","RegisterAnyVar","RegisterTypeDef","@21","TypeDeComp",
"@22","RegisterCollector","MessageType","DefMessage","Borne","Nom","Expression",
"@23","@24","@25","@26","@27","@28","@29","@30","Term","@31","@32","Facteur",
"Variable","List","@33","ChaineDeCaractaire","ElementList","NewObject","@34",
"@35","PrametreNew","NewObjectName","Rule","@36","@37","RuleCondition","@38",
"Clause","@39","FactPattern","@40","LogicVarSet","@41","LogicVar","FuzzyRuleSet",
"@42","ListFuzzyRule","@43","@44","FuzzyRule","@45","@46","FuzzyCondition","@47",
"FuzzyClause","@48","FuzzyFactPattern","@49","@50", NULL
};
#endif

static const short yyr1[] = {     0,
    87,    87,    88,    88,    88,    89,    90,    89,    91,    92,
    94,    93,    95,    95,    97,    96,    98,    98,    99,    99,
   100,   100,   100,   101,   101,   102,   102,   103,   105,   104,
   107,   106,   109,   108,   110,   110,   112,   111,   111,   113,
   113,   114,   114,   116,   115,   118,   117,   119,   119,   120,
   120,    98,   121,   122,   123,   121,   124,   124,   125,   126,
   126,   126,   126,   127,   127,   127,   127,   127,   129,   130,
   128,   132,   131,   133,   134,   133,   136,   135,   137,   135,
   138,   139,   140,   140,   141,   142,   142,   144,   143,   146,
   147,   145,   148,   148,   148,   148,   149,   149,   150,   151,
   150,   153,   152,   154,   154,   155,   155,   156,   156,   157,
   158,   159,   159,   160,   159,   161,   159,   159,   162,   159,
   163,   159,   164,   159,   165,   159,   166,   159,   167,   159,
   168,   169,   168,   170,   168,   171,   171,   171,   171,   171,
   171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
   171,   171,   171,   172,   172,   172,   172,   174,   173,   175,
   176,   176,   178,   179,   177,   180,   180,   181,   183,   184,
   182,    -1,   186,   185,   187,   188,   187,   187,    -1,    -1,
   190,   189,   191,   192,   191,   193,   195,   194,   197,   196,
   198,   196,   200,   201,   199,   203,   202,   204,   205,   204,
   207,   208,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1
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
     1,     1,     1,     1,     1,     3,     1,     1,     1,     1,
     1,     1,     1,     1,     3,     3,     3,     0,     4,     1,
     1,     2,     0,     0,     6,     2,     1,     1,     0,     0,
     6,     1,     0,     3,     1,     0,     4,     3,     2,     2,
     0,     6,     1,     0,     3,     2,     0,     4,     0,     3,
     0,     3,     0,     0,     6,     0,     3,     1,     0,     4,
     0,     0,     7,     0,     0,     0,     7,     1,     0,     3,
     1
};

static const short yydefact[] = {     0,
     0,     9,     0,     1,     5,     0,     0,     0,    11,     2,
     0,     0,     0,    10,     0,     7,    40,     4,     0,    41,
     0,    19,    21,    24,    23,    35,     0,    14,     0,    13,
    12,     6,     0,     0,     3,    22,    29,    25,     0,    36,
    44,     0,    42,    43,     0,   143,   154,     0,   158,     0,
     0,    64,     0,   160,   142,   136,   137,   138,   139,   140,
   141,     0,     0,     0,     0,     0,   163,     0,    60,     0,
     0,   145,    74,     0,     0,    61,    87,    86,   114,   112,
   131,   144,   147,   149,   148,   151,   150,   153,   152,   102,
    93,     0,    27,     0,     0,    97,     0,    98,     0,    31,
     0,     0,     0,    39,    37,   111,    15,     0,   154,   145,
   114,   144,     0,   118,   113,   186,    85,   114,   173,   169,
   196,   193,   187,     0,     8,    62,    63,    66,    65,    67,
    75,    77,    68,    88,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
    26,     0,    94,     0,    99,     0,     0,    33,    28,    45,
    17,    50,     0,    48,     0,     0,     0,     0,   146,   161,
     0,    90,     0,     0,     0,     0,     0,   168,     0,   155,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   156,   157,     0,     0,     0,    96,
   100,    30,     0,     0,     0,     0,    47,     0,     0,    59,
    52,    57,    38,     0,     0,   159,   162,     0,     0,   174,
   175,   170,     0,   197,   198,   194,   188,   189,   164,    76,
     0,     0,    83,    82,    80,     0,   115,   117,   120,   122,
   124,   126,   128,   130,   133,   135,    73,   103,     0,   104,
     0,    32,     0,     0,    18,    49,    51,     0,     0,     0,
     0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     0,     0,    78,    89,   108,     0,   106,   109,     0,   101,
    34,    55,    58,    16,     0,    91,   181,   178,     0,   171,
     0,     0,   195,   190,   192,   167,     0,   165,    84,     0,
   105,     0,     0,    70,     0,     0,   177,     0,   200,   166,
   107,     0,    56,     0,     0,   184,     0,   202,   110,    71,
    92,     0,   182,     0,   185,   203,     0,     0
};

static const short yydefgoto[] = {     3,
     4,     5,    33,     6,     7,     8,    15,    14,    30,   167,
   162,    21,    22,    23,    24,    38,    39,    99,   101,   157,
   159,   204,    25,    26,   166,    27,    42,    43,   102,    44,
   103,   163,   164,   165,   205,   303,   211,   212,    68,    69,
    70,   108,   314,    71,   147,   110,   181,    73,   182,   183,
    74,   235,   232,    75,    76,    77,   184,    78,   218,   305,
    94,    95,    96,   251,    97,   148,    98,   276,   277,   278,
   107,    79,   135,   136,   137,   138,   139,   140,   141,   142,
    80,   143,   144,    81,   112,    83,   113,    84,   171,    85,
   124,   271,   298,   179,    86,   174,   265,   120,   173,   220,
   264,   221,   306,   317,   322,    87,    88,   177,   227,   269,
   270,    89,   176,   268,   122,   175,   224,   267,   225,   291,
   324
};

static const short yypact[] = {    20,
     3,-32768,    13,-32768,-32768,    33,    47,    54,-32768,-32768,
    90,    51,     7,-32768,   109,    60,-32768,-32768,   114,-32768,
   133,-32768,    16,    94,    16,-32768,   144,-32768,   137,-32768,
-32768,-32768,   417,    12,-32768,    16,-32768,-32768,   100,-32768,
   150,    10,-32768,-32768,   121,-32768,   128,   576,-32768,   585,
   585,-32768,   153,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   576,   576,    88,    99,   165,-32768,   170,-32768,   129,
   145,   108,-32768,   173,   149,-32768,-32768,-32768,    96,   117,
   151,    57,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   166,-32768,    25,   155,-32768,    62,-32768,   154,-32768,
   127,   182,   185,-32768,   417,-32768,-32768,   163,-32768,   160,
   525,     1,   576,   117,   117,-32768,   203,   104,-32768,-32768,
-32768,-32768,-32768,   189,-32768,-32768,-32768,-32768,-32768,-32768,
   190,   192,-32768,-32768,   174,   172,   195,   199,   201,   206,
   207,   210,   197,   202,   228,   231,   225,   222,   121,-32768,
-32768,   211,-32768,   156,-32768,   169,   213,-32768,-32768,-32768,
    17,   232,    55,   212,   247,   183,   242,   244,-32768,   203,
   472,-32768,   226,   188,   257,   194,   186,-32768,   261,-32768,
   585,   576,   263,   417,   585,   585,   585,   585,   585,   585,
   585,   585,   585,   585,-32768,-32768,   576,   235,   113,-32768,
-32768,-32768,   198,   241,   260,   272,-32768,   185,   185,-32768,
   265,-32768,-32768,   185,   214,-32768,   203,   417,   275,-32768,
    44,-32768,   281,-32768,   215,-32768,-32768,   218,-32768,   264,
   151,    71,   203,-32768,-32768,   252,   117,   117,   117,   117,
   117,   117,   117,   117,   151,   151,   203,-32768,   635,-32768,
   283,-32768,   229,   296,-32768,   232,   232,   247,   120,   297,
   307,   299,   226,   217,   226,-32768,   220,   257,   298,   186,
   524,   576,-32768,-32768,    37,   123,-32768,-32768,   273,-32768,
-32768,-32768,-32768,-32768,   303,-32768,-32768,-32768,   226,-32768,
   233,   257,-32768,-32768,-32768,-32768,    80,-32768,   203,   635,
-32768,    26,   295,-32768,   417,   576,-32768,   309,-32768,-32768,
-32768,   585,-32768,   311,   362,    89,   312,-32768,   151,-32768,
-32768,   576,-32768,   313,-32768,-32768,   316,-32768
};

static const short yypgoto[] = {-32768,
   318,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -185,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   301,    66,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    67,   -40,   -65,
-32768,-32768,-32768,-32768,-32768,   -33,-32768,   146,-32768,-32768,
-32768,   101,    61,-32768,   -64,-32768,-32768,-32768,-32768,-32768,
-32768,   240,-32768,-32768,-32768,-32768,-32768,-32768,    38,-32768,
   177,   -37,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -23,-32768,-32768,  -175,   -32,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -180,
-32768,-32768,-32768,    15,-32768,-32768,-32768,-32768,    70,-32768,
-32768,  -172,-32768,-32768,-32768,-32768,  -231,-32768,-32768,-32768,
-32768
};


#define	YYLAST		719


static const short yytable[] = {    72,
    82,   105,   126,   127,   228,   231,   -81,     9,    72,    82,
   111,    28,   327,    46,    47,    48,    90,   245,   246,    49,
    17,   -53,   256,   257,   117,   118,   114,   115,   259,    90,
   195,    50,   -54,   146,    72,    82,   293,    51,    11,   126,
   127,    52,  -154,    91,    53,    54,    55,    56,    57,    58,
    59,    60,    61,    29,    12,    17,   150,     1,   312,    18,
   309,   207,   -81,     2,     1,    20,    62,   208,   -72,  -154,
     2,    72,    82,   279,   154,   170,   155,   234,    92,   104,
    63,    93,   288,   272,   290,    64,   310,    65,    66,   146,
    40,    92,   272,    67,   151,  -183,    16,   228,    19,    13,
    20,    40,  -129,  -123,  -121,  -125,  -127,  -119,   307,  -129,
  -123,  -121,  -125,  -127,  -119,  -114,  -116,  -129,  -123,  -121,
  -125,  -127,  -119,  -116,   279,   249,   263,   250,  -176,    32,
   134,  -116,   206,   217,   284,   300,   319,   301,   172,   130,
   131,    35,    31,   236,   233,  -132,  -134,    34,    37,    41,
    72,    82,    45,   100,   -46,   -69,   106,   116,   119,   247,
   128,   237,   238,   239,   240,   241,   242,   243,   244,   121,
   126,   127,   123,    46,    47,    48,   129,   261,   132,    49,
   133,   149,   158,   145,    72,    82,   153,   156,   160,   161,
   168,    50,   131,   178,   180,   126,   127,    51,   -79,   186,
   185,    52,    72,    82,    53,    54,    55,    56,    57,    58,
    59,    60,    61,   187,   188,   189,  -129,  -123,  -121,  -125,
  -127,  -119,   190,   192,   191,   193,    62,    72,    82,  -114,
  -116,   194,   195,   233,   299,   196,   197,   198,   202,   125,
    63,   201,   200,   209,   206,    64,   203,    65,    66,   126,
   127,   210,   213,    67,   214,    46,    47,    48,   215,   222,
   219,    49,   223,    65,   315,   226,   229,   252,   316,   234,
   248,    72,    82,    50,   253,   254,   255,   258,   260,    51,
   262,    72,    82,    52,   316,   266,    53,    54,    55,    56,
    57,    58,    59,    60,    61,  -191,   -74,   280,   281,  -199,
   282,   289,   285,   287,   292,   302,   294,   304,    62,   313,
    46,    47,    48,   318,   308,   328,    49,   320,   323,   326,
    10,   274,    63,    36,   283,   199,   230,    64,    50,    65,
    66,   297,   273,   152,    51,    67,   325,   311,    52,   295,
   286,    53,    54,    55,    56,    57,    58,    59,    60,    61,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    62,     0,    46,    47,    48,     0,     0,
     0,    49,     0,     0,     0,     0,     0,    63,     0,     0,
     0,     0,    64,    50,    65,    66,     0,     0,     0,    51,
    67,     0,     0,    52,     0,     0,    53,    54,    55,    56,
    57,    58,    59,    60,    61,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    62,     0,
    46,    47,    48,     0,     0,     0,    49,     0,     0,     0,
     0,   321,    63,     0,     0,     0,     0,    64,    50,    65,
    66,     0,     0,     0,    51,    67,     0,     0,    52,     0,
     0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    62,     0,    46,   109,    48,     0,     0,
     0,    49,   216,     0,     0,     0,     0,    63,     0,     0,
     0,     0,    64,    50,    65,    66,     0,     0,     0,    51,
    67,     0,     0,     0,     0,     0,    53,    54,    55,    56,
    57,    58,    59,    60,    61,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    46,   109,    48,
   296,   169,     0,    49,     0,     0,     0,     0,  -129,  -123,
  -121,  -125,  -127,  -119,     0,    50,     0,    64,     0,    65,
    66,    51,  -116,     0,     0,    67,     0,     0,    53,    54,
    55,    56,    57,    58,    59,    60,    61,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    46,
   109,    48,     0,     0,     0,    49,     0,     0,    46,   109,
    48,     0,     0,     0,    49,     0,     0,    50,     0,    64,
     0,    65,    66,    51,     0,     0,     0,    67,     0,     0,
    53,    54,    55,    56,    57,    58,    59,    60,    61,    53,
    54,    55,    56,    57,    58,    59,    60,    61,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    46,   275,
    48,     0,     0,     0,    49,     0,     0,     0,     0,     0,
     0,    64,     0,    65,    66,     0,     0,     0,     0,    67,
    64,     0,    65,    66,     0,     0,     0,     0,    67,    53,
    54,    55,    56,    57,    58,    59,    60,    61,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    64,     0,    65,    66,     0,     0,     0,     0,    67
};

static const short yycheck[] = {    33,
    33,    42,    68,    68,   177,   181,     6,     5,    42,    42,
    48,     5,     0,     4,     5,     6,     5,   193,   194,    10,
     5,     5,   208,   209,    62,    63,    50,    51,   214,     5,
     5,    22,    16,    33,    68,    68,   268,    28,     6,   105,
   105,    32,     6,    32,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    47,     8,     5,    32,    45,    33,     9,
   292,     7,     6,    51,    45,    50,    57,    13,    12,    33,
    51,   105,   105,   249,    13,   113,    15,     7,    67,    70,
    71,    70,   263,    13,   265,    76,     7,    78,    79,    33,
    25,    67,    13,    84,    70,     7,     7,   270,    48,    46,
    50,    36,    14,    15,    16,    17,    18,    19,   289,    14,
    15,    16,    17,    18,    19,    27,    28,    14,    15,    16,
    17,    18,    19,    28,   300,    13,    83,    15,    85,    70,
    35,    28,    13,   171,    15,    13,   312,    15,    35,    32,
    33,     9,    34,   184,   182,    29,    30,    34,    55,     6,
   184,   184,    16,    54,     5,    28,    36,     5,    71,   197,
    32,   185,   186,   187,   188,   189,   190,   191,   192,    71,
   236,   236,     8,     4,     5,     6,    32,   218,     6,    10,
    32,    16,    56,    33,   218,   218,    32,    34,     7,     5,
    28,    22,    33,     5,     5,   261,   261,    28,     7,    28,
    27,    32,   236,   236,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    19,    16,    15,    14,    15,    16,    17,
    18,    19,    17,    14,    18,    29,    57,   261,   261,    27,
    28,    30,     5,   271,   272,     5,    12,    16,    70,    70,
    71,    86,    32,    32,    13,    76,    34,    78,    79,   315,
   315,     5,    70,    84,    13,     4,     5,     6,    15,    72,
    35,    10,     6,    78,   305,    72,     6,    70,   306,     7,
    36,   305,   305,    22,    34,    16,     5,    13,    65,    28,
     6,   315,   315,    32,   322,     5,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    78,    33,    15,    70,    85,
     5,    85,     6,     5,    85,    33,     9,     5,    57,    15,
     4,     5,     6,     5,    82,     0,    10,     7,     7,     7,
     3,    70,    71,    23,   258,   149,   181,    76,    22,    78,
    79,   271,   232,    94,    28,    84,   322,   300,    32,   270,
    34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    57,    -1,     4,     5,     6,    -1,    -1,
    -1,    10,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
    -1,    -1,    76,    22,    78,    79,    -1,    -1,    -1,    28,
    84,    -1,    -1,    32,    -1,    -1,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
     4,     5,     6,    -1,    -1,    -1,    10,    -1,    -1,    -1,
    -1,    70,    71,    -1,    -1,    -1,    -1,    76,    22,    78,
    79,    -1,    -1,    -1,    28,    84,    -1,    -1,    32,    -1,
    -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    57,    -1,     4,     5,     6,    -1,    -1,
    -1,    10,    11,    -1,    -1,    -1,    -1,    71,    -1,    -1,
    -1,    -1,    76,    22,    78,    79,    -1,    -1,    -1,    28,
    84,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,
     7,     7,    -1,    10,    -1,    -1,    -1,    -1,    14,    15,
    16,    17,    18,    19,    -1,    22,    -1,    76,    -1,    78,
    79,    28,    28,    -1,    -1,    84,    -1,    -1,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,
     5,     6,    -1,    -1,    -1,    10,    -1,    -1,     4,     5,
     6,    -1,    -1,    -1,    10,    -1,    -1,    22,    -1,    76,
    -1,    78,    79,    28,    -1,    -1,    -1,    84,    -1,    -1,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,
     6,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
    -1,    76,    -1,    78,    79,    -1,    -1,    -1,    -1,    84,
    76,    -1,    78,    79,    -1,    -1,    -1,    -1,    84,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    76,    -1,    78,    79,    -1,    -1,    -1,    -1,    84
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
								if(!affectation()) return false;
							;
    break;}
case 75:
#line 467 "grammar.yacc"
{
								
							;
    break;}
case 77:
#line 475 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 79:
#line 483 "grammar.yacc"
{								
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 81:
#line 493 "grammar.yacc"
{
								nameMethodeProcessing();
							;
    break;}
case 82:
#line 500 "grammar.yacc"
{	
								_ExpressionType = _ExpressionTypeTmp.back();
								_ExpressionTypeTmp.pop_back();								
								if(!callFunction()) return 0;	
							;
    break;}
case 83:
#line 510 "grammar.yacc"
{
								pushParamExpression();								
							;
    break;}
case 84:
#line 515 "grammar.yacc"
{
								pushParamExpression();
							;
    break;}
case 85:
#line 523 "grammar.yacc"
{
								if(!typeOfMethod()) return false;
							;
    break;}
case 88:
#line 534 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 89:
#line 539 "grammar.yacc"
{
								interrogationEnd();
							;
    break;}
case 90:
#line 547 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 91:
#line 552 "grammar.yacc"
{								
								ifInterrogationPoint()
							;
    break;}
case 92:
#line 557 "grammar.yacc"
{
								ifInterrogationEnd();
							;
    break;}
case 100:
#line 575 "grammar.yacc"
{
								CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);								
								if(c != NULL) c->Local = true;								
							;
    break;}
case 102:
#line 583 "grammar.yacc"
{								
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
								_LastRegistered = ((IClassInterpret *)_SelfClass.get())->registerComponent(_LastString);
							;
    break;}
case 103:
#line 589 "grammar.yacc"
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
#line 621 "grammar.yacc"
{	
								if(_FacteurEval)
								{								
									allocExpression(NULL);
									setTypeExpression();
								}
							;
    break;}
case 113:
#line 629 "grammar.yacc"
{								
								allocExpression(new CNegOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opAdd,"(-)");
								
							;
    break;}
case 114:
#line 635 "grammar.yacc"
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
#line 647 "grammar.yacc"
{									
								allocExpression(new CAddOpCode,true);		
								setTypeExpressionD(NLAIC::CTypeOfOperator::opAdd,"+");	
							;
    break;}
case 116:
#line 652 "grammar.yacc"
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
#line 663 "grammar.yacc"
{								
								allocExpression(new CSubOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSub,"-");							
							;
    break;}
case 118:
#line 668 "grammar.yacc"
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
#line 679 "grammar.yacc"
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
#line 689 "grammar.yacc"
{								
								allocExpression(new CDiffOpCode,true);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiff,"!=");
								
							;
    break;}
case 121:
#line 695 "grammar.yacc"
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
#line 706 "grammar.yacc"
{								
								allocExpression(new CInfOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInf,"<");							
							;
    break;}
case 123:
#line 711 "grammar.yacc"
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
#line 721 "grammar.yacc"
{								
								allocExpression(new CSupOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSup,">");							
							;
    break;}
case 125:
#line 726 "grammar.yacc"
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
#line 736 "grammar.yacc"
{								
								allocExpression(new CSupEqOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSupEq,">=");
							;
    break;}
case 127:
#line 741 "grammar.yacc"
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
#line 751 "grammar.yacc"
{								
								allocExpression(new CInfEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInfEq,"<=");
							;
    break;}
case 129:
#line 756 "grammar.yacc"
{
								setTypeExpressionG();	
								allocExpression(NULL);														
							;
    break;}
case 130:
#line 761 "grammar.yacc"
{								
								allocExpression(new CEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opEq,"==");

							;
    break;}
case 131:
#line 769 "grammar.yacc"
{
								_FacteurEval = true;	
							;
    break;}
case 132:
#line 773 "grammar.yacc"
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
#line 784 "grammar.yacc"
{									
								setTypeExpressionD(NLAIC::CTypeOfOperator::opMul,"+");
								allocExpression(new CMulOpCode,false);
								_FacteurEval = false;
							;
    break;}
case 134:
#line 790 "grammar.yacc"
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
#line 802 "grammar.yacc"
{																
								allocExpression(new CDivOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiv,"/");
								_FacteurEval = false;	
							;
    break;}
case 136:
#line 811 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PExec);
							;
    break;}
case 137:
#line 816 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAchieve);
							;
    break;}
case 138:
#line 821 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAsk);
							;
    break;}
case 139:
#line 826 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PBreak);
							;
    break;}
case 140:
#line 831 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PTell);
							;
    break;}
case 141:
#line 836 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PKill);
							;
    break;}
case 142:
#line 841 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVarNill();
							;
    break;}
case 143:
#line 847 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVar();
							;
    break;}
case 144:
#line 852 "grammar.yacc"
{								
								_IsFacteurIsExpression = false;
								if(!processingVar()) return false;
							;
    break;}
case 145:
#line 857 "grammar.yacc"
{								
								_IsFacteurIsExpression = true;								
								setMethodVar();
							;
    break;}
case 146:
#line 862 "grammar.yacc"
{							
								_IsFacteurIsExpression = true;
							;
    break;}
case 147:
#line 866 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 148:
#line 870 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 149:
#line 874 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 150:
#line 878 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 151:
#line 882 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 152:
#line 886 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 153:
#line 890 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 154:
#line 897 "grammar.yacc"
{
								_LasVarStr.clear();
								_LasVarStr.push_back(NLAISCRIPT::CStringType(LastyyText[1]));
								_LastFact.VarType = varTypeUndef;
								_IsFacteurIsExpression = false;								
							;
    break;}
case 155:
#line 905 "grammar.yacc"
{
								_LasVarStr.clear();
								cleanTypeList();
								_LasVarStr.push_back(LastyyText[1]);
								setMethodVar();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();								
							;
    break;}
case 156:
#line 915 "grammar.yacc"
{	
								cleanTypeList();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();				
							;
    break;}
case 157:
#line 923 "grammar.yacc"
{									
								if(_IsFacteurIsExpression)
								{
									IOpType *c = _TypeList.back();
									_TypeList.pop_back();
									c->release();
									_TypeList.push_back(_FlotingExpressionType);
									_FlotingExpressionType->incRef();
									_LasVarStr.clear();
								}
								_LasVarStr.push_back(LastyyText[1]);
							;
    break;}
case 158:
#line 939 "grammar.yacc"
{									
								setListVar();
							;
    break;}
case 160:
#line 947 "grammar.yacc"
{									
								setChaineVar();
							;
    break;}
case 161:
#line 953 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));								
							;
    break;}
case 162:
#line 957 "grammar.yacc"
{								
								_LastBloc->addCode((new CAddOpCode));
							;
    break;}
case 163:
#line 963 "grammar.yacc"
{
								_LastStringParam.push_back(new NLAIAGENT::CGroupType());								

							;
    break;}
case 164:
#line 968 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
							;
    break;}
case 166:
#line 974 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 167:
#line 978 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 168:
#line 985 "grammar.yacc"
{							
								_LastStringParam.back()->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(LastyyText[1])));
								_Param.push_back(new CParam);								
							;
    break;}
case 169:
#line 992 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );				
						;
    break;}
case 170:
#line 997 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 171:
#line 1001 "grammar.yacc"
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
case 173:
#line 1016 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 174:
#line 1020 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbNewOpCode( CClause() ) );
							//_LastBloc->addCode( new CAddOpCode() );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::CClause::idCClause;
						;
    break;}
case 175:
#line 1032 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 176:
#line 1036 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 177:
#line 1040 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 178:
#line 1045 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );													
						;
    break;}
case 179:
#line 1051 "grammar.yacc"
{
							_LastBloc->addCode( new CTellOpCode() );
						;
    break;}
case 180:
#line 1057 "grammar.yacc"
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
case 181:
#line 1070 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
							char *txt = LastyyText[1];
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode(new CAddOpCode() );
							_LastAssert = NLAIAGENT::CStringVarName(LastyyText[1]);
						;
    break;}
case 182:
#line 1078 "grammar.yacc"
{
							setStackVar( CFactPattern::IdFactPattern );
							IBaseAssert *my_assert = _FactBase.addAssert( _LastAssert, _NbLogicParams );
							_NbLogicParams = 0;
							_LastBloc->addCode(new CLdbNewOpCode( CFactPattern( my_assert ) ) );
						;
    break;}
case 183:
#line 1088 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 184:
#line 1094 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 186:
#line 1103 "grammar.yacc"
{
							char buf[256];
							strcpy(buf, LastyyText[1]);
							setStackVar( CVar::IdVar );
							_LastBloc->addCode( new CLdbOpCode(CVar( LastyyText[1] )) );
						;
    break;}
case 187:
#line 1113 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
						;
    break;}
case 189:
#line 1120 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 190:
#line 1124 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 191:
#line 1128 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 192:
#line 1132 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 193:
#line 1138 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );
						;
    break;}
case 194:
#line 1143 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 195:
#line 1147 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyRule() ) );		
							setStackVar(CFuzzyRule::IdFuzzyRule);
						;
    break;}
case 196:
#line 1155 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 197:
#line 1159 "grammar.yacc"
{
							
						;
    break;}
case 198:
#line 1165 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 199:
#line 1169 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 201:
#line 1176 "grammar.yacc"
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
case 202:
#line 1194 "grammar.yacc"
{
							_LastString = NLAIAGENT::CStringVarName( LastyyText[0] );
						;
    break;}
case 203:
#line 1198 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++ );
							_LastBloc->addCode(new CLdbNewOpCode( CSimpleFuzzyCond(NULL, NULL) ) );
						;
    break;}
case 204:
#line 1205 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 205:
#line 1209 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 206:
#line 1214 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 207:
#line 1218 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyVar(NLAIAGENT::CStringVarName("Inst"),0,1) ) );
						;
    break;}
case 208:
#line 1225 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 209:
#line 1229 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 211:
#line 1236 "grammar.yacc"
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
#line 1240 "grammar.yacc"

