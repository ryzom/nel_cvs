/** \file nel_export.h
 * <File description>
 *
 * $Id: nel_export.h,v 1.2 2001/04/30 17:01:00 corvazier Exp $
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

#ifndef __NEL_EXPORT__H
#define __NEL_EXPORT__H

#include "resource.h"
/*#include "nel/3d/zone.h"
#include "../nel_patch_lib/rpo.h"*/

#include <vector>

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define CNELEXPORT_CLASS_ID	Class_ID(0x8c02158, 0x5a9e252b)

namespace NL3D
{
	class CSkeletonShape;
}

class CNelExport : public UtilityObj 
{
private:

public:
	HWND			hPanel;
	IUtil			*iu;
	Interface		*ip;
	
	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void DeleteThis() { }		
	CNelExport();
	~CNelExport();		

	HWND			hBar;

	TCHAR* fixupName(TCHAR* name);
	bool nodeEnum(INode* pNode, bool selected);
	bool doExport(bool selected=false);
	bool freeExported(void);
	void getSelectedNode (std::vector<INode*>& vectNode);

	static	bool	exportZone	(const char *sName, INode& node, TimeValue time);
	static	bool	exportMesh	(const char *sPath, INode& node, Interface& ip, TimeValue time);
	static	bool	exportAnim	(const char *sPath, std::vector<INode*>& vectNode, Interface& ip, TimeValue time, bool scene);
	static	bool	exportSWT	(const char *sPath, std::vector<INode*>& vectNode, Interface& ip);
	static	bool	exportScene	(const char *sPath, std::vector<INode*>& vectNode, Interface& ip);
	static	bool	exportSkeleton	(const char *sPath, INode* pNode, Interface& ip, TimeValue time);

	static	void	viewMesh (Interface& ip, TimeValue time);

	ULONG ExtractFileName(char* Path, char* Name);
	ULONG ExtractPath(char* FullPath, char* Path);
	ULONG SelectFileForLoad(HWND Parent, char* Title, const char* Mask, char* FileName);
	ULONG SelectFileForSave(HWND Parent, char* Title, const char* Mask, char* FileName);
	ULONG SelectDir(HWND Parent, char* Title, char* Path);
	ULONG FileExists(char* FileName);
	ULONG GetFileSize(char* FileName);
	ULONG ProcessDir(char* Dir, const char* Mask, unsigned long flag, ULONG Fnct(char* FileName) );
	ULONG CleanFileName(char* FileName);
	ULONG CreateBAKFile(char* FileName);
};

extern CNelExport theCNelExport;


#endif // __NEL_EXPORT__H
