#include "nel/ai/script/compilateur.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/message_script.h"
#include "nel/ai/logic/operator_script.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/interpret_object_manager.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_message_action.h"
#include "nel/ai/script/interpret_message_getvalue.h"
#include "nel/ai/script/interpret_message_connect.h"
#include "nel/ai/script/interpret_message_setvalue.h"
#include "nel/ai/agent/main_agent_script.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/agent/msg_group.h"
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/agent_proxy_mailer.h"
#include "nel/ai/logic/logic.h"
#include "nel/ai/fuzzy/fuzzy.h"
#include "nel/ai/fuzzy/fuzzy_script.h"
#include "nel/ai/agent/actor.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/script/interpret_fsm.h"
#include "nel/ai/logic/fsm_script.h"
#include "nel/ai/logic/fsm_seq_script.h"
#include "nel/ai/agent/performative.h"
#include "nel/ai/agent/object_ident.h"
#include "nel/ai/agent/goal_stack.h"

#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/msg_notify.h"
#include "nel/ai/agent/msg_goal.h"
#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/agent/msg_debug.h"
#include "nel/ai/agent/msg_action.h"
#include "nel/ai/agent/msg_getvalue.h"
#include "nel/ai/agent/msg_on_change.h"
#include "nel/ai/agent/msg_connect.h"
#include "nel/ai/agent/msg_setvalue.h"
#include "nel/ai/script/param_container.h"

	using namespace NLAIAGENT;
	using namespace NLAILOGIC;
	using namespace NLAIFUZZY;

	NLMISC::CSynchronized<CLocWordNumRef::tMapRef> CLocWordNumRef::_LocRefence("CLocWordNumRef::_LocRefence");// = new CLocWordNumRef::tMapRef;
	CAgentNumber CNumericIndex::_I = CAgentNumber();	

	CIndexedVarName::CNameStruc *CIndexedVarName::_TableName = NULL;
	const sint32 CIndexedVarName::_Bank = 256;
	sint32 CIndexedVarName::_Count = 0;
	
	CIndexedVarName::tMapName *CIndexedVarName::_Map = new CIndexedVarName::tMapName;
	std::list<CIndexedVarName::CNameStruc *> *CIndexedVarName::_Empty = new std::list<CIndexedVarName::CNameStruc *>;

	CMessageGroup CMessageGroup::systemGroup(1);
	CMessageGroup CMessageGroup::msgScriptingGroup(2);


	const NLAIC::CIdentType *CObjectIdent::IdObjectIdent = NULL;/*("ObjectIdent",NLAIC::CSelfClassFactory(CObjectIdent("0:0:0")),
																		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
																		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));*/

	static CNumericIndex staticId;
	static const IAgent staticAgent(NULL);
	const NLAIC::CIdentType *CLocWordNumRef::IdLocWordNumRef = NULL;/*("LocWordNumRef",NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)staticAgent),
																		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
																		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));*/

	const NLAIC::CIdentType *IAgent::IdAgent = NULL;/*("GenericAgent", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)staticAgent), 
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));*/

	const NLAIC::CIdentType CSimpleLocalMailBox::IdSimpleLocalMailBox("SimpleLocalMailBox",
														NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CSimpleLocalMailBox(NULL)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	

	const NLAIC::CIdentType CScriptMailBox::IdScriptMailBox("ScriptMailBox",
														NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CScriptMailBox(NULL)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	


	const NLAIC::CIdentType CLocalMailBox::IdLocalMailBox("LocalMailBox",
														NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CLocalMailBox(NULL)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	

		
	const NLAIC::CIdentType CStringType::IdStringType("String",	NLAIC::CSelfClassFactory(CStringType(CStringVarName("XX"))),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
																NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
																NLAIC::CTypeOfOperator::opSub | 																
																NLAIC::CTypeOfOperator::opEq | 
																NLAIC::CTypeOfOperator::opInf | 
																NLAIC::CTypeOfOperator::opSup | 
																NLAIC::CTypeOfOperator::opInfEq | 
																NLAIC::CTypeOfOperator::opSupEq | 																
																NLAIC::CTypeOfOperator::opDiff));

	const NLAIC::CIdentType DigitalType::IdDigitalType("Float",	NLAIC::CSelfClassFactory(DigitalType(0)),
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
/*
	const NLAIC::CIdentType CInt16::IdInt16("Int16",	NLAIC::CSelfClassFactory(DigitalType(0)),
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
																*/


	const NLAIC::CIdentType BorneDigitalType::IdBorneDigitalType("BornedFloat",	NLAIC::CSelfClassFactory(BorneDigitalType(-1,1)),
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

	const NLAIC::CIdentType DDigitalType::IdDDigitalType("Double",	NLAIC::CSelfClassFactory(DDigitalType(0)),
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

	const NLAIC::CIdentType BorneDDigitalType::IdBorneDDigitalType("BornedDouble",	NLAIC::CSelfClassFactory(BorneDDigitalType(-1,1)),
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


	const NLAIC::CIdentType IntegerType::IdIntegerType("sint32",	NLAIC::CSelfClassFactory(IntegerType(0)),
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

	const NLAIC::CIdentType ShortIntegerType::IdShortIntegerType("sint16",	NLAIC::CSelfClassFactory(ShortIntegerType(0)),
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

	const NLAIC::CIdentType CharType::IdCharType("sint8",	NLAIC::CSelfClassFactory(CharType(0)),
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

	const NLAIC::CIdentType UInt8Type::IdUInt8Type("uint8",	NLAIC::CSelfClassFactory(UInt8Type(0)),
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

		const NLAIC::CIdentType UInt16Type::IdUInt16Type("uint16",	NLAIC::CSelfClassFactory(UInt16Type(0)),
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

		const NLAIC::CIdentType UInt32Type::IdUInt32Type("uint32",	NLAIC::CSelfClassFactory(UInt32Type(0)),
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

		const NLAIC::CIdentType UInt64Type::IdUInt64Type("uint64",	NLAIC::CSelfClassFactory(UInt64Type(0)),
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

	const NLAIC::CIdentType CPaireType::IdPaireType("Pair",	NLAIC::CSelfClassFactory(CPaireType(new CharType,new CharType)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
																NLAIC::CTypeOfOperator::opEq );
	
	const NLAIC::CIdentType CVectorGroupManager::IdVectorGroupManager("VectorGroupManager",NLAIC::CSelfClassFactory(CVectorGroupManager()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
															NLAIC::CTypeOfOperator::opSub | 
															NLAIC::CTypeOfOperator::opNot));

	const NLAIC::CIdentType CVectorGroupType::IdVectorGroupType("Vector",NLAIC::CSelfClassFactory(CVectorGroupType()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
															NLAIC::CTypeOfOperator::opSub | 
															NLAIC::CTypeOfOperator::opNot));

	const NLAIC::CIdentType CListGroupManager::IdListGroupManager("ListGroupManager",NLAIC::CSelfClassFactory(CListGroupManager()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
															NLAIC::CTypeOfOperator::opSub | 
															NLAIC::CTypeOfOperator::opNot));

	const NLAIC::CIdentType CGroupType::IdGroupType("List",NLAIC::CSelfClassFactory(CGroupType()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList),
															
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
															NLAIC::CTypeOfOperator::opSub | 
															NLAIC::CTypeOfOperator::opNot));

	const NLAIC::CIdentType CIndexedVarName::IdIndexedVarName("IndexedVarName",NLAIC::CSelfClassFactory(CIndexedVarName("Inst")),
																  NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CStringVarName::IdStringVarName("StringVarName",NLAIC::CSelfClassFactory(CStringVarName("Inst")),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CMessageGroup::IdMessageGroup(	"MessageGroup",NLAIC::CSelfClassFactory(CMessageGroup(1)),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject | NLAIC::CTypeOfObject::tMessage),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CMessageList::IdMessage("MessageList",	NLAIC::CSelfClassFactory(CMessageList()),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList | NLAIC::CTypeOfObject::tMessage),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | NLAIC::CTypeOfOperator::opSub ));
	const NLAIC::CIdentType CMessageVector::IdMessageVector("MessageVector",	NLAIC::CSelfClassFactory(CMessageVector()),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tList | NLAIC::CTypeOfObject::tMessage),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | NLAIC::CTypeOfOperator::opSub ));

	

	const NLAIC::CIdentType CBoolType::IdBoolType("Bool",NLAIC::CSelfClassFactory(CBoolType(false)),	NLAIC::CTypeOfObject::tLogic  | 
																									NLAIC::CTypeOfObject::tNombre,
																									NLAIC::CTypeOfOperator::opAdd |
																									NLAIC::CTypeOfOperator::opSub |
																									NLAIC::CTypeOfOperator::opMul |
																									NLAIC::CTypeOfOperator::opEq  |
																									NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType CAgentScript::IdAgentScript("AgentScript", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)CAgentScript(NULL) ),
		NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret,
		NLAIC::CTypeOfOperator::opEq );

/*	const NLAIC::CIdentType CGDAgentScript::IdGDAgentScript("GDAgentScript", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)CGDAgentScript(NULL) ),
		NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tAgentInterpret,
		NLAIC::CTypeOfOperator::opEq );
*/
	const NLAIC::CIdentType CFsmScript::IdFsmScript("FsmScript", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)CFsmScript(NULL) ),
		NLAIC::CTypeOfObject( NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret  | NLAIC::CTypeOfObject::tActor ),
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CSeqFsmScript::IdSeqFsmScript("SeqFsmScript", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)CSeqFsmScript(NULL) ),
		NLAIC::CTypeOfObject( NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret  | NLAIC::CTypeOfObject::tActor ),
		NLAIC::CTypeOfOperator::opEq );

	static COperatorScript staticOperatorScript(NULL);
	const NLAIC::CIdentType COperatorScript::IdOperatorScript("OperatorScript", 
		NLAIC::CSelfClassFactory( (NLAIC::IBasicInterface &) staticOperatorScript ),
		NLAIC::CTypeOfObject( NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret  | NLAIC::CTypeOfObject::tActor ),
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );
	

	const NLAIC::CIdentType CActorScript::IdActorScript("ActorScript", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)CActorScript(NULL) ),
								NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tActor),
								NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CMainAgentScript::IdMainAgentScript("MainAgentScript", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)CMainAgentScript(NULL) ),
													NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret,
													NLAIC::CTypeOfOperator::opEq );



	const NLAIC::CIdentType CMessageScript::IdMessageScript("MessageScript", NLAIC::CSelfClassFactory( CMessageScript() ),
		NLAIC::CTypeOfObject::tMessage,
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CFuzzyInterval::IdFuzzyInterval = NLAIC::CIdentType("FuzzyInterval",NLAIC::CSelfClassFactory(CFuzzyInterval("Inst",0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CTriangleFuzzySet::IdTriangleFuzzySet = NLAIC::CIdentType("TriangleFuzzySet",
		NLAIC::CSelfClassFactory(CTriangleFuzzySet("Inst",0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CLeftFuzzySet::IdLeftFuzzySet = NLAIC::CIdentType("LeftFuzzySet",NLAIC::CSelfClassFactory(CLeftFuzzySet("Inst",0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CTrapezeFuzzySet::IdTrapezeFuzzySet = NLAIC::CIdentType("TrapezeFuzzySet",NLAIC::CSelfClassFactory(CTrapezeFuzzySet("Inst",0,0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );


	const NLAIC::CIdentType CRightFuzzySet::IdRightFuzzySet = NLAIC::CIdentType("RightFuzzySet",NLAIC::CSelfClassFactory(CRightFuzzySet("Inst",0,0,0)),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CFirstOrderAssert::IdFirstOrderAssert = NLAIC::CIdentType( "FirstOrderAssert", NLAIC::CSelfClassFactory(CFirstOrderAssert( CStringVarName("Inst") )),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 


	const NLAIC::CIdentType CGoal::IdGoal = NLAIC::CIdentType( "GoalObject", NLAIC::CSelfClassFactory( CGoal( CStringVarName("Inst") )),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 

	const NLAIC::CIdentType CGoalStack::IdGoalStack = NLAIC::CIdentType( "GoalStack", NLAIC::CSelfClassFactory( CGoalStack( )),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 

	const NLAIC::CIdentType CBoolAssert::IdBoolAssert = NLAIC::CIdentType( "BoolAssert", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CBoolAssert( (const NLAIAGENT::IVarName &) CStringVarName("Inst") )),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 

	CStringVarName TrucInst("Inst");
	const NLAIC::CIdentType CFact::IdFact = NLAIC::CIdentType( "FactObject", NLAIC::CSelfClassFactory( CFact( TrucInst ) ),
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNeg | 
		NLAIC::CTypeOfOperator::opNot ); 

	const NLAIC::CIdentType CFactBase::IdFactBase = NLAIC::CIdentType( "FactBase", NLAIC::CSelfClassFactory(CFactBase()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CClause::IdClause = NLAIC::CIdentType( "Clause", NLAIC::CSelfClassFactory(CClause()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CRule::IdRule = NLAIC::CIdentType( "Rule", NLAIC::CSelfClassFactory(CRule()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CValueSet::IdValueSet = NLAIC::CIdentType( "ValueSet", NLAIC::CSelfClassFactory( CValueSet(0) ),
		NLAIC::CTypeOfObject::tLogic |
		NLAIC::CTypeOfObject::tList,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNot |
		NLAIC::CTypeOfOperator::opDiff );

	const NLAIC::CIdentType CVar::IdVar("Var",NLAIC::CSelfClassFactory(CVar( CStringVarName("Inst") )),
											NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CVarSet::IdVarSet("VarSet", NLAIC::CSelfClassFactory( CVarSet() ),
		NLAIC::CTypeOfObject::tLogic |
		NLAIC::CTypeOfObject::tList,
		NLAIC::CTypeOfOperator::opEq |
		NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType FuzzyType::IdFuzzyType("FuzzyType",NLAIC::CSelfClassFactory(FuzzyType()),
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

	const NLAIC::CIdentType CFuzzyVar::IdFuzzyVar = NLAIC::CIdentType("FuzzyVar",NLAIC::CSelfClassFactory( CFuzzyVar(CStringVarName("Inst"),0,0) ),
		NLAIC::CTypeOfObject::tLogic  | 
		NLAIC::CTypeOfObject::tNombre,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opSub |
		NLAIC::CTypeOfOperator::opMul |
		NLAIC::CTypeOfOperator::opEq  |
		NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType CVectorMsgContainer::IdVectorMsgContainer("VectorMsgContainer",NLAIC::CSelfClassFactory(CVectorMsgContainer()),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject | NLAIC::CTypeOfObject::tMessage)  ,NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CFactPattern::IdFactPattern = NLAIC::CIdentType( "FactPattern", NLAIC::CSelfClassFactory(CFactPattern()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd |
		NLAIC::CTypeOfOperator::opEq);

	/*const NLAIC::CIdentType CFuzzyControlerScript::IdFuzzyControlerScript("FuzzyControlerScript", NLAIC::CSelfClassFactory( CFuzzyControlerScript() ),
		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));*/

	const NLAIC::CIdentType CFuzzyRuleSet::idFuzzyRuleSet("FuzzyRuleSet", NLAIC::CSelfClassFactory( CFuzzyRuleSet() ),
		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CFuzzyRule::IdFuzzyRule("FuzzyRule",NLAIC::CSelfClassFactory( CFuzzyRule() ),
		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CFuzzyFact::IdFuzzyFact = NLAIC::CIdentType("FuzzyFact",NLAIC::CSelfClassFactory( CFuzzyFact(NULL,0) ),
		NLAIC::CTypeOfObject::tLogic  | 
		NLAIC::CTypeOfObject::tNombre,
		NLAIC::CTypeOfOperator::opEq  |
		NLAIC::CTypeOfOperator::opNot );

	const NLAIC::CIdentType CFirstOrderOperator::IdCFirstOrderOperator = NLAIC::CIdentType( "FirstOrderOperator", NLAIC::CSelfClassFactory(CFirstOrderOperator()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CBoolOperator::IdBoolOperator = NLAIC::CIdentType( "BoolOperator", NLAIC::CSelfClassFactory(CBoolOperator()), 
		NLAIC::CTypeOfObject::tLogic,
		NLAIC::CTypeOfOperator::opAdd );

	const NLAIC::CIdentType CLocalAgentMail::LocalAgentMail = NLAIC::CIdentType( "LocalAgentMail", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CLocalAgentMail((IBasicAgent *)&staticAgent)), 
		NLAIC::CTypeOfObject::tAgent,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CProxyAgentMail::IdProxyAgentMail = NLAIC::CIdentType( "ProxyAgentMail", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CProxyAgentMail()), 
		NLAIC::CTypeOfObject::tAgent,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPExec::IdPExec = NLAIC::CIdentType( "Exec", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPExec()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPAchieve::IdPAchieve = NLAIC::CIdentType( "Achieve", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPAchieve()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPAsk::IdPAsk = NLAIC::CIdentType( "Ask", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPAsk()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPBreak::IdPBreak = NLAIC::CIdentType( "Break", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPBreak()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPTell::IdPTell = NLAIC::CIdentType( "Tell", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPTell()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPKill::IdPKill = NLAIC::CIdentType( "Kill", 
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPKill()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPError::IdPError = NLAIC::CIdentType( "Error",
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPError()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPEven::IdEven = NLAIC::CIdentType( "Even",
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPEven()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

	const NLAIC::CIdentType CPService::IdService = NLAIC::CIdentType( "Service",
		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)CPService()), 
		NLAIC::CTypeOfObject::tPerformative,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone) );

//}

namespace NLAISCRIPT
{

	const NLAIC::CIdentType CMethodeName::IdMethodeName("MethodeName",NLAIC::CSelfClassFactory( CMethodeName() ),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opEq);

	const NLAIC::CIdentType CParam::IdParam("Param",NLAIC::CSelfClassFactory( CParam() ),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator::opEq);

	const NLAIC::CIdentType CPramContainer::IdPramContainer("Container",NLAIC::CSelfClassFactory( CPramContainer() ),
		NLAIC::CTypeOfObject::tObject,
		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opAdd | 
		NLAIC::CTypeOfOperator::opSub | 
		NLAIC::CTypeOfOperator::opNot));

	


	NLAIAGENT::IObjectIA **CVarPStack::_LocalTableRef = NULL;
	#ifdef NL_DEBUG
	NLAIC::CIdentType CVarPStack::IdVarPStack ("VarPStack",	NLAIC::CSelfClassFactory(CVarPStack(0,"Inst")),
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
	NLAIC::CIdentType CVarPStack::IdVarPStack ("VarPStack",	NLAIC::CSelfClassFactory(CVarPStack(0)),
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

	#ifdef NL_DEBUG
	NLAIC::CIdentType CVarPStackParam::IdVarPStackParam ("VarPStackParam",	NLAIC::CSelfClassFactory(CVarPStack(0,"Inst")),
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
	NLAIC::CIdentType CVarPStackParam::IdVarPStackParam ("VarPStackParam",	NLAIC::CSelfClassFactory(CVarPStack(0)),
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
	
	const NLAIC::CIdentType CCodeBrancheRun::IdCodeBrancheRun("CodeBrancheRun",NLAIC::CSelfClassFactory(CCodeBrancheRun(1,CHaltOpCode())),NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
	const NLAIC::CIdentType CCodeBrancheRunDebug::IdCodeBrancheRunDebug("CodeBrancheRunDebug",NLAIC::CSelfClassFactory(CCodeBrancheRunDebug(1,CHaltOpCode())),NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
	const NLAIC::CIdentType IBlock::IdBlock("IBlock",NLAIC::CSelfClassFactory(IBlock(false)),NLAIC::CTypeOfObject(0),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CAgentClass agentClass(NLAIAGENT::CStringVarName("XXX_Agent"));
	const NLAIC::CIdentType CAgentClass::IdAgentClass("Agent", CClassInterpretFactory((const IClassInterpret &)agentClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	

	static CActorClass actorClass(NLAIAGENT::CStringVarName("XXX_Actor"));
	const NLAIC::CIdentType CActorClass::IdActorClass("Actor", CClassInterpretFactory((const IClassInterpret &)actorClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CFsmClass fsmClass(NLAIAGENT::CStringVarName("XXX_Fsm"));
	const NLAIC::CIdentType CFsmClass::IdFsmClass("Fsm", CClassInterpretFactory((const IClassInterpret &)fsmClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CSeqFsmClass seqFsmClass(NLAIAGENT::CStringVarName("XXX_SeqFsm"));
	const NLAIC::CIdentType CSeqFsmClass::IdSeqFsmClass("SeqFsm", CClassInterpretFactory((const IClassInterpret &)seqFsmClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CMessageClass messageClass(NLAIAGENT::CStringVarName("XXX_Message"));
	const NLAIC::CIdentType CMessageClass::IdMessageClass("Message", CClassInterpretFactory((const IClassInterpret &)messageClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tMessage | NLAIC::CTypeOfObject::tInterpret),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CMsgNotifyParentClass msgNotifyParentClass(NLAIAGENT::CStringVarName("XXX_MsgNotifyParent"));
	const NLAIC::CIdentType CMsgNotifyParentClass::IdMsgNotifyParentClass("MsgNotifyParent", CClassInterpretFactory((const IClassInterpret &)msgNotifyParentClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CGoalMsgClass goalMsgClass(NLAIAGENT::CStringVarName("XXX_GoalMsg"));
	const NLAIC::CIdentType CGoalMsgClass::IdGoalMsgClass("GoalMsg", CClassInterpretFactory((const IClassInterpret &)goalMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CCancelGoalMsgClass cancelGoalMsgClass(NLAIAGENT::CStringVarName("XXX_CancelGoalMsg"));
	const NLAIC::CIdentType CCancelGoalMsgClass::IdCancelGoalMsgClass("CancelGoalMsg", CClassInterpretFactory((const IClassInterpret &)cancelGoalMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CFactMsgClass factMsgClass(NLAIAGENT::CStringVarName("XXX_FactMsg"));
	const NLAIC::CIdentType CFactMsgClass::IdFactMsgClass("FactMsg", CClassInterpretFactory((const IClassInterpret &)factMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),	
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CSuccessMsgClass successMsgClass(NLAIAGENT::CStringVarName("XXX_SuccessMsg"));
	const NLAIC::CIdentType CSuccessMsgClass::IdSuccessMsgClass("SuccessMsg", CClassInterpretFactory((const IClassInterpret &)successMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),	
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	

	static CFailureMsgClass failureMsgClass(NLAIAGENT::CStringVarName("XXX_FailureMsg"));
	const NLAIC::CIdentType CFailureMsgClass::IdFailureMsgClass("FailureMsg", CClassInterpretFactory((const IClassInterpret &)failureMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CGetValueMsgClass getvalueMsgClass(NLAIAGENT::CStringVarName("XXX_GetValueMsg"));
	const NLAIC::CIdentType CGetValueMsgClass::IdGetValueMsgClass("GetValueMsg", CClassInterpretFactory((const IClassInterpret &)getvalueMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CConnectObjectValueMsgClass connectObjectValueMsgClass(NLAIAGENT::CStringVarName("XXX_ConnectObjectValueMsg"));
	const NLAIC::CIdentType CConnectObjectValueMsgClass::IdConnectObjectValueMsgClass("ConnectObjectValueMsg", CClassInterpretFactory((const IClassInterpret &)connectObjectValueMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),	
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	

	static CSetValueMsgClass setValueMsgClass(NLAIAGENT::CStringVarName("XXX_setValueMsg"));
	const NLAIC::CIdentType CSetValueMsgClass::IdSetValueMsgClass("SetValueMsg", CClassInterpretFactory((const IClassInterpret &)setValueMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),	
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));	
	
	static CDebugMsgClass msgDebugMsgClass(NLAIAGENT::CStringVarName("MsgDebug"));
	const NLAIC::CIdentType CDebugMsgClass::IdDebugMsgClass("MsgDebug", CClassInterpretFactory((const IClassInterpret &)msgDebugMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));


	static COnChangeMsgClass onChangeMsgClass;
	const NLAIC::CIdentType COnChangeMsgClass::IdOnChangeMsgClass("OnChangeMsg", CClassInterpretFactory((const IClassInterpret &)onChangeMsgClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static COperatorClass operatorClass(NLAIAGENT::CStringVarName("XXX_Operator"));
	const NLAIC::CIdentType COperatorClass::IdOperatorClass("Operator", CClassInterpretFactory((const IClassInterpret &)operatorClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));


	const NLAIC::CIdentType CContextDebug::IdContextDebug("ContextDebug", NLAIC::CSelfClassFactory(CContextDebug()),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	static CManagerClass managerClass(NLAIAGENT::CStringVarName("XXX_AgentManager"));
	const NLAIC::CIdentType CManagerClass::IdManagerClass("AgentManager", CClassInterpretFactory((const IClassInterpret &)managerClass),
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));


	static CStackPointer	staticStack(1),staticHeap(1);
	const NLAIC::CIdentType CCodeContext::IdCodeContext = NLAIC::CIdentType("CodeContext",
																		NLAIC::CSelfClassFactory(CCodeContext(staticStack,staticHeap,NULL,NULL,NULL)),
																		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CScriptDebugSourceFile::IdScriptDebugSourceFile("ScriptDebugSourceFile",NLAIC::CSelfClassFactory(CScriptDebugSourceFile("")),NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const NLAIC::CIdentType CScriptDebugSourceMemory::IdScriptDebugSourceMemory("ScriptDebugSourceMemory",NLAIC::CSelfClassFactory(CScriptDebugSourceMemory("","")),NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
}
	const NLAIC::CIdentType CNotifyParentScript::IdNotifyParentScript("NotifyParentScript", NLAIC::CSelfClassFactory( CNotifyParentScript(&NLAISCRIPT::msgNotifyParentClass) ),
			NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CGoalMsg::IdGoalMsg("GoalMsgScript", NLAIC::CSelfClassFactory( CGoalMsg(&NLAISCRIPT::goalMsgClass) ),
																NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
																NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CCancelGoalMsg::IdCancelGoalMsg("CancelGoalMsgScript", NLAIC::CSelfClassFactory( CCancelGoalMsg(&NLAISCRIPT::cancelGoalMsgClass) ),
				NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
				NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CSuccessMsg::IdSuccessMsg("SuccessMsgScript", NLAIC::CSelfClassFactory( CSuccessMsg(&NLAISCRIPT::successMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CFailureMsg::IdFailureMsg("FailureMsgScript", NLAIC::CSelfClassFactory( CFailureMsg(&NLAISCRIPT::failureMsgClass) ),
		NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
		NLAIC::CTypeOfOperator::opEq );	

	const NLAIC::CIdentType CGetValueMsg::IdGetValueMsg("GetValueMsgScript", NLAIC::CSelfClassFactory( CGetValueMsg(&NLAISCRIPT::getvalueMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CConnectObjectValueMsg::IdConnectObjectValueMsg("ConnectObjectValueMsgScript", NLAIC::CSelfClassFactory( CConnectObjectValueMsg(&NLAISCRIPT::connectObjectValueMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CSetValueMsg::IdSetValueMsg("SetValueMsgScript", NLAIC::CSelfClassFactory( CSetValueMsg(&NLAISCRIPT::setValueMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CFactMsg::IdFactMsg("FactMsgScript", NLAIC::CSelfClassFactory( CFactMsg(&NLAISCRIPT::factMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );

	const NLAIC::CIdentType CMsgDebug::IdMsgDebug("MsgDebugScript", NLAIC::CSelfClassFactory( CMsgDebug(&NLAISCRIPT::msgDebugMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );


	const NLAIC::CIdentType COnChangeMsg::IdOnChangeMsg("OnChangeMsg", NLAIC::CSelfClassFactory( COnChangeMsg(&NLAISCRIPT::onChangeMsgClass) ),
			NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage,
			NLAIC::CTypeOfOperator::opEq );
