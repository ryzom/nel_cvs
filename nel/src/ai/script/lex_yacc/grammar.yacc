%{
#include <malloc.h>
#include <list>
#define PARSER_NO_REDEF_YYTABLE_H_
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/constraint.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/logic/logic.h"
#include "nel/ai/fuzzy/fuzzy.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/interpret_fsm.h"

using  namespace NLAISCRIPT;
using  namespace NLAILOGIC;
using  namespace NLAIFUZZY;
%}
%token	FIN
%token	NOMBRE		IDENT
%token	PAR_G		PAR_D		ACCOL_G		ACCOL_D	CROCHER_G CROCHER_D
%token	EG_MATH		VIRGULE
%token	EG_LOG		SUP			INF			SUP_EG			INF_EG		DIFF
%token	OR_LOG		AND_LOG
%token	NON_BIN		OR_BIN		AND_BIN		XOR_BIN		SCOP
%token	PLUS		MOINS		FOIS		DIV			POWER
%token	POINT_VI	LEPOINT		POINT_DEUX	INTERROGATION CHAINE NILL 
%token	EXEC ACHIEVE ASK BREAK TELL KILL PERROR EVEN

%token  BEGIN_GRAMMAR


// Agent class defintion tokens
%token	FROM 
%token	DEFINE	GROUP
%token	COMPONENT CONSTRUCTION DESTRUCTION MESSAGE_MANAGER MAESSAGELOOP
%token	LOCAL STATIC

%token	AS
%token	DIGITAL	COLLECTOR	
%token	WITH DO 
%token	END IF THEN BEGINING
%token	END_GRAMMAR

// New token used for object instanciation
%token	NEW 

// Operator tokens
%token	TRIGGER	PRECONDITION POSTCONDITION GOAL RETURN COMMENT STEPS UPDATEEVERY PRIORITY 
%token	MSG

// Logic tokens
%token	LOGICVAR RULE IA_ASSERT OR AND

// Fuzzy sets and rules tokens
%token	FUZZY FUZZYRULE FUZZYRULESET SETS FUZZYVAR FIS 

// Binary operations tokens
%left	NON_BIN		OR_BIN		AND_BIN		XOR_BIN

// Arithmetic functions tokens
%left	PLUS		MOINS		
%left	FOIS		DIV		POWER
%token	COS SIN TAN POW LN LOG FACT 


%%

	program				:	DefinitionClass
							{
								
							}
						|	program DefinitionClass
						;

	DefinitionClass		:	CorpDeDefinition 
							ACCOL_G
							DefinitionDeProgram
							ACCOL_D
							{						
								if(!computContraint()) return false;
								onEndClass();
							}								
						|	CorpDeDefinition 
							ACCOL_G 
							ACCOL_D
							{
								onEndClass();								
							}
						|	MessageRun
						;

	MessageRun			:	MessageManager
							PAR_G PAR_D

							END
						|	MessageManager 							
							PAR_G PAR_D	
							{
								initMessageManager();
							}			
							DuCode
							END
							{
								if(!endMessageManager()) return 0;
							}
						;

	MessageManager		:	MESSAGE_MANAGER 							
							{
								if(_ResultCompile != NULL) 
								{
									yyerror("an MessageManager block had all ready declared");
									return 0;
								}								
							}
						;

	CorpDeDefinition	:	HeritageDeType EnteteDeDefinition
						;

	HeritageDeType		:	FROM IDENT
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
							}
							POINT_DEUX
						;	

	EnteteDeDefinition	:	DEFINE DefinitionDeGroup
						|	DEFINE IDENT
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
									( (IClassInterpret *) _SelfClass.get() )->setClassName(NLAIAGENT::CStringVarName(LastyyText[1]));
									( (IClassInterpret *) _SelfClass.get() )->buildVTable();
									RegisterClass();
								}
							}
						;

	DefinitionDeGroup	:	GROUP INF Nom 
							{
								((IClassInterpret *)_SelfClass.get())->setClassName(NLAIAGENT::CStringVarName(LastyyText[1]));
							}	VIRGULE CParam SUP
						;

	CParam				:	IDENT
						|	CParam VIRGULE IDENT
						;	
	DefinitionDeProgram	:	BlocDeDefinition						
						|	DefinitionDeProgram
						;
	
	BlocDeDefinition	:	UnBloc
						|	UnBloc BlocDeDefinition;
							

	UnBloc				:	Register
						|	BlocPourLesCode
						|	RegisterOperator
							{
								if ( classIsAnOperator() )
								{
									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
									op_class->buildLogicTables();
								}
							}
						|	RegisterSteps
						;

	Register			:	RegistDesAttributs
						;

	RegistDesAttributs	:	COMPONENT POINT_DEUX TypeDeDeclaration END;
						|	COMPONENT POINT_DEUX END
						;

	RegisterOperator	:	OpBloc
						|	OpBloc RegisterOperator
						;

	RegisterSteps		:	STEPS 
							POINT_DEUX 
							Steps;

	Steps				:	IDENT 
							{
#ifdef NL_DEBUG
								const char *dbg_y0 = LastyyText[0];
								const char *dbg_y1 = LastyyText[1];
#endif
								CSeqFsmClass *fsm_class = (CSeqFsmClass *) _SelfClass.get();
								fsm_class->addStep( NLAIAGENT::CStringVarName( LastyyText[1] ) );
							}
							POINT_VI
						|	IDENT
							{
#ifdef NL_DEBUG
								const char *dbg_y0 = LastyyText[0];
								const char *dbg_y1 = LastyyText[1];
#endif
								CSeqFsmClass *fsm_class = (CSeqFsmClass *) _SelfClass.get();
								fsm_class->addStep( NLAIAGENT::CStringVarName( LastyyText[0] ) );
							}
							Steps
						;

	OpBloc				:	OpComment
						|	PreCondition
						|	PostCondition 
						|	Goal
						|	UpdateCycles
						|	Priority
						;	

	

	OpComment			:	COMMENT
							POINT_DEUX
							CHAINE
							{
								char *txt1 = LastyyText[0];
								char *txt2 = LastyyText[1];
								if ( classIsAnOperator() )
								{
									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
									op_class->setComment( LastyyText[1] );
								}


							}
							;

	PreCondition		:	PRECONDITION POINT_DEUX 
							{
								is_cond = true;
								initParam();
								_InCond = true;
							}
							OperatorCond
							{
								// Adds the conds to the operator class
								if ( classIsAnOperator() )
								{
									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
									while ( _LastCodeBranche.size() )
									{
										op_class->addCodeCond( _LastCodeBranche.front() );
										_LastCodeBranche.pop_front();
									}
								}
								_InCond = false;
							}
							END
							;


	Goal				:	GOAL
							{
								if ( !classIsAnOperator() )
								 	return 0;
							} 
							POINT_DEUX
							FirstOrderPattern
							{
								if ( _Goal ) 
								{
									_Goal->release();
									_Goal = NULL;
								}
								
								if ( classIsAnOperator() )
								{
									// Builds a FactPattern

									// Adds it as goal to the operator class
									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
									op_class->setGoal( _LastAsserts.back(), _LastLogicParams.back() );
//									((NLAIAGENT::IObjectIA *)(_LastAsserts.back()))->release();
									_LastAsserts.pop_back();
//									((NLAIAGENT::IObjectIA *)(_LastLogicParams.back()))->release();
									_LastLogicParams.pop_back();
								}
							}
							POINT_VI
							;

	UpdateCycles			:	UPDATEEVERY POINT_DEUX NOMBRE	
								{
									if ( classIsAnOperator() )
									{
										sint32 update = (sint32) LastyyNum;
										COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
										op_class->setUpdateEvery( update );
										if(_LastFact.Value != NULL) 
										{
											_LastFact.Value->release();
											_LastFact.Value = NULL;
										}

										if(_FlotingExpressionType != NULL)
										{
											_FlotingExpressionType->release();
											_FlotingExpressionType = NULL;
										}
									}	
								}
								POINT_VI
							;

	Priority				:	PRIORITY POINT_DEUX NOMBRE	
								{
									if ( classIsAnOperator() )
									{
										float priority = (float) LastyyNum;
										COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
										op_class->setPriority( priority );
										if(_LastFact.Value != NULL) 
										{
											_LastFact.Value->release();
											_LastFact.Value = NULL;
										}

										if(_FlotingExpressionType != NULL)
										{
											_FlotingExpressionType->release();
											_FlotingExpressionType = NULL;
										}
									}	
								}
								POINT_VI
							;




	PostCondition		:	POSTCONDITION POINT_DEUX 
							{
								is_cond = false;
								initParam();
								_InCond = true;
							}
							OperatorCond
							{
								// Adds the conds to the operator class
								if ( classIsAnOperator() )
								{

									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
/*									while ( _LastAsserts.size() )
									{
										op_class->addFirstOrderConc( _LastAsserts.back(), _LastLogicParams.back() );
										_LastAsserts.pop_back();
										_LastLogicParams.pop_back();
									}
									*/

									while ( _LastCodeBranche.size() )
									{
										op_class->addCodeConc( _LastCodeBranche.back() );
										_LastCodeBranche.pop_back();
									}
								}
								_InCond = false;
							}
							END
							;

	OperatorCond		:	SingleOpCond 
							{
#ifdef NL_DEBUG
								for (int i = 0; i < 20; i++);	// To put breakpoints for debugging...
#endif
							}
						|	SingleOpCond 
							{
#ifdef NL_DEBUG
								for (int i = 0; i < 20; i++);	// To put breakpoints for debugging...
#endif
							}
							OperatorCond
							{
#ifdef NL_DEBUG
								for (int i = 0; i < 20; i++);	// To put breakpoints for debugging...
#endif
							}

						;

	SingleOpCond		:	BooleanCond
							{
#ifdef NL_DEBUG
								for (int i = 0; i < 20; i++);	// To put breakpoints for debugging...
#endif
							}
							POINT_VI
						|	FuzzyCond
							{
								if ( classIsAnOperator() )
								{

									NLAIAGENT::CStringVarName *vvar = _LastFuzzyConds.back();
									_LastFuzzyConds.pop_back();

									NLAIAGENT::CStringVarName *vset = _LastFuzzyConds.back();
									_LastFuzzyConds.pop_back();

									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
									op_class->addFuzzyCond(vvar, vset);
								}
							}
						|	FirstOrderPattern
							POINT_VI
							{									
								COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
								while ( _LastAsserts.size() )
								{
									if ( is_cond )
										op_class->addFirstOrderCond( _LastAsserts.back(), _LastLogicParams.back() );
									else
										op_class->addFirstOrderConc( _LastAsserts.back(), _LastLogicParams.back() );

									_LastAsserts.pop_back();
									_LastLogicParams.pop_back();
								}
							}
						|	ACCOL_G
							{
								clean();
								_LastBloc = new IBlock(_Debug);
								_Heap -= (sint32)_Heap;
								CVarPStack::_LocalTableRef = &_Heap[0];
								_VarState.clear();								
								_VarState.pushMark();
								if(!_InLineParse) addOpCode((new CMarkAlloc));
							}
							DuCode
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
										addOpCode(x);
									}									
									addOpCode(new CHaltOpCode());

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
//										((IClassInterpret *)_SelfClass.get())->getBrancheCode().setCode((IOpCode *)listCode);										
										_LastCodeBranche.push_back( (IOpCode *) listCode );
										_Heap -= (sint32)_Heap;
										if(_Heap.restoreStackState()) _Heap.restoreStack();
										if(_Heap.restoreShiftState()) _Heap.restoreShift();
										_Heap -= (sint32)_Heap;									
										//listCode->release();
									}
												
									_DecalageHeap = 0;
									CVarPStackParam::_Shift = 0;
								}							

/*								if(_IsVoid)
								{
									IOpType *x = new COperandVoid();									
									((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(x);
								}
								else
								{
									_IsVoid = true;
								}				
*/				
							}
							ACCOL_D
							| TrigMsgCond
							{
								if ( classIsAnOperator() )
								{
									const char *msg_name = _LastBooleanConds.back()->getString();
									_LastBooleanConds.pop_back();
									const char *class_name = _LastBooleanConds.back()->getString();
									_LastBooleanConds.pop_back();
									COperatorClass *op_class = (COperatorClass *) _SelfClass.get();
									try
									{	
										op_class->RegisterMessage(_LastPerf, class_name, msg_name );
									}
									catch(NLAIE::IException &a)
									{									
										yyerror((char *)a.what());
										return 0;
									}
								}
							}
							;

	BooleanCond			:	INTERROGATION IDENT
						{
							char *param_name = LastyyText[1];
							_LastBooleanConds.push_back( new NLAIAGENT::CStringVarName( param_name ) );
						}
						;


	Performatif			: ASK
						{
							_LastPerf = NLAIAGENT::IMessageBase::PAsk;
						}
						| TELL
						{
							_LastPerf = NLAIAGENT::IMessageBase::PTell;
						}
						| ACHIEVE
						{
							_LastPerf = NLAIAGENT::IMessageBase::PAchieve;
						}
						;


						


	TrigMsgCond			:	MSG PAR_G 
							IDENT
							{
								char *param_name = LastyyText[1];
								_LastBooleanConds.push_back( new NLAIAGENT::CStringVarName( param_name ) );
							}
							Performatif
							IDENT
							{
								char *param_name = LastyyText[1];
								_LastBooleanConds.push_back( new NLAIAGENT::CStringVarName( param_name ) );
							}
							PAR_D POINT_VI
							;


	AskCond					: ASK PAR_G
							IDENT
							{
								
							}
							ACCOL_G
							DuCode
							ACCOL_D
							;


	FuzzyCond			:	FUZZY	PAR_G
							IDENT
							{
								char *param_name = LastyyText[1];
								_LastFuzzyConds.push_back( new NLAIAGENT::CStringVarName( param_name ) );
							}
							FIS 
							{
								for (int i = 0; i < 20; i++);  // To put breakpoints for debugging...
							}
							IDENT
							{
								char *param_name = LastyyText[1];
								_LastFuzzyConds.push_back( new NLAIAGENT::CStringVarName( param_name ) );
							}
							PAR_D POINT_VI
							;

	FirstOrderPattern	: INTERROGATION 
						PAR_G 
						IDENT 
						{
							const char *assert_name = LastyyText[1];
							_LastAsserts.push_back( new NLAIAGENT::CStringVarName( LastyyText[1] ) );
							_LastLogicParams.push_back( std::list<const NLAIAGENT::IVarName *>() );
						}
						OpLogicVarSet 
						PAR_D
						;


	OpLogicVarSet		:	OpLogicVar
							{
								for (int i = 0; i < 20; i++);  // To put breakpoints for debugging...
							}
						|	OpLogicVar
							{
								for (int i = 0; i < 20; i++);  // To put breakpoints for debugging...
							}
							OpLogicVarSet
						;

	OpLogicVar			:	INTERROGATION IDENT
						{
							char *var_name = LastyyText[1];
							_LastLogicParams.back().push_back( new NLAIAGENT::CStringVarName( var_name ) );
						}
						;


	BlocPourLesCode		:	BlocAvecCode
						|	BlocPourLesCode BlocAvecCode
						;

	BlocAvecCode		:	Methode
							Argument DuCode
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
							}
							END							
						|	Methode 							
							Argument END
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

							}
						;
							

	Methode				:	IDENT
							{				
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							}						
						|	DESTRUCTION
							{
								NLAIAGENT::CStringVarName name(LastyyText[1]);
								//name += NLAIAGENT::CStringVarName(LastyyText[1]);
								//((IClassInterpret *)_SelfClass.get())->addBrancheCode(name);
								_MethodName.push_back(name);
								_IsVoid = true;
							}
						;				
										
	Argument			:	ArgumentVide
						|	ArgumentListe
						;

	ArgumentVide		:	PAR_G
							{								
								initParam();
							}
							PAR_D
							{
								if(!registerMethod()) return false;
							}
						;

	ArgumentListe		:	PAR_G
							{
								initParam();
							} 
							ListesDeVariables PAR_D
							{
								registerMethod();
							}
						;

	ListesDeVariables	:	DesParams
						|	ListesDeVariables VIRGULE CParam
						;

	DesParams			:	CParam
						|	DesParams POINT_VI CParam
						;

	CParam				:	TypeOfParam
							DeclarationVariables
						;

	TypeOfParam			:	IDENT
							{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[0]);
								_BaseObjectDef = false;
							}							
						|	IDENT 
							{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							}							
							INF
							IDENT
							{
								_LastBaseObjectDef = NLAIAGENT::CStringVarName(LastyyText[1]);
								_BaseObjectDef = true;

							}
							SUP
						;
							

	DeclarationVariables:	NonDeVariable
						|	DeclarationVariables VIRGULE NonDeVariable
						;

	NonDeVariable		:	IDENT
							{
								if(!setParamVarName()) return 0;
							}
						;


	DuCode				:	Code
						|	Evaluation						
						|	DuCode Code
						|	DuCode Evaluation
						;

	Code				:	POINT_VI
						|	Affectation POINT_VI
						|	StaticCast POINT_VI
						|	AppelleDeFonction POINT_VI
							{								
								if(!checkReturnMethod())
								{
									return 0;
								}
								_LastStringParam.back()->release();
								_LastStringParam.pop_back();
								_Param.back()->release();
								_Param.pop_back();								
							}
						|	RetourDeFonction 
							POINT_VI
							{
								if(!_InLineParse)
								{
									
									IOpCode *x;				
									int i;
									for(i = 0; i < _VarState.markSize(); i++)
									{
										if (_Debug)
										{
											x = new CFreeAllocDebug();
										}
										else
										{
											x = new CFreeAlloc();
										}
										addOpCode(x);										
									}
								}								
								addOpCode(new CHaltOpCode());


							}
						;

	StaticCast			:	IDENT 
							{
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
							}
							MOINS
							SUP
							AS 
							PAR_G 
							IDENT
							{					
								NLAIAGENT::CStringVarName x(LastyyText[1]);			
								if(!castVariable(_LastString,x)) return false;
							}
							PAR_D;

	Affectation			:	Variable
							{								
								std::list<NLAISCRIPT::CStringType>::iterator i = _LasVarStr.begin();
								_LasAffectationVarStr.clear();
								while(i != _LasVarStr.end())
								{
									_LasAffectationVarStr.push_back(*i++);
								}												
							}
							EG_MATH
							Expression
							{
								if(!affectation()) 
								{
									_FromStack.pop_back();
									return false;
								}
								_FromStack.pop_back();
							}
						;
	

	AppelleDeFonction	:	AppelleDeFoncDirect
						|	AppelleDeFonction LEPOINT
							{
								
							}
							AppelleDeFoncDirect
						;	
	

	AppelleDeFoncDirect:	NonDeFonction PAR_G 
							{								
								addParamCont();
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							}
							Prametre CallFunction
						|	NonDeFonction PAR_G
							{	
								addParamCont();							
								_Param.push_back(new CParam);								
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							}
							CallFunction
						;

	NonDeFonction		:	Variable
							{
								nameMethodeProcessing();
							}						
						;
	

	CallFunction		:	PAR_D
							{	
								_ExpressionType = _ExpressionTypeTmp.back();
								_ExpressionTypeTmp.pop_back();								
								if(!callFunction())
								{
									_FromStack.pop_back();
									return 0;	
								}
								_FromStack.pop_back();
							}
						;
	
	Prametre			:	Expression
							{
								pushParamExpression();								
							}
						|	Prametre VIRGULE 
							Expression
							{
								pushParamExpression();
							}
						;
	
	RetourDeFonction	:	RETURN								
							Expression
							{
								if ( !_InCond )
									if(!typeOfMethod()) return false;
							}
						;		

	Evaluation			:	EvaluationState
						|	EvaluationSimpleState
						;

	EvaluationSimpleState:	Expression 
							INTERROGATION
							{								
								ifInterrogation();
							}
							DuCode
							END
							{
								interrogationEnd();
							}
						;

	EvaluationState		:	IF
							Expression 
							INTERROGATION
							{								
								ifInterrogation();
							}
							DuCode							
							POINT_DEUX
							{								
								ifInterrogationPoint()
							}
							DuCode							
							END
							{
								ifInterrogationEnd();
							}
						;

	TypeDeDeclaration	:	POINT_VI
						|	RegisterAnyVar POINT_VI 
						|	TypeDeDeclaration POINT_VI 
						|	TypeDeDeclaration RegisterAnyVar POINT_VI
						;

	RegisterAnyVar		:	RegisterTypeDef
						|	RegisterCollector
						;	
						

	RegisterTypeDef		:	TypeDeComp SUP
						|	TypeDeComp VIRGULE LOCAL 
							{
								CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);								
								if(c != NULL) c->Local = true;								
							}
							SUP
						|	StaticDef
						;


	StaticDef			:	TypeDeComp VIRGULE STATIC 
							{
								CComponent *c = ((IClassInterpret *)_SelfClass.get())->getComponent(_LastRegistered);								
								if(c != NULL) c->Static = true;
							}
							SUP
						;

	TypeDeComp			:	IDENT
							{								
								_LastString = NLAIAGENT::CStringVarName(LastyyText[1]);
								_LastRegistered = ((IClassInterpret *)_SelfClass.get())->registerComponent(_LastString);
							}
							INF
							CHAINE 
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
							}
						;

	RegisterCollector	:	COLLECTOR INF Nom SUP
						|	COLLECTOR INF Nom VIRGULE MessageType SUP
						;

	MessageType			:	DefMessage
						|	MessageType VIRGULE DefMessage
						;

	DefMessage			:	IDENT
						|	Borne
						;

	Borne				:	Facteur LEPOINT LEPOINT Facteur

	Nom					:	CHAINE
						;

	Expression			:	Term
							{	
								if(_FacteurEval)
								{								
									allocExpression(NULL);
									setTypeExpression();
								}
							}
						|	MOINS Term
							{								
								allocExpression(new CNegOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opAdd,"(-)");
								
							}		
						|	Expression 
							{	
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opAdd))
								{
									yyerror("erreur sémantique l'operateur + n'est pas supporté par cette expression");
									return 0;
								}*/								
								setTypeExpressionG();
								allocExpression(NULL);
								
								
							}	
							PLUS Term
							{									
								allocExpression(new CAddOpCode,true);		
								setTypeExpressionD(NLAIC::CTypeOfOperator::opAdd,"+");	
							}
						|	Expression 
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opSub))
								{
									yyerror("erreur sémantique l'operateur - n'est pas supporté par cette expression");
									return 0;
								}*/				
								setTypeExpressionG();				
								allocExpression(NULL);
								
							}
							MOINS Term
							{								
								allocExpression(new CSubOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSub,"-");							
							}	
						|	NON_BIN Term
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opNot))
								{
									yyerror("erreur sémantique l'operateur ! n'est pas supporté par cette expression");
									return 0;
								}*/								
								allocExpression(new CNotOpCode);
								setTypeExpression(NLAIC::CTypeOfOperator::opNot,"!");
								
							}	
						|	Expression 
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opDiff))
								{
									yyerror("erreur sémantique l'operateur != n'est pas supporté par cette expression");
									return 0;
								}*/					
								setTypeExpressionG();			
								allocExpression(NULL);								
							}
							DIFF Term
							{								
								allocExpression(new CDiffOpCode,true);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiff,"!=");
								
							}	
						|	Expression
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opInf))
								{
									yyerror("erreur sémantique l'operateur < n'est pas supporté par cette expression");
									return 0;
								}*/					
								setTypeExpressionG();			
								allocExpression(NULL);
								
							}
							INF Term
							{								
								allocExpression(new CInfOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInf,"<");							
							}	
						|	Expression
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opSup))
								{
									yyerror("erreur sémantique l'operateur > n'est pas supporté par cette expression");
									return 0;
								}*/				
								setTypeExpressionG();				
								allocExpression(NULL);								
							}
							SUP Term
							{								
								allocExpression(new CSupOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSup,">");							
							}	
						|	Expression 
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opSupEq))
								{
									yyerror("erreur sémantique l'operateur >= n'est pas supporté par cette expression");
									return 0;
								}*/					
								setTypeExpressionG();			
								allocExpression(NULL);
							}
							SUP_EG Term
							{								
								allocExpression(new CSupEqOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opSupEq,">=");
							}	
						|	Expression 
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opInfEq))
								{
									yyerror("erreur sémantique l'operateur <= n'est pas supporté par cette expression");
									return 0;
								}*/
								setTypeExpressionG();			
								allocExpression(NULL);								
							}
							INF_EG Term
							{								
								allocExpression(new CInfEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opInfEq,"<=");
							}	
						|	Expression
							{
								setTypeExpressionG();	
								allocExpression(NULL);														
							}
							EG_LOG Term
							{								
								allocExpression(new CEqOpCode,false);
								setTypeExpressionD(NLAIC::CTypeOfOperator::opEq,"==");

							}	
						;

	Term				:	Facteur
							{
								_FacteurEval = true;	
							}
						|	Term 
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opMul))
								{
									yyerror("erreur sémantique l'operateur * n'est pas supporté par cette expression");
									return 0;
								}*/
								setTypeExpressionG();								
								allocExpression(NULL);
								_FacteurEval = false;								
							}
							FOIS Facteur
							{									
								setTypeExpressionD(NLAIC::CTypeOfOperator::opMul,"+");
								allocExpression(new CMulOpCode,false);
								_FacteurEval = false;
							}	
						|	Term 
							{
								/*if(!(_lastOperatorType & NLAIC::CTypeOfOperator::opDiv))
								{
									yyerror("erreur sémantique l'operateur / n'est pas supporté par cette expression");
									return 0;
								}*/
											
								setTypeExpressionG();
								allocExpression(NULL);
								_FacteurEval = false;								
							}
							DIV Facteur
							{																
								allocExpression(new CDivOpCode,false);	
								setTypeExpressionD(NLAIC::CTypeOfOperator::opDiv,"/");
								_FacteurEval = false;	
							}	
						;

	Facteur				:	
							EXEC
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PExec);
							}
						|	ACHIEVE 
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAchieve);
							}
						|	ASK 
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PAsk);
							}
						|	BREAK 
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PBreak);
							}
						|	PERROR
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PError);
							}
						|	TELL 
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PTell);
							}
						|	KILL
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PKill);
							}
						|	EVEN
							{
								_IsFacteurIsExpression = false;
								setPerformative(NLAIAGENT::IMessageBase::PEven);
							}
						|	NILL
							{
								_IsFacteurIsExpression = false;
								setImediateVarNill();
							}
						|
							NOMBRE
							{
								_IsFacteurIsExpression = false;
								setImediateVar();
							}							
						|	Variable
							{								
								_IsFacteurIsExpression = false;
								if(!processingVar())
								{
									_FromStack.pop_back();
									return false;
								}
								_FromStack.pop_back();
							}	
						|	AppelleDeFonction
							{								
								_IsFacteurIsExpression = true;								
								setMethodVar();
							}	
						|	PAR_G Expression PAR_D
							{							
								_IsFacteurIsExpression = true;
							}
						|	List
							{							
								_IsFacteurIsExpression = false;
							}
						|	NewObject
							{							
								_IsFacteurIsExpression = false;
							}
						|	ChaineDeCaractaire
							{							
								_IsFacteurIsExpression = false;
							}
						|	LogicVar
							{
								_IsFacteurIsExpression = false;
							}
						|	FactPattern
							{
								_IsFacteurIsExpression = false;
							}
						|	Rule
							{
								_IsFacteurIsExpression = false;
							}
						|	FuzzyRule
							{
								_IsFacteurIsExpression = false;
							}
						|	FuzzyRuleSet
							{
								_IsFacteurIsExpression = false;
							}
						;
						

	Variable			:	IDENT
							{
								_LasVarStr.clear();
								_LasVarStr.push_back(NLAISCRIPT::CStringType(LastyyText[1]));
								_LastFact.VarType = varTypeUndef;
								_IsFacteurIsExpression = false;	
								_FromStack.push_back(false);							
							}

						|	AppelleDeFonction LEPOINT IDENT
							{
								_LasVarStr.clear();
								cleanTypeList();
								_LasVarStr.push_back(LastyyText[1]);
								setMethodVar();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();
								_FromStack.push_back(true);

							}

						|	Facteur LEPOINT IDENT
							{	
								cleanTypeList();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();
								_FromStack.push_back(true);
							}						

						|	Variable
							LEPOINT IDENT
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
							}						
						
						;

	List				:	CROCHER_G
							{									
								setListVar();
							}
							ElementList
							CROCHER_D
						;

	ChaineDeCaractaire	:	CHAINE
							{									
								setChaineVar();
							}
						;

	ElementList			:	Expression
							{
								_LastBloc->addCode((new CAddOpCode));								
							}
						|	ElementList Expression
							{								
								_LastBloc->addCode((new CAddOpCode));
							}
						;	

	NewObject			:	NEW 
							{
								_LastStringParam.push_back(new NLAIAGENT::CGroupType());								

							}
							NewObjectName PAR_G
							{
								addParamCont();
								//addOpCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
							}
							PrametreNew
						;
	PrametreNew			:	Prametre PAR_D
							{
								if(!buildObject()) return false;
							}
						|	PAR_D
							{
								if(!buildObject()) return false;
							}
						;


	NewObjectName		:	IDENT
							{							
								_LastStringParam.back()->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(LastyyText[1])));
								_Param.push_back(new CParam);								
							}
						;	

	Rule			:	RULE RuleCondition
						{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );				
						}
						THEN
						{
							//_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
							addParamRuleCont();
						}
						Clause
						{
							_LastBloc->addCode( new CAddOpCode() );
//							_LastBloc->addCode( new CLdbNewOpCode(CRule()) );		
							setStackVar( CVar::IdVar );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tUndef);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::Rule::idRule;
						}
						;

	SimpleLogicCond		:	RuleCondition
						;

	RuleCondition	:	IF
						{
							//_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
							addParamRuleCont();
						}
						Clause
						{
							_LastBloc->addCode( new CLdbNewOpCode( CClause() ) );
							//_LastBloc->addCode( new CAddOpCode() );
//							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
//							_LastFact.varType = varForFunc;
//							_LastFact.isUsed = false;
//							_lastIdentType = NLAIAGENT::CClause::idCClause;
						}
						;

	
	Clause				: FactPattern
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						| FactPattern
						{
							_LastBloc->addCode( new CAddOpCode() );
						} 
						AND Clause
						{
							//_LastBloc->addCode( new CAddOpCode() );							
						}
						| FactPattern
						OR Clause
						{
							//_LastBloc->addCode( new CAddOpCode() );													
						}
						;

	LogicFact		:	IA_ASSERT FactPattern
						{
							_LastBloc->addCode( new CTellOpCode() );
						}
						;

	BoolType		:	POWER IDENT
						{
							/*char buf[256];
							strcpy(buf, LastyyText[1]);
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CBoolType( NLAIAGENT::CStringVarName(LastyyText[1]) ) ) );
							_lastObjectType = (uint)(_lastObjectType & NLAIC::CTypeOfObject::tLogic);
							_LastFact.varType = varForFunc;
							_LastFact.isUsed = false;
							_lastIdentType = NLAIAGENT::CVar::idBoolType;
							*/
						}

	FactPattern			: INTERROGATION PAR_G 
						IDENT 
						{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
							char *txt = LastyyText[1];
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode(new CAddOpCode() );
							//_LastAssert = NLAIAGENT::CStringVarName(LastyyText[1]);
						}
						LogicVarSet PAR_D
						{
/*							setStackVar( CFactPattern::IdFactPattern );
							IBaseAssert *my_assert = _FactBase->addAssert( _LastAssert, _NbLogicParams );
							_NbLogicParams = 0;
							_LastBloc->addCode(new CLdbNewOpCode( CFactPattern( my_assert ) ) );
							*/
						} 


						| INTERROGATION IDENT
						{
						}
							
						;


	LogicVarSet			:	Expression
							{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							}
						|	Expression
							{
								_LastBloc->addCode((new CAddOpCode));
//								_param.back()->push(_lastIdentType);
								_NbLogicParams++;
							}
							LogicVarSet
						;

	LogicVar		:	INTERROGATION IDENT
						{
							char buf[256];
							strcpy(buf, LastyyText[1]);
							setStackVar( CVar::IdVar );
							_LastBloc->addCode( new CLdbOpCode(CVar( LastyyText[1] )) );
						}
						;


	FuzzyRuleSet	:	FUZZYRULESET ACCOL_G
						{
							_LastBloc->addCode(new CLdbOpCode( NLAIAGENT::CGroupType()) );
						}
						ListFuzzyRule
						;

	ListFuzzyRule		: FuzzyRule	
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						ACCOL_D
						{
							for (sint32 i = 0; i < 20; i++);
						}
						| FuzzyRule
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						ListFuzzyRule
						{
							for (sint32 i = 0; i < 20; i++);
						}
						;

	FuzzyRule			:	FUZZYRULE FuzzyCondition
						{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );
						}
						THEN
						{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						}
						FuzzyClause
						{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyRule() ) );		
							setStackVar(CFuzzyRule::IdFuzzyRule);
						}
						;

	FuzzyCondition		: IF 
						{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						}
						FuzzyClause
						{
							
						}
						;

	FuzzyClause			: FuzzyFactPattern
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						| FuzzyFactPattern 
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						AND FuzzyClause
						;

	FuzzyFactPattern    : PAR_G IDENT
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

						}
						FIS IDENT
						{
							_LastString = NLAIAGENT::CStringVarName( LastyyText[0] );
						}
						PAR_D
						{
							for (sint32 i = 0; i < 20; i++ );
							_LastBloc->addCode(new CLdbNewOpCode( CSimpleFuzzyCond(NULL, NULL) ) );
						}
						;

	FuzzyVar			:	FUZZYVAR 
						{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						}	
						IDENT
						{
							_LastBloc->addCode( new CLdbOpCode( (NLAIAGENT::IObjectIA &) NLAIAGENT::CStringVarName( LastyyText[1] ) ) );
							_LastBloc->addCode( new CAddOpCode() );
						}
						SETS
						{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
						}
						FuzzySets
						{
							_LastBloc->addCode( new CAddOpCode() );
							_LastBloc->addCode( new CLdbNewOpCode( CFuzzyVar(NLAIAGENT::CStringVarName("Inst"),0,1) ) );
						}
						;

	FuzzySets			: FSet
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						| FSet 
						{
							_LastBloc->addCode( new CAddOpCode() );
						}
						FuzzySets
						;

	FSet				: Expression
						{
							for (sint32 i = 0; i < 20; i++);
						}
						;						
%%
