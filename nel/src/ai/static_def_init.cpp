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

/*namespace NLIAC
{
	tRegistry *registry = new tRegistry;
}*/

//namespace NLIALINK
//{
	using namespace NLIAAGENT;
	using namespace NLIALOGIC;
	using namespace NLIAFUZZY;

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
	const NLIAC::CIdentType CLocWordNumRef::IdLocWordNumRef("LocWordNumRef",NLIAC::CSelfClassCFactory((const IWordNumRef &)staticAgent),
																		NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),
																		NLIAC::CTypeOfOperator(0));
	const NLIAC::CIdentType IAgent::IdAgent("GenericAgent",NLIAC::CSelfClassCFactory(staticAgent), NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgent),
																NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CSimpleLocalMailBox::IdSimpleLocalMailBox("SimpleLocalMailBox",NLIAC::CSelfClassCFactory(CSimpleLocalMailBox(NULL)),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));	

	const NLIAC::CIdentType CLocalMailBox::IdLocalMailBox(	"LocalMailBox",NLIAC::CSelfClassCFactory(CLocalMailBox(NULL)),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));	

	const NLIAC::CIdentType CStringType::IdStringType("String",	NLIAC::CSelfClassCFactory(CStringType(CStringVarName("XX"))),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tList),
																NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
																NLIAC::CTypeOfOperator::opSub | 																
																NLIAC::CTypeOfOperator::opEq | 
																NLIAC::CTypeOfOperator::opInf | 
																NLIAC::CTypeOfOperator::opSup | 
																NLIAC::CTypeOfOperator::opInfEq | 
																NLIAC::CTypeOfOperator::opSupEq | 																
																NLIAC::CTypeOfOperator::opDiff));

	const NLIAC::CIdentType DigitalType::IdDigitalType("Digital",	NLIAC::CSelfClassCFactory(DigitalType(0)),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
																	NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
																	NLIAC::CTypeOfOperator::opSub | 
																	NLIAC::CTypeOfOperator::opMul | 
																	NLIAC::CTypeOfOperator::opDiv | 
																	NLIAC::CTypeOfOperator::opEq | 
																	NLIAC::CTypeOfOperator::opInf | 
																	NLIAC::CTypeOfOperator::opSup | 
																	NLIAC::CTypeOfOperator::opInfEq | 
																	NLIAC::CTypeOfOperator::opSupEq |
																	NLIAC::CTypeOfOperator::opNot |
																	NLIAC::CTypeOfOperator::opDiff));	

	const NLIAC::CIdentType BorneDigitalType::IdBorneDigitalType("BorneDigital",NLIAC::CSelfClassCFactory(BorneDigitalType(-1,1)),
																				NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
																				NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
																				NLIAC::CTypeOfOperator::opSub | 
																				NLIAC::CTypeOfOperator::opMul | 
																				NLIAC::CTypeOfOperator::opDiv | 
																				NLIAC::CTypeOfOperator::opEq | 
																				NLIAC::CTypeOfOperator::opInf |
																				NLIAC::CTypeOfOperator::opSup | 
																				NLIAC::CTypeOfOperator::opInfEq | 
																				NLIAC::CTypeOfOperator::opSupEq |
																				NLIAC::CTypeOfOperator::opNot |
																				NLIAC::CTypeOfOperator::opDiff));

	/*const NLIAC::CIdentType IntegerType::IdIntegerType("Integer",	NLIAC::CSelfClassCFactory(IntegerType(0)),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
																	NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
																	NLIAC::CTypeOfOperator::opSub | 
																	NLIAC::CTypeOfOperator::opMul | 
																	NLIAC::CTypeOfOperator::opDiv | 
																	NLIAC::CTypeOfOperator::opEq | 
																	NLIAC::CTypeOfOperator::opInf | 
																	NLIAC::CTypeOfOperator::opSup | 
																	NLIAC::CTypeOfOperator::opInfEq | 
																	NLIAC::CTypeOfOperator::opSupEq |
																	NLIAC::CTypeOfOperator::opNot |
																	NLIAC::CTypeOfOperator::opDiff));*/

	/*const NLIAC::CIdentType CPaireType::IdPaireType("Paire",NLIAC::CSelfClassCFactory(CPaireType( CStringType(CStringVarName("XX")),BorneDigitalType(-1,1))),
															NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));*/
	
	const NLIAC::CIdentType CVectorGroupType::IdVectorGroupType("Vector",NLIAC::CSelfClassCFactory(CVectorGroupType()),
															NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tList),
															NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
															NLIAC::CTypeOfOperator::opSub | 
															NLIAC::CTypeOfOperator::opNot));

	const NLIAC::CIdentType CGroupType::IdGroupType("List",NLIAC::CSelfClassCFactory(CGroupType()),
															NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tList),
															NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
															NLIAC::CTypeOfOperator::opSub | 
															NLIAC::CTypeOfOperator::opNot));
	

	const NLIAC::CIdentType CIndexedVarName::IdIndexedVarName("IndexedVarName",NLIAC::CSelfClassCFactory(CIndexedVarName("Inst")),
																  NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
	const NLIAC::CIdentType CStringVarName::IdStringVarName("StringVarName",NLIAC::CSelfClassCFactory(CStringVarName("Inst")),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CMessageGroup::IdMessageGroup(	"MessageGroup",NLIAC::CSelfClassCFactory(CMessageGroup(1)),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CMessage::IdMessage("MessageGroup",	NLIAC::CSelfClassCFactory(CMessage()),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tList),
														NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | NLIAC::CTypeOfOperator::opSub ));
	const NLIAC::CIdentType CMessageVector::IdMessageVector("MessageVector",	NLIAC::CSelfClassCFactory(CMessageVector()),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tList),
														NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | NLIAC::CTypeOfOperator::opSub ));

	

	const NLIAC::CIdentType CBoolType::IdBoolType("BoolType",NLIAC::CSelfClassCFactory(CBoolType(false)),	NLIAC::CTypeOfObject::tLogic  | 
																									NLIAC::CTypeOfObject::tNombre,
																									NLIAC::CTypeOfOperator::opAdd |
																									NLIAC::CTypeOfOperator::opSub |
																									NLIAC::CTypeOfOperator::opMul |
																									NLIAC::CTypeOfOperator::opEq  |
																									NLIAC::CTypeOfOperator::opNot );

	const NLIAC::CIdentType CAgentScript::IdAgentScript("AgentScript", NLIAC::CSelfClassCFactory( CAgentScript(NULL) ),
		NLIAC::CTypeOfObject::tAgent | NLIAC::CTypeOfObject::tAgentInterpret,
		NLIAC::CTypeOfOperator::opEq );

	const NLIAC::CIdentType COperatorScript::IdOperatorScript("OperatorScript", NLIAC::CSelfClassCFactory( COperatorScript() ),
		NLIAC::CTypeOfObject::tAgent | NLIAC::CTypeOfObject::tAgentInterpret,
		NLIAC::CTypeOfOperator(0) );

	const NLIAC::CIdentType CActor::IdActor("CActor", NLIAC::CSelfClassCFactory( CActor() ),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgent),
													NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CActorScript::IdActorScript("ActorScript", NLIAC::CSelfClassCFactory( CActorScript(NULL) ),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgentInterpret),
													NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CMainAgentScript::IdMainAgentScript("MainAgentScript", NLIAC::CSelfClassCFactory( CMainAgentScript(NULL) ),
													NLIAC::CTypeOfObject::tAgent | NLIAC::CTypeOfObject::tAgentInterpret,
													NLIAC::CTypeOfOperator::opEq );



	const NLIAC::CIdentType CMessageScript::IdMessageScript("MessageScript", NLIAC::CSelfClassCFactory( CMessageScript() ),
		NLIAC::CTypeOfObject::tAgent | NLIAC::CTypeOfObject::tAgentInterpret,
		NLIAC::CTypeOfOperator::opEq );

	const NLIAC::CIdentType CFuzzyInterval::IdFuzzyInterval = NLIAC::CIdentType("FuzzyInterval",NLIAC::CSelfClassCFactory(CFuzzyInterval("Inst",0,0)),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opEq );

	const NLIAC::CIdentType CTriangleFuzzySet::IdTriangleFuzzySet = NLIAC::CIdentType("TriangleFuzzySet",NLIAC::CSelfClassCFactory(CTriangleFuzzySet("Inst",0,0,0)),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opEq );

	const NLIAC::CIdentType CLeftFuzzySet::IdLeftFuzzySet = NLIAC::CIdentType("LeftFuzzySet",NLIAC::CSelfClassCFactory(CLeftFuzzySet("Inst",0,0,0)),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opEq );

	const NLIAC::CIdentType CTrapezeFuzzySet::IdTrapezeFuzzySet = NLIAC::CIdentType("TrapezeFuzzySet",NLIAC::CSelfClassCFactory(CTrapezeFuzzySet("Inst",0,0,0,0)),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opEq );


	const NLIAC::CIdentType CRightFuzzySet::IdRightFuzzySet = NLIAC::CIdentType("RightFuzzySet",NLIAC::CSelfClassCFactory(CRightFuzzySet("Inst",0,0,0)),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opEq );

	const NLIAC::CIdentType CFirstOrderAssert::IdFirstOrderAssert = NLIAC::CIdentType( "FirstOrderAssert", NLIAC::CSelfClassCFactory(CFirstOrderAssert( CStringVarName("Inst") )),
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opEq |
		NLIAC::CTypeOfOperator::opNeg | 
		NLIAC::CTypeOfOperator::opNot ); 

	const NLIAC::CIdentType CBoolAssert::IdBoolAssert = NLIAC::CIdentType( "BoolAssert", NLIAC::CSelfClassCFactory(CBoolAssert( CStringVarName("Inst") )),
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opEq |
		NLIAC::CTypeOfOperator::opNeg | 
		NLIAC::CTypeOfOperator::opNot ); 

	const NLIAC::CIdentType CFactBase::IdFactBase = NLIAC::CIdentType( "FactBase", NLIAC::CSelfClassCFactory(CFactBase()), 
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd );

	const NLIAC::CIdentType CClause::IdClause = NLIAC::CIdentType( "Clause", NLIAC::CSelfClassCFactory(CClause()), 
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd );

	const NLIAC::CIdentType CRule::IdRule = NLIAC::CIdentType( "Rule", NLIAC::CSelfClassCFactory(CRule()), 
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd );

	const NLIAC::CIdentType CValueSet::IdValueSet = NLIAC::CIdentType( "ValueSet", NLIAC::CSelfClassCFactory( CValueSet(0) ),
		NLIAC::CTypeOfObject::tLogic |
		NLIAC::CTypeOfObject::tList,
		NLIAC::CTypeOfOperator::opEq |
		NLIAC::CTypeOfOperator::opNot |
		NLIAC::CTypeOfOperator::opDiff );

	const NLIAC::CIdentType CVar::IdVar("Var",NLIAC::CSelfClassCFactory(CVar( CStringVarName("Inst") )),
											NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CVarSet::IdVarSet("VarSet", NLIAC::CSelfClassCFactory( CVarSet() ),
		NLIAC::CTypeOfObject::tLogic |
		NLIAC::CTypeOfObject::tList,
		NLIAC::CTypeOfOperator::opEq |
		NLIAC::CTypeOfOperator::opNot );

	const NLIAC::CIdentType FuzzyType::IdFuzzyType("FuzzyType",NLIAC::CSelfClassCFactory(FuzzyType()),
		NLIAC::CTypeOfObject::tNombre |
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opMul |
		NLIAC::CTypeOfOperator::opDiv |
		NLIAC::CTypeOfOperator::opEq |
		NLIAC::CTypeOfOperator::opInf |
		NLIAC::CTypeOfOperator::opSup |
		NLIAC::CTypeOfOperator::opNot 
	);

	const NLIAC::CIdentType CFuzzyVar::IdFuzzyVar = NLIAC::CIdentType("FuzzyVar",NLIAC::CSelfClassCFactory( CFuzzyVar(CStringVarName("Inst"),0,0) ),
		NLIAC::CTypeOfObject::tLogic  | 
		NLIAC::CTypeOfObject::tNombre,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opSub |
		NLIAC::CTypeOfOperator::opMul |
		NLIAC::CTypeOfOperator::opEq  |
		NLIAC::CTypeOfOperator::opNot );

	const NLIAC::CIdentType CVectorMsgContainer::IdVectorMsgContainer("VectorMsgContainer",NLIAC::CSelfClassCFactory(CVectorMsgContainer()),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CFactPattern::IdFactPattern = NLIAC::CIdentType( "FactPattern", NLIAC::CSelfClassCFactory(CFactPattern()), 
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd |
		NLIAC::CTypeOfOperator::opEq);

	const NLIAC::CIdentType CFuzzyRule::IdFuzzyRule("FuzzyRule",NLIAC::CSelfClassCFactory( CFuzzyRule() ),
		NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),
		NLIAC::CTypeOfOperator(0));

	const NLIAC::CIdentType CFuzzyFact::IdFuzzyFact = NLIAC::CIdentType("FuzzyFact",NLIAC::CSelfClassCFactory( CFuzzyFact(NULL,0) ),
		NLIAC::CTypeOfObject::tLogic  | 
		NLIAC::CTypeOfObject::tNombre,
		NLIAC::CTypeOfOperator::opEq  |
		NLIAC::CTypeOfOperator::opNot );

	const NLIAC::CIdentType CFirstOrderOperator::IdCFirstOrderOperator = NLIAC::CIdentType( "FirstOrderOperator", NLIAC::CSelfClassCFactory(CFirstOrderOperator()), 
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd );

	const NLIAC::CIdentType CBoolOperator::IdBoolOperator = NLIAC::CIdentType( "BoolOperator", NLIAC::CSelfClassCFactory(CBoolOperator()), 
		NLIAC::CTypeOfObject::tLogic,
		NLIAC::CTypeOfOperator::opAdd );

	const NLIAC::CIdentType CLocalAgentMail::LocalAgentMail = NLIAC::CIdentType( "LocalAgentMail", NLIAC::CSelfClassCFactory(CLocalAgentMail()), 
		NLIAC::CTypeOfObject::tAgent,
		NLIAC::CTypeOfOperator(0) );

//}

namespace NLIASCRIPT
{
	const NLIAC::CIdentType CMethodeName::IdMethodeName("MethodeName",NLIAC::CSelfClassCFactory( CMethodeName() ),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opEq);

	const NLIAC::CIdentType CParam::IdParam("Param",NLIAC::CSelfClassCFactory( CParam() ),
		NLIAC::CTypeOfObject::tObject,
		NLIAC::CTypeOfOperator::opEq);


	NLIAAGENT::IObjectIA **CVarPStack::_LocalTableRef = NULL;
	#ifdef _DEBUG
	NLIAC::CIdentType CVarPStack::IdVarPStack ("VarPStack",	NLIAC::CSelfClassCFactory(CVarPStack(0,"Inst")),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
														NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
														NLIAC::CTypeOfOperator::opSub | 
														NLIAC::CTypeOfOperator::opMul | 
														NLIAC::CTypeOfOperator::opDiv | 
														NLIAC::CTypeOfOperator::opEq | 
														NLIAC::CTypeOfOperator::opInf | 
														NLIAC::CTypeOfOperator::opSup | 
														NLIAC::CTypeOfOperator::opInfEq | 
														NLIAC::CTypeOfOperator::opSupEq |
														NLIAC::CTypeOfOperator::opNot |
														NLIAC::CTypeOfOperator::opDiff));	
	#else
	NLIAC::CIdentType CVarPStack::IdVarPStack ("VarPStack",	NLIAC::CSelfClassCFactory(CVarPStack(0)),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
														NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
														NLIAC::CTypeOfOperator::opSub | 
														NLIAC::CTypeOfOperator::opMul | 
														NLIAC::CTypeOfOperator::opDiv | 
														NLIAC::CTypeOfOperator::opEq | 
														NLIAC::CTypeOfOperator::opInf | 
														NLIAC::CTypeOfOperator::opSup | 
														NLIAC::CTypeOfOperator::opInfEq | 
														NLIAC::CTypeOfOperator::opSupEq |
														NLIAC::CTypeOfOperator::opNot |
														NLIAC::CTypeOfOperator::opDiff));
	#endif													

	#ifdef _DEBUG
	NLIAC::CIdentType CVarPStackParam::IdVarPStackParam ("VarPStackParam",	NLIAC::CSelfClassCFactory(CVarPStack(0,"Inst")),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
														NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
														NLIAC::CTypeOfOperator::opSub | 
														NLIAC::CTypeOfOperator::opMul | 
														NLIAC::CTypeOfOperator::opDiv | 
														NLIAC::CTypeOfOperator::opEq | 
														NLIAC::CTypeOfOperator::opInf | 
														NLIAC::CTypeOfOperator::opSup | 
														NLIAC::CTypeOfOperator::opInfEq | 
														NLIAC::CTypeOfOperator::opSupEq |
														NLIAC::CTypeOfOperator::opNot |
														NLIAC::CTypeOfOperator::opDiff));	
	#else
	NLIAC::CIdentType CVarPStackParam::IdVarPStackParam ("VarPStackParam",	NLIAC::CSelfClassCFactory(CVarPStack(0)),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tNombre),
														NLIAC::CTypeOfOperator(NLIAC::CTypeOfOperator::opAdd | 
														NLIAC::CTypeOfOperator::opSub | 
														NLIAC::CTypeOfOperator::opMul | 
														NLIAC::CTypeOfOperator::opDiv | 
														NLIAC::CTypeOfOperator::opEq | 
														NLIAC::CTypeOfOperator::opInf | 
														NLIAC::CTypeOfOperator::opSup | 
														NLIAC::CTypeOfOperator::opInfEq | 
														NLIAC::CTypeOfOperator::opSupEq |
														NLIAC::CTypeOfOperator::opNot |
														NLIAC::CTypeOfOperator::opDiff));
	#endif	

	sint32 CVarPStackParam::_Shift = 0;
	
	const NLIAC::CIdentType CCodeBrancheRun::IdCodeBrancheRun("CodeBrancheRun",NLIAC::CSelfClassCFactory(CCodeBrancheRun(1,CHaltOpCode())),NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
	const NLIAC::CIdentType CCodeBrancheRunDebug::IdCodeBrancheRunDebug("CodeBrancheRunDebug",NLIAC::CSelfClassCFactory(CCodeBrancheRunDebug(1,CHaltOpCode())),NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
	const NLIAC::CIdentType IBlock::IdBlock("IBlock",NLIAC::CSelfClassCFactory(IBlock(false)),NLIAC::CTypeOfObject(0),NLIAC::CTypeOfOperator(0));

	static CAgentClass agentClass;	
	const NLIAC::CIdentType CAgentClass::IdAgentClass("Agent", NLIAC::CSelfClassCFactory(agentClass),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgentInterpret),
													NLIAC::CTypeOfOperator(0));	


	static CActorClass actorClass;
	const NLIAC::CIdentType CActorClass::IdActorClass("Actor", NLIAC::CSelfClassCFactory(actorClass),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgentInterpret),
													NLIAC::CTypeOfOperator(0));
	

	static CMessageClass messageClass;
	const NLIAC::CIdentType CMessageClass::IdMessageClass("Message", NLIAC::CSelfClassCFactory(messageClass),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgentInterpret),
													NLIAC::CTypeOfOperator(0));

	static COperatorClass operatorClass;
	const NLIAC::CIdentType COperatorClass::IdOperatorClass("Operator", NLIAC::CSelfClassCFactory(operatorClass),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgentInterpret),
													NLIAC::CTypeOfOperator(0));


	const NLIAC::CIdentType CContextDebug::IdContextDebug("ContextDebug", NLIAC::CSelfClassCFactory(CContextDebug()),
														NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),
														NLIAC::CTypeOfOperator(0));

	static CAgentClass managerClass;	
	const NLIAC::CIdentType CManagerClass::IdManagerClass("AgentManager", NLIAC::CSelfClassCFactory(managerClass),
													NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tAgentInterpret),
													NLIAC::CTypeOfOperator(0));


	static CStackPointer	staticStack(1),staticHeap(1);
	const NLIAC::CIdentType CCodeContext::IdCodeContext = NLIAC::CIdentType("CodeContext",
																		NLIAC::CSelfClassCFactory(CCodeContext(staticStack,staticHeap,NULL,NULL,NULL)),
																		NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
}