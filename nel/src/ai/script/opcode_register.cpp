/** \file opcode_register.cpp
 *
 * $Id: opcode_register.cpp,v 1.7 2001/03/14 13:19:34 chafik Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/libcode.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_load.h"

namespace NLAISCRIPT
{

const NLAIC::CIdentType CLdbOpCode::IdLdbOpCode = NLAIC::CIdentType("LdbOpCode",
																NLAIC::CSelfClassFactory(CLdbOpCode(NLAIAGENT::DigitalType(0.0))),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

std::list<sint32> l;

const NLAIC::CIdentType CLdbMemberiOpCode::IdLdbMemberiOpCode = NLAIC::CIdentType("LdbMemberiOpCode",
																NLAIC::CSelfClassFactory(CLdbMemberiOpCode(l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLdbStackMemberiOpCode::IdLdbStackMemberiOpCode = NLAIC::CIdentType("LdbStackMemberiOpCode",
																NLAIC::CSelfClassFactory(CLdbStackMemberiOpCode(l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLdbHeapMemberiOpCode::IdLdbHeapMemberiOpCode = NLAIC::CIdentType("LdbHeapMemberiOpCode",
																NLAIC::CSelfClassFactory(CLdbHeapMemberiOpCode(l,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLdbMemberOpCode::IdLdbMemberOpCode = NLAIC::CIdentType("LdbMemberOpCode",
																NLAIC::CSelfClassFactory(CLdbMemberOpCode(0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));


const NLAIC::CIdentType CLdbNewOpCode::IdLdbNewOpCode = NLAIC::CIdentType("LdbNewOpCode",
																NLAIC::CSelfClassFactory(CLdbNewOpCode(NLAIAGENT::DigitalType(0.0))),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

CLdbRefOpCode iLdbRefOpCode(0);
const NLAIC::CIdentType CLdbRefOpCode::IdLdbRefOpCode = NLAIC::CIdentType("LdbRefOpCode",
																	NLAIC::CSelfClassFactory(iLdbRefOpCode),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CNegOpCode::IdNegOpCode = NLAIC::CIdentType("NegOpCode",
																NLAIC::CSelfClassFactory(CNegOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CAddOpCode::IdAddOpCode = NLAIC::CIdentType("AddOpCode",
																NLAIC::CSelfClassFactory(CAddOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CSubOpCode::IdSubOpCode = NLAIC::CIdentType("SubOpCode",
																NLAIC::CSelfClassFactory(CSubOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CDivOpCode::IdDivOpCode = NLAIC::CIdentType("DivOpCode",
																NLAIC::CSelfClassFactory(CDivOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CMulOpCode::IdMulOpCode = NLAIC::CIdentType("MulOpCode",
																NLAIC::CSelfClassFactory(CMulOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CSupOpCode::IdSupOpCode = NLAIC::CIdentType("SupOpCode",
																NLAIC::CSelfClassFactory(CSupOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CInfOpCode::IdInfOpCode = NLAIC::CIdentType("InfOpCode",
																NLAIC::CSelfClassFactory(CInfOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CEqOpCode::IdEqOpCode = NLAIC::CIdentType("EqOpCode",
															NLAIC::CSelfClassFactory(CEqOpCode()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CSupEqOpCode::IdSupEqOpCode = NLAIC::CIdentType("SupEqOpCode",
																	NLAIC::CSelfClassFactory(CSupEqOpCode()),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CInfEqOpCode::IdInfEqOpCode = NLAIC::CIdentType("InfEqOpCode",
																	NLAIC::CSelfClassFactory(CInfEqOpCode()),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CDiffOpCode::IdDiffOpCode = NLAIC::CIdentType("DiffOpCode",
																NLAIC::CSelfClassFactory(CDiffOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CNotOpCode::IdNotOpCode = NLAIC::CIdentType("NotOpCode",
																NLAIC::CSelfClassFactory(CNotOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CJFalseOpCode::IdJFalseOpCode = NLAIC::CIdentType("JFalseOpCode",
																	NLAIC::CSelfClassFactory(CJFalseOpCode(-1)),
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CJmpOpCode::IdJmpOpCode = NLAIC::CIdentType("JmpOpCode",
																NLAIC::CSelfClassFactory(CJmpOpCode(-1)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CAffOpCode::IdAffOpCode = NLAIC::CIdentType("AffOpCode",
																NLAIC::CSelfClassFactory(CAffOpCode(-1)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CAffOpCodeDebug::IdAffOpCodeDebug = NLAIC::CIdentType("AffOpCodeDebug",
																NLAIC::CSelfClassFactory(CAffOpCodeDebug(-1," ")),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CAffMemberOpCode::IdAffMemberOpCode = NLAIC::CIdentType("AffMemberOpCode",
																NLAIC::CSelfClassFactory(CAffMemberOpCode(-1)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));


const NLAIC::CIdentType CAffMemberiOpCode::IdAffMemberiOpCode = NLAIC::CIdentType("AffMemberiOpCode",
																NLAIC::CSelfClassFactory(CAffMemberiOpCode(l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CAffHeapMemberiOpCode::IdAffHeapMemberiOpCode = NLAIC::CIdentType("AffHeapMemberiOpCode",
																NLAIC::CSelfClassFactory(CAffHeapMemberiOpCode(l,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLocAlloc::IdLocAlloc = NLAIC::CIdentType("LocAlloc",
															NLAIC::CSelfClassFactory(CLocAlloc()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLocAllocDebug::IdLocAllocDebug = NLAIC::CIdentType("LocAllocDebug",
															NLAIC::CSelfClassFactory(CLocAllocDebug()),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CMarkAlloc::IdMarkAlloc = NLAIC::CIdentType(	"MarkAlloc",
																NLAIC::CSelfClassFactory(CMarkAlloc()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CFreeAlloc::IdFreeAlloc = NLAIC::CIdentType(	"FreeAlloc",
																NLAIC::CSelfClassFactory(CFreeAlloc()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CFreeAllocDebug::IdFreeAllocDebug = NLAIC::CIdentType(	"FreeAllocDebug",
																NLAIC::CSelfClassFactory(CFreeAllocDebug()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CNopOpCode::IdNopOpCode = NLAIC::CIdentType(	"NopOpCode",
																NLAIC::CSelfClassFactory(CNopOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CHaltOpCode::IdHaltOpCode = NLAIC::CIdentType(	"HaltOpCode",
																NLAIC::CSelfClassFactory(CHaltOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CCallMethod::IdCallMethode = NLAIC::CIdentType("CallMethode",
																NLAIC::CSelfClassFactory(CCallMethod(new CMethodContext(),0,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CCallMethodi::IdCallMethodei = NLAIC::CIdentType("CallMethodei",
																NLAIC::CSelfClassFactory(CCallMethodi(new CMethodContext(),0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CCallHeapMethodi::IdCallHeapMethodei = NLAIC::CIdentType("CallHeapMethodei",
																NLAIC::CSelfClassFactory(CCallHeapMethodi(new CMethodContext(),0,0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CCallStackNewMethodi::IdCallStackNewMethodei = NLAIC::CIdentType("CallStackNewMethodei",																			 
																NLAIC::CSelfClassFactory(CCallStackNewMethodi(new CMethodContext(),0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));


const NLAIC::CIdentType CCallStackMethodi::IdCallStackMethodei = NLAIC::CIdentType("CallStackMethodei",																			 
																NLAIC::CSelfClassFactory(CCallStackMethodi(new CMethodContext(),0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));


const NLAIC::CIdentType CLibMemberMethod::IdLibMemberMethod = NLAIC::CIdentType("LibMemberMethod",
																NLAIC::CSelfClassFactory(CLibMemberMethod(0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibMemberInheritedMethod::IdLibMemberInheritedMethod = NLAIC::CIdentType("LibMemberInheritedMethod",
																NLAIC::CSelfClassFactory(CLibMemberInheritedMethod( 0,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibMemberMethodi::IdLibMemberMethodi = NLAIC::CIdentType("LibMemberMethodi",
																NLAIC::CSelfClassFactory(CLibMemberMethodi( 0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibCallMethod::IdLibCallMethod = NLAIC::CIdentType("LibCallMethod",
																NLAIC::CSelfClassFactory(CLibCallMethod(0,CCallPrint())),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibCallInheritedMethod::IdLibCallInheritedMethod = NLAIC::CIdentType("LibCallInheritedMethod",
																NLAIC::CSelfClassFactory(CLibCallInheritedMethod( 0,0,CCallPrint())),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibCallMethodi::IdLibCallMethodi = NLAIC::CIdentType("LibCallMethodi",
																NLAIC::CSelfClassFactory(CLibCallMethodi( 0,0,l,CCallPrint())),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibStackMemberMethod::IdLibStackMemberMethod = NLAIC::CIdentType("LibStackMemberMethod",
																NLAIC::CSelfClassFactory(CLibStackMemberMethod( 0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibStackNewMemberMethod::IdLibStackNewMemberMethod = NLAIC::CIdentType("LibStackNewMemberMethod",
																NLAIC::CSelfClassFactory(CLibStackNewMemberMethod( 0,0,l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CLibHeapMemberMethod::IdLibHeapMemberMethod = NLAIC::CIdentType("LibHeapMemberMethod",
																NLAIC::CSelfClassFactory(CLibHeapMemberMethod( 0,0,l,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));


const NLAIC::CIdentType CTellOpCode::IdTellOpCode = NLAIC::CIdentType("TellOpCode",
																NLAIC::CSelfClassFactory(CTellOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
const NLAIC::CIdentType CMakeArgOpCode::IdMakeArgOpCode = NLAIC::CIdentType("MakeArgOpCode",
																NLAIC::CSelfClassFactory(CMakeArgOpCode()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
const NLAIC::CIdentType CMarkMsg::IdMarkMsg = NLAIC::CIdentType("MarkMsg",
																NLAIC::CSelfClassFactory(CMarkMsg(0,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CFindRunMsg::IdFindRunMsg = NLAIC::CIdentType("FindRunMsg",
																NLAIC::CSelfClassFactory(CFindRunMsg(NLAIAGENT::CGroupType(),CParam(),COperandVoid(),CLoadSelfObject()) ),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
const NLAIC::CIdentType CMsgSetSender::IdMsgSetSender = NLAIC::CIdentType("MsgSetSender",
																NLAIC::CSelfClassFactory(CMsgSetSender()),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

const NLAIC::CIdentType CAddParamNameDebug::IdAddParamNameDebug = NLAIC::CIdentType("AddParamNameDebug",
																NLAIC::CSelfClassFactory(CAddParamNameDebug(NLAIAGENT::CGroupType())),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));


const NLAIC::CIdentType CLoadSelfObject::IdLoadSelfObject = NLAIC::CIdentType("LoadSelfObject",
																NLAIC::CSelfClassFactory(CLoadSelfObject(l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
const NLAIC::CIdentType CLoadStackObject::IdLoadSelfObject = NLAIC::CIdentType("LoadStackObject",
																NLAIC::CSelfClassFactory(CLoadStackObject(l)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
const NLAIC::CIdentType CLoadHeapObject::IdLoadHeapObject = NLAIC::CIdentType("LoadHeapObject",
																NLAIC::CSelfClassFactory(CLoadHeapObject(l,0)),
																NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));

}
