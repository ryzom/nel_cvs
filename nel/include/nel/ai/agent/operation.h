/** \file operation.h
 * <File description>
 * File contain class to transform an agent script to an IObjectOp
 * $Id: operation.h,v 1.6 2003/01/27 16:54:43 chafik Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef OPERATION_H
#define OPERATION_H

#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/agent_operator.h"

namespace NLAIAGENT
{	

	/**
	Class CAgentOperation this class allow user to transform an agent script to an IObjectOp, in partuicular, user have an subscribe
	list, that allow to have automatically a send of modification content.

	*/
	class CAgentOperation: public CAgentScript, public virtual IObjetOp
	{
	public:
		static const NLAIC::CIdentType *IdAgentOperation;
		static const NLAIC::CIdentType *idMsgOnChangeMsg;
		static const NLAIAGENT::IMessageBase *MsgOnChangeMsg;

	public:
		enum{
			TSetValue,
			TGetValue,
			TSetName,
			TGetName,
			TUpdate,
			TIsChange,
			TConnect,
			TLastM
		};
		static CMethodCall **StaticMethod;

	protected:
		IObjetOp *_Op;

	private:		
		CStringType *_Name;
		std::list < IConnectIA * >_Connection;
		bool _Change;

	public:
		CAgentOperation();
		CAgentOperation(IObjetOp *o);
		CAgentOperation(const CAgentOperation &);

		virtual ~CAgentOperation();

		virtual void setValue(IObjetOp *o)
		{
			if(_Op != NULL) 
					_Op->release();
			_Op = o;
		}

		const IObjetOp *getsetValue() const
		{
			return _Op;
		}

		void setName(const CStringType &name)
		{
			if(_Name != NULL) 
					_Name->release();
			_Name = new CStringType (CStringVarName(name.getStr().getString()));
		}

		const CStringType *getName() const
		{
			return _Name;
		}

		virtual bool isTrue() const;

		virtual IObjetOp &neg();
		virtual IObjetOp &operator += (const IObjetOp &);
		virtual IObjetOp &operator -= (const IObjetOp &);
		virtual IObjetOp &operator *= (const IObjetOp &);
		virtual IObjetOp &operator /= (const IObjetOp &);

		virtual IObjectIA &operator = (const IObjectIA &a)
		{
			std::string text;
			text = NLAIC::stringGetBuild("IObjectIA &CAgentOperation::operator = (const IObjectIA &a) is very ambiguous don't use it");
			throw NLAIE::CExceptionNotImplemented(text.c_str());
		}

		virtual CProcessResult runActivity();
		virtual bool haveActivity() const
		{
			return true;
		}

		virtual IMessageBase *runTell(const IMessageBase &);

		virtual void onKill(IConnectIA *A);
		virtual const CProcessResult &run();
		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual void load(NLMISC::IStream &is);
		virtual void save(NLMISC::IStream &os);
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(std::string &t) const;

		virtual const NLAIC::CIdentType &getType() const
		{
			return  *IdAgentOperation;
		}

		virtual void update()
		{
			_Change = true;
		}

		virtual bool changed() const
		{			
			return _Change;
		}		

		/**
		Subscribe to a mirror systeme affter call, when change hapend this agent send tell caller the change.
		*/
		virtual void connectOnChange(IConnectIA *ref);		

		virtual void changeIsDone()
		{
			_Change = false;
		}

		virtual int getBaseMethodCount() const;
		virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
		virtual sint32 getMethodIndexSize() const;
		virtual CProcessResult runMethodBase(int, int, IObjectIA *);
		virtual CProcessResult runMethodBase(int, IObjectIA *);

	protected:
		///manage the update of tha attribute.
		void update(IObjectIA *obj);

	public:
		static void initClass();
		static void releaseClass();
	};
}
#endif
