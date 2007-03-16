/** \file effect_context.h
 * Allow the binding of effect' context parameters
 *
 * $Id: effect_context.h,v 1.1.2.1 2007/03/16 11:11:11 legallo Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_EFFECT_CONTEXT_H
#define NL_EFFECT_CONTEXT_H

//MISC
#include "nel/misc/singleton.h"
#include "nel/misc/string_conversion.h"

namespace NL3D {


class IDriver;

//-------------------------------------------------------------------------------
//---------------------------- CProgramTypeParameter ----------------------------
//-------------------------------------------------------------------------------
struct CProgramTypeParameter
{
public:
	
	enum TProgramType {VertexProgram, PixelProgram};

	CProgramTypeParameter() 
	{
		ProgramType = VertexProgram;
	}

	CProgramTypeParameter(TProgramType progType) 
	{
		ProgramType = progType;
	}

	TProgramType getProgramType() { return ProgramType; }
	void setProgramType(TProgramType progType) { ProgramType = progType; }

private:

	TProgramType ProgramType;
};

//-------------------------------------------------------------------------------
//------------------------------- CTypeParameter --------------------------------
//-------------------------------------------------------------------------------
struct CTypeParameter
{
public:

	enum TTypeParameter {Double, Float, Int, Bool, Sampler};
	typedef std::map< std::string, TTypeParameter > TTypeParameterMap;
	
	CTypeParameter()
	{
		Type		= Double;
		LineSize	= 1;
		ColumnSize	= 1;
	}

	CTypeParameter(TTypeParameter type, uint lSize=1, uint cSize=1)
	{
		Type		= type;
		LineSize	= lSize;
		ColumnSize	= cSize;
	}

	bool operator ==(const CTypeParameter & typeParam) const
	{
		return (Type==typeParam.Type && LineSize==typeParam.LineSize && ColumnSize==typeParam.ColumnSize);
	}

	static TTypeParameterMap & getConverterTypeStrToEnum()
	{
		initTypeParameterMap();
		return _ConverterTypeStrToEnum;
	}

	TTypeParameter getType()	{ return Type; }
	uint getLineSize()			{ return LineSize; }
	uint getColumnSize()			{ return ColumnSize; }

	void setType(TTypeParameter type)	{ Type=type; }
	void setLineSize(uint lSize)			{ LineSize=lSize; }
	void setColumnSize(uint cSize)		{ ColumnSize=cSize; }

protected:

	TTypeParameter	Type;
	uint				LineSize;
	uint				ColumnSize;
	
private:

	static void initTypeParameterMap()
	{
		_ConverterTypeStrToEnum["bool"] = Bool;
		_ConverterTypeStrToEnum["double"] = Double;
		_ConverterTypeStrToEnum["float"] = Float;
		_ConverterTypeStrToEnum["int"] = Int;
		_ConverterTypeStrToEnum["sampler"] = Sampler;
	}

	static TTypeParameterMap _ConverterTypeStrToEnum;
};

//-------------------------------------------------------------------------------
//------------------------------- CEffectContext --------------------------------
//-------------------------------------------------------------------------------
struct CEffectContextParameter;

class CEffectContext : public NLMISC::CSingleton<CEffectContext>
{

public:

	enum TContextParameter 
	{
		ModelViewMatrix, 
		InverseModelView,
		ProjMatrix, 
		ModelViewProjection, 
		ViewPosition, 
		Unknow
	};
	typedef std::map< TContextParameter, CTypeParameter> TContextTypeParameterMap;

	CEffectContext();
	~CEffectContext();

	bool setContextParameterValue(IDriver * driver, CEffectContextParameter * contextParam);
	
	bool getContextParameter(CTypeParameter * param, const std::string & contextType, TContextParameter & contextEnum);

private:

	void init();

	static const NLMISC::CStringConversion<TContextParameter> &getConversionTable()
	{
		NL_BEGIN_STRING_CONVERSION_TABLE(TContextParameter)
			NL_STRING_CONVERSION_TABLE_ENTRY(ModelViewMatrix)
			NL_STRING_CONVERSION_TABLE_ENTRY(InverseModelView)
			NL_STRING_CONVERSION_TABLE_ENTRY(ProjMatrix)
			NL_STRING_CONVERSION_TABLE_ENTRY(ModelViewProjection)
			NL_STRING_CONVERSION_TABLE_ENTRY(ViewPosition)
			NL_STRING_CONVERSION_TABLE_ENTRY(Unknow)
		};                                                                                             
		static NLMISC::CStringConversion<TContextParameter>                                                                
		conversionTable(TContextParameter_nl_string_conversion_table, sizeof(TContextParameter_nl_string_conversion_table)   
		/ sizeof(TContextParameter_nl_string_conversion_table[0]),  Unknow);

		return conversionTable;
	}

	TContextParameter getContextParameter(const std::string &str)
	{
		return getConversionTable().fromString(str);
	}

	TContextTypeParameterMap  _TypeParameterMap;
};

//-------------------------------------------------------------------------------
//-------------------------- CEffectContextParameter ----------------------------
//-------------------------------------------------------------------------------
// Context Parameter storage
struct CEffectContextParameter : public CProgramTypeParameter
{
public:

	CEffectContextParameter(TProgramType progType, sint32 rgsNb, CEffectContext::TContextParameter context)
		: CProgramTypeParameter(progType)
	{
		RegisterNb = rgsNb;
		ContextEnum = context;
	}

	sint32 getRegisterNb() { return RegisterNb; }
	void setRegisterNb(sint32 rgsNb) { RegisterNb=rgsNb; }

	CEffectContext::TContextParameter getContextEnum() { return ContextEnum; } 

private:

	sint32			RegisterNb;
	CEffectContext::TContextParameter ContextEnum;
};
typedef	std::list< CEffectContextParameter > TEffectContextParametersList;
typedef	TEffectContextParametersList::iterator ItEffectContextParametersList;

} // NL3D


#endif // NL_EFFECT_CONTEXT_H

/* End of effect_context.h */
