/** \file constraint_find_run.h
 * Class for the compiler fonctionality.
 *
 * $Id: constraint_find_run.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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
#ifndef NL_CONSTRAINT_FIND_RUN_H
#define NL_CONSTRAINT_FIND_RUN_H

namespace NLIASCRIPT
{	
	/**
	* Class CConstraintFindRun.
	* 
	* This class is allow to find the run(msg) method.
	* This method derive from CConstraintMethode, where the CConstraintMethode base class run with a CConstraintMethode::searchCall.
	* 
	* Note that this class is used after a send message to searche the offset of the run that can process the message.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/
	class CConstraintFindRun: public CConstraintMethode
	{	
	private:

		///Type of class that contain the run method.
		TCallTypeOpCode _TypeOfCallType;
	public:
		
		CConstraintFindRun(	CConstraintMethode::TCallTypeOpCode ,int posHeap,IConstraint *baseClass,
							NLIAAGENT::IBaseGroupType *methodName,CParam *param,int lin,int col);
		
		virtual void run(CCompilateur &);		

		const IConstraint *clone() const;		

		ConstraintTypeEnum getTypeOfClass() const
		{
			return constraintFindRun;
		}

		virtual void setOpCode(CCompilateur &comp,IOpCode *x,IConstraint *cType,bool del);
		virtual void setOpCode(IOpCode *x);

		virtual ~CConstraintFindRun();
	};
}

#endif
