
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
#define	BEGIN_GRAMMAR	293
#define	FROM	294
#define	DEFINE	295
#define	GROUP	296
#define	COMPONENT	297
#define	CONSTRUCTION	298
#define	DESTRUCTION	299
#define	MESSAGE_MANAGER	300
#define	MAESSAGELOOP	301
#define	TRIGGER	302
#define	PRESCONDITION	303
#define	POSTCONDITION	304
#define	RETURN	305
#define	COS	306
#define	SIN	307
#define	TAN	308
#define	POW	309
#define	LN	310
#define	LOG	311
#define	FACT	312
#define	AS	313
#define	DIGITAL	314
#define	COLLECTOR	315
#define	WITH	316
#define	DO	317
#define	END	318
#define	IF	319
#define	THEN	320
#define	BEGINING	321
#define	END_GRAMMAR	322
#define	LOGICVAR	323
#define	RULE	324
#define	IA_ASSERT	325
#define	FUZZYRULE	326
#define	FUZZYRULESET	327
#define	SETS	328
#define	FUZZYVAR	329
#define	FIS	330
#define	OR	331
#define	NEW	332
#define	AND	333
#define	LOCAL	334

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



#define	YYFINAL		304
#define	YYFLAG		-32768
#define	YYNTBASE	80

#define YYTRANSLATE(x) ((unsigned)(x) <= 334 ? yytranslate[x] : 193)

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
    76,    77,    78,    79
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
   364,   365,   370,   371,   376,   378,   380,   382,   384,   388,
   390,   392,   394,   396,   398,   400,   402,   404,   408,   412,
   416,   417,   422,   424,   426,   429,   430,   431,   438,   441,
   443,   445,   446,   447,   454,   456,   457,   461,   463,   464,
   469,   473,   476,   479,   480,   487,   489,   490,   494,   497,
   498,   503,   504,   508,   509,   513,   514,   515,   522,   523,
   527,   529,   530,   535,   536,   537,   545,   546,   547,   548,
   556,   558,   559,   563
};

static const short yyrhs[] = {    81,
     0,    80,    81,     0,    85,     8,    92,     9,     0,    85,
     8,     9,     0,    82,     0,    84,     6,     7,    63,     0,
     0,    84,     6,     7,    83,   110,    63,     0,    45,     0,
    86,    88,     0,     0,    39,     5,    87,    34,     0,    40,
    89,     0,    40,     5,     0,     0,    41,    16,   142,    90,
    13,    91,    15,     0,     5,     0,    91,    13,     5,     0,
    93,     0,    93,    94,     0,    94,     0,    42,    34,   132,
    63,     0,    42,    34,    63,     0,    95,     0,    94,    95,
     0,     0,    97,    98,   110,    96,    63,     0,    97,    98,
    63,     0,     5,     0,    44,     0,    99,     0,   101,     0,
     0,     6,   100,     7,     0,     0,     6,   102,   103,     7,
     0,   104,     0,   103,    13,    91,     0,    91,     0,   104,
    32,    91,     0,   105,   108,     0,     5,     0,     0,     0,
     5,   106,    16,     5,   107,    15,     0,   109,     0,   108,
    13,   109,     0,     5,     0,   111,     0,   126,     0,   110,
   111,     0,   110,   126,     0,    32,     0,   115,    32,     0,
   112,    32,     0,   117,    32,     0,   125,    32,     0,     0,
     0,     5,   113,    28,    15,    58,     6,     5,   114,     7,
     0,     0,   156,   116,    12,   143,     0,   119,     0,     0,
   117,    33,   118,   119,     0,     0,   122,     6,   120,   124,
   123,     0,     0,   122,     6,   121,   123,     0,   156,     0,
     7,     0,   143,     0,   124,    13,   143,     0,    50,   143,
     0,   129,     0,   127,     0,     0,   143,    35,   128,   110,
    63,     0,     0,     0,    64,   143,    35,   130,   110,    34,
   131,   110,    63,     0,    32,     0,   133,    32,     0,   132,
    32,     0,   132,   133,    32,     0,   134,     0,   138,     0,
   136,    15,     0,     0,   136,    13,    79,   135,    15,     0,
     0,     5,   137,    16,    36,     0,    60,    16,   142,    15,
     0,    60,    16,   142,    13,   139,    15,     0,   140,     0,
   139,    13,   140,     0,     5,     0,   141,     0,   155,    33,
    33,   155,     0,    36,     0,   152,     0,    28,   152,     0,
     0,   143,   144,    27,   152,     0,     0,   143,   145,    28,
   152,     0,    22,   152,     0,     0,   143,   146,    19,   152,
     0,     0,   143,   147,    16,   152,     0,     0,   143,   148,
    15,   152,     0,     0,   143,   149,    17,   152,     0,     0,
   143,   150,    18,   152,     0,     0,   143,   151,    14,   152,
     0,   155,     0,     0,   152,   153,    29,   155,     0,     0,
   152,   154,    30,   155,     0,    37,     0,     4,     0,   156,
     0,   117,     0,     6,   143,     7,     0,   157,     0,   161,
     0,   159,     0,   177,     0,   166,     0,   183,     0,   178,
     0,     5,     0,   117,    33,     5,     0,   155,    33,     5,
     0,   156,    33,     5,     0,     0,    10,   158,   160,    11,
     0,    36,     0,   143,     0,   160,   143,     0,     0,     0,
    77,   162,   165,     6,   163,   164,     0,   124,     7,     0,
     7,     0,     5,     0,     0,     0,    69,   169,   167,    65,
   168,   171,     0,   169,     0,     0,    64,   170,   171,     0,
   173,     0,     0,   173,   172,    78,   171,     0,   173,    76,
   171,     0,    70,   173,     0,    31,     5,     0,     0,    35,
     6,     5,   174,   175,     7,     0,   143,     0,     0,   143,
   176,   175,     0,    35,     5,     0,     0,    72,     8,   179,
   180,     0,     0,   183,   181,     9,     0,     0,   183,   182,
   180,     0,     0,     0,    71,   186,   184,    65,   185,   188,
     0,     0,    64,   187,   188,     0,   190,     0,     0,   190,
   189,    78,   188,     0,     0,     0,     6,     5,   191,    75,
     5,   192,     7,     0,     0,     0,     0,    74,     0,     5,
     0,    73,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   143,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    49,    53,    56,    64,    70,    73,    77,    83,    89,    99,
   102,   118,   120,   121,   140,   143,   146,   147,   150,   151,
   152,   155,   156,   159,   160,   163,   221,   221,   281,   289,
   302,   303,   306,   311,   316,   320,   326,   327,   330,   331,
   334,   338,   343,   348,   355,   358,   359,   362,   369,   370,
   371,   372,   375,   376,   377,   378,   385,   392,   397,   405,
   407,   417,   424,   425,   430,   433,   440,   441,   449,   451,
   458,   468,   472,   480,   487,   488,   491,   497,   503,   510,
   515,   521,   522,   523,   524,   527,   528,   532,   533,   539,
   541,   547,   562,   563,   566,   567,   570,   571,   574,   576,
   579,   587,   593,   605,   610,   621,   626,   637,   647,   653,
   664,   669,   679,   684,   694,   699,   709,   714,   719,   727,
   731,   742,   748,   760,   768,   773,   779,   784,   789,   793,
   797,   801,   805,   809,   813,   817,   824,   832,   842,   849,
   866,   871,   874,   880,   884,   890,   895,   900,   901,   905,
   912,   919,   925,   929,   940,   943,   948,   959,   963,   967,
   971,   978,   984,   996,  1005,  1015,  1021,  1028,  1030,  1040,
  1045,  1047,  1052,  1055,  1060,  1065,  1071,  1075,  1082,  1087,
  1092,  1096,  1100,  1103,  1121,  1126,  1132,  1137,  1142,  1146,
  1152,  1156,  1161,  1163
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","FIN","NOMBRE",
"IDENT","PAR_G","PAR_D","ACCOL_G","ACCOL_D","CROCHER_G","CROCHER_D","EG_MATH",
"VIRGULE","EG_LOG","SUP","INF","SUP_EG","INF_EG","DIFF","OR_LOG","AND_LOG","NON_BIN",
"OR_BIN","AND_BIN","XOR_BIN","SCOP","PLUS","MOINS","FOIS","DIV","POWER","POINT_VI",
"LEPOINT","POINT_DEUX","INTERROGATION","CHAINE","NILL","BEGIN_GRAMMAR","FROM",
"DEFINE","GROUP","COMPONENT","CONSTRUCTION","DESTRUCTION","MESSAGE_MANAGER",
"MAESSAGELOOP","TRIGGER","PRESCONDITION","POSTCONDITION","RETURN","COS","SIN",
"TAN","POW","LN","LOG","FACT","AS","DIGITAL","COLLECTOR","WITH","DO","END","IF",
"THEN","BEGINING","END_GRAMMAR","LOGICVAR","RULE","IA_ASSERT","FUZZYRULE","FUZZYRULESET",
"SETS","FUZZYVAR","FIS","OR","NEW","AND","LOCAL","program","DefinitionClass",
"MessageRun","@1","MessageManager","CorpDeDefinition","HeritageDeType","@2",
"EnteteDeDefinition","DefinitionDeGroup","@3","CParam","BlocDeDefinition","RegistDesAttributs",
"BlocPourLesCode","BlocAvecCode","@4","Methode","Argument","ArgumentVide","@5",
"ArgumentListe","@6","ListesDeVariables","DesParams","TypeOfParam","@7","@8",
"DeclarationVariables","NonDeVariable","DuCode","Code","StaticCast","@9","@10",
"Affectation","@11","AppelleDeFonction","@12","AppelleDeFoncDirect","@13","@14",
"NonDeFonction","CallFunction","Prametre","RetourDeFonction","Evaluation","EvaluationSimpleState",
"@15","EvaluationState","@16","@17","TypeDeDeclaration","RegisterAnyVar","RegisterTypeDef",
"@18","TypeDeComp","@19","RegisterCollector","MessageType","DefMessage","Borne",
"Nom","Expression","@20","@21","@22","@23","@24","@25","@26","@27","Term","@28",
"@29","Facteur","Variable","List","@30","ChaineDeCaractaire","ElementList","NewObject",
"@31","@32","PrametreNew","NewObjectName","Rule","@33","@34","RuleCondition",
"@35","Clause","@36","FactPattern","@37","LogicVarSet","@38","LogicVar","FuzzyRuleSet",
"@39","ListFuzzyRule","@40","@41","FuzzyRule","@42","@43","FuzzyCondition","@44",
"FuzzyClause","@45","FuzzyFactPattern","@46","@47", NULL
};
#endif

static const short yyr1[] = {     0,
    80,    80,    81,    81,    81,    82,    83,    82,    84,    85,
    87,    86,    88,    88,    90,    89,    91,    91,    92,    92,
    92,    93,    93,    94,    94,    96,    95,    95,    97,    97,
    98,    98,   100,    99,   102,   101,   103,   103,   104,   104,
    91,   105,   106,   107,   105,   108,   108,   109,   110,   110,
   110,   110,   111,   111,   111,   111,   111,   113,   114,   112,
   116,   115,   117,   118,   117,   120,   119,   121,   119,   122,
   123,   124,   124,   125,   126,   126,   128,   127,   130,   131,
   129,   132,   132,   132,   132,   133,   133,   134,   135,   134,
   137,   136,   138,   138,   139,   139,   140,   140,   141,   142,
   143,   143,   144,   143,   145,   143,   143,   146,   143,   147,
   143,   148,   143,   149,   143,   150,   143,   151,   143,   152,
   153,   152,   154,   152,   155,   155,   155,   155,   155,   155,
   155,   155,   155,   155,   155,   155,   156,   156,   156,   156,
   158,   157,   159,   160,   160,   162,   163,   161,   164,   164,
   165,   167,   168,   166,    -1,   170,   169,   171,   172,   171,
   171,    -1,    -1,   174,   173,   175,   176,   175,   177,   179,
   178,   181,   180,   182,   180,   184,   185,   183,   187,   186,
   188,   189,   188,   191,   192,   190,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1
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
     0,     4,     0,     4,     1,     1,     1,     1,     3,     1,
     1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
     0,     4,     1,     1,     2,     0,     0,     6,     2,     1,
     1,     0,     0,     6,     1,     0,     3,     1,     0,     4,
     3,     2,     2,     0,     6,     1,     0,     3,     2,     0,
     4,     0,     3,     0,     3,     0,     0,     6,     0,     3,
     1,     0,     4,     0,     0,     7,     0,     0,     0,     7,
     1,     0,     3,     1
};

static const short yydefact[] = {     0,
     0,     9,     0,     1,     5,     0,     0,     0,    11,     2,
     0,     0,     0,    10,     0,     7,    29,     4,     0,    30,
     0,    19,    21,    24,     0,    14,     0,    13,    12,     6,
     0,     0,     3,    20,    25,    33,     0,    31,    32,     0,
   126,   137,     0,   141,     0,     0,    53,     0,   143,   125,
     0,     0,     0,     0,     0,   146,     0,    49,     0,     0,
   128,    63,     0,     0,    50,    76,    75,   103,   101,   120,
   127,   130,   132,   131,   134,   133,   136,   135,    91,    82,
     0,    23,     0,     0,    86,     0,    87,     0,     0,    28,
    26,   100,    15,     0,   137,   128,   103,   127,     0,   107,
   102,   169,    74,   103,   156,   152,   179,   176,   170,     0,
     8,    51,    52,    55,    54,    56,    64,    66,    57,    77,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    84,    22,     0,    83,     0,
    88,    34,    17,    39,     0,    37,     0,     0,     0,     0,
   129,   144,     0,    79,     0,     0,     0,     0,     0,   151,
     0,   138,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   139,   140,     0,     0,
     0,    85,    89,     0,     0,    36,     0,     0,    48,    41,
    46,    27,     0,     0,   142,   145,     0,     0,   157,   158,
   153,     0,   180,   181,   177,   171,   172,   147,    65,     0,
     0,    72,    71,    69,     0,   104,   106,   109,   111,   113,
   115,   117,   119,   122,   124,    62,    92,     0,    93,     0,
     0,    18,    38,    40,     0,     0,     0,     0,     0,     0,
     0,     0,   184,     0,     0,     0,     0,     0,     0,    67,
    78,    97,     0,    95,    98,     0,    90,    44,    47,    16,
     0,    80,   164,   161,     0,   154,     0,     0,   178,   173,
   175,   150,     0,   148,    73,     0,    94,     0,     0,    59,
     0,     0,   160,     0,   183,   149,    96,     0,    45,     0,
     0,   167,     0,   185,    99,    60,    81,     0,   165,     0,
   168,   186,     0,     0
};

static const short yydefgoto[] = {     3,
     4,     5,    31,     6,     7,     8,    15,    14,    28,   149,
   144,    21,    22,    23,    24,   148,    25,    37,    38,    88,
    39,    89,   145,   146,   147,   184,   279,   190,   191,    57,
    58,    59,    94,   290,    60,   133,    96,   163,    62,   164,
   165,    63,   214,   211,    64,    65,    66,   166,    67,   197,
   281,    83,    84,    85,   230,    86,   134,    87,   253,   254,
   255,    93,    68,   121,   122,   123,   124,   125,   126,   127,
   128,    69,   129,   130,    70,    98,    72,    99,    73,   153,
    74,   110,   248,   274,   161,    75,   156,   242,   106,   155,
   199,   241,   200,   282,   293,   298,    76,    77,   159,   206,
   246,   247,    78,   158,   245,   108,   157,   203,   244,   204,
   267,   300
};

static const short yypact[] = {    -5,
    20,-32768,    10,-32768,-32768,    27,    34,    36,-32768,-32768,
    40,    23,    13,-32768,    16,     0,-32768,-32768,    67,-32768,
    95,     7,     7,-32768,   108,-32768,   114,-32768,-32768,-32768,
   360,     6,-32768,     7,-32768,   133,     9,-32768,-32768,   105,
-32768,   115,   398,-32768,   185,   185,-32768,   137,-32768,-32768,
   398,   398,    93,   101,   146,-32768,   166,-32768,   126,   127,
    99,-32768,   156,   136,-32768,-32768,-32768,   443,   110,   140,
    46,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   153,-32768,    24,   142,-32768,    70,-32768,   168,   172,-32768,
   360,-32768,-32768,   150,-32768,   147,   109,    29,   398,   110,
   110,-32768,   400,   465,-32768,-32768,-32768,-32768,-32768,   174,
-32768,-32768,-32768,-32768,-32768,-32768,   176,   175,-32768,-32768,
   160,   164,   177,   181,   178,   182,   186,   191,   171,   179,
   201,   205,   199,   196,   105,-32768,-32768,   183,-32768,   138,
-32768,-32768,    52,   206,    89,   192,   213,   162,   210,   211,
-32768,   400,    84,-32768,   193,   167,   221,   169,   173,-32768,
   225,-32768,   185,   398,   226,   360,   185,   185,   185,   185,
   185,   185,   185,   185,   185,   185,-32768,-32768,   398,   209,
    94,-32768,-32768,   233,   247,-32768,   172,   172,-32768,   240,
-32768,-32768,   172,   197,-32768,   400,   360,   253,-32768,    32,
-32768,   260,-32768,   188,-32768,-32768,   198,-32768,   234,   140,
    90,   400,-32768,-32768,   236,   110,   110,   110,   110,   110,
   110,   110,   110,   140,   140,   400,-32768,   370,-32768,   255,
   269,-32768,   206,   206,   213,    98,   270,   284,   272,   193,
   204,   193,-32768,   207,   221,   266,   173,   274,   398,-32768,
-32768,    37,   102,-32768,-32768,   250,-32768,-32768,-32768,-32768,
   282,-32768,-32768,-32768,   193,-32768,   216,   221,-32768,-32768,
-32768,-32768,    92,-32768,   400,   370,-32768,    15,   277,-32768,
   360,   398,-32768,   288,-32768,-32768,-32768,   185,-32768,   290,
   322,   436,   291,-32768,   140,-32768,-32768,   398,-32768,   294,
-32768,-32768,   295,-32768
};

static const short yypgoto[] = {-32768,
   300,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -95,-32768,-32768,   292,    48,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    69,   -33,
   -55,-32768,-32768,-32768,-32768,-32768,   -31,-32768,   152,-32768,
-32768,-32768,   106,    74,-32768,   -52,-32768,-32768,-32768,-32768,
-32768,-32768,   241,-32768,-32768,-32768,-32768,-32768,-32768,    47,
-32768,   190,   -35,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   -22,-32768,-32768,  -154,   -30,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -165,-32768,-32768,-32768,    31,-32768,-32768,-32768,-32768,    83,
-32768,-32768,  -156,-32768,-32768,-32768,-32768,  -215,-32768,-32768,
-32768,-32768
};


#define	YYLAST		500


static const short yytable[] = {    61,
    71,   112,   207,    91,   113,    61,    71,    97,   210,   303,
    79,    17,    41,    42,    43,   103,   104,    26,    44,   177,
   224,   225,   100,   101,     9,    61,    71,    17,    79,   269,
    45,    18,    11,     1,   -70,   112,    46,    80,   113,     2,
    47,    12,  -137,    48,    49,    50,    16,   288,     1,    29,
    20,   -70,   285,    27,     2,   136,   -42,   -61,    51,    61,
    71,   132,    30,   152,    19,    81,    20,   -43,    82,  -137,
    35,    90,    52,   256,   264,    13,   266,    53,   132,    54,
    55,    35,   140,    81,   141,    56,   137,    41,    95,    43,
   207,   233,   234,    44,   195,   186,   213,   236,   286,   283,
    32,   187,   249,    33,   249,    45,   228,   240,   229,  -159,
   185,    46,   260,    36,   276,   151,   277,   196,    48,    49,
    50,   256,  -118,  -112,  -110,  -114,  -116,  -108,   212,    40,
   116,   117,   215,   295,    61,    71,  -105,   -35,  -121,  -123,
    92,   102,   -58,   226,   216,   217,   218,   219,   220,   221,
   222,   223,    53,   109,    54,    55,   105,   114,   115,   112,
    56,   118,   113,   238,   107,    61,    71,   119,   135,    41,
    42,    43,   131,   139,   142,    44,   143,   150,   160,   117,
   162,   -68,   112,    61,    71,   113,   167,    45,    41,    95,
    43,   168,   171,    46,    44,   169,   170,    47,   172,   175,
    48,    49,    50,   173,   174,   177,    61,    71,   176,   178,
   179,   180,   212,   275,   182,    51,   183,   189,   185,    48,
    49,    50,   193,   188,   192,   194,   202,   198,   111,    52,
   208,   201,   213,   205,    53,   112,    54,    55,   113,    41,
    42,    43,    56,    54,   227,    44,   292,   291,   231,    61,
    71,   232,   235,    53,   237,    54,    55,    45,   239,    61,
    71,    56,   292,    46,   243,  -182,   -63,    47,  -174,   257,
    48,    49,    50,   258,   270,   261,   263,    41,    95,    43,
   272,   265,   278,    44,   268,    51,   280,    41,    42,    43,
   284,   289,   294,    44,   304,    45,   296,   299,   251,    52,
   302,    46,    10,   259,    53,    45,    54,    55,    48,    49,
    50,    46,    56,    34,   209,    47,   250,   262,    48,    49,
    50,   273,   287,   138,   181,    41,    42,    43,   301,   271,
     0,    44,     0,    51,     0,     0,     0,     0,     0,     0,
     0,     0,    53,    45,    54,    55,     0,    52,     0,    46,
    56,     0,    53,    47,    54,    55,    48,    49,    50,     0,
    56,     0,     0,    41,    42,    43,     0,     0,     0,    44,
     0,    51,     0,    41,   252,    43,     0,     0,     0,    44,
     0,    45,     0,     0,   297,    52,     0,    46,     0,     0,
    53,    47,    54,    55,    48,    49,    50,     0,    56,     0,
     0,    41,    95,    43,    48,    49,    50,    44,     0,    51,
     0,     0,     0,  -118,  -112,  -110,  -114,  -116,  -108,    45,
     0,     0,     0,    52,     0,    46,  -103,  -105,    53,     0,
    54,    55,    48,    49,    50,     0,    56,     0,    53,     0,
    54,    55,  -166,     0,     0,     0,    56,     0,     0,  -118,
  -112,  -110,  -114,  -116,  -108,     0,  -118,  -112,  -110,  -114,
  -116,  -108,  -103,  -105,     0,     0,    53,     0,    54,    55,
  -105,     0,     0,     0,    56,     0,     0,   120,  -118,  -112,
  -110,  -114,  -116,  -108,     0,     0,     0,     0,     0,     0,
     0,     0,  -105,     0,     0,     0,     0,     0,     0,   154
};

static const short yycheck[] = {    31,
    31,    57,   159,    37,    57,    37,    37,    43,   163,     0,
     5,     5,     4,     5,     6,    51,    52,     5,    10,     5,
   175,   176,    45,    46,     5,    57,    57,     5,     5,   245,
    22,     9,     6,    39,     6,    91,    28,    32,    91,    45,
    32,     8,     6,    35,    36,    37,     7,    33,    39,    34,
    44,     6,   268,    41,    45,    32,     5,    12,    50,    91,
    91,    33,    63,    99,    42,    60,    44,    16,    63,    33,
    23,    63,    64,   228,   240,    40,   242,    69,    33,    71,
    72,    34,    13,    60,    15,    77,    63,     4,     5,     6,
   247,   187,   188,    10,    11,     7,     7,   193,     7,   265,
    34,    13,    13,     9,    13,    22,    13,    76,    15,    78,
    13,    28,    15,     6,    13,     7,    15,   153,    35,    36,
    37,   276,    14,    15,    16,    17,    18,    19,   164,    16,
    32,    33,   166,   288,   166,   166,    28,     5,    29,    30,
    36,     5,    28,   179,   167,   168,   169,   170,   171,   172,
   173,   174,    69,     8,    71,    72,    64,    32,    32,   215,
    77,     6,   215,   197,    64,   197,   197,    32,    16,     4,
     5,     6,    33,    32,     7,    10,     5,    28,     5,    33,
     5,     7,   238,   215,   215,   238,    27,    22,     4,     5,
     6,    28,    15,    28,    10,    19,    16,    32,    17,    29,
    35,    36,    37,    18,    14,     5,   238,   238,    30,     5,
    12,    16,   248,   249,    32,    50,    79,     5,    13,    35,
    36,    37,    13,    32,    63,    15,     6,    35,    63,    64,
     6,    65,     7,    65,    69,   291,    71,    72,   291,     4,
     5,     6,    77,    71,    36,    10,   282,   281,    16,   281,
   281,     5,    13,    69,    58,    71,    72,    22,     6,   291,
   291,    77,   298,    28,     5,    78,    33,    32,    71,    15,
    35,    36,    37,     5,     9,     6,     5,     4,     5,     6,
     7,    78,    33,    10,    78,    50,     5,     4,     5,     6,
    75,    15,     5,    10,     0,    22,     7,     7,    63,    64,
     7,    28,     3,   235,    69,    22,    71,    72,    35,    36,
    37,    28,    77,    22,   163,    32,   211,    34,    35,    36,
    37,   248,   276,    83,   135,     4,     5,     6,   298,   247,
    -1,    10,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    69,    22,    71,    72,    -1,    64,    -1,    28,
    77,    -1,    69,    32,    71,    72,    35,    36,    37,    -1,
    77,    -1,    -1,     4,     5,     6,    -1,    -1,    -1,    10,
    -1,    50,    -1,     4,     5,     6,    -1,    -1,    -1,    10,
    -1,    22,    -1,    -1,    63,    64,    -1,    28,    -1,    -1,
    69,    32,    71,    72,    35,    36,    37,    -1,    77,    -1,
    -1,     4,     5,     6,    35,    36,    37,    10,    -1,    50,
    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    22,
    -1,    -1,    -1,    64,    -1,    28,    27,    28,    69,    -1,
    71,    72,    35,    36,    37,    -1,    77,    -1,    69,    -1,
    71,    72,     7,    -1,    -1,    -1,    77,    -1,    -1,    14,
    15,    16,    17,    18,    19,    -1,    14,    15,    16,    17,
    18,    19,    27,    28,    -1,    -1,    69,    -1,    71,    72,
    28,    -1,    -1,    -1,    77,    -1,    -1,    35,    14,    15,
    16,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35
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
#line 50 "grammar.yacc"
{
								
							;
    break;}
case 3:
#line 60 "grammar.yacc"
{						
								if(!computContraint()) return false;
								((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
							;
    break;}
case 4:
#line 67 "grammar.yacc"
{
								((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
							;
    break;}
case 7:
#line 79 "grammar.yacc"
{
									initMessageManager();
							;
    break;}
case 8:
#line 84 "grammar.yacc"
{
								if(!endMessageManager()) return 0;
							;
    break;}
case 9:
#line 90 "grammar.yacc"
{
								if(_ResultCompile != NULL) 
								{
									yyerror("an MessageManager block had all ready declared");
									return 0;
								}								
							;
    break;}
case 11:
#line 103 "grammar.yacc"
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
#line 122 "grammar.yacc"
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
#line 141 "grammar.yacc"
{
								((IClassInterpret *)_SelfClass.get())->setClassName(NLAIAGENT::CStringVarName(LastyyText[1]));
							;
    break;}
case 26:
#line 165 "grammar.yacc"
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
#line 223 "grammar.yacc"
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
#line 282 "grammar.yacc"
{				
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							;
    break;}
case 30:
#line 290 "grammar.yacc"
{
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							;
    break;}
case 33:
#line 307 "grammar.yacc"
{								
								initParam();
							;
    break;}
case 34:
#line 311 "grammar.yacc"
{
								if(!registerMethod()) return false;
							;
    break;}
case 35:
#line 317 "grammar.yacc"
{
								initParam();
							;
    break;}
case 36:
#line 321 "grammar.yacc"
{
								registerMethod();
							;
    break;}
case 42:
#line 339 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[0]);
								_BaseObjectDef = false;
							;
    break;}
case 43:
#line 344 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							;
    break;}
case 44:
#line 349 "grammar.yacc"
{
								_LastBaseObjectDef = NLAIAGENT::CStringVarName(LastyyText[1]);
								_BaseObjectDef = true;

							;
    break;}
case 48:
#line 363 "grammar.yacc"
{
								setParamVarName();
							;
    break;}
case 56:
#line 379 "grammar.yacc"
{
								_LastStringParam.back()->release();
								_LastStringParam.pop_back();
								_Param.back()->release();
								_Param.pop_back();								
							;
    break;}
case 57:
#line 387 "grammar.yacc"
{
								_LastBloc->addCode((new CHaltOpCode));
							;
    break;}
case 58:
#line 393 "grammar.yacc"
{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							;
    break;}
case 59:
#line 401 "grammar.yacc"
{					
								NLAIAGENT::CStringVarName x(LastyyText[1]);			
								if(!castVariable(_LastString,x)) return false;
							;
    break;}
case 61:
#line 408 "grammar.yacc"
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
#line 418 "grammar.yacc"
{
								if(!affectation()) return false;
							;
    break;}
case 64:
#line 426 "grammar.yacc"
{
								
							;
    break;}
case 66:
#line 434 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 68:
#line 442 "grammar.yacc"
{								
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							;
    break;}
case 70:
#line 452 "grammar.yacc"
{
								nameMethodeProcessing();
							;
    break;}
case 71:
#line 459 "grammar.yacc"
{	
								_ExpressionType = _ExpressionTypeTmp.back();
								_ExpressionTypeTmp.pop_back();								
								callFunction();								
							;
    break;}
case 72:
#line 469 "grammar.yacc"
{
								pushParamExpression();								
							;
    break;}
case 73:
#line 474 "grammar.yacc"
{
								pushParamExpression();
							;
    break;}
case 74:
#line 482 "grammar.yacc"
{
								if(!typeOfMethod()) return false;
							;
    break;}
case 77:
#line 493 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 78:
#line 498 "grammar.yacc"
{
								interrogationEnd();
							;
    break;}
case 79:
#line 506 "grammar.yacc"
{								
								ifInterrogation();
							;
    break;}
case 80:
#line 511 "grammar.yacc"
{								
								ifInterrogationPoint()
							;
    break;}
case 81:
#line 516 "grammar.yacc"
{
								ifInterrogationEnd();
							;
    break;}
case 89:
#line 534 "grammar.yacc"
{
								CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);								
								if(c != NULL) c->Local = true;								
							;
    break;}
case 91:
#line 542 "grammar.yacc"
{								
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
								_LastRegistered = ((IClassInterpret *)_SelfClass.get())->registerComponent(_LastString);
							;
    break;}
case 92:
#line 548 "grammar.yacc"
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
#line 580 "grammar.yacc"
{	
								if(_FacteurEval)
								{								
									allocExpression(NULL);
									setTypeExpression();
								}
							;
    break;}
case 102:
#line 588 "grammar.yacc"
{								
								allocExpression(new CNegOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opAdd,"(-)");
								
							;
    break;}
case 103:
#line 594 "grammar.yacc"
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
#line 606 "grammar.yacc"
{									
								allocExpression(new CAddOpCode,true);		
								setTypeExpressionD(NLAIC::CTypeOfOperator::opAdd,"+");	
							;
    break;}
case 105:
#line 611 "grammar.yacc"
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
#line 622 "grammar.yacc"
{								
								allocExpression(new CSubOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSub,"-");							
							;
    break;}
case 107:
#line 627 "grammar.yacc"
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
#line 638 "grammar.yacc"
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
#line 648 "grammar.yacc"
{								
								allocExpression(new CDiffOpCode,true);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiff,"!=");
								
							;
    break;}
case 110:
#line 654 "grammar.yacc"
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
#line 665 "grammar.yacc"
{								
								allocExpression(new CInfOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInf,"<");							
							;
    break;}
case 112:
#line 670 "grammar.yacc"
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
#line 680 "grammar.yacc"
{								
								allocExpression(new CSupOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSup,">");							
							;
    break;}
case 114:
#line 685 "grammar.yacc"
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
#line 695 "grammar.yacc"
{								
								allocExpression(new CSupEqOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSupEq,">=");
							;
    break;}
case 116:
#line 700 "grammar.yacc"
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
#line 710 "grammar.yacc"
{								
								allocExpression(new CInfEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInfEq,"<=");
							;
    break;}
case 118:
#line 715 "grammar.yacc"
{
								setTypeExpressionG();	
								allocExpression(NULL);														
							;
    break;}
case 119:
#line 720 "grammar.yacc"
{								
								allocExpression(new CEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opEq,"==");

							;
    break;}
case 120:
#line 728 "grammar.yacc"
{
								_FacteurEval = true;	
							;
    break;}
case 121:
#line 732 "grammar.yacc"
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
#line 743 "grammar.yacc"
{									
								setTypeExpressionD(NLAIC::CTypeOfOperator::opMul,"+");
								allocExpression(new CMulOpCode,false);
								_FacteurEval = false;
							;
    break;}
case 123:
#line 749 "grammar.yacc"
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
#line 761 "grammar.yacc"
{																
								allocExpression(new CDivOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiv,"/");
								_FacteurEval = false;	
							;
    break;}
case 125:
#line 769 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVarNill();
							;
    break;}
case 126:
#line 775 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
								setImediateVar();
							;
    break;}
case 127:
#line 780 "grammar.yacc"
{								
								_IsFacteurIsExpression = false;
								if(!processingVar()) return false;
							;
    break;}
case 128:
#line 785 "grammar.yacc"
{								
								_IsFacteurIsExpression = true;								
								setMethodVar();
							;
    break;}
case 129:
#line 790 "grammar.yacc"
{							
								_IsFacteurIsExpression = true;
							;
    break;}
case 130:
#line 794 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 131:
#line 798 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 132:
#line 802 "grammar.yacc"
{							
								_IsFacteurIsExpression = false;
							;
    break;}
case 133:
#line 806 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 134:
#line 810 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 135:
#line 814 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 136:
#line 818 "grammar.yacc"
{
								_IsFacteurIsExpression = false;
							;
    break;}
case 137:
#line 825 "grammar.yacc"
{
								_LasVarStr.clear();
								_LasVarStr.push_back(NLAISCRIPT::CStringType(LastyyText[1]));
								_LastFact.VarType = varTypeUndef;
								_IsFacteurIsExpression = false;								
							;
    break;}
case 138:
#line 833 "grammar.yacc"
{
								_LasVarStr.clear();
								cleanTypeList();
								_LasVarStr.push_back(LastyyText[1]);
								setMethodVar();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();								
							;
    break;}
case 139:
#line 843 "grammar.yacc"
{	
								cleanTypeList();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();				
							;
    break;}
case 140:
#line 851 "grammar.yacc"
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
case 141:
#line 867 "grammar.yacc"
{									
								setListVar();
							;
    break;}
case 143:
#line 875 "grammar.yacc"
{									
								setChaineVar();
							;
    break;}
case 144:
#line 881 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));								
							;
    break;}
case 145:
#line 885 "grammar.yacc"
{								
								_LastBloc->addCode((new CAddOpCode));
							;
    break;}
case 146:
#line 891 "grammar.yacc"
{
								_LastStringParam.push_back(new NLAIAGENT::CGroupType());								

							;
    break;}
case 147:
#line 896 "grammar.yacc"
{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
							;
    break;}
case 149:
#line 902 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 150:
#line 906 "grammar.yacc"
{
								if(!buildObject()) return false;
							;
    break;}
case 151:
#line 913 "grammar.yacc"
{							
								_LastStringParam.back()->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(LastyyText[1])));
								_Param.push_back(new CParam);								
							;
    break;}
case 152:
#line 920 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );				
						;
    break;}
case 153:
#line 925 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 154:
#line 929 "grammar.yacc"
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
case 156:
#line 944 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 157:
#line 948 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbNewOpCode( CClause() ) );
							//_LastBloc->addCode( new CAddOpCode() );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::CClause::idCClause;
						;
    break;}
case 158:
#line 960 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 159:
#line 964 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 160:
#line 968 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );							
						;
    break;}
case 161:
#line 973 "grammar.yacc"
{
							//_LastBloc->addCode( new CAddOpCode() );													
						;
    break;}
case 162:
#line 979 "grammar.yacc"
{
							_LastBloc->addCode( new CTellOpCode() );
						;
    break;}
case 163:
#line 985 "grammar.yacc"
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
case 164:
#line 998 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
							char *txt = LastyyText[1];
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode(new CAddOpCode() );
							_LastAssert = NLAIAGENT::CStringVarName(LastyyText[1]);
						;
    break;}
case 165:
#line 1006 "grammar.yacc"
{
							setStackVar( CFactPattern::IdFactPattern );
							IBaseAssert *my_assert = _FactBase.addAssert( _LastAssert, _NbLogicParams );
							_NbLogicParams = 0;
							_LastBloc->addCode(new CLdbNewOpCode( CFactPattern( my_assert ) ) );
						;
    break;}
case 166:
#line 1016 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 167:
#line 1022 "grammar.yacc"
{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							;
    break;}
case 169:
#line 1031 "grammar.yacc"
{
							char buf[256];
							strcpy(buf, LastyyText[1]);
							setStackVar( CVar::IdVar );
							_LastBloc->addCode( new CLdbOpCode(CVar( LastyyText[1] )) );
						;
    break;}
case 170:
#line 1041 "grammar.yacc"
{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
						;
    break;}
case 172:
#line 1048 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 173:
#line 1052 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 174:
#line 1056 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 175:
#line 1060 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++);
						;
    break;}
case 176:
#line 1066 "grammar.yacc"
{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );
						;
    break;}
case 177:
#line 1071 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 178:
#line 1075 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyRule() ) );		
							setStackVar(CFuzzyRule::IdFuzzyRule);
						;
    break;}
case 179:
#line 1083 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 180:
#line 1087 "grammar.yacc"
{
							
						;
    break;}
case 181:
#line 1093 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 182:
#line 1097 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 184:
#line 1104 "grammar.yacc"
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
case 185:
#line 1122 "grammar.yacc"
{
							_LastString = NLAIAGENT::CStringVarName( LastyyText[0] );
						;
    break;}
case 186:
#line 1126 "grammar.yacc"
{
							for (sint32 i = 0; i < 20; i++ );
							_LastBloc->addCode(new CLdbNewOpCode( CSimpleFuzzyCond(NULL, NULL) ) );
						;
    break;}
case 187:
#line 1133 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 188:
#line 1137 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 189:
#line 1142 "grammar.yacc"
{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						;
    break;}
case 190:
#line 1146 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyVar(NLAIAGENT::CStringVarName("Inst"),0,1) ) );
						;
    break;}
case 191:
#line 1153 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 192:
#line 1157 "grammar.yacc"
{
							_LastBloc->addCode( new CAddOpCode() );
						;
    break;}
case 194:
#line 1164 "grammar.yacc"
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
#line 1168 "grammar.yacc"

