/** \file nel_export.h
 * <File description>
 *
 * $Id: nel_export.h,v 1.12 2002/03/26 10:11:43 corvazier Exp $
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
#include "../nel_mesh_lib/export_nel.h"
#include "3d/mesh.h"

#include <vector>
#include <string>

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
	static	bool	exportMesh	(const char *sPath, INode& node, Interface& ip, TimeValue time, CExportNelOptions &opt, bool errorInDialog);
	static	bool	exportAnim	(const char *sPath, std::vector<INode*>& vectNode, Interface& ip, TimeValue time, bool scene);
	static	bool	exportSWT	(const char *sPath, std::vector<INode*>& vectNode, Interface& ip);
	//static	bool	exportScene	(std::vector<INode*>& vectNode);
	//static	bool	isMeshLM(INode& node);
	//static	bool	exportMeshLM(const char *sPath, INode& node, Interface& ip, TimeValue time);
	//static  bool	calculateLM(NL3D::CMesh::CMeshBuild *pZeMeshBuild, INode& ZeNode, Interface& ip, TimeValue tvTime, bool absolutePath);
	static	bool	exportInstanceGroup	(std::string filename, std::vector<INode*>& vectNode, Interface& ip );
	static	bool	exportSkeleton	(const char *sPath, INode* pNode, Interface& ip, TimeValue time);

	static	bool	exportCollision	(const char *sPath, std::vector<INode *> &nodes, Interface& ip, TimeValue time, CExportNelOptions &opt);

	static	bool	exportPACSPrimitives (const char *sPath, std::vector<INode *> &nodes, Interface& ip, TimeValue time);

	static	bool	exportVegetable (const char *sPath, INode& node, Interface& ip, TimeValue time, bool errorInDialog);

	static	void	viewMesh (Interface& ip, TimeValue time, CExportNelOptions &opt);

	static void initOptions(); // read the CNelExportSceneStruct from disk or init it
	static void deleteLM(INode& ZeNode); // the export scene struct MUST be initialized before calling this fn
	void			OnNodeProperties (const std::set<INode*> &listNode);

	ULONG ExtractFileName(char* Path, char* Name);
	ULONG ExtractPath(char* FullPath, char* Path);
	ULONG SelectFileForLoad(HWND Parent, char* Title, const char* Mask, char* FileName);
	ULONG SelectFileForSave(HWND Parent, char* Title, const char* Mask, char* FileName);
	ULONG SelectDir(HWND Parent, char* Title, char* Path);
	static ULONG FileExists(const char* FileName);
	ULONG GetFileSize(char* FileName);
	ULONG ProcessDir(char* Dir, const char* Mask, unsigned long flag, ULONG Fnct(char* FileName) );
	ULONG CleanFileName(char* FileName);
	ULONG CreateBAKFile(char* FileName);
};

class CNelExportClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() {return _T("NeL Export");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return CNELEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return _T("NeL Tools");}
	const TCHAR*	InternalName() { return _T("NeL export and view"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

extern CNelExportClassDesc CNelExportDesc;

extern CNelExport theCNelExport;


#endif // __NEL_EXPORT__H
