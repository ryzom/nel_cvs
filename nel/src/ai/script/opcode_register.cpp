/** \file opcode_register.cpp
 *
 * $Id: opcode_register.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "script/compilateur.h"
#include "script/libcode.h"
#include "script/type_def.h"
#include "script/object_load.h"

namespace NLIASCRIPT
{

const NLIAC::CIdentType CLdbOpCode::IdLdbOpCode = NLIAC::CIdentType("LdbOpCode",
																NLIAC::CSelfClassCFactory(CLdbOpCode(NLIAAGENT::DigitalType(0.0))),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

std::list<sint32> l;

const NLIAC::CIdentType CLdbMemberiOpCode::IdLdbMemberiOpCode = NLIAC::CIdentType("LdbMemberiOpCode",
																NLIAC::CSelfClassCFactory(CLdbMemberiOpCode(l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLdbStackMemberiOpCode::IdLdbStackMemberiOpCode = NLIAC::CIdentType("LdbStackMemberiOpCode",
																NLIAC::CSelfClassCFactory(CLdbStackMemberiOpCode(l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLdbHeapMemberiOpCode::IdLdbHeapMemberiOpCode = NLIAC::CIdentType("LdbHeapMemberiOpCode",
																NLIAC::CSelfClassCFactory(CLdbHeapMemberiOpCode(l,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLdbMemberOpCode::IdLdbMemberOpCode = NLIAC::CIdentType("LdbMemberOpCode",
																NLIAC::CSelfClassCFactory(CLdbMemberOpCode(0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));


const NLIAC::CIdentType CLdbNewOpCode::IdLdbNewOpCode = NLIAC::CIdentType("LdbNewOpCode",
																NLIAC::CSelfClassCFactory(CLdbNewOpCode(NLIAAGENT::DigitalType(0.0))),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

CLdbRefOpCode iLdbRefOpCode(0);
const NLIAC::CIdentType CLdbRefOpCode::IdLdbRefOpCode = NLIAC::CIdentType("LdbRefOpCode",
																	NLIAC::CSelfClassCFactory(iLdbRefOpCode),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CNegOpCode::IdNegOpCode = NLIAC::CIdentType("NegOpCode",
																NLIAC::CSelfClassCFactory(CNegOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CAddOpCode::IdAddOpCode = NLIAC::CIdentType("AddOpCode",
																NLIAC::CSelfClassCFactory(CAddOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CSubOpCode::IdSubOpCode = NLIAC::CIdentType("SubOpCode",
																NLIAC::CSelfClassCFactory(CSubOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CDivOpCode::IdDivOpCode = NLIAC::CIdentType("DivOpCode",
																NLIAC::CSelfClassCFactory(CDivOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CMulOpCode::IdMulOpCode = NLIAC::CIdentType("MulOpCode",
																NLIAC::CSelfClassCFactory(CMulOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CSupOpCode::IdSupOpCode = NLIAC::CIdentType("SupOpCode",
																NLIAC::CSelfClassCFactory(CSupOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CInfOpCode::IdInfOpCode = NLIAC::CIdentType("InfOpCode",
																NLIAC::CSelfClassCFactory(CInfOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CEqOpCode::IdEqOpCode = NLIAC::CIdentType("EqOpCode",
															NLIAC::CSelfClassCFactory(CEqOpCode()),
															NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CSupEqOpCode::IdSupEqOpCode = NLIAC::CIdentType("SupEqOpCode",
																	NLIAC::CSelfClassCFactory(CSupEqOpCode()),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CInfEqOpCode::IdInfEqOpCode = NLIAC::CIdentType("InfEqOpCode",
																	NLIAC::CSelfClassCFactory(CInfEqOpCode()),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CDiffOpCode::IdDiffOpCode = NLIAC::CIdentType("DiffOpCode",
																NLIAC::CSelfClassCFactory(CDiffOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CNotOpCode::IdNotOpCode = NLIAC::CIdentType("NotOpCode",
																NLIAC::CSelfClassCFactory(CNotOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CJFalseOpCode::IdJFalseOpCode = NLIAC::CIdentType("JFalseOpCode",
																	NLIAC::CSelfClassCFactory(CJFalseOpCode(-1)),
																	NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CJmpOpCode::IdJmpOpCode = NLIAC::CIdentType("JmpOpCode",
																NLIAC::CSelfClassCFactory(CJmpOpCode(-1)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CAffOpCode::IdAffOpCode = NLIAC::CIdentType("AffOpCode",
																NLIAC::CSelfClassCFactory(CAffOpCode(-1)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CAffOpCodeDebug::IdAffOpCodeDebug = NLIAC::CIdentType("AffOpCodeDebug",
																NLIAC::CSelfClassCFactory(CAffOpCodeDebug(-1," ")),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CAffMemberOpCode::IdAffMemberOpCode = NLIAC::CIdentType("AffMemberOpCode",
																NLIAC::CSelfClassCFactory(CAffMemberOpCode(-1)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));


const NLIAC::CIdentType CAffMemberiOpCode::IdAffMemberiOpCode = NLIAC::CIdentType("AffMemberiOpCode",
																NLIAC::CSelfClassCFactory(CAffMemberiOpCode(l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CAffHeapMemberiOpCode::IdAffHeapMemberiOpCode = NLIAC::CIdentType("AffHeapMemberiOpCode",
																NLIAC::CSelfClassCFactory(CAffHeapMemberiOpCode(l,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLocAlloc::IdLocAlloc = NLIAC::CIdentType("LocAlloc",
															NLIAC::CSelfClassCFactory(CLocAlloc()),
															NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLocAllocDebug::IdLocAllocDebug = NLIAC::CIdentType("LocAllocDebug",
															NLIAC::CSelfClassCFactory(CLocAllocDebug()),
															NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CMarkAlloc::IdMarkAlloc = NLIAC::CIdentType(	"MarkAlloc",
																NLIAC::CSelfClassCFactory(CMarkAlloc()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CFreeAlloc::IdFreeAlloc = NLIAC::CIdentType(	"FreeAlloc",
																NLIAC::CSelfClassCFactory(CFreeAlloc()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CFreeAllocDebug::IdFreeAllocDebug = NLIAC::CIdentType(	"FreeAllocDebug",
																NLIAC::CSelfClassCFactory(CFreeAllocDebug()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CNopOpCode::IdNopOpCode = NLIAC::CIdentType(	"NopOpCode",
																NLIAC::CSelfClassCFactory(CNopOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CHaltOpCode::IdHaltOpCode = NLIAC::CIdentType(	"HaltOpCode",
																NLIAC::CSelfClassCFactory(CHaltOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CCallMethod::IdCallMethode = NLIAC::CIdentType("CallMethode",
																NLIAC::CSelfClassCFactory(CCallMethod(new CMethodContext(),0,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CCallMethodi::IdCallMethodei = NLIAC::CIdentType("CallMethodei",
																NLIAC::CSelfClassCFactory(CCallMethodi(new CMethodContext(),0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CCallHeapMethodi::IdCallHeapMethodei = NLIAC::CIdentType("CallHeapMethodei",
																NLIAC::CSelfClassCFactory(CCallHeapMethodi(new CMethodContext(),0,0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CCallStackNewMethodi::IdCallStackNewMethodei = NLIAC::CIdentType("CallStackNewMethodei",																			 
																NLIAC::CSelfClassCFactory(CCallStackNewMethodi(new CMethodContext(),0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));


const NLIAC::CIdentType CCallStackMethodi::IdCallStackMethodei = NLIAC::CIdentType("CallStackMethodei",																			 
																NLIAC::CSelfClassCFactory(CCallStackMethodi(new CMethodContext(),0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));


const NLIAC::CIdentType CLibMemberMethod::IdLibMemberMethod = NLIAC::CIdentType("LibMemberMethod",
																NLIAC::CSelfClassCFactory(CLibMemberMethod(0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibMemberInheritedMethod::IdLibMemberInheritedMethod = NLIAC::CIdentType("LibMemberInheritedMethod",
																NLIAC::CSelfClassCFactory(CLibMemberInheritedMethod( 0,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibMemberMethodi::IdLibMemberMethodi = NLIAC::CIdentType("LibMemberMethodi",
																NLIAC::CSelfClassCFactory(CLibMemberMethodi( 0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibCallMethod::IdLibCallMethod = NLIAC::CIdentType("LibCallMethod",
																NLIAC::CSelfClassCFactory(CLibCallMethod(0,CCallPrint())),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibCallInheritedMethod::IdLibCallInheritedMethod = NLIAC::CIdentType("LibCallInheritedMethod",
																NLIAC::CSelfClassCFactory(CLibCallInheritedMethod( 0,0,CCallPrint())),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibCallMethodi::IdLibCallMethodi = NLIAC::CIdentType("LibCallMethodi",
																NLIAC::CSelfClassCFactory(CLibCallMethodi( 0,0,l,CCallPrint())),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibStackMemberMethod::IdLibStackMemberMethod = NLIAC::CIdentType("LibStackMemberMethod",
																NLIAC::CSelfClassCFactory(CLibStackMemberMethod( 0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibStackNewMemberMethod::IdLibStackNewMemberMethod = NLIAC::CIdentType("LibStackNewMemberMethod",
																NLIAC::CSelfClassCFactory(CLibStackNewMemberMethod( 0,0,l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CLibHeapMemberMethod::IdLibHeapMemberMethod = NLIAC::CIdentType("LibHeapMemberMethod",
																NLIAC::CSelfClassCFactory(CLibHeapMemberMethod( 0,0,l,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));


const NLIAC::CIdentType CTellOpCode::IdTellOpCode = NLIAC::CIdentType("TellOpCode",
																NLIAC::CSelfClassCFactory(CTellOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
const NLIAC::CIdentType CMakeArgOpCode::IdMakeArgOpCode = NLIAC::CIdentType("MakeArgOpCode",
																NLIAC::CSelfClassCFactory(CMakeArgOpCode()),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
const NLIAC::CIdentType CMarkMsg::IdMarkMsg = NLIAC::CIdentType("MarkMsg",
																NLIAC::CSelfClassCFactory(CMarkMsg(0,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CFindRunMsg::IdFindRunMsg = NLIAC::CIdentType("FindRunMsg",
																NLIAC::CSelfClassCFactory(CFindRunMsg(NLIAAGENT::CGroupType(),CParam(),COperandVoid(),CLoadSelfObject()) ),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

const NLIAC::CIdentType CAddParamNameDebug::IdAddParamNameDebug = NLIAC::CIdentType("AddParamNameDebug",
																NLIAC::CSelfClassCFactory(CAddParamNameDebug(NLIAAGENT::CGroupType())),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));


const NLIAC::CIdentType CLoadSelfObject::IdLoadSelfObject = NLIAC::CIdentType("LoadSelfObject",
																NLIAC::CSelfClassCFactory(CLoadSelfObject(l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
const NLIAC::CIdentType CLoadStackObject::IdLoadSelfObject = NLIAC::CIdentType("LoadStackObject",
																NLIAC::CSelfClassCFactory(CLoadStackObject(l)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));
const NLIAC::CIdentType CLoadHeapObject::IdLoadHeapObject = NLIAC::CIdentType("LoadHeapObject",
																NLIAC::CSelfClassCFactory(CLoadHeapObject(l,0)),
																NLIAC::CTypeOfObject(NLIAC::CTypeOfObject::tObject),NLIAC::CTypeOfOperator(0));

}