/** \file effect.cpp
 * TODO: File description
 *
 * $Id: effect.cpp,v 1.1.2.2 2007/03/27 14:01:46 legallo Exp $
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

#include "std3d.h"

#include "effect.h"
#include "driver.h"

// MISC
#include "nel/misc/file.h"
#include "nel/misc/path.h"


using namespace NLMISC;
using namespace std;

namespace NL3D
{

//-------------------------------------------------------------------------------
//--------------------------------- CEffect -------------------------------------
//-------------------------------------------------------------------------------
CEffect::CEffect(const string & filename)
{
	_FileName = filename;
}

//-------------------------------------------------------------------------------	
CEffect::~CEffect()
{
	for(ItUserValuesMap it=_VertexUserValuesMap.begin(); it!=_VertexUserValuesMap.end(); it++)
		delete it->second;

	for(ItUserValuesMap it=_PixelUserValuesMap.begin(); it!=_PixelUserValuesMap.end(); it++)
		delete it->second;
}

//-------------------------------------------------------------------------------
bool CEffect::setup()
{
	return _EffectDrv->bind(this);
}
	
//-------------------------------------------------------------------------------
bool CEffect::setupedIntoDriver() const 
{
	return _EffectDrv!=NULL;
}

//-------------------------------------------------------------------------------
const string & CEffect::getName()
{
	return _FileName;
}

//-------------------------------------------------------------------------------
NLMISC::CSmartPtr<CEffectDrvInfos> & CEffect::getEffectDrvInfos()
{
	return _EffectDrv;
}

//-------------------------------------------------------------------------------
void CEffect::setEffectDrvInfos(NLMISC::CSmartPtr<CEffectDrvInfos> & effectDrv)
{
	_EffectDrv = effectDrv;
}

//-------------------------------------------------------------------------------
void CEffect::addUserParameterValue(const std::string & name, IUserParameter * value)
{
	TUserValuesMap * valuesMap = NULL;
	switch(value->getProgramType())
	{
	case IUserParameter::VertexProgram:
		valuesMap = &_VertexUserValuesMap;
		break;
	case IUserParameter::PixelProgram:
		valuesMap = &_PixelUserValuesMap;
		break;
	}

	if(valuesMap->find(name)!=valuesMap->end())
		nlwarning("The user parameter '%s' already exists in map", name.c_str());

	(*valuesMap)[name] = value;
}

//-------------------------------------------------------------------------------
bool CEffect::setUserParameterValue(CEffectUserParameter * param, IDriver * driver)
{
	TUserValuesMap * valuesMap = NULL;
	switch(param->getProgramType())
	{
	case CEffectUserParameter::VertexProgram:
		valuesMap = &_VertexUserValuesMap;
		break;
	case CEffectUserParameter::PixelProgram:
		valuesMap = &_PixelUserValuesMap;
		break;
	}

	ItUserValuesMap it = valuesMap->find(param->getName());
	if(it!=valuesMap->end()) return it->second->setUserParameterValue(param, driver);

	nlwarning("Unable to find in map the value of the user parameter '%s'", param->getName().c_str());
	return false;
}

//-------------------------------------------------------------------------------
void CEffect::addUserTexture(const std::string & name, ITexture * tex)
{
	_TexturesMap[name] = tex;
}

//-------------------------------------------------------------------------------
bool CEffect::setUserTexture(CEffectUserParameter * param)
{
	uint8 stage = (uint8)param->getRegisterNb();

	nlassert(stage<IDRV_EFFECT_MAXTEXTURES);

	ItUserTexturesMap it = _TexturesMap.find(param->getName());
	if(it!=_TexturesMap.end())
	{
		_Textures[stage] = it->second;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------
ITexture * CEffect::getTexture(uint8 stage)
{
	nlassert(stage<IDRV_EFFECT_MAXTEXTURES);
	return _Textures[stage];
}

//-------------------------------------------------------------------------------
//------------------------------ CEffectDrvInfos --------------------------------
//-------------------------------------------------------------------------------

CEffectDrvInfos::CEffectDrvInfos(IDriver *drv, ItEffectDrvInfoPtrMap it) 
{
	_Driver = drv; 
	_DriverIterator = it;

	_VertexProgram = NULL;
	_PixelProgram = NULL;

	string errorOutput;
	if(!initPrograms(errorOutput))
	{
		nlwarning("Unable to parse the effect file '%s'.", _DriverIterator->first.c_str());
		#ifdef NL_DEBUG
			nlerror(errorOutput.c_str());
		#endif
	}
}

//-------------------------------------------------------------------------------
CEffectDrvInfos::~CEffectDrvInfos()
{
//	if(_Driver)
//		_Driver->removeEffectDrvInfoPtr(_DriverIterator);

	if(_VertexProgram) delete _VertexProgram;
	if(_PixelProgram) delete _PixelProgram;
}

//-------------------------------------------------------------------------------
bool CEffectDrvInfos::initPrograms(string & errorOutput)
{
	const string & filename = _DriverIterator->first;
	const string & pathFile = CPath::lookup(CFile::getFilenameWithoutExtension(filename)+".fx", false, false);

	// open our input file
	CIFile inf;
	if(!inf.open(pathFile))
	{
		errorOutput = "Unable to open file of name '" + filename + "'";
		return false;
	}

	// read the file contents into a string 's'
	CSString s;
	s.resize(inf.getFileSize());
	inf.serialBuffer((uint8*)&s[0],s.size());

	// close the file
	inf.close();


	CEffectParser parser;
	// search for vertex program string with "VertexProgramCode" tag
	CSString vertexProgramStr;
	if(!parser.getTagContent(s, "VertexProgramCode", vertexProgramStr))
	{
		errorOutput = parser.getErrorStr();
		return false;
	}
	_VertexProgram = new CVertexProgram(vertexProgramStr.c_str(), true);

	// search for pixel program string with "PixelProgramCode" tag
	CSString pixelProgramStr;
	if(!parser.getTagContent(s, "PixelProgramCode", pixelProgramStr))
	{
		errorOutput = parser.getErrorStr();
		return false;
	}
	_PixelProgram = new CPixelProgram(pixelProgramStr.c_str(), true);
	
	// search for program parameters
	if(!parseProgramParameters(s, CProgramTypeParameter::VertexProgram, errorOutput)) return false;
	if(!parseProgramParameters(s, CProgramTypeParameter::PixelProgram, errorOutput)) return false;
	
	return true;
}

//-------------------------------------------------------------------------------
bool CEffectDrvInfos::parseProgramParameters(CSString & s, CProgramTypeParameter::TProgramType progType, std::string & errorOutput)
{
	IProgram * program;
	TEffectUserParametersList * userParams;
	TEffectContextParametersList * contextParams;
	switch(progType)
	{
	case CProgramTypeParameter::VertexProgram:
	{
		program = _VertexProgram;
		userParams = &_VertexUserParams;
		contextParams = &_VertexContextParams;
		break;
	}
	case CProgramTypeParameter::PixelProgram:
	{
		program = _PixelProgram;
		userParams = &_PixelUserParams;
		contextParams = &_PixelContextParams;
		break;
	}
	}

	CEffectParser parser;
	TEffectParametersMap constantsTable;
	if(!program->convertInASM(constantsTable))
	{
		errorOutput = "Unable to convert HLSL program in ASM code";
		return false;
	}

	// search for program user parameters 
	if(!parser.getUserParametersList(s, progType, *userParams, constantsTable))
	{
		errorOutput = parser.getErrorStr();
		return false;
	}
	
	// search for program context parameters 
	if(!parser.getContextParametersList(s, progType, *contextParams, constantsTable))
	{
		errorOutput = parser.getErrorStr();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------
bool CEffectDrvInfos::bind(CEffect * effect)
{
	ItEffectUserParametersList userIt;
	ItEffectContextParametersList contextIt;

	for(userIt=_VertexUserParams.begin(); userIt!=_VertexUserParams.end(); userIt++)
	{
		CEffectUserParameter * userParam = &(*userIt);
		if(!userParam->isTexture())
		{
			if(!effect->setUserParameterValue(userParam, _Driver)) return false;
		}
		else
		{
			if(!effect->setUserTexture(userParam)) return false;
		}
	}

	for(userIt=_PixelUserParams.begin(); userIt!=_PixelUserParams.end(); userIt++)
	{
		CEffectUserParameter * userParam = &(*userIt);
		if(!userParam->isTexture())
		{
			if(!effect->setUserParameterValue(userParam, _Driver)) return false;
		}
		else
		{
			if(!effect->setUserTexture(userParam)) return false;
		}
	}

	for(contextIt=_VertexContextParams.begin(); contextIt!=_VertexContextParams.end(); contextIt++)
	{
		if(!CEffectContext::getInstance().setContextParameterValue(_Driver, &(*contextIt))) return false;
	}

	for(contextIt=_PixelContextParams.begin(); contextIt!=_PixelContextParams.end(); contextIt++)
	{
		if(!CEffectContext::getInstance().setContextParameterValue(_Driver, &(*contextIt))) return false;
	}

	return true;
}

//-------------------------------------------------------------------------------
//------------------------------- CEffectParser ---------------------------------
//-------------------------------------------------------------------------------

CEffectParser::CEffectParser()
{
}

//-------------------------------------------------------------------------------
CEffectParser::~CEffectParser()
{
}

//-------------------------------------------------------------------------------
bool CEffectParser::getTagContent(CSString str, char * tag, CSString & contentStr)
{
	string::size_type startPos, endPos;
	return getTagContent(str, tag, contentStr, startPos, endPos);
}

bool CEffectParser::getTagContent(CSString str, char * tag, CSString & contentStr, 
								  string::size_type & startPos, string::size_type & endPos)
{
	// search for tag
	bool findTag = false;
	while(!findTag)
	{
		string::size_type p = str.find(tag);
		startPos = p;
		string::size_type p2 = str.find("{", p); // to calculate 'endPos'

		if(p == string::npos) break;

		str = CSString(str.substr(p+string(tag).length())).strip();

		// check next word is '=', then '{'
		CSString & firstWord = str.firstWord();
		str = str.tailFromFirstWord().strip();
		CSString & secondWord = str.firstWord();
		if(firstWord=="=" && secondWord=="{")
		{
			// search for position of the matching '}' to obtain substring
			uint openCount = 1;
			uint closeCount = 0;
			string::size_type openPos, closePos, tempOpenPos, tempClosePos;
			openPos = 0;
			closePos = 0;
			while(openCount!=closeCount && closePos!=string::npos)
			{
				tempOpenPos = str.find("{", openPos+1);
				tempClosePos = str.find("}", closePos+1);

				if(tempOpenPos<tempClosePos)
				{
					openCount++;
					openPos=tempOpenPos;
				}
				else
				{
					closeCount++;
					closePos=tempClosePos;
				}
			}
			if(closePos != string::npos)
			{
				findTag = true;	
				contentStr = CSString(str.substr(1, closePos-1)).strip();
			}

			endPos = closePos + p2 - startPos;
		}
	}

	if(!findTag) _ErrorStr = string("Error, don't find tag '") + tag + "' with correct syntax 'tag = {...}'";
	return findTag;
}

//-------------------------------------------------------------------------------
bool CEffectParser::getUserParametersList(CSString & str, CProgramTypeParameter::TProgramType progType, 
										  TEffectUserParametersList & params, const TEffectParametersMap & paramsMap)
{
	char * tag;
	switch(progType)
	{
	case CProgramTypeParameter::VertexProgram:
		tag = "VertexProgramParameters";
		break;
	case CProgramTypeParameter::PixelProgram:
		tag = "PixelProgramParameters";
		break;
	}

	// search for list of User and Context parameters
	CSString userContextParams;
	if(!getTagContent(str, tag, userContextParams)) return false;

	char * tagTypeListe = "User";
	char * endNameTag = ";";

	// search for list of User parameters
	CSString paramsList;
	if(!getTagContent(userContextParams, tagTypeListe, paramsList)) return false;

	// split parameters list into lines
	CSString s(paramsList);
	NLMISC::CVectorSString lines;
	s.splitLines(lines);
	// process the lines one by one
	for (uint i=0;i<lines.size();++i)
	{
		// remove comments and encapsulating blanks
		CSString line= lines[i].splitTo("//").strip();
		if(line.empty()) continue;

		// split into words
		NLMISC::CVectorSString words;
		line.splitWords(words);

		CSString name;
		//check syntax : [<paramName>;] for a User parameter
		// check if the line contains exactly 2 words
		// check if the second word is à ';'
		if(words.size()==2 && words[1]==";")
		{
			name = words[0];
		}

		if(name!="")
		{
			ItEffectParametersMap itParam = paramsMap.find(name);
			if(itParam != paramsMap.end())
			{
				CEffectUserParameter userParam(itParam->second);
				userParam.setProgramType(progType);
				params.push_back(userParam);
			}
			else
			{
				nlwarning("'%s' parameter type and size impossible to find for the parameter '%s'", tagTypeListe, name.c_str());
				continue;
			}
		}
		else
		{
			_ErrorStr = string("'") + tagTypeListe + "' parameter impossible to extract from the line '" + line + "'";
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------
bool CEffectParser::getContextParametersList(CSString & str, CProgramTypeParameter::TProgramType progType, 
											 TEffectContextParametersList & params, const TEffectParametersMap & paramsMap)
{
	char * tag;
	switch(progType)
	{
	case CProgramTypeParameter::VertexProgram:
		tag = "VertexProgramParameters";
		break;
	case CProgramTypeParameter::PixelProgram:
		tag = "PixelProgramParameters";
		break;
	}

	// search for list of User and Context parameters
	CSString userContextParams;
	if(!getTagContent(str, tag, userContextParams)) return false;

	char * tagTypeListe = "Context";
	char * endNameTag = "=";

	// delete User list from global list to obtain only Context parameters list
	CSString paramsList;
	CSString userParams;
	string::size_type startPos, endPos;
	if(!getTagContent(userContextParams, "User", userParams, startPos, endPos)) return false;
	string firstPart, lastPart;
	firstPart = userContextParams.substr(0, startPos);
	lastPart = userContextParams.substr(endPos+1);
	paramsList = CSString(firstPart + lastPart).strip();

	// split parameters list into lines
	CSString s(paramsList);
	NLMISC::CVectorSString lines;
	s.splitLines(lines);
	// process the lines one by one
	for (uint i=0;i<lines.size();++i)
	{
		// remove comments and encapsulating blanks
		CSString line= lines[i].splitTo("//").strip();
		if(line.empty() || line.find(tagTypeListe)==string::npos) continue;

		// split into words
		NLMISC::CVectorSString words;
		line.splitWords(words);

		//check syntax : [<paramName> = Context.<contextType>] for a context parameter
		// check if the line contains exactly 6 words
		// check if the second word is a '='
		// check if the third word is 'Context'
		// check if the forth word is '.'
		// check if the last word is ';'
		CSString name, contextType;
		if((words.size()==6) && (words[1]=="=") && (words[2]==tagTypeListe) && (words[3]==".") && (words[5]==";"))
		{
			name = words[0];
			contextType = words[4];
		}
		
		if(name!="" && contextType!="")
		{
			ItEffectParametersMap itParam = paramsMap.find(name);
			if(itParam != paramsMap.end())
			{
				CEffectParameter param = itParam->second;
				CEffectContext::TContextParameter contextEnum;
				if(CEffectContext::getInstance().getContextParameter(&param, contextType, contextEnum))
				{
					if(param.getRegisterNb()>=0)
					{
						params.push_back(CEffectContextParameter(progType, param.getRegisterNb(), contextEnum));
					}
					else
					{
						nlwarning("context parameter '%s' isn't used in shader program", name.c_str());
					}
				}
				else
				{
					_ErrorStr = string("'") + tagTypeListe + "' context parameter enum impossible to extract from the line '" + line + "'";
					return false;
				}
			}
			else
			{
				nlwarning("'%s' parameter type and size impossible to find for the parameter '%s'", tagTypeListe, name.c_str());
				continue;
			}
		}
		else
		{
			_ErrorStr = string("'") + "' parameter impossible to extract from the line '" + line + "'";
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------
void CUserParameterDouble::setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
													 uint index, uint num, const double *src)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstant(index, num, src);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstant(index, num, src);
		break;
	}
}

//-------------------------------------------------------------------------------
void CUserParameterFloat::setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
												    uint index, uint num, const float *src)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstant(index, num, src);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstant(index, num, src);
		break;
	}
}

//-------------------------------------------------------------------------------
void CUserParameterInt::setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
											      uint index, uint num, const float *src)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstant(index, num, src);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstant(index, num, src);
		break;
	}
}

//-------------------------------------------------------------------------------
void CUserParameterBool::setDriverProgramConstant(IDriver * driver, CEffectUserParameter::TProgramType progType, 
											      uint index, uint num, const float *src)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstant(index, num, src);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstant(index, num, src);
		break;
	}
}

} // NL3D