#include "script/compilateur.h"
#include "agent/agent.h"
#include "agent/agent_script.h"
#include "agent/message_script.h"
#include "logic/operator_script.h"
#include "logic/interpret_object_operator.h"
#include "script/interpret_object_manager.h"
#include "script/interpret_object_message.h"
#include "agent/main_agent_script.h"
#include "agent/agent_digital.h"
#include "agent/msg_group.h"
#include "agent/agent_mailer.h"
#include "logic/logic.h"
#include "fuzzy/fuzzy.h"
#include "agent/actor.h"
#include "agent/actor_script.h"
#include "script/interpret_actor.h"

#include "c/registry_class.h"

/*namespace NLAIC
{
	tRegistry *registry = new tRegistry;
}*/

//namespace NLIALINK
//{
	using namespace NLAIAGENT;
	using namespace NLAILOGIC;
	using namespace NLAIFUZZY;

	CLocWordNumRef::tMapRef *CLocWordNumRef::_LocRefence = new CLocWordNumRef::tMapRef;
	CIndexVariant<maxIndex>	CNumericIndex::_I = CIndexVariant<maxIndex>((sint32)0);

	CIndexedVarName::CNameStruc *CIndexedVarName::_TableName = NULL;
	const sint32 CIndexedVarName::_Bank = 256;
	sint32 CIndexedVarName::_Count = 0;
	//sint32 CIndexedVarName::_staticIndex = 1;
	CIndexedVarName::tMapName *CIndexedVarName::_Map = new CIndexedVarName::tMapName;
	std::list<CIndexedVarName::CNameStruc *> *CIndexedVarName::_Empty = new std::list<CIndexedVarName::CNameStruc *>;


	static CNumericIndex staticId;
	static const IAgent staticAgent(NULL);
	const NLAIC::CIdentType CLocWordNumRef::IdLocWordNumRef("LocWordNumRef",NLAIC::CSelfClassCFactory((const IWordNumRef &)staticAgent),
																		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
																		NLAIC::CTypeOfOperator(0));
	const NLAIC::CIdentType IAgent::IdAgent("GenericAgent",NLAIC::CSelfClassCFactory(staticAgent), NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
																NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CSimpleLocalMailBox::IdSimpleLocalMailBox("SimpleLocalMailBox",NLAIC::CSelfClassCFactory(CSimpleLocalMailBox(NULL)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));	

	const NLAIC::CIdentType CLocalMailBox::IdLocalMailBox(	"LocalMailBox",NLAIC::CSelfClassCFactory(CLocalMailBox(NULL)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));	

	const NLAIC::CIdentType CStringType::IdStringType("String",	NLAIC::CSelfClassCFactory(CStringType(CStringVarName("XX"))),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
																NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
																NLAIC::CTypeOfOperator::opSub | 																
																NLAIC::CTypeOfOperator::opEq | 
																NLAIC::CTypeOfOperator::opInf | 
																NLAIC::CTypeOfOperator::opSup | 
																NLAIC::CTypeOfOperator::opInfEq | 
																NLAIC::CTypeOfOperator::opSupEq | 																
																NLAIC::CTypeOfOperator::opDiff));

	const NLAIC::CIdentType DigitalType::IdDigitalType("Digital",	NLAIC::CSelfClassCFactory(DigitalType(0)),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
																	NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
																	NLAIC::CTypeOfOperator::opSub | 
																	NLAIC::CTypeOfOperator::opMul | 
																	NLAIC::CTypeOfOperator::opDiv | 
																	NLAIC::CTypeOfOperator::opEq | 
																	NLAIC::CTypeOfOperator::opInf | 
																	NLAIC::CTypeOfOperator::opSup | 
																	NLAIC::CTypeOfOperator::opInfEq | 
																	NLAIC::CTypeOfOperator::opSupEq |
																	NLAIC::CTypeOfOperator::opNot |
																	NLAIC::CTypeOfOperator::opDiff));	

	const NLAIC::CIdentType BorneDigitalType::IdBorneDigitalType("BorneDigital",NLAIC::CSelfClassCFactory(BorneDigitalType(-1,1)),
																				NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
																				NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
																				NLAIC::CTypeOfOperator::opSub | 
																				NLAIC::CTypeOfOperator::opMul | 
																				NLAIC::CTypeOfOperator::opDiv | 
																				NLAIC::CTypeOfOperator::opEq | 
																				NLAIC::CTypeOfOperator::opInf |
																				NLAIC::CTypeOfOperator::opSup | 
																				NLAIC::CTypeOfOperator::opInfEq | 
																				NLAIC::CTypeOfOperator::opSupEq |
																				NLAIC::CTypeOfOperator::opNot |
																				NLAIC::CTypeOfOperator::opDiff));

	/*const NLAIC::CIdentType IntegerType::IdIntegerType("Integer",	NLAIC::CSelfClassCFactory(IntegerType(0)),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
																	NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
																	NLAIC::CTypeOfOperator::opSub | 
																	NLAIC::CTypeOfOperator::opMul | 
																	NLAIC::CTypeOfOperator::opDiv | 
																	NLAIC::CTypeOfOperator::opEq | 
																	NLAIC::CTypeOfOperator::opInf | 
																	NLAIC::CTypeOfOperator::opSup | 
																	NLAIC::CTypeOfOperator::opInfEq | 
																	NLAIC::CTypeOfOperator::opSupEq |
																	NLAIC::CTypeOfOperator::opNot |
																	NLAIC::CTypeOfOperator::opDiff));*/

	/*const NLAIC::CIdentType CPaireType::IdPaireType("Paire",NLAIC::CSelfClassCFactory(CPaireType( CStringType(CStringVarName("XX")),BorneDigitalType(-1,1))),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));*/
	
	const NLAIC::CIdentType CVectorGroupType::IdVectorGroupType("Vector",NLAIC::CSelfClassCFactory(CVectorGroupType()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
															NLAIC::CTypeOfOperator::opSub | 
															NLAIC::CTypeOfOperator::opNot));

	const NLAIC::CIdentType CGroupType::IdGroupType("List",NLAIC::CSelfClassCFactory(CGroupType()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
															NLAIC::CTypeOfOperator::opSub | 
															NLAIC::CTypeOfOperator::opNot));
	

	const NLAIC::CIdentType CIndexedVarName::IdIndexedVarName("IndexedVarName",NLAIC::CSelfClassCFactory(CIndexedVarName("Inst")),
																  NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
	const NLAIC::CIdentType CStringVarName::IdStringVarName("StringVarName",NLAIC::CSelfClassCFactory(CStringVarName("Inst")),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CMessageGroup::IdMessageGroup(	"MessageGroup",NLAIC::CSelfClassCFactory(CMessageGroup(1)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CMessage::IdMessage("MessageGroup",	NLAIC::CSelfClassCFactory(CMessage()),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | NLAIC::CTypeOfOperator::opSub ));
	const NLAIC::CIdentType CMessageVector::IdMessageVector("MessageVector",	NLAIC::CSelfClassCFactory(CMessageVector()),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | NLAIC::CTypeOfOperator::opSub ));

	

	const NLAIC::CIdentType CBoolType::IdBoolType("BoolType",NLAIC::CSelfClassCFactory(CBoolType(false)),	NLAIC::CTypeOfObject::tLogic  | 
																									NLAIC::CTypeOfObject::tNombre,
																									NLAIC::CTypeOfOperator::opAdd |
																									NLAIC::CTypeOfOperator::opSub |
																									NLAIC::CTypeOfOperator::opMul |
																									NLAIC::CTypeOfOperator::opEq  |
																									NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType CAgentScript::IdAgentScript("AgentScript", NLAIC::CSelfClassCFactory( CAgentScript(NULL) ),
		NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tAgentInterpret,
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType COperatorScript::IdOperatorScript("OperatorScript", NLAIC::CSelfClassCFactory( COperatorScript() ),
		NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tAgentInterpret,
		NLAIC::CTypeOfOperator(0) );

	const NLAIC::CIdentType CActor::IdActor("CActor", NLAIC::CSelfClassCFactory( CActor() ),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
													NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CActorScript::IdActorScript("ActorScript", NLAIC::CSelfClassCFactory( CActorScript(NULL) ),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret),
													NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CMainAgentScript::IdMainAgentScript("MainAgentScript", NLAIC::CSelfClassCFactory( CMainAgentScript(NULL) ),
													NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tAgentInterpret,
													NLAIC::CTypeOfOperator::opEq );



	const NLAIC::CIdentType CMessageScript::IdMessageScript("MessageScript", NLAIC::CSelfClassCFactory( CMessageScript() ),
		NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tAgentInterpret,
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CFuzzyInterval::IdFuzzyInterval = NLAIC::CIdentType("FuzzyInterval",NLAIC::CSelfClassCFactory(CFuzzyInterval("Inst",0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CTriangleFuzzySet::IdTriangleFuzzySet = NLAIC::CIdentType("TriangleFuzzySet",NLAIC::CSelfClassCFactory(CTriangleFuzzySet("Inst",0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CLeftFuzzySet::IdLeftFuzzySet = NLAIC::CIdentType("LeftFuzzySet",NLAIC::CSelfClassCFactory(CLeftFuzzySet("Inst",0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CTrapezeFuzzySet::IdTrapezeFuzzySet = NLAIC::CIdentType("TrapezeFuzzySet",NLAIC::CSelfClassCFactory(CTrapezeFuzzySet("Inst",0,0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );


	const NLAIC::CIdentType CRightFuzzySet::IdRightFuzzySet = NLAIC::CIdentType("RightFuzzySet",NLAIC::CSelfClassCFactory(CRightFuzzySet("Inst",0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CFirstOrderAssert::IdFirstOrderAssert = NLAIC::CIdentType( "FirstOrderAssert", NLAIC::CSelfClassCFactory(CFirstOrderAssert( CStringVarName("Inst") )),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 

	const NLAIC::CIdentType CBoolAssert::IdBoolAssert = NLAIC::CIdentType( "BoolAssert", NLAIC::CSelfClassCFactory(CBoolAssert( CStringVarName("Inst") )),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 

	const NLAIC::CIdentType CFactBase::IdFactBase = NLAIC::CIdentType( "FactBase", NLAIC::CSelfClassCFactory(CFactBase()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CClause::IdClause = NLAIC::CIdentType( "Clause", NLAIC::CSelfClassCFactory(CClause()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CRule::IdRule = NLAIC::CIdentType( "Rule", NLAIC::CSelfClassCFactory(CRule()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CValueSet::IdValueSet = NLAIC::CIdentType( "ValueSet", NLAIC::CSelfClassCFactory( CValueSet(0) ),
		NLAIC::CTypeOfObject::tLogic |
		NLAIC::CTypeOfObject::tList,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNot |
		NLAIC::CTypeOfOperator::opDiff );

	const NLAIC::CIdentType CVar::IdVar("Var",NLAIC::CSelfClassCFactory(CVar( CStringVarName("Inst") )),
											NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CVarSet::IdVarSet("VarSet", NLAIC::CSelfClassCFactory( CVarSet() ),
		NLAIC::CTypeOfObject::tLogic |
		NLAIC::CTypeOfObject::tList,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType FuzzyType::IdFuzzyType("FuzzyType",NLAIC::CSelfClassCFactory(FuzzyType()),
		NLAIC::CTypeOfObject::tNombre |
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opMul |
		NLAIC::CTypeOfOperator::opDiv |
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opInf |
		NLAIC::CTypeOfOperator::opSup |
		NLAIC::CTypeOfOperator::opNot 
	);

	const NLAIC::CIdentType CFuzzyVar::IdFuzzyVar = NLAIC::CIdentType("FuzzyVar",NLAIC::CSelfClassCFactory( CFuzzyVar(CStringVarName("Inst"),0,0) ),
		NLAIC::CTypeOfObject::tLogic  | 
		NLAIC::CTypeOfObject::tNombre,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opMul |
		NLAIC::CTypeOfOperator::opEq  |
		NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType CVectorMsgContainer::IdVectorMsgContainer("VectorMsgContainer",NLAIC::CSelfClassCFactory(CVectorMsgContainer()),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CFactPattern::IdFactPattern = NLAIC::CIdentType( "FactPattern", NLAIC::CSelfClassCFactory(CFactPattern()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opEq);

	const NLAIC::CIdentType CFuzzyRule::IdFuzzyRule("FuzzyRule",NLAIC::CSelfClassCFactory( CFuzzyRule() ),
		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
		NLAIC::CTypeOfOperator(0));

	const NLAIC::CIdentType CFuzzyFact::IdFuzzyFact = NLAIC::CIdentType("FuzzyFact",NLAIC::CSelfClassCFactory( CFuzzyFact(NULL,0) ),
		NLAIC::CTypeOfObject::tLogic  | 
		NLAIC::CTypeOfObject::tNombre,
		NLAIC::CTypeOfOperator::opEq  |
		NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType CFirstOrderOperator::IdCFirstOrderOperator = NLAIC::CIdentType( "FirstOrderOperator", NLAIC::CSelfClassCFactory(CFirstOrderOperator()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CBoolOperator::IdBoolOperator = NLAIC::CIdentType( "BoolOperator", NLAIC::CSelfClassCFactory(CBoolOperator()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CLocalAgentMail::LocalAgentMail = NLAIC::CIdentType( "LocalAgentMail", NLAIC::CSelfClassCFactory(CLocalAgentMail()), 
		NLAIC::CTypeOfObject::tAgent,
		NLAIC::CTypeOfOperator(0) );

//}

namespace NLIASCRIPT
{
	const NLAIC::CIdentType CMethodeName::IdMethodeName("MethodeName",NLAIC::CSelfClassCFactory( CMethodeName() ),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opEq);

	const NLAIC::CIdentType CParam::IdParam("Param",NLAIC::CSelfClassCFactory( CParam() ),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opEq);


	NLAIAGENT::IObjectIA **CVarPStack::_LocalTableRef = NULL;
	#ifdef _DEBUG
	NLAIC::CIdentType CVarPStack::IdVarPStack ("VarPStack",	NLAIC::CSelfClassCFactory(CVarPStack(0,"Inst")),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
														NLAIC::CTypeOfOperator::opSub | 
														NLAIC::CTypeOfOperator::opMul | 
														NLAIC::CTypeOfOperator::opDiv | 
														NLAIC::CTypeOfOperator::opEq | 
														NLAIC::CTypeOfOperator::opInf | 
														NLAIC::CTypeOfOperator::opSup | 
														NLAIC::CTypeOfOperator::opInfEq | 
														NLAIC::CTypeOfOperator::opSupEq |
														NLAIC::CTypeOfOperator::opNot |
														NLAIC::CTypeOfOperator::opDiff));	
	#else
	NLAIC::CIdentType CVarPStack::IdVarPStack ("VarPStack",	NLAIC::CSelfClassCFactory(CVarPStack(0)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
														NLAIC::CTypeOfOperator::opSub | 
														NLAIC::CTypeOfOperator::opMul | 
														NLAIC::CTypeOfOperator::opDiv | 
														NLAIC::CTypeOfOperator::opEq | 
														NLAIC::CTypeOfOperator::opInf | 
														NLAIC::CTypeOfOperator::opSup | 
														NLAIC::CTypeOfOperator::opInfEq | 
														NLAIC::CTypeOfOperator::opSupEq |
														NLAIC::CTypeOfOperator::opNot |
														NLAIC::CTypeOfOperator::opDiff));
	#endif													

	#ifdef _DEBUG
	NLAIC::CIdentType CVarPStackParam::IdVarPStackParam ("VarPStackParam",	NLAIC::CSelfClassCFactory(CVarPStack(0,"Inst")),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
														NLAIC::CTypeOfOperator::opSub | 
														NLAIC::CTypeOfOperator::opMul | 
														NLAIC::CTypeOfOperator::opDiv | 
														NLAIC::CTypeOfOperator::opEq | 
														NLAIC::CTypeOfOperator::opInf | 
														NLAIC::CTypeOfOperator::opSup | 
														NLAIC::CTypeOfOperator::opInfEq | 
														NLAIC::CTypeOfOperator::opSupEq |
														NLAIC::CTypeOfOperator::opNot |
														NLAIC::CTypeOfOperator::opDiff));	
	#else
	NLAIC::CIdentType CVarPStackParam::IdVarPStackParam ("VarPStackParam",	NLAIC::CSelfClassCFactory(CVarPStack(0)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tNombre),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
														NLAIC::CTypeOfOperator::opSub | 
														NLAIC::CTypeOfOperator::opMul | 
														NLAIC::CTypeOfOperator::opDiv | 
														NLAIC::CTypeOfOperator::opEq | 
														NLAIC::CTypeOfOperator::opInf | 
														NLAIC::CTypeOfOperator::opSup | 
														NLAIC::CTypeOfOperator::opInfEq | 
														NLAIC::CTypeOfOperator::opSupEq |
														NLAIC::CTypeOfOperator::opNot |
														NLAIC::CTypeOfOperator::opDiff));
	#endif	

	sint32 CVarPStackParam::_Shift = 0;
	
	const NLAIC::CIdentType CCodeBrancheRun::IdCodeBrancheRun("CodeBrancheRun",NLAIC::CSelfClassCFactory(CCodeBrancheRun(1,CHaltOpCode())),NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
	const NLAIC::CIdentType CCodeBrancheRunDebug::IdCodeBrancheRunDebug("CodeBrancheRunDebug",NLAIC::CSelfClassCFactory(CCodeBrancheRunDebug(1,CHaltOpCode())),NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
	const NLAIC::CIdentType IBlock::IdBlock("IBlock",NLAIC::CSelfClassCFactory(IBlock(false)),NLAIC::CTypeOfObject(0),NLAIC::CTypeOfOperator(0));

	static CAgentClass agentClass;	
	const NLAIC::CIdentType CAgentClass::IdAgentClass("Agent", NLAIC::CSelfClassCFactory(agentClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret),
													NLAIC::CTypeOfOperator(0));	


	static CActorClass actorClass;
	const NLAIC::CIdentType CActorClass::IdActorClass("Actor", NLAIC::CSelfClassCFactory(actorClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret),
													NLAIC::CTypeOfOperator(0));
	

	static CMessageClass messageClass;
	const NLAIC::CIdentType CMessageClass::IdMessageClass("Message", NLAIC::CSelfClassCFactory(messageClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret),
													NLAIC::CTypeOfOperator(0));

	static COperatorClass operatorClass;
	const NLAIC::CIdentType COperatorClass::IdOperatorClass("Operator", NLAIC::CSelfClassCFactory(operatorClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret),
													NLAIC::CTypeOfOperator(0));


	const NLAIC::CIdentType CContextDebug::IdContextDebug("ContextDebug", NLAIC::CSelfClassCFactory(CContextDebug()),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
														NLAIC::CTypeOfOperator(0));

	static CAgentClass managerClass;	
	const NLAIC::CIdentType CManagerClass::IdManagerClass("AgentManager", NLAIC::CSelfClassCFactory(managerClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret),
													NLAIC::CTypeOfOperator(0));


	static CStackPointer	staticStack(1),staticHeap(1);
	const NLAIC::CIdentType CCodeContext::IdCodeContext = NLAIC::CIdentType("CodeContext",
																		NLAIC::CSelfClassCFactory(CCodeContext(staticStack,staticHeap,NULL,NULL,NULL)),
																		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
}