/** \file effect.h
 * effect
 *
 * $Id: effect.h,v 1.1.2.2 2007/03/27 14:01:46 legallo Exp $
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

private:

	friend	class	CEffect;
	friend class CEffectParser;

	bool bind(CEffect * effect);

	bool initPrograms(std::string & errorOutput);

	bool parseProgramParameters(NLMISC::CSString & s, CProgramTypeParameter::TProgramType progType, std::string & errorOutput);

	IDriver *					_Driver;
	ItEffectDrvInfoPtrMap		_DriverIterator;

	CVertexProgram *			_VertexProgram;
	CPixelProgram *				_PixelProgram;

	TEffectUserParametersList _VertexUserParams;
	TEffectUserParametersList _PixelUserParams;

	TEffectContextParametersList _VertexContextParams;
	TEffectContextParametersList _PixelContextParams;
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

	IUserParameterValue();

	IUserParameterValue(TProgramType progType, uint cSize, uint lSize);

	IUserParameterValue(T * values, TProgramType progType, uint cSize, uint lSize);

	~IUserParameterValue();

	virtual bool setUserParameterValue(CEffectUserParameter* userParam, IDriver * driver);

	void setValues(T * values) { Values=values; }

protected:

	void setProgramConstant(IDriver * driver, CEffectUserParameter* userParam);

	void reduceValues(CEffectUserParameter* userParam);

	void addNulValues(CEffectUserParameter* userParam);

	void setProgramConstantMatrix(IDriver * driver, CEffectUserParameter* userParam);

	virtual void setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
										  uint index, uint num, const T *src) = 0;

	virtual const char * getTypeParameter();

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

	virtual void setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
										  uint index, uint num, const double *src);
};

//-------------------------------------------------------------
class CUserParameterFloat : public IUserParameterValue<float>
{
public:
	CUserParameterFloat() : IUserParameterValue<float>() {}
	CUserParameterFloat(float * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<float>(values, progType, cSize, lSize) {}

protected:

	virtual void setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
										  uint index, uint num, const float *src);
};

//-------------------------------------------------------------
class CUserParameterInt : public IUserParameterValue<float>
{
public:
	CUserParameterInt() : IUserParameterValue<float>() {}
	CUserParameterInt(int * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<float>(progType, cSize, lSize)
	{
		uint size = LineSize*ColumnSize;
		float * Values = new float[size];
		for(uint i=0; i<size; i++) Values[i] = (float) values[i];
	}

protected:

	virtual void setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
										  uint index, uint num, const float *src);
};

//-------------------------------------------------------------
class CUserParameterBool : public IUserParameterValue<float>
{
public:
	CUserParameterBool() : IUserParameterValue<float>() {}
	CUserParameterBool(bool * values, TProgramType progType, uint cSize, uint lSize) 
		: IUserParameterValue<float>(progType, cSize, lSize)
	{
		uint size = LineSize*ColumnSize;
		float * Values = new float[size];
		for(uint i=0; i<size; i++) Values[i] = (float) values[i];
	}

protected:

	virtual void setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
										  uint index, uint num, const float *src);
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



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//---------------- Template IUserParameterValue implementation ------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
template<class T>
IUserParameterValue<T>::IUserParameterValue() 
: IUserParameter() 
{
	Values = NULL;
}

//-------------------------------------------------------------------------------
template<class T>
IUserParameterValue<T>::IUserParameterValue(TProgramType progType, uint cSize, uint lSize) 
: IUserParameter(progType, cSize, lSize)
{
	Values = NULL;
}

//-------------------------------------------------------------------------------
template<class T>
IUserParameterValue<T>::IUserParameterValue(T * values, TProgramType progType, uint cSize, uint lSize) 
: IUserParameter(progType, cSize, lSize)
{
	uint size = cSize*lSize;
	Values = new T[size];
	memcpy(Values, values, size*sizeof(T));
}

//-------------------------------------------------------------------------------
template<class T>
IUserParameterValue<T>::~IUserParameterValue() 
{
	if(Values) delete [] Values;
}

//-------------------------------------------------------------------------------
template<class T>
bool IUserParameterValue<T>::setUserParameterValue(CEffectUserParameter* userParam, IDriver * driver)
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

//-------------------------------------------------------------------------------
template<class T>
void IUserParameterValue<T>::setProgramConstant(IDriver * driver, CEffectUserParameter* userParam)
{
	if(userParam->getColumnSize()>1)
	{
		setProgramConstantMatrix(driver, userParam);
	}
	else
	{
		reduceValues(userParam);
		addNulValues(userParam);

		uint quadruplesNb = (int)(LineSize/4);
		setDriverProgramConstant(driver, userParam->getProgramType(), userParam->getRegisterNb(),
			quadruplesNb, Values);
	}
}

//-------------------------------------------------------------------------------
template<class T>
void IUserParameterValue<T>::reduceValues(CEffectUserParameter* userParam)
{
	if(userParam->getRegisterCount()*4 < (sint)LineSize)
	{
		uint size = userParam->getRegisterCount()*4;
		LineSize = size;
		userParam->setLineSize(size);

		T * values = new T[size];
		memcpy(values, Values, size*sizeof(T));
		delete [] Values;
		Values = values;
	}
}

//-------------------------------------------------------------------------------
template<class T>
void IUserParameterValue<T>::addNulValues(CEffectUserParameter* userParam)
{
	uint mod = (uint)fmod(LineSize, 4);
	if(mod!=0)
	{
		uint size = LineSize + (4-mod);
		T * values = new T[size];
		memset(values, 0, size*sizeof(T));
		memcpy(values, Values, LineSize*sizeof(T));
		
		LineSize = size;
		userParam->setLineSize(size);
		delete [] Values;
		Values = values;
	}
}

//-------------------------------------------------------------------------------
template<class T>
void IUserParameterValue<T>::setProgramConstantMatrix(IDriver * driver, CEffectUserParameter* userParam)
{
	uint index = userParam->getRegisterNb();
	uint rowSize = userParam->getLineSize();
	uint columnSize = userParam->getColumnSize();
	
	uint quadruplesNb = (int)(columnSize/4) + (fmod(columnSize, 4)!=0);
	uint eltId, quadrupleIndex;
	T values[4];

	for(uint r=0; r<rowSize; r++)
	{
		eltId = r;
		for(uint q=0; q<quadruplesNb; q++)
		{
			quadrupleIndex = index+quadruplesNb*r+q;
			if(quadrupleIndex>=(uint)userParam->getRegisterCount()+index) return;

			for(uint i=0; i<4; i++)
			{
				if(eltId < rowSize*columnSize)
					values[i] = Values[eltId];
				else
					values[i] = 0.0f;

				eltId += rowSize;
			}

			setDriverProgramConstant(driver, userParam->getProgramType(), quadrupleIndex, 1, values);
		}
	}
}

//-------------------------------------------------------------------------------
template<class T>
const char * IUserParameterValue<T>::getTypeParameter() 
{ 
	const type_info &ti = typeid(T);
	return ti.name();
}


} // NL3D

#endif // NL_EFFECT_H

/* End of effect.h */
