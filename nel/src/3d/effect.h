/** \file effect.h
 * effect
 *
 * $Id: effect.h,v 1.1.2.1 2007/03/16 11:11:11 legallo Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_EFFECT_H
#define NL_EFFECT_H

#include "effect_context.h"

#include "texture.h"

//MISC
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"

#include <typeinfo>

namespace NL3D 
{

const uint32 IDRV_EFFECT_MAXTEXTURES	=	4;

class IDriver;
class CVertexProgram;
class CPixelProgram;

//-------------------------------------------------------------------------------
//----------------------------- CEffectParameter --------------------------------
//-------------------------------------------------------------------------------
struct CEffectParameter : public CTypeParameter, public CProgramTypeParameter
{
public:

	CEffectParameter()
	{
		IsTexture = false;
		Name = "";
		RegisterNb	= -1;
		RegisterCount	= -1;
	}

	CEffectParameter(std::string & name)
		: CTypeParameter(), CProgramTypeParameter()
	{
		IsTexture = false;
		Name = name;
		RegisterNb = -1;
		RegisterCount	= -1;
	}

	CEffectParameter(sint32 rgsNb, std::string & name, TProgramType progType, TTypeParameter type, uint lSize=1, uint cSize=1)
		: CTypeParameter(type, lSize, cSize), CProgramTypeParameter(progType)
	{
		IsTexture = false;
		Name = name;
		RegisterNb	= rgsNb;
		RegisterCount	= -1;
	}

	const std::string & getName() { return Name; }

	void setTexture(bool isText) { IsTexture = isText; }
	bool isTexture() { return IsTexture; }

	sint32 getRegisterNb() { return RegisterNb; }
	void setRegisterNb(sint32 rgsNb) { RegisterNb=rgsNb; }

	sint32 getRegisterCount() { return RegisterCount; }
	void setRegisterCount(sint32 rgsCount) { RegisterCount=rgsCount; }

protected:

	bool IsTexture;
	std::string Name;
	sint32	RegisterNb;
	sint32	RegisterCount;
};
typedef	std::map<std::string, CEffectParameter >	TEffectParametersMap;
typedef TEffectParametersMap::const_iterator		ItEffectParametersMap;

//-------------------------------------------------------------------------------
//---------------------------- CEffectUserParameter -----------------------------
//-------------------------------------------------------------------------------
// User Parameter storage
struct CEffectUserParameter : public CEffectParameter
{
public:
	
	CEffectUserParameter() : CEffectParameter() {}

	CEffectUserParameter(std::string & name) : CEffectParameter(name) {}

	CEffectUserParameter(sint32 rgsNb, std::string & name,TProgramType progType, TTypeParameter type, uint lSize=1, uint cSize=0)
		: CEffectParameter(rgsNb, name, progType, type, lSize, cSize) {}

	CEffectUserParameter(const CEffectParameter & param) : CEffectParameter(param) {}
};
typedef	std::list< CEffectUserParameter > TEffectUserParametersList;
typedef	TEffectUserParametersList::iterator ItEffectUserParametersList;



//-------------------------------------------------------------------------------
//------------------------------ CEffectDrvInfos --------------------------------
//-------------------------------------------------------------------------------

// List typedef.
class CEffectDrvInfos;
class TEffectDrvInfoPtrMap : public std::map< std::string, CEffectDrvInfos*> {};
typedef	TEffectDrvInfoPtrMap::iterator ItEffectDrvInfoPtrMap;

// Class for interaction of effects with Driver.
// CEffectDrvInfos represent the real data of the effect, stored into the driver.
class CEffectDrvInfos : public NLMISC::CRefCount
{
public:

	CEffectDrvInfos(IDriver *drv, ItEffectDrvInfoPtrMap it);
	virtual ~CEffectDrvInfos(void);

	CVertexProgram * getVertexProgram() { return _VertexProgram; }
	CPixelProgram * getPixelProgram() { return _PixelProgram; }

	bool parseProgramParameters(CProgramTypeParameter::TProgramType progType, std::string & errorOutput);

private:

	friend	class	CEffect;
	friend class CEffectParser;

	bool bind(CEffect * effect);

	bool initPrograms(std::string & errorOutput);

	IDriver *					_Driver;
	ItEffectDrvInfoPtrMap		_DriverIterator;

	CVertexProgram *			_VertexProgram;
	CPixelProgram *				_PixelProgram;

	TEffectUserParametersList _VertexUserParams;
	TEffectUserParametersList _PixelUserParams;

	TEffectContextParametersList _VertexContextParams;
	TEffectContextParametersList _PixelContextParams;

	bool _InitPrograms;
	bool _ParsedVertexProgram;
	bool _ParsedPixelProgram;
};


//-------------------------------------------------------------------------------
//--------------------------------- CEffect -------------------------------------
//-------------------------------------------------------------------------------
struct IUserParameter;
typedef std::map< std::string , IUserParameter * >				TUserValuesMap;
typedef TUserValuesMap::iterator								ItUserValuesMap;
typedef std::map< std::string, NLMISC::CSmartPtr<ITexture> >	TUserTexturesMap;
typedef TUserTexturesMap::iterator								ItUserTexturesMap;

class CEffect : public NLMISC::CRefCount
{

public:

	//enum TProgramType {VertexProgram, PixelProgram};

	CEffect(const std::string & filename);
	~CEffect();

	/** 
	 * setup user and context parmaeters.
	 */	
	bool setup();

	/** 
	 * Tells if the effect has been setuped by the driver.
	 */	
	bool setupedIntoDriver() const;

	void setEffectDriverInfos(CEffectDrvInfos * effectDrvInfo) { _EffectDrv=effectDrvInfo; }

	const std::string & getName();

	NLMISC::CSmartPtr<CEffectDrvInfos> & getEffectDrvInfos();
	void setEffectDrvInfos(NLMISC::CSmartPtr<CEffectDrvInfos> & effectDrv);

	void addUserParameterValue(const std::string & name, IUserParameter * value);

	bool setUserParameterValue(CEffectUserParameter * param, IDriver * driver);

	// for IDriver::setupMaterial
	ITexture * getTexture(uint8 stage);

	void addUserTexture(const std::string & name, ITexture * tex);
	bool setUserTexture(CEffectUserParameter * param);

private:

	friend	class	CEffectDrvInfos;

	std::string		_FileName;

	NLMISC::CSmartPtr<CEffectDrvInfos> _EffectDrv;

	TUserValuesMap	_VertexUserValuesMap;
	TUserValuesMap	_PixelUserValuesMap;

	NLMISC::CSmartPtr<ITexture>		_Textures[IDRV_EFFECT_MAXTEXTURES];
	TUserTexturesMap				_TexturesMap;
};

//-------------------------------------------------------------------------------
//------------------------------ IUserParameter ---------------------------------
//-------------------------------------------------------------------------------
struct IUserParameter : public CProgramTypeParameter
{
public:

	IUserParameter() 
	{
		ColumnSize = 1;
		LineSize = 1;
	}
	IUserParameter(TProgramType progType, uint cSize, uint lSize) 
		: CProgramTypeParameter(progType)
	{
		ColumnSize = cSize;
		LineSize = lSize;
	}
	~IUserParameter() {}

	virtual bool setUserParameterValue(CEffectUserParameter* userParam, IDriver * driver) = 0;

	void setLineSize(uint lSize)	{ LineSize=lSize; }
	void setColumnSize(uint cSize)	{ ColumnSize=cSize; }

protected:

	virtual void setProgramConstant(IDriver * driver, CEffectUserParameter* userParam) = 0;

	virtual const char * getTypeParameter() = 0;

	uint				LineSize;
	uint				ColumnSize;
};


//-------------------------------------------------------------------------------
//-------------------------- IUserParameterValue --------------------------------
//-------------------------------------------------------------------------------
template<class T> struct IUserParameterValue : public IUserParameter
{
public:

	IUserParameterValue() : IUserParameter() {}

	IUserParameterValue(T * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameter(progType, cSize, lSize)
	{
		Values = values;
	}

	~IUserParameterValue() {}

	virtual bool setUserParameterValue(CEffectUserParameter* userParam, IDriver * driver)
	{
		CEffectParameter::TTypeParameterMap & converter = CEffectUserParameter::getConverterTypeStrToEnum();
		CEffectParameter::TTypeParameter type = converter[getTypeParameter()];
		if(type==userParam->getType() && LineSize==userParam->getLineSize() && ColumnSize==userParam->getColumnSize())
		{
			setProgramConstant(driver, userParam);
			return true;
		}
		
		nlwarning("Don't find user parameter value '%s' with correct type and size %i, %i", 
			userParam->getName().c_str(), userParam->getLineSize(), userParam->getColumnSize());
		return false;
	}

	void setValues(T * values) { Values=values; }

protected:

	virtual const char * getTypeParameter() 
	{ 
		const type_info &ti = typeid(T);
		return ti.name();
	}

	T *	Values;
};

//-------------------------------------------------------------
class CUserParameterDouble : public IUserParameterValue<double>
{
public:
	CUserParameterDouble() : IUserParameterValue<double>() {}
	CUserParameterDouble(double * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<double>(values, progType, cSize, lSize) {}
	
protected:
	virtual void setProgramConstant(IDriver * driver, CEffectUserParameter* userParam);
};

//-------------------------------------------------------------
class CUserParameterFloat : public IUserParameterValue<float>
{
public:
	CUserParameterFloat() : IUserParameterValue<float>() {}
	CUserParameterFloat(float * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<float>(values, progType, cSize, lSize) {}

protected:
	virtual void setProgramConstant(IDriver * driver, CEffectUserParameter* userParam);
};

//-------------------------------------------------------------
class CUserParameterInt : public IUserParameterValue<int>
{
public:
	CUserParameterInt() : IUserParameterValue<int>() {}
	CUserParameterInt(int * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<int>(values, progType, cSize, lSize) {}

protected:
	virtual void setProgramConstant(IDriver * driver, CEffectUserParameter* userParam);
};

//-------------------------------------------------------------
class CUserParameterBool : public IUserParameterValue<bool>
{
public:
	CUserParameterBool() : IUserParameterValue<bool>() {}
	CUserParameterBool(bool * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<bool>(values, progType, cSize, lSize) {}

protected:
	virtual void setProgramConstant(IDriver * driver, CEffectUserParameter* userParam);
};

//-------------------------------------------------------------------------------
//------------------------------- CEffectParser ---------------------------------
//-------------------------------------------------------------------------------
class CEffectParser
{
public:

	CEffectParser();
	~CEffectParser();


	bool getTagContent(NLMISC::CSString str, char * tag, NLMISC::CSString & contentStr,
					   std::string::size_type & startPos, std::string::size_type & endPos);
	bool getTagContent(NLMISC::CSString str, char * tag, NLMISC::CSString & contentStr);

	bool getUserParametersList(NLMISC::CSString & str, CProgramTypeParameter::TProgramType progType, 
		TEffectUserParametersList & params, const TEffectParametersMap & paramsMap);

	bool getContextParametersList(NLMISC::CSString & str, CProgramTypeParameter::TProgramType progType, 
		TEffectContextParametersList & params, const TEffectParametersMap & paramsMap);

	const std::string & getErrorStr() { return _ErrorStr; }

private:

	std::string _ErrorStr;
};

} // NL3D

#endif // NL_EFFECT_H

/* End of effect.h */
