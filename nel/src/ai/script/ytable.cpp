
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
#define	PRESCONDITION	309
#define	POSTCONDITION	310
#define	RETURN	311
#define	COS	312
#define	SIN	313
#define	TAN	314
#define	POW	315
#define	LN	316
#define	LOG	317
#define	FACT	318
#define	AS	319
#define	DIGITAL	320
#define	COLLECTOR	321
#define	WITH	322
#define	DO	323
#define	END	324
#define	IF	325
#define	THEN	326
#define	BEGINING	327
#define	END_GRAMMAR	328
#define	LOGICVAR	329
#define	RULE	330
#define	IA_ASSERT	331
#define	FUZZYRULE	332
#define	FUZZYRULESET	333
#define	SETS	334
#define	FUZZYVAR	335
#define	FIS	336
#define	OR	337
#define	NEW	338
#define	AND	339
#define	LOCAL	340

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



#define	YYFINAL		310
#define	YYFLAG		-32768
#define	YYNTBASE	86

#define YYTRANSLATE(x) ((unsigned)(x) <= 340 ? yytranslate[x] : 199)

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
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,    10,    14,    16,    21,    22,    29,    31,
    34,    35,    40,    43,    46,    47,    55,    57,    61,    63,
    66,    68,    73,    77,    79,    82,    83,    89,    93,    95,
    97,    99,   101,   102,   106,   107,   112,   114,   118,   120,
   124,   127,   129,   130,   131,   138,   140,   144,   146,   148,
   150,   153,   156,   158,   161,   164,   167,   170,   171,   172,
   182,   183,   188,   190,   191,   196,   197,   203,   204,   209,
   211,   213,   215,   219,   222,   224,   226,   227,   233,   234,
   235,   245,   247,   250,   253,   257,   259,   261,   264,   265,
   271,   272,   277,   282,   289,   291,   295,   297,   299,   304,
   306,   308,   311,   312,   317,   318,   323,   326,   327,   332,
   333,   338,   339,   344,   345,   350,   351,   356,   357,   362,
   364,   365,   370,   371,   376,   378,   380,   382,   384,   386,
   388,   390,   392,   394,   396,   400,   402,   404,   406,   408,
   410,   412,   414,   416,   420,   424,   428,   429,   434,   436,
   438,   441,   442,   443,   450,   453,   455,   457,   458,   459,
   466,   468,   469,   473,   475,   476,   481,   485,   488,   491,
   492,   499,   501,   502,   506,   509,   510,   515,   516,   520,
   521,   525,   526,   527,   534,   535,   539,   541,   542,   547,
   548,   549,   557,   558,   559,   560,   568,   570,   571,   575
};

static const short yyrhs[] = {    87,
     0,    86,    87,     0,    91,     8,    98,     9,     0,    91,
     8,     9,     0,    88,     0,    90,     6,     7,    69,     0,
     0,    90,     6,     7,    89,   116,    69,     0,    51,     0,
    92,    94,     0,     0,    45,     5,    93,    34,     0,    46,
    95,     0,    46,     5,     0,     0,    47,    16,   148,    96,
    13,    97,    15,     0,     5,     0,    97,    13,     5,     0,
    99,     0,    99,   100,     0,   100,     0,    48,    34,   138,
    69,     0,    48,    34,    69,     0,   101,     0,   100,   101,
     0,     0,   103,   104,   116,   102,    69,     0,   103,   104,
    69,     0,     5,     0,    50,     0,   105,     0,   107,     0,
     0,     6,   106,     7,     0,     0,     6,   108,   109,     7,
     0,   110,     0,   109,    13,    97,     0,    97,     0,   110,
    32,    97,     0,   111,   114,     0,     5,     0,     0,     0,
     5,   112,    16,     5,   113,    15,     0,   115,     0,   114,
    13,   115,     0,     5,     0,   117,     0,   132,     0,   116,
   117,     0,   116,   132,     0,    32,     0,   121,    32,     0,
   118,    32,     0,   123,    32,     0,   131,    32,     0,     0,
     0,     5,   119,    28,    15,    64,     6,     5,   120,     7,
     0,     0,   162,   122,    12,   149,     0,   125,     0,     0,
   123,    33,   124,   125,     0,     0,   128,     6,   126,   130,
   129,     0,     0,   128,     6,   127,   129,     0,   162,     0,
     7,     0,   149,     0,   130,    13,   149,     0,    56,   149,
     0,   135,     0,   133,     0,     0,   149,    35,   134,   116,
    69,     0,     0,     0,    70,   149,    35,   136,   116,    34,
   137,   116,    69,     0,    32,     0,   139,    32,     0,   138,
    32,     0,   138,   139,    32,     0,   140,     0,   144,     0,
   142,    15,     0,     0,   142,    13,    85,   141,    15,     0,
     0,     5,   143,    16,    36,     0,    66,    16,   148,    15,
     0,    66,    16,   148,    13,   145,    15,     0,   146,     0,
   145,    13,   146,     0,     5,     0,   147,     0,   161,    33,
    33,   161,     0,    36,     0,   158,     0,    28,   158,     0,
     0,   149,   150,    27,   158,     0,     0,   149,   151,    28,
   158,     0,    22,   158,     0,     0,   149,   152,    19,   158,
     0,     0,   149,   153,    16,   158,     0,     0,   149,   154,
    15,   158,     0,     0,   149,   155,    17,   158,     0,     0,
   149,   156,    18,   158,     0,     0,   149,   157,    14,   158,
     0,   161,     0,     0,   158,   159,    29,   161,     0,     0,
   158,   160,    30,   161,     0,    38,     0,    39,     0,    40,
     0,    41,     0,    42,     0,    43,     0,    37,     0,     4,
     0,   162,     0,   123,     0,     6,   149,     7,     0,   163,
     0,   167,     0,   165,     0,   183,     0,   172,     0,   189,
     0,   184,     0,     5,     0,   123,    33,     5,     0,   161,
    33,     5,     0,   162,    33,     5,     0,     0,    10,   164,
   166,    11,     0,    36,     0,   149,     0,   166,   149,     0,
     0,     0,    83,   168,   171,     6,   169,   170,     0,   130,
     7,     0,     7,     0,     5,     0,     0,     0,    75,   175,
   173,    71,   174,   177,     0,   175,     0,     0,    70,   176,
   177,     0,   179,     0,     0,   179,   178,    84,   177,     0,
   179,    82,   177,     0,    76,   179,     0,    31,     5,     0,
     0,    35,     6,     5,   180,   181,     7,     0,   149,     0,
     0,   149,   182,   181,     0,    35,     5,     0,     0,    78,
     8,   185,   186,     0,     0,   189,   187,     9,     0,     0,
   189,   188,   186,     0,     0,     0,    77,   192,   190,    71,
   191,   194,     0,     0,    70,   193,   194,     0,   196,     0,
     0,   196,   195,    84,   194,     0,     0,     0,     6,     5,
   197,    81,     5,   198,     7,     0,     0,     0,     0,    80,
     0,     5,     0,    79,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   149,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    50,    54,    57,    65,    71,    74,    78,    84,    90,   100,
   103,   119,   121,   122,   141,   144,   147,   148,   151,   152,
   153,   156,   157,   160,   161,   164,   222,   222,   282,   290,
   303,   304,   307,   312,   317,   321,   327,   328,   331,   332,
   335,   339,   344,   349,   356,   359,   360,   363,   370,   371,
   372,   373,   376,   377,   378,   379,   386,   393,   398,   406,
   408,   418,   425,   426,   431,   434,   441,   442,   450,   452,
   459,   469,   473,   481,   488,   489,   492,   498,   504,   511,
   516,   522,   523,   524,   525,   528,   529,   533,   534,   540,
   542,   548,   563,   564,   567,   568,   571,   572,   575,   577,
   580,   588,   594,   606,   611,   622,   627,   638,   648,   654,
   665,   670,   680,   685,   695,   700,   710,   715,   720,   728,
   732,   743,   749,   761,   769,   775,   780,   785,   790,   795,
   800,   805,   811,   816,   821,   825,   829,   833,   837,   841,
   845,   849,   856,   864,   874,   881,   898,   903,   906,   912,
   916,   922,   927,   932,   933,   937,   944,   951,   957,   961,
   972,   975,   980,   991,   995,   999,  1003,  1010,  1016,  1028,
  1037,  1047,  1053,  1060,  1062,  1072,  1077,  1079,  1084,  1087,
  1092,  1097,  1103,  1107,  1114,  1119,  1124,  1128,  1132,  1135,
  1153,  1158,  1164,  1169,  1174,  1178,  1184,  1188,  1193,  1195
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","FIN","NOMBRE",
"IDENT","PAR_G","PAR_D","ACCOL_G","ACCOL_D","CROCHER_G","CROCHER_D","EG_MATH",
"VIRGULE","EG_LOG","SUP","INF","SUP_EG","INF_EG","DIFF","OR_LOG","AND_LOG","NON_BIN",
"OR_BIN","AND_BIN","XOR_BIN","SCOP","PLUS","MOINS","FOIS","DIV","POWER","POINT_VI",
"LEPOINT","POINT_DEUX","INTERROGATION","CHAINE","NILL","EXEC","ACHIEVE","ASK",
"BREAK","TELL","KILL","BEGIN_GRAMMAR","FROM","DEFINE","GROUP","COMPONENT","CONSTRUCTION",
"DESTRUCTION","MESSAGE_MANAGER","MAESSAGELOOP","TRIGGER","PRESCONDITION","POSTCONDITION",
"RETURN","COS","SIN","TAN","POW","LN","LOG","FACT","AS","DIGITAL","COLLECTOR",
"WITH","DO","END","IF","THEN","BEGINING","END_GRAMMAR","LOGICVAR","RULE","IA_ASSERT",
"FUZZYRULE","FUZZYRULESET","SETS","FUZZYVAR","FIS","OR","NEW","AND","LOCAL",
"program","DefinitionClass","MessageRun","@1","MessageManager","CorpDeDefinition",
"HeritageDeType","@2","EnteteDeDefinition","DefinitionDeGroup","@3","CParam",
"BlocDeDefinition","RegistDesAttributs","BlocPourLesCode","BlocAvecCode","@4",
"Methode","Argument","ArgumentVide","@5","ArgumentListe","@6","ListesDeVariables",
"DesParams","TypeOfParam","@7","@8","DeclarationVariables","NonDeVariable","DuCode",
"Code","StaticCast","@9","@10","Affectation","@11","AppelleDeFonction","@12",
"AppelleDeFoncDirect","@13","@14","NonDeFonction","CallFunction","Prametre",
"RetourDeFonction","Evaluation","EvaluationSimpleState","@15","EvaluationState",
"@16","@17","TypeDeDeclaration","RegisterAnyVar","RegisterTypeDef","@18","TypeDeComp",
"@19","RegisterCollector","MessageType","DefMessage","Borne","Nom","Expression",
"@20","@21","@22","@23","@24","@25","@26","@27","Term","@28","@29","Facteur",
"Variable","List","@30","ChaineDeCaractaire","ElementList","NewObject","@31",
"@32","PrametreNew","NewObjectName","Rule","@33","@34","RuleCondition","@35",
"Clause","@36","FactPattern","@37","LogicVarSet","@38","LogicVar","FuzzyRuleSet",
"@39","ListFuzzyRule","@40","@41","FuzzyRule","@42","@43","FuzzyCondition","@44",
"FuzzyClause","@45","FuzzyFactPattern","@46","@47", NULL
};
#endif

static const short yyr1[] = {     0,
    86,    86,    87,    87,    87,    88,    89,    88,    90,    91,
    93,    92,    94,    94,    96,    95,    97,    97,    98,    98,
    98,    99,    99,   100,   100,   102,   101,   101,   103,   103,
   104,   104,   106,   105,   108,   107,   109,   109,   110,   110,
    97,   111,   112,   113,   111,   114,   114,   115,   116,   116,
   116,   116,   117,   117,   117,   117,   117,   119,   120,   118,
   122,   121,   123,   124,   123,   126,   125,   127,   125,   128,
   129,   130,   130,   131,   132,   132,   134,   133,   136,   137,
   135,   138,   138,   138,   138,   139,   139,   140,   141,   140,
   143,   142,   144,   144,   145,   145,   146,   146,   147,   148,
   149,   149,   150,   149,   151,   149,   149,   152,   149,   153,
   149,   154,   149,   155,   149,   156,   149,   157,   149,   158,
   159,   158,   160,   158,   161,   161,   161,   161,   161,   161,
   161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
   161,   161,   162,   162,   162,   162,   164,   163,   165,   166,
   166,   168,   169,   167,   170,   170,   171,   173,   174,   172,
    -1,   176,   175,   177,   178,   177,   177,    -1,    -1,   180,
   179,   181,   182,   181,   183,   185,   184,   187,   186,   188,
   186,   190,   191,   189,   193,   192,   194,   195,   194,   197,
   198,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1
};

static const short yyr2[] = {     0,
     1,     2,     4,     3,     1,     4,     0,     6,     1,     2,
     0,     4,     2,     2,     0,     7,     1,     3,     1,     2,
     1,     4,     3,     1,     2,     0,     5,     3,     1,     1,
     1,     1,     0,     3,     0,     4,     1,     3,     1,     3,
     2,     1,     0,     0,     6,     1,     3,     1,     1,     1,
     2,     2,     1,     2,     2,     2,     2,     0,     0,     9,
     0,     4,     1,     0,     4,     0,     5,     0,     4,     1,
     1,     1,     3,     2,     1,     1,     0,     5,     0,     0,
     9,     1,     2,     2,     3,     1,     1,     2,     0,     5,
     0,     4,     4,     6,     1,     3,     1,     1,     4,     1,
     1,     2,     0,     4,     0,     4,     2,     0,     4,     0,
     4,     0,     4,     0,     4,     0,     4,     0,     4,     1,
     0,     4,     0,     4,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
     1,     1,     1,     3,     3,     3,     0,     4,     1,     1,
     2,     0,     0,     6,     2,     1,     1,     0,     0,     6,
     1,     0,     3,     1,     0,     4,     3,     2,     2,     0,
     6,     1,     0,     3,     2,     0,     4,     0,     3,     0,
     3,     0,     0,     6,     0,     3,     1,     0,     4,     0,
     0,     7,     0,     0,     0,     7,     1,     0,     3,     1
};

static const short yydefact[] = {     0,
     0,     9,     0,     1,     5,     0,     0,     0,    11,     2,
     0,     0,     0,    10,     0,     7,    29,     4,     0,    30,
     0,    19,    21,    24,     0,    14,     0,    13,    12,     6,
     0,     0,     3,    20,    25,    33,     0,    31,    32,     0,
   132,   143,     0,   147,     0,     0,    53,     0,   149,   131,
   125,   126,   127,   128,   129,   130,     0,     0,     0,     0,
     0,   152,     0,    49,     0,     0,   134,    63,     0,     0,
    50,    76,    75,   103,   101,   120,   133,   136,   138,   137,
   140,   139,   142,   141,    91,    82,     0,    23,     0,     0,
    86,     0,    87,     0,     0,    28,    26,   100,    15,     0,
   143,   134,   103,   133,     0,   107,   102,   175,    74,   103,
   162,   158,   185,   182,   176,     0,     8,    51,    52,    55,
    54,    56,    64,    66,    57,    77,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    84,    22,     0,    83,     0,    88,    34,    17,    39,
     0,    37,     0,     0,     0,     0,   135,   150,     0,    79,
     0,     0,     0,     0,     0,   157,     0,   144,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   145,   146,     0,     0,     0,    85,    89,     0,
     0,    36,     0,     0,    48,    41,    46,    27,     0,     0,
   148,   151,     0,     0,   163,   164,   159,     0,   186,   187,
   183,   177,   178,   153,    65,     0,     0,    72,    71,    69,
     0,   104,   106,   109,   111,   113,   115,   117,   119,   122,
   124,    62,    92,     0,    93,     0,     0,    18,    38,    40,
     0,     0,     0,     0,     0,     0,     0,     0,   190,     0,
     0,     0,     0,     0,     0,    67,    78,    97,     0,    95,
    98,     0,    90,    44,    47,    16,     0,    80,   170,   167,
     0,   160,     0,     0,   184,   179,   181,   156,     0,   154,
    73,     0,    94,     0,     0,    59,     0,     0,   166,     0,
   189,   155,    96,     0,    45,     0,     0,   173,     0,   191,
    99,    60,    81,     0,   171,     0,   174,   192,     0,     0
};

static const short yydefgoto[] = {     3,
     4,     5,    31,     6,     7,     8,    15,    14,    28,   155,
   150,    21,    22,    23,    24,   154,    25,    37,    38,    94,
    39,    95,   151,   152,   153,   190,   285,   196,   197,    63,
    64,    65,   100,   296,    66,   139,   102,   169,    68,   170,
   171,    69,   220,   217,    70,    71,    72,   172,    73,   203,
   287,    89,    90,    91,   236,    92,   140,    93,   259,   260,
   261,    99,    74,   127,   128,   129,   130,   131,   132,   133,
   134,    75,   135,   136,    76,   104,    78,   105,    79,   159,
    80,   116,   254,   280,   167,    81,   162,   248,   112,   161,
   205,   247,   206,   288,   299,   304,    82,    83,   165,   212,
   252,   253,    84,   164,   251,   114,   163,   209,   250,   210,
   273,   306
};

static const short yypact[] = {    74,
    21,-32768,     9,-32768,-32768,    28,    30,    55,-32768,-32768,
    44,     5,    15,-32768,    47,    63,-32768,-32768,   110,-32768,
   139,    11,    11,-32768,   156,-32768,   147,-32768,-32768,-32768,
   430,    25,-32768,    11,-32768,   159,     7,-32768,-32768,   129,
-32768,   138,   586,-32768,    68,    68,-32768,   163,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   586,   586,   101,   111,
   161,-32768,   206,-32768,   148,   150,    89,-32768,   180,   157,
-32768,-32768,-32768,    99,   108,   165,    53,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   183,-32768,    26,   168,
-32768,     6,-32768,   194,   197,-32768,   430,-32768,-32768,   175,
-32768,   171,   178,    46,   586,   108,   108,-32768,   356,   301,
-32768,-32768,-32768,-32768,-32768,   200,-32768,-32768,-32768,-32768,
-32768,-32768,   202,   201,-32768,-32768,   182,   187,   198,   205,
   203,   207,   204,   209,   196,   199,   221,   222,   218,   215,
   129,-32768,-32768,   208,-32768,   154,-32768,-32768,    59,   219,
   113,   220,   228,   166,   237,   236,-32768,   356,   484,-32768,
   223,   184,   253,   189,   186,-32768,   255,-32768,    68,   586,
   257,   430,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    68,-32768,-32768,   586,   229,    56,-32768,-32768,   252,
   265,-32768,   197,   197,-32768,   258,-32768,-32768,   197,   213,
-32768,   356,   430,   273,-32768,    16,-32768,   275,-32768,   210,
-32768,-32768,   211,-32768,   249,   165,   116,   356,-32768,-32768,
   268,   108,   108,   108,   108,   108,   108,   108,   108,   165,
   165,   356,-32768,   595,-32768,   270,   281,-32768,   219,   219,
   228,   118,   285,   322,   282,   223,   214,   223,-32768,   217,
   253,   283,   186,   535,   586,-32768,-32768,    50,   134,-32768,
-32768,   260,-32768,-32768,-32768,-32768,   290,-32768,-32768,-32768,
   223,-32768,   216,   253,-32768,-32768,-32768,-32768,   123,-32768,
   356,   595,-32768,    35,   284,-32768,   430,   586,-32768,   297,
-32768,-32768,-32768,    68,-32768,   305,   376,   160,   306,-32768,
   165,-32768,-32768,   586,-32768,   307,-32768,-32768,   321,-32768
};

static const short yypgoto[] = {-32768,
   319,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -97,-32768,-32768,   303,    65,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    82,   -33,
   -61,-32768,-32768,-32768,-32768,-32768,   -31,-32768,   162,-32768,
-32768,-32768,   117,    76,-32768,   -60,-32768,-32768,-32768,-32768,
-32768,-32768,   244,-32768,-32768,-32768,-32768,-32768,-32768,    57,
-32768,   212,   -35,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   -21,-32768,-32768,  -154,   -30,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -159,-32768,-32768,-32768,    31,-32768,-32768,-32768,-32768,    87,
-32768,-32768,  -160,-32768,-32768,-32768,-32768,  -233,-32768,-32768,
-32768,-32768
};


#define	YYLAST		678


static const short yytable[] = {    67,
    77,   118,   119,    97,   213,    67,    77,   103,   309,    17,
    41,    42,    43,    18,   216,    17,    44,   275,   146,    26,
   147,   109,   110,   106,   107,     9,   230,   231,    45,    85,
    85,    67,    77,    11,    46,   118,   119,    12,    47,   183,
   291,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    16,   -70,    19,     1,    20,  -143,    86,   142,   -70,     2,
    20,    27,    57,   -42,   -61,    67,    77,   294,   234,   158,
   235,    41,   101,    43,   -43,    96,    58,    44,   138,   262,
    29,    59,  -143,    60,    61,   138,   270,    35,   272,    62,
    87,    87,   213,    88,   143,   239,   240,   246,    35,  -165,
    13,   242,    48,    49,    50,    51,    52,    53,    54,    55,
    56,   289,  -118,  -112,  -110,  -114,  -116,  -108,     1,   192,
   122,   123,   219,   202,     2,   193,  -105,   262,   255,   292,
   191,    30,   266,   126,   218,   255,  -121,  -123,   221,   301,
    67,    77,    59,    32,    60,    61,   282,    33,   283,   232,
    62,   222,   223,   224,   225,   226,   227,   228,   229,   118,
   119,    36,    40,   -35,    98,   -58,  -172,   108,   115,   244,
   111,    67,    77,  -118,  -112,  -110,  -114,  -116,  -108,   120,
   113,   121,   118,   119,   157,   124,  -103,  -105,   125,    67,
    77,  -118,  -112,  -110,  -114,  -116,  -108,   137,   141,   145,
   148,   149,   156,   123,   166,  -105,   168,   -68,   173,    41,
    42,    43,    67,    77,   174,    44,   175,   177,   218,   281,
   176,   179,   180,   178,   181,   183,   184,    45,   182,   185,
   186,   191,   195,    46,   198,   118,   119,    47,   189,   188,
    48,    49,    50,    51,    52,    53,    54,    55,    56,   199,
   200,   194,   298,   297,   207,    67,    77,   204,   208,   211,
   214,    57,    60,   219,   233,    67,    77,   237,   298,   238,
   241,    41,    42,    43,   117,    58,   243,    44,   245,   249,
    59,   -63,    60,    61,   263,   264,   269,  -180,    62,    45,
   267,   276,   284,  -188,   286,    46,   290,   271,   295,    47,
   274,   300,    48,    49,    50,    51,    52,    53,    54,    55,
    56,   302,   305,   308,  -118,  -112,  -110,  -114,  -116,  -108,
   310,    10,   265,    57,    34,    41,    42,    43,  -105,   279,
   215,    44,   144,   256,   307,   160,   257,    58,   293,   277,
     0,     0,    59,    45,    60,    61,     0,     0,     0,    46,
    62,     0,   187,    47,     0,   268,    48,    49,    50,    51,
    52,    53,    54,    55,    56,     0,     0,     0,     0,  -118,
  -112,  -110,  -114,  -116,  -108,     0,     0,    57,     0,    41,
    42,    43,  -103,  -105,     0,    44,     0,     0,     0,     0,
     0,    58,     0,     0,     0,     0,    59,    45,    60,    61,
     0,     0,     0,    46,    62,     0,     0,    47,     0,     0,
    48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    57,     0,    41,    42,    43,     0,     0,     0,    44,
     0,     0,     0,     0,   303,    58,     0,     0,     0,     0,
    59,    45,    60,    61,     0,     0,     0,    46,    62,     0,
     0,    47,     0,     0,    48,    49,    50,    51,    52,    53,
    54,    55,    56,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    57,     0,    41,   101,    43,
     0,     0,     0,    44,   201,     0,     0,     0,     0,    58,
     0,     0,     0,     0,    59,    45,    60,    61,     0,     0,
     0,    46,    62,     0,     0,     0,     0,     0,    48,    49,
    50,    51,    52,    53,    54,    55,    56,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    41,   101,
    43,   278,     0,     0,    44,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    45,     0,    59,     0,
    60,    61,    46,     0,     0,     0,    62,     0,     0,    48,
    49,    50,    51,    52,    53,    54,    55,    56,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    41,
   101,    43,     0,     0,     0,    44,     0,     0,    41,   258,
    43,     0,     0,     0,    44,     0,     0,    45,     0,    59,
     0,    60,    61,    46,     0,     0,     0,    62,     0,     0,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    48,
    49,    50,    51,    52,    53,    54,    55,    56,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    59,     0,    60,    61,     0,     0,     0,     0,    62,    59,
     0,    60,    61,     0,     0,     0,     0,    62
};

static const short yycheck[] = {    31,
    31,    63,    63,    37,   165,    37,    37,    43,     0,     5,
     4,     5,     6,     9,   169,     5,    10,   251,    13,     5,
    15,    57,    58,    45,    46,     5,   181,   182,    22,     5,
     5,    63,    63,     6,    28,    97,    97,     8,    32,     5,
   274,    35,    36,    37,    38,    39,    40,    41,    42,    43,
     7,     6,    48,    45,    50,     6,    32,    32,     6,    51,
    50,    47,    56,     5,    12,    97,    97,    33,    13,   105,
    15,     4,     5,     6,    16,    69,    70,    10,    33,   234,
    34,    75,    33,    77,    78,    33,   246,    23,   248,    83,
    66,    66,   253,    69,    69,   193,   194,    82,    34,    84,
    46,   199,    35,    36,    37,    38,    39,    40,    41,    42,
    43,   271,    14,    15,    16,    17,    18,    19,    45,     7,
    32,    33,     7,   159,    51,    13,    28,   282,    13,     7,
    13,    69,    15,    35,   170,    13,    29,    30,   172,   294,
   172,   172,    75,    34,    77,    78,    13,     9,    15,   185,
    83,   173,   174,   175,   176,   177,   178,   179,   180,   221,
   221,     6,    16,     5,    36,    28,     7,     5,     8,   203,
    70,   203,   203,    14,    15,    16,    17,    18,    19,    32,
    70,    32,   244,   244,     7,     6,    27,    28,    32,   221,
   221,    14,    15,    16,    17,    18,    19,    33,    16,    32,
     7,     5,    28,    33,     5,    28,     5,     7,    27,     4,
     5,     6,   244,   244,    28,    10,    19,    15,   254,   255,
    16,    18,    14,    17,    29,     5,     5,    22,    30,    12,
    16,    13,     5,    28,    69,   297,   297,    32,    85,    32,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    13,
    15,    32,   288,   287,    71,   287,   287,    35,     6,    71,
     6,    56,    77,     7,    36,   297,   297,    16,   304,     5,
    13,     4,     5,     6,    69,    70,    64,    10,     6,     5,
    75,    33,    77,    78,    15,     5,     5,    77,    83,    22,
     6,     9,    33,    84,     5,    28,    81,    84,    15,    32,
    84,     5,    35,    36,    37,    38,    39,    40,    41,    42,
    43,     7,     7,     7,    14,    15,    16,    17,    18,    19,
     0,     3,   241,    56,    22,     4,     5,     6,    28,   254,
   169,    10,    89,   217,   304,    35,    69,    70,   282,   253,
    -1,    -1,    75,    22,    77,    78,    -1,    -1,    -1,    28,
    83,    -1,   141,    32,    -1,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    14,
    15,    16,    17,    18,    19,    -1,    -1,    56,    -1,     4,
     5,     6,    27,    28,    -1,    10,    -1,    -1,    -1,    -1,
    -1,    70,    -1,    -1,    -1,    -1,    75,    22,    77,    78,
    -1,    -1,    -1,    28,    83,    -1,    -1,    32,    -1,    -1,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    56,    -1,     4,     5,     6,    -1,    -1,    -1,    10,
    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,    -1,    -1,
    75,    22,    77,    78,    -1,    -1,    -1,    28,    83,    -1,
    -1,    32,    -1,    -1,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    56,    -1,     4,     5,     6,
    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    -1,    70,
    -1,    -1,    -1,    -1,    75,    22,    77,    78,    -1,    -1,
    -1,    28,    83,    -1,    -1,    -1,    -1,    -1,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,
     6,     7,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    75,    -1,
    77,    78,    28,    -1,    -1,    -1,    83,    -1,    -1,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,
     5,     6,    -1,    -1,    -1,    10,    -1,    -1,     4,     5,
     6,    -1,    -1,    -1,    10,    -1,    -1,    22,    -1,    75,
    -1,    77,    78,    28,    -1,    -1,    -1,    83,    -1,    -1,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    75,    -1,    77,    78,    -1,    -1,    -1,    -1,    83,    75,
    -1,    77,    78,    -1,    -1,    -1,    -1,    83
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
case 26:
#line 166 "grammar.yacc"
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
							;
    break;}
case 28:
#line 224 "grammar.yacc"
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

							;
    break;}
case 29:
#line 283 "grammar.yacc"
{				
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							;
    break;}
case 30:
#line 291 "grammar.yacc"
{
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							;
    break;}
case 33:
#line 308 "grammar.yacc"
{								
								initParam();
							;
    break;}
case 34:
#line 312 "grammar.yacc"
{
								if(!registerMethod()) return false;
							;
    break;}
case 35:
#line 318 "grammar.yacc"
{
								initParam();
							;
    break;}
case 36:
#line 322 "grammar.yacc"
{
								registerMethod();
							;
    break;}
case 42:
#line 340 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[0]);
								_BaseObjectDef = false;
							;
    break;}
case 43:
#line 345 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							;
    break;}
case 44:
#line 350 "grammar.yacc"
{
								_LastBaseObjectDef = NLAIAGENT::CStringVarName(LastyyText[1]);
								_BaseObjectDef = true;

							;
    break;}
case 48:
#line 364 "grammar.yacc"
{
								setParamVarName();
							;
    break;}
case 56:
#line 380 "grammar.yacc"
{
								_LastStringParam.back()->release();
								_LastStringParam.pop_back();
								_Param.back()->release();
								_Param.pop_back();								
							;
    break;}
case 57:
#line 388 "grammar.yacc"
{
								_LastBloc->addCode((new CHaltOpCode));
							;
    break;}
case 58:
#line 394 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							;
    break;}
case 59:
#line 402 "grammar.yacc"
{					
								NLAIAGENT::CStringVarName x(LastyyText[1]);			
								if(!castVariable(_LastString,x)) return false;
							;
    break;}
case 61:
#line 409 "grammar.yacc"
{								
								std::list<NLAISCRIPT::CStringType>::iterator i = _LasVarStr.begin();
								_LasAffectationVarStr.clear();
								while(i != _LasVarStr.end())
								{
									_LasAffectationVarStr.push_back(*i++);
								}												
							;
    break;}
case 62:
#line 419 "grammar.yacc"
{
								if(!affectation()) return false;
							;
    break;}
case 64:
#line 427 "grammar.yacc"
{
								
							;
    break;}
case 66:
#line 435 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 68:
#line 443 "grammar.yacc"
{								
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 70:
#line 453 "grammar.yacc"
{
								nameMethodeProcessing();
							;
    break;}
case 71:
#line 460 "grammar.yacc"
{	
								_ExpressionType = _ExpressionTypeTmp.back();
								_ExpressionTypeTmp.pop_back();								
								if(!callFunction()) return 0;	
							;
    break;}
case 72:
#line 470 "grammar.yacc"
{
								pushParamExpression();								
							;
    break;}
case 73:
#line 475 "grammar.yacc"
{
								pushParamExpression();
							;
    break;}
case 74:
#line 483 "grammar.yacc"
{
								if(!typeOfMethod()) return false;
							;
    break;}
case 77:
#line 494 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 78:
#line 499 "grammar.yacc"
{
								interrogationEnd();
							;
    break;}
case 79:
#line 507 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 80:
#line 512 "grammar.yacc"
{								
								ifInterrogationPoint()
							;
    break;}
case 81:
#line 517 "grammar.yacc"
{
								ifInterrogationEnd();
							;
    break;}
case 89:
#line 535 "grammar.yacc"
{
								CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);								
								if(c != NULL) c->Local = true;								
							;
    break;}
case 91:
#line 543 "grammar.yacc"
{								
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
								_LastRegistered = ((IClassInterpret *)_SelfClass.get())->registerComponent(_LastString);
							;
    break;}
case 92:
#line 549 "grammar.yacc"
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
case 101:
#line 581 "grammar.yacc"
{	
								if(_FacteurEval)
								{								
									allocExpression(NULL);
									setTypeExpression();
								}
							;
    break;}
case 102:
#line 589 "grammar.yacc"
{								
								allocExpression(new CNegOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opAdd,"(-)");
								
							;
    break;}
case 103:
#line 595 "grammar.yacc"
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
case 104:
#line 607 "grammar.yacc"
{									
								allocExpression(new CAddOpCode,true);		
								setTypeExpressionD(NLAIC::CTypeOfOperator::opAdd,"+");	
							;
    break;}
case 105:
#line 612 "grammar.yacc"
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
case 106:
#line 623 "grammar.yacc"
{								
								allocExpression(new CSubOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSub,"-");							
							;
    break;}
case 107:
#line 628 "grammar.yacc"
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
case 108:
#line 639 "grammar.yacc"
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
case 109:
#line 649 "grammar.yacc"
{								
								allocExpression(new CDiffOpCode,true);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiff,"!=");
								
							;
    break;}
case 110:
#line 655 "grammar.yacc"
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
case 111:
#line 666 "grammar.yacc"
{								
								allocExpression(new CInfOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInf,"<");							
							;
    break;}
case 112:
#line 671 "grammar.yacc"
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
case 113:
#line 681 "grammar.yacc"
{								
								allocExpression(new CSupOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSup,">");							
							;
    break;}
case 114:
#line 686 "grammar.yacc"
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
case 115:
#line 696 "grammar.yacc"
{								
								allocExpression(new CSupEqOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSupEq,">=");
							;
    break;}
case 116:
#line 701 "grammar.yacc"
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
case 117:
#line 711 "grammar.yacc"
{								
								allocExpression(new CInfEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInfEq,"<=");
							;
    break;}
case 118:
#line 716 "grammar.yacc"
{
								setTypeExpressionG();	
								allocExpression(NULL);														
							;
    break;}
case 119:
#line 721 "grammar.yacc"
{								
								allocExpression(new CEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opEq,"==");

							;
    break;}
case 120:
#line 729 "grammar.yacc"
{
								_FacteurEval = true;	
							;
    break;}
case 121:
#line 733 "grammar.yacc"
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
case 122:
#line 744 "grammar.yacc"
{									
								setTypeExpressionD(NLAIC::CTypeOfOperator::opMul,"+");
								allocExpression(new CMulOpCode,false);
								_FacteurEval = false;
							;
    break;}
case 123:
#line 750 "grammar.yacc"
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
case 124:
#line 762 "grammar.yacc"
{																
								allocExpression(new CDivOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiv,"/");
								_FacteurEval = false;	
							;
    break;}
case 125:
#line 771 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PExec);
							;
    break;}
case 126:
#line 776 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAchieve);
							;
    break;}
case 127:
#line 781 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAsk);
							;
    break;}
case 128:
#line 786 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PBreak);
							;
    break;}
case 129:
#line 791 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PTell);
							;
    break;}
case 130:
#line 796 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PKill);
							;
    break;}
case 131:
#line 801 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVarNill();
							;
    break;}
case 132:
#line 807 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVar();
							;
    break;}
case 133:
#line 812 "grammar.yacc"
{								
								_IsFacteurIsExpression = false;
								if(!processingVar()) return false;
							;
    break;}
case 134:
#line 817 "grammar.yacc"
{								
								_IsFacteurIsExpression = true;								
								setMethodVar();
							;
    break;}
case 135:
#line 822 "grammar.yacc"
{							
								_IsFacteurIsExpression = true;
							;
    break;}
case 136:
#line 826 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 137:
#line 830 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 138:
#line 834 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 139:
#line 838 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 140:
#line 842 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 141:
#line 846 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 142:
#line 850 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 143:
#line 857 "grammar.yacc"
{
								_LasVarStr.clear();
								_LasVarStr.push_back(NLAISCRIPT::CStringType(LastyyText[1]));
								_LastFact.VarType = varTypeUndef;
								_IsFacteurIsExpression = false;								
							;
    break;}
case 144:
#line 865 "grammar.yacc"
{
								_LasVarStr.clear();
								cleanTypeList();
								_LasVarStr.push_back(LastyyText[1]);
								setMethodVar();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();								
							;
    break;}
case 145:
#line 875 "grammar.yacc"
{	
								cleanTypeList();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();				
							;
    break;}
case 146:
#line 883 "grammar.yacc"
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
case 147:
#line 899 "grammar.yacc"
{									
								setListVar();
							;
    break;}
case 149:
#line 907 "grammar.yacc"
{									
								setChaineVar();
							;
    break;}
case 150:
#line 913 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));								
							;
    break;}
case 151:
#line 917 "grammar.yacc"
{								
								_LastBloc->addCode((new CAddOpCode));
							;
    break;}
case 152:
#line 923 "grammar.yacc"
{
								_LastStringParam.push_back(new NLAIAGENT::CGroupType());								

							;
    break;}
case 153:
#line 928 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
							;
    break;}
case 155:
#line 934 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 156:
#line 938 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 157:
#line 945 "grammar.yacc"
{							
								_LastStringParam.back()->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(LastyyText[1])));
								_Param.push_back(new CParam);								
							;
    break;}
case 158:
#line 952 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );				
						;
    break;}
case 159:
#line 957 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 160:
#line 961 "grammar.yacc"
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
case 162:
#line 976 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 163:
#line 980 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbNewOpCode( CClause() ) );
							//_LastBloc->addCode( new CAddOpCode() );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::CClause::idCClause;
						;
    break;}
case 164:
#line 992 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 165:
#line 996 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 166:
#line 1000 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 167:
#line 1005 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );													
						;
    break;}
case 168:
#line 1011 "grammar.yacc"
{
							_LastBloc->addCode( new CTellOpCode() );
						;
    break;}
case 169:
#line 1017 "grammar.yacc"
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
case 170:
#line 1030 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
							char *txt = LastyyText[1];
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode(new CAddOpCode() );
							_LastAssert = NLAIAGENT::CStringVarName(LastyyText[1]);
						;
    break;}
case 171:
#line 1038 "grammar.yacc"
{
							setStackVar( CFactPattern::IdFactPattern );
							IBaseAssert *my_assert = _FactBase.addAssert( _LastAssert, _NbLogicParams );
							_NbLogicParams = 0;
							_LastBloc->addCode(new CLdbNewOpCode( CFactPattern( my_assert ) ) );
						;
    break;}
case 172:
#line 1048 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 173:
#line 1054 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 175:
#line 1063 "grammar.yacc"
{
							char buf[256];
							strcpy(buf, LastyyText[1]);
							setStackVar( CVar::IdVar );
							_LastBloc->addCode( new CLdbOpCode(CVar( LastyyText[1] )) );
						;
    break;}
case 176:
#line 1073 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
						;
    break;}
case 178:
#line 1080 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 179:
#line 1084 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 180:
#line 1088 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 181:
#line 1092 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 182:
#line 1098 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );
						;
    break;}
case 183:
#line 1103 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 184:
#line 1107 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyRule() ) );		
							setStackVar(CFuzzyRule::IdFuzzyRule);
						;
    break;}
case 185:
#line 1115 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 186:
#line 1119 "grammar.yacc"
{
							
						;
    break;}
case 187:
#line 1125 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 188:
#line 1129 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 190:
#line 1136 "grammar.yacc"
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
case 191:
#line 1154 "grammar.yacc"
{
							_LastString = NLAIAGENT::CStringVarName( LastyyText[0] );
						;
    break;}
case 192:
#line 1158 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++ );
							_LastBloc->addCode(new CLdbNewOpCode( CSimpleFuzzyCond(NULL, NULL) ) );
						;
    break;}
case 193:
#line 1165 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 194:
#line 1169 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 195:
#line 1174 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 196:
#line 1178 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyVar(NLAIAGENT::CStringVarName("Inst"),0,1) ) );
						;
    break;}
case 197:
#line 1185 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 198:
#line 1189 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 200:
#line 1196 "grammar.yacc"
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
#line 1200 "grammar.yacc"

