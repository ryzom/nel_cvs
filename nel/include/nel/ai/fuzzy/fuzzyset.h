/** \file fuzzyset.h
 *	Basic fuzzy sets
 *
 * $Id: fuzzyset.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_FUZZYSET_H
#define NL_FUZZYSET_H

#include "agent/agent.h"
#include "agent/agent_digital.h"

namespace NLIAFUZZY {

	class IFuzzySet : public NLIAAGENT::IObjectIA {
		protected:			
			char *_Name;
			std::list<double> _Facts;
			double _Value;

		public:
			IFuzzySet(char *name)
			{
				_Name = new char[strlen(name) + 1];
				strcpy(_Name, name);
			}

			virtual ~IFuzzySet()
			{
				delete[] _Name;
			}

			char *getName()
			{
				return _Name;
			}

			virtual double membership(double) = 0;
			virtual double surface() = 0;
			virtual bool isIn(double) = 0;
			virtual double center() = 0;
			virtual void addFact(double);
			virtual double agregate();
			virtual double getValue();
	};

	class CFuzzyInterval : public IFuzzySet
	{
		private:
			double _Min;
			double _Max;
		public:
			CFuzzyInterval(char *,double min, double max);
			CFuzzyInterval(const CFuzzyInterval &cp);
			virtual double membership(double val);
			virtual bool isIn(double val);
			virtual double surface();
			virtual double center();

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *txt) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual const CProcessResult &run();

			static const NLIAC::CIdentType IdFuzzyInterval;
			virtual void init(NLIAAGENT::IObjectIA *);
	};

	class CRightFuzzySet : public IFuzzySet {
		private:
			double _X1;
			double _X2;
			double _X3;

		public:
			CRightFuzzySet(char *, double, double, double);
			CRightFuzzySet(const CRightFuzzySet &);
			virtual double membership(double val);
			virtual bool isIn(double val);
			virtual double surface();
			virtual double center();

			static const NLIAC::CIdentType IdRightFuzzySet;

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual const CProcessResult &run();
			virtual void init(NLIAAGENT::IObjectIA *);
	};

	class CTriangleFuzzySet : public IFuzzySet {
		private:
			double _X1;
			double _X2;
			double _X3;

		public:
			CTriangleFuzzySet(char *,double, double, double);
			CTriangleFuzzySet(const CTriangleFuzzySet &);

			virtual double membership(double);
			virtual bool isIn(double);
			virtual double surface();
			virtual double center();

			static const NLIAC::CIdentType IdTriangleFuzzySet;
			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual const CProcessResult &run();

			virtual void init(NLIAAGENT::IObjectIA *);
	};

	class CTrapezeFuzzySet : public IFuzzySet {
		private:
			double _X1;
			double _X2;
			double _X3;
			double _X4;

		public:
			static const NLIAC::CIdentType IdTrapezeFuzzySet;

			CTrapezeFuzzySet(char *,double p1,double p2,double p3, double p4);
			CTrapezeFuzzySet(const CTrapezeFuzzySet &);
			~CTrapezeFuzzySet();
			virtual double membership(double val);
			virtual bool isIn(double val);
			virtual double surface();
			virtual double center();

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual const CProcessResult &run();

			virtual void init(NLIAAGENT::IObjectIA *);
	};

	class CLeftFuzzySet : public IFuzzySet {
		private:
			double _X1;
			double _X2;
			double _X3;

		public:
			CLeftFuzzySet(char *,double p1,double p2,double p3);
			CLeftFuzzySet(const CLeftFuzzySet &);
			~CLeftFuzzySet();
			virtual double membership(double val);
			virtual bool isIn(double val);
			virtual double surface();
			virtual double center();

			static const NLIAC::CIdentType IdLeftFuzzySet;

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual const CProcessResult &run();

			virtual void init(NLIAAGENT::IObjectIA *);
	};

/*
	class CompositeFuzzySet : public IFuzzySet {
		private:
			std::list<Filter *> _filters;
		public:
			CompositeFuzzySet();
			CompositeFuzzySet(const CompositeFilter &cp);
			void addFilter(Filter *f);
			virtual double membership(double val);
			virtual bool isIn(double val );
			virtual double surface();
			virtual double center();

			virtual const CProcessResult &run();
			static const NLIAC::CIdentType idCompositeFilter;
			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
	};
*/

}
#endif


