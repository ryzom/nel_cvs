// lightmap_optimizer
// ------------------
// the goal is to regroup lightmap of a level into lightmap with a higher level

#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/log.h"
#include "nel/misc/path.h"

#include "3d/mesh_base.h"
#include "3d/mesh.h"
#include "3d/mesh_mrm.h"
#include "3d/mesh_multi_lod.h"
#include "3d/vertex_buffer.h"
#include "3d/texture_file.h"
#include "3d/register_3d.h"

#include "windows.h"

#include <vector>
#include <string>

// ---------------------------------------------------------------------------

using namespace std;
using namespace NL3D;

// ---------------------------------------------------------------------------
char sExeDir[MAX_PATH];

void outString (const string &sText)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (sExeDir);
	NLMISC::createDebug ();
	NLMISC::InfoLog->displayRaw(sText.c_str());
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void dir (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	sAllFiles.clear ();
	GetCurrentDirectory (MAX_PATH, sCurDir);
	hFind = FindFirstFile (sFilter.c_str(), &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY))
		{
			if (bFullPath)
				sAllFiles.push_back(string(sCurDir) + "\\" + findData.cFileName);
			else
				sAllFiles.push_back(findData.cFileName);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
}

// ---------------------------------------------------------------------------
bool fileExist (const std::string &sFileName)
{
	HANDLE hFile = CreateFile (sFileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	CloseHandle (hFile);
	return true;
}

// -----------------------------------------------------------------------------------------------
// Try all position to put pSrc in pDst
bool tryAllPos (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 &x, sint32 &y)
{
	uint32 i, j;
	vector<uint8> &rSrcPix = pSrc->getPixels();
	vector<uint8> &rDstPix = pDst->getPixels();

	// Recalculate real size of the source (without padding to power of 2)
	uint32 nSrcWidth = 0, nSrcHeight = 0;
	for (j = 0; j < pSrc->getHeight(); ++j)
	for (i = 0; i < pSrc->getWidth(); ++i)
	{
		if (rSrcPix[4*(i+j*pSrc->getWidth())+3] != 0)
		{
			if ((i+1) > nSrcWidth)
				nSrcWidth = i+1;
			if ((j+1) > nSrcHeight)
				nSrcHeight= j+1;
		}
	}

	if (nSrcWidth > pDst->getWidth() ) return false;
	if (nSrcHeight > pDst->getHeight() ) return false;

	// For all position test if the Src plane can be put in
	for (j = 0; j < (pDst->getHeight() - nSrcHeight); ++j)
	for (i = 0; i < (pDst->getWidth() - nSrcWidth); ++i)
	{
		x = i; y = j;
		
		uint32 a, b;
		bool bCanPut = true;
		for (b = 0; b < nSrcHeight; ++b)
		{
			for (a = 0; a < nSrcWidth; ++a)
			{
				if (rSrcPix[4*(a+b*pSrc->getWidth())+3] != 0)
				{
					if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0 )
					{
						bCanPut = false;
						break;
					}
				}
			}
			if (bCanPut == false)
				break;
		}
		if (bCanPut)
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
bool putIn (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 x, sint32 y)
{
	uint32 a, b;

	vector<uint8> &rSrcPix = pSrc->getPixels();
	vector<uint8> &rDstPix = pDst->getPixels();

	for (b = 0; b < pSrc->getHeight(); ++b)
	for (a = 0; a < pSrc->getWidth(); ++a)
		if (rSrcPix[4*(a+b*pSrc->getWidth())+3] != 0)
		{
			if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
				return false;
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+0] = rSrcPix[4*(a+b*pSrc->getWidth())+0];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+1] = rSrcPix[4*(a+b*pSrc->getWidth())+1];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+2] = rSrcPix[4*(a+b*pSrc->getWidth())+2];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] = rSrcPix[4*(a+b*pSrc->getWidth())+3];
		}
	return true;
}

// ---------------------------------------------------------------------------
string getBaseName (const string &fullname)
{
	string sTmp2 = "";
	int pos = fullname.rfind('_');
	for (int j = 0; j <= pos; ++j)
		sTmp2 += fullname[j];
	return sTmp2;
}

// ---------------------------------------------------------------------------
uint8 getLayerNb (const string &fullname)
{
	string sTmp2;
	uint8 nRet = 0;
	int beg = fullname.rfind('_');
	int end = fullname.rfind('.');
	for (int j = beg+1; j < end; ++j)
		sTmp2 += fullname[j];

	return atoi(sTmp2.c_str());
}

// Flag all vertices linked to face with material m
// ---------------------------------------------------------------------------
void FlagVertices (CMeshGeom &mg, uint InMatID, vector<bool> &verticesNeedRemap)
{
	CVertexBuffer &vertexBuffer = const_cast<CVertexBuffer&>(mg.getVertexBuffer());

	// For each matrix block
	uint matrixBlock;
	uint nbMatrixBlock=mg.getNbMatrixBlock();
	for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
	{
		// For each render pass
		uint renderPass;
		uint numRenderPass=mg.getNbRdrPass(matrixBlock);
		for (renderPass=0; renderPass<numRenderPass; renderPass++)
		{
			// Render pass material
			uint32 matId=mg.getRdrPassMaterial(matrixBlock, renderPass);

			if (matId == InMatID) // Same Material -> Flag all vertices of this pass
			{
				// Get primitives
				const CPrimitiveBlock &primitiveBlock=mg.getRdrPassPrimitiveBlock(matrixBlock,renderPass);

				// Set of vertex to remap
				std::set<uint> vertexToRemap;

				// Remap triangles
				uint index;
				const uint32 *indexPtr=primitiveBlock.getTriPointer();
				uint32 numIndex=3*primitiveBlock.getNumTri();
				for (index=0; index<numIndex; index++)
					vertexToRemap.insert (indexPtr[index]);

				// Remap quad
				indexPtr=primitiveBlock.getQuadPointer();
				numIndex=4*primitiveBlock.getNumQuad();
				for (index=0; index<numIndex; index++)
					vertexToRemap.insert (indexPtr[index]);

				// Remap the vertex
				std::set<uint>::iterator iteRemap=vertexToRemap.begin();
				while (iteRemap!=vertexToRemap.end())
				{
					// Remap the vertex
					verticesNeedRemap[*iteRemap] = true;

					// Next vertex
					iteRemap++;
				}
			}
		}
	}
}
void FlagVerticesMRM (CMeshMRMGeom &mg, uint InMatID, vector<bool> &verticesNeedRemap)
{
	CVertexBuffer &vertexBuffer = const_cast<CVertexBuffer&>(mg.getVertexBuffer());

	// For each matrix block
	uint matrixBlock;
	uint nbMatrixBlock=1;//mg.getNbMatrixBlock(); // ASK YOYO
	for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
	{
		// For each render pass
		uint renderPass;
		uint numRenderPass=mg.getNbRdrPass(matrixBlock);
		for (renderPass=0; renderPass<numRenderPass; renderPass++)
		{
			// Render pass material
			uint32 matId=mg.getRdrPassMaterial(matrixBlock, renderPass);

			if (matId == InMatID) // Same Material -> Flag all vertices of this pass
			{
				// Get primitives
				const CPrimitiveBlock &primitiveBlock=mg.getRdrPassPrimitiveBlock(matrixBlock,renderPass);

				// Set of vertex to remap
				std::set<uint> vertexToRemap;

				// Remap triangles
				uint index;
				const uint32 *indexPtr=primitiveBlock.getTriPointer();
				uint32 numIndex=3*primitiveBlock.getNumTri();
				for (index=0; index<numIndex; index++)
					vertexToRemap.insert (indexPtr[index]);

				// Remap quad
				indexPtr=primitiveBlock.getQuadPointer();
				numIndex=4*primitiveBlock.getNumQuad();
				for (index=0; index<numIndex; index++)
					vertexToRemap.insert (indexPtr[index]);

				// Remap the vertex
				std::set<uint>::iterator iteRemap=vertexToRemap.begin();
				while (iteRemap!=vertexToRemap.end())
				{
					// Remap the vertex
					verticesNeedRemap[*iteRemap] = true;

					// Next vertex
					iteRemap++;
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int nNbArg, char **ppArgs)
{
	
	if (nNbArg != 3)
	{
		outString ("ERROR : Wrong number of arguments\n");
		outString ("USAGE : lightmap_optimizer <path_lightmaps> <path_shapes>\n");
		return -1;
	}
	
	vector<string> AllShapeNames;
	vector<CMeshBase*> AllShapes;
	char sLMPDir[MAX_PATH];
	char sSHPDir[MAX_PATH];

	GetCurrentDirectory (MAX_PATH, sExeDir);
	// Get absolute directory for lightmaps
	if (!SetCurrentDirectory(ppArgs[1]))
	{
		outString (string("ERROR : directory ") + ppArgs[1] + " do not exists\n");
		return -1;
	}
	GetCurrentDirectory (MAX_PATH, sLMPDir);
	SetCurrentDirectory (sExeDir);
	// Get absolute directory for shapes
	if (!SetCurrentDirectory(ppArgs[2]))
	{
		outString (string("ERROR : directory ") + ppArgs[2] + " do not exists\n");
		return -1;
	}
	GetCurrentDirectory (MAX_PATH, sSHPDir);
	dir ("*.shape", AllShapeNames, false);
	registerSerial3d ();
	for (uint32 nShp = 0; nShp < AllShapeNames.size(); ++nShp)
	{
		try
		{
			CShapeStream mesh;
			NLMISC::CIFile meshfile (AllShapeNames[nShp]);
			meshfile.serial( mesh );
			meshfile.close();

			// Add the shape to the map.
			CMeshBase *pMB = dynamic_cast<CMeshBase*>(mesh.getShapePointer());
			if (pMB != NULL)
				AllShapes.push_back (pMB);
		}
		catch (NLMISC::EPathNotFound &e)
		{
			outString(string("ERROR: shape not found ")+AllShapeNames[nShp]+" - "+e.what());
			return -1;
		}
	}

	SetCurrentDirectory (sExeDir);

	for (uint32 nNbLayer = 0; nNbLayer < 256; ++nNbLayer)
	{
		// Get all lightmaps with same number of layer == nNbLayer

		vector<string> AllLightmapNames;
		vector<NLMISC::CBitmap*> AllLightmaps;
		sint32 i, j, k, m, n;
		string sFilter;

		sFilter = "*_" + NLMISC::toString(nNbLayer) + ".tga";
		SetCurrentDirectory (sLMPDir);
		dir (sFilter, AllLightmapNames, false);
		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			string sTmp2 = getBaseName (AllLightmapNames[i]);
			sTmp2 += NLMISC::toString(nNbLayer+1) + ".tga";
			if (fileExist(sTmp2))
			{	// More layer than expected delete name i
				for (j = i+1; j < (sint32)AllLightmapNames.size(); ++j)
					AllLightmapNames[j-1] = AllLightmapNames[j];
				AllLightmapNames.resize (AllLightmapNames.size()-1);
				i = -1;
			}
		}

		// Check if all layer of the same lightmap has the same size
		if (nNbLayer > 0)
		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			string sTmp2;
			sTmp2 = getBaseName (AllLightmapNames[i]) + "0.tga";
			uint32 wRef, hRef;
			try
			{
				NLMISC::CIFile inFile;
				inFile.open(sTmp2);
				CBitmap::loadSize(inFile, wRef, hRef);
			}
			catch (NLMISC::Exception &e)
			{
				outString (string("ERROR :") + e.what());
				return -1;
			}

			bool bFound = false;
			for (k = 1; k <= (sint32)nNbLayer; ++k)
			{
				string sTmp3 = getBaseName (AllLightmapNames[i]) + NLMISC::toString(k) + ".tga";
				uint32 wCur = wRef, hCur = hRef;
				try
				{
					NLMISC::CIFile inFile;
					inFile.open(sTmp3);
					CBitmap::loadSize(inFile, wCur, hCur);
				}
				catch (NLMISC::Exception &)
				{
				}

				if ((wCur != wRef) || (hCur != hRef))
				{
					bFound = true;
					break;
				}
			}
			// Should delete all layers of this lightmap (in fact in lightmapnames list we have
			// only the name of the current layer)
			if (bFound)
			{
				sTmp2 = getBaseName (AllLightmapNames[i]);
				outString(string("ERROR: lightmaps ")+sTmp2+"*.tga not all the same size\n");
				for (k = 0; k < (sint32)AllLightmapNames.size(); ++k)
				{
					if (strnicmp(AllLightmapNames[k].c_str(), sTmp2.c_str(), sTmp2.size()) == 0)
					{
						for (j = k+1; j < (sint32)AllLightmapNames.size(); ++j)
							AllLightmapNames[j-1] = AllLightmapNames[j];
						AllLightmapNames.resize (AllLightmapNames.size()-1);
						k = -1;
						i = -1;
					}
				}
			}
		}
		
		if (AllLightmapNames.size() == 0)
			continue;
		
		// Load all the lightmaps
		AllLightmaps.resize (AllLightmapNames.size());
		for (i = 0; i < (sint32)AllLightmaps.size(); ++i)
		{
			try
			{
				NLMISC::CBitmap *pBtmp = new NLMISC::CBitmap;
				NLMISC::CIFile inFile;
				inFile.open(AllLightmapNames[i]);
				pBtmp->load(inFile);
				AllLightmaps[i] = pBtmp;
			}
			catch (NLMISC::Exception &e)
			{
				outString (string("ERROR :") + e.what());
				return -1;
			}
		}

		// Sort all lightmaps by decreasing size
		for (i = 0; i < (sint32)(AllLightmaps.size()-1); ++i)
		for (j = i+1; j < (sint32)AllLightmaps.size(); ++j)
		{
			NLMISC::CBitmap *pBI = AllLightmaps[i];
			NLMISC::CBitmap *pBJ = AllLightmaps[j];
			if ((pBI->getWidth()*pBI->getHeight()) < (pBJ->getWidth()*pBJ->getHeight()))
			{
				NLMISC::CBitmap *pBTmp = AllLightmaps[i];
				AllLightmaps[i] = AllLightmaps[j];
				AllLightmaps[j] = pBTmp;

				string sTmp = AllLightmapNames[i];
				AllLightmapNames[i] = AllLightmapNames[j];
				AllLightmapNames[j] = sTmp;
			}
		}

		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			bool bAssigned = false;
			for (j = 0; j < i; ++j)
			{
				// Try to place the texture i into the texture j
				NLMISC::CBitmap *pBI = AllLightmaps[i];
				NLMISC::CBitmap *pBJ = AllLightmaps[j];
				sint32 x, y;
				if (tryAllPos (pBI, pBJ, x, y))
				{
					bAssigned = true;

					if (!putIn (pBI, pBJ, x, y))
					{
						outString (string("ERROR : cannot put reference lightmap ")+AllLightmapNames[i]+
									" in "+AllLightmapNames[j]);
						return -1;
					}
					// Put texture i into texture j for all layers of the lightmap !
					for (k = 0; k <= (sint32)nNbLayer; ++k)
					{
						string sTexNameI = getBaseName (AllLightmapNames[i]) + NLMISC::toString(k) + ".tga";
						string sTexNameJ = getBaseName (AllLightmapNames[j]) + NLMISC::toString(k) + ".tga";
						NLMISC::CBitmap BitmapI;
						NLMISC::CBitmap BitmapJ;
						NLMISC::CIFile inFile;

						outString (string("INFO : Transfering ")+sTexNameI+" in "+sTexNameJ+
									" at ("+NLMISC::toString(x)+","+NLMISC::toString(y)+")\n");

						try
						{
							inFile.open (sTexNameI);
							BitmapI.load (inFile);
							inFile.close ();
							inFile.open (sTexNameJ);
							BitmapJ.load (inFile);
							inFile.close ();
						}
						catch (NLMISC::Exception &e)
						{
							outString (string("ERROR :") + e.what());
							return -1;
						}
						
						if (!putIn (&BitmapI, &BitmapJ, x, y))
						{
							outString (string("ERROR : cannot put lightmap ")+sTexNameI+" in "+sTexNameJ+"\n");
							return -1;
						}

						// Delete File
						DeleteFile (sTexNameI.c_str());
						outString (string("INFO : Deleting file ")+sTexNameI+"\n");

						// Save destination image
						NLMISC::COFile outFile;
						outFile.open (sTexNameJ);
						BitmapJ.writeTGA (outFile, 32);
						outString (string("INFO : Saving file ")+sTexNameJ+"\n");
					}

					// Change shapes uvs related and names to the lightmap
					// ---------------------------------------------------

					SetCurrentDirectory (sSHPDir);

					for (k = 0; k < (sint32)AllShapes.size(); ++k)
					{
						CMeshBase *pMB = AllShapes[k];
						uint nNbMat = pMB->getNbMaterial ();
						vector< vector<bool> > VerticesNeedRemap;
						bool bMustSave = false;
						// Initialize all VerticesNeedRemap
						CMesh *pMesh = dynamic_cast<CMesh*>(pMB);
						CMeshMRM *pMeshMRM = dynamic_cast<CMeshMRM*>(pMB);
						CMeshMultiLod *pMeshML = dynamic_cast<CMeshMultiLod*>(pMB);

						if (pMesh != NULL)
						{
							VerticesNeedRemap.resize(1); // Only one meshgeom
							vector<bool> &rVNR = VerticesNeedRemap[0];
							rVNR.resize (pMesh->getMeshGeom().getVertexBuffer().getNumVertices(), false);
						}
						else if (pMeshMRM != NULL)
						{
							VerticesNeedRemap.resize(1); // Only one meshmrmgeom
							vector<bool> &rVNR = VerticesNeedRemap[0];
							rVNR.resize (pMeshMRM->getMeshGeom().getVertexBuffer().getNumVertices(), false);
						}
						else if (pMeshML != NULL)
						{
							sint32 nNumSlot = pMeshML->getNumSlotMesh();
							VerticesNeedRemap.resize(nNumSlot);
							for (m = 0; m < nNumSlot; ++m)
							{
								vector<bool> &rVNR = VerticesNeedRemap[m];
								const CMeshGeom *pMG = dynamic_cast<const CMeshGeom*>(&pMeshML->getMeshGeom(m));
								if (pMG != NULL)
									rVNR.resize (pMG->getVertexBuffer().getNumVertices(), false);
								else
									rVNR.resize(0);
							}
						}
						else continue; // Next mesh
						

						// All materials must have the lightmap names changed
						for (m = 0; m < (sint32)nNbMat; ++m)
						{
							bool bMustRemapUV = false;
							CMaterial& rMat = const_cast<CMaterial&>(pMB->getMaterial (m));
							if (rMat.getShader() == CMaterial::LightMap)
							{
								// Begin with stage 0
								uint8 stage = 0;
								while (rMat.getLightMap(stage) != NULL)
								{
									ITexture *pIT = rMat.getLightMap (stage);
									CTextureFile *pTF = dynamic_cast<CTextureFile*>(pIT);
									if (pTF != NULL)
									{
										string sTexName = NLMISC::strlwr(getBaseName(pTF->getFileName()));
										string sTexNameMoved = NLMISC::strlwr(getBaseName(AllLightmapNames[i]));
										if (sTexName == sTexNameMoved)
										{
											// We must remap the name and indicate to remap uvs
											bMustRemapUV = true;
											//string sNewTexName = NLMISC::strlwr(getBaseName(AllLightmapNames[j]));
											//sNewTexName += NLMISC::toString(getLayerNb(pTF->getFileName())) + ".tga";
											//pTF->setFileName (sNewTexName);
										}
									}
									++stage;
								}
							}
							// We have to remap the uvs of this mesh for this material
							if (bMustRemapUV) // Flaggage of the vertices to remap
							{
								if (pMesh != NULL)
								{
									// Flag all vertices linked to face with material m
									FlagVertices (const_cast<CMeshGeom&>(pMesh->getMeshGeom()), m, VerticesNeedRemap[0]);
								}
								else if (pMeshMRM != NULL)
								{
									FlagVerticesMRM (const_cast<CMeshMRMGeom&>(pMeshMRM->getMeshGeom()), m, VerticesNeedRemap[0]);
								}
								else if (pMeshML != NULL)
								{
									sint32 nNumSlot = pMeshML->getNumSlotMesh();
									for (n = 0; n < nNumSlot; ++n)
									{
										// Get the mesh geom
										CMeshGeom *pMG = const_cast<CMeshGeom*>(dynamic_cast<const CMeshGeom*>(&pMeshML->getMeshGeom(n)));
										if (pMG)
										{
											// Flag the vertices
											FlagVertices (*pMG, m, VerticesNeedRemap[n]);
										}
										else
										{
											// Get the mesh MRM geom
											CMeshMRMGeom *pMMRMG = const_cast<CMeshMRMGeom*>(dynamic_cast<const CMeshMRMGeom*>(&pMeshML->getMeshGeom(n)));
											if (pMMRMG)
											{
												// Flag the vertices
												FlagVerticesMRM (*pMMRMG, m, VerticesNeedRemap[n]);
											}
										}
									}
								}
							}
						}

						// Change lightmap names
						for (m = 0; m < (sint32)nNbMat; ++m)
						{
							CMaterial& rMat = const_cast<CMaterial&>(pMB->getMaterial (m));
							if (rMat.getShader() == CMaterial::LightMap)
							{
								// Begin with stage 0
								uint8 stage = 0;
								while (rMat.getLightMap(stage) != NULL)
								{
									ITexture *pIT = rMat.getLightMap (stage);
									CTextureFile *pTF = dynamic_cast<CTextureFile*>(pIT);
									if (pTF != NULL)
									{
										string sTexName = NLMISC::strlwr(getBaseName(pTF->getFileName()));
										string sTexNameMoved = NLMISC::strlwr(getBaseName(AllLightmapNames[i]));
										if (sTexName == sTexNameMoved)
										{
											string sNewTexName = NLMISC::strlwr(getBaseName(AllLightmapNames[j]));
											sNewTexName += NLMISC::toString(getLayerNb(pTF->getFileName())) + ".tga";
											pTF->setFileName (sNewTexName);
										}
									}
									++stage;
								}
							}
						}

						// We have now the list of vertices to remap for all material that have been changed
						// So parse this list and apply the transformation : (uv * TexSizeI + decalXY) / TexSizeJ
						for (m = 0; m < (sint32)VerticesNeedRemap.size(); ++m)
						{
							CVertexBuffer *pVB;							
							if (pMesh != NULL)
							{
								pVB = const_cast<CVertexBuffer*>(&pMesh->getMeshGeom().getVertexBuffer());
							}
							else if (pMeshMRM != NULL)
							{
								pVB = const_cast<CVertexBuffer*>(&pMeshMRM->getMeshGeom().getVertexBuffer());
							}
							else if (pMeshML != NULL)
							{
								const CMeshGeom *pMG = dynamic_cast<const CMeshGeom*>(&pMeshML->getMeshGeom(m));
								pVB = const_cast<CVertexBuffer*>(&pMG->getVertexBuffer());
							}

							vector<bool> &rVNR = VerticesNeedRemap[m];
							for (n = 0; n < (sint32)rVNR.size(); ++n)
							if (rVNR[n])
							{
								CUV *pUV = (CUV*)pVB->getTexCoordPointer (n,1);
								pUV->U = (pUV->U*pBI->getWidth() + x) / pBJ->getWidth();
								pUV->V = (pUV->V*pBI->getHeight() + y) / pBJ->getHeight();
								bMustSave = true;
							}
						}

						if (bMustSave)
						{
							try
							{
								CShapeStream mesh;
								mesh.setShapePointer (AllShapes[k]);
								NLMISC::COFile meshfile (AllShapeNames[k]);
								meshfile.serial (mesh);
								meshfile.close ();
							}
							catch (NLMISC::EPathNotFound &e)
							{
								outString(string("ERROR: cannot save shape ")+AllShapeNames[k]+" - "+e.what());
								return -1;
							}
						}
					}

					SetCurrentDirectory (sLMPDir);

					// Get out of the j loop
					break;
				}
			}		
			// if assigned to another bitmap -> delete the bitmap i
			if (bAssigned)
			{
				// Delete Names
				for (j = i+1; j < (sint32)AllLightmapNames.size(); ++j)
					AllLightmapNames[j-1] = AllLightmapNames[j];
				AllLightmapNames.resize (AllLightmapNames.size()-1);
				// Delete Lightmaps
				delete AllLightmaps[i];
				for (j = i+1; j < (sint32)AllLightmaps.size(); ++j)
					AllLightmaps[j-1] = AllLightmaps[j];
				AllLightmaps.resize (AllLightmaps.size()-1);
				i = i - 1;
			}
		}

	}
	

	

	return 0;
}
