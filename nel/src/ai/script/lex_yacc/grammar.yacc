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

%token  BEGIN_GRAMMAR
%token	FROM 
%token	DEFINE	GROUP
%token	COMPONENT CONSTRUCTION DESTRUCTION MESSAGE_MANAGER MAESSAGELOOP
%token	TRIGGER	PRESCONDITION	POSTCONDITION RETURN
%token	COS SIN TAN POW LN LOG FACT 
%token	AS
%token	DIGITAL	COLLECTOR	
%token	WITH DO 
%token	END IF THEN BEGINING
%token	END_GRAMMAR
%token	LOGICVAR RULE IA_ASSERT
%token	FUZZYRULE FUZZYRULESET SETS FUZZYVAR FIS OR
%token	NEW AND	LOCAL 

%left	NON_BIN		OR_BIN		AND_BIN		XOR_BIN
%left	PLUS		MOINS		
%left	FOIS		DIV		POWER


%%

	program				:	DefinitionClass
							{
								
							}
						|	program DefinitionClass
						;

	DefinitionClass		:	CorpDeDefinition 
							ACCOL_G
							BlocDeDefinition
							ACCOL_D
							{						
								if(!computContraint()) return false;
								((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
							}								
						|	CorpDeDefinition 
							ACCOL_G 
							ACCOL_D
							{
								((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
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
									((IClassInterpret *)_SelfClass.get())->setClassName(NLAIAGENT::CStringVarName(LastyyText[1]));
									((IClassInterpret *)_SelfClass.get())->buildVTable();
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

	BlocDeDefinition	:	RegistDesAttributs
						|	RegistDesAttributs BlocPourLesCode
						|	BlocPourLesCode
						;

	RegistDesAttributs	:	COMPONENT POINT_DEUX TypeDeDeclaration END;
						|	COMPONENT POINT_DEUX END
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
									x->incRef();
									((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(x);
								}
								else
								{
									_IsVoid = true;
								}
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
									x->incRef();
									((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(x);
								}
								else
								{
									_IsVoid = true;
								}

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
								setParamVarName();
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
								_LastStringParam.back()->release();
								_LastStringParam.pop_back();
								_Param.back()->release();
								_Param.pop_back();								
							}
						|	RetourDeFonction 
							POINT_VI
							{
								_LastBloc->addCode((new CHaltOpCode));
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
								if(!affectation()) return false;
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
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);
								_Param.back()->incRef();
								_ExpressionTypeTmp.push_back(_ExpressionType); 
								_ExpressionType = NULL;
							}
							Prametre CallFunction
						|	NonDeFonction PAR_G
							{								
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
								_Param.push_back(new CParam);
								_Param.back()->incRef();
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
								callFunction();								
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
									c->ObjectName->incRef();
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

	Facteur				:	NILL
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
								if(!processingVar()) return false;
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
							}

						|	AppelleDeFonction LEPOINT IDENT
							{
								_LasVarStr.clear();
								cleanTypeList();
								_LasVarStr.push_back(LastyyText[1]);
								setMethodVar();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();								
							}

						|	Facteur LEPOINT IDENT
							{	
								cleanTypeList();
								_TypeList.push_back(_FlotingExpressionType);
								_FlotingExpressionType->incRef();				
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
								_LastStringParam.back()->incRef();

							}
							NewObjectName PAR_G
							{
								_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));
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
								_Param.back()->incRef();
							}
						;	

	Rule			:	RULE RuleCondition
						{
							// Met la clause en somment de pile dans une liste
							_LastBloc->addCode( new CMakeArgOpCode() );				
						}
						THEN
						{
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
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
							_LastBloc->addCode( new CLdbOpCode( NLAIAGENT::CGroupType() ) );
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
							_LastAssert = NLAIAGENT::CStringVarName(LastyyText[1]);
						}
						LogicVarSet PAR_D
						{
							setStackVar( CFactPattern::IdFactPattern );
							IBaseAssert *my_assert = _FactBase.addAssert( _LastAssert, _NbLogicParams );
							_NbLogicParams = 0;
							_LastBloc->addCode(new CLdbNewOpCode( CFactPattern( my_assert ) ) );
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
