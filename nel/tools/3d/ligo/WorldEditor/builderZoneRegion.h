// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONEREGION_H
#define BUILDERZONEREGION_H

// ***************************************************************************

#define STRING_UNUSED		"< UNUSED >"
#define STRING_OUT_OF_BOUND "< OOB >"

// ***************************************************************************
namespace NLLIGO
{
	class CZoneBank;
	class CZoneBankElement;
}

namespace NLMISC
{
	class IStream;
}

class CBuilderZone;

// ***************************************************************************

// CZoneRegion contains informations about the zones painted
class CBuilderZoneRegion
{

public:

	struct SPiece
	{
		sint32				w, h;			// Max 255x255
		std::vector<uint8>	Tab;
	};

private:

	// An element of the grid
	struct SZoneUnit
	{
		std::string			ZoneName;
		uint8				PosX, PosY; // Position in a large piece
		uint8				Rot, Flip; // Rot 0-0°, 1-90°, 2-180°, 3-270°, Flip 0-false, 1-true

		// Work Data : For transition				[2 3]
		std::string			SharingMatNames[4];	//  [0 1]
		uint8				SharingCutEdges[4]; // 0-Up, 1-Down, 2-Left, 3-Right (value [0-2])

		SZoneUnit();
		void			serial (NLMISC::IStream &f);
		const SZoneUnit&operator=(const SZoneUnit&zu);
	};

	// An element of the graph
	struct SMatNode
	{
		std::string			Name;
		std::vector<uint32>	Arcs; // Position in the tree (vector of nodes)
	};

private:

	std::vector<SZoneUnit>		_Zones;
	sint32						_MinX, _MinY;
	sint32						_MaxX, _MaxY;

	bool						_MustAskSave;

	NLLIGO::CZoneBank			*_ZeBank;
	CBuilderZone				*_Builder; // To use the global mask

	static std::string			_StringOutOfBound;

	std::vector<SMatNode>		_MatTree; // The tree of transition between materials

private:

	void				resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);

	void				addToUpdateAndCreate (CBuilderZoneRegion* pBZRfrom, sint32 sharePos, sint32 x, sint32 y, const std::string &sNewMat, void *pInt1, void *pInt2);

	void				putTransitions (sint32 x, sint32 y, const SPiece &rMask, const std::string &MatName, void *pInternal);
	void				putTransition (sint32 x, sint32 y, const std::string &MatName);
	void				updateTrans (sint32 x, sint32 y);

	std::string			getNextMatInTree(const std::string &sMatA, const std::string &sMatB);
	void				tryPath (uint32 posA, uint32 posB, std::vector<uint32> &vPath);

	SZoneUnit			*getZoneAmongRegions (CBuilderZoneRegion*& pBZRfromAndRet, sint32 x, sint32 y);

	void				set (sint32 x, sint32 y, sint32 nPosX, sint32 nPosY, const std::string &ZoneName, bool transition=false);
	void				setRot (sint32 x, sint32 y, uint8 rot);
	void				setFlip (sint32 x, sint32 y, uint8 flip);

public:

	CBuilderZoneRegion ();

	// Tools
	void				rotFlip (SPiece &sMask, uint8 rot, uint8 flip);

	// New interface
	void				init (NLLIGO::CZoneBank *pBank, CBuilderZone *pBuilder);
	void				add (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt);
	void				del (sint32 x, sint32 y, bool transition=false, void *pInternal=NULL);
	void				serial (NLMISC::IStream &f);
	void				move (sint32 x, sint32 y);
	void				setStart (sint32 x, sint32 y);
	void				reduceMin ();

	// Accessors
	const std::string	&getName (sint32 x, sint32 y);
	uint8				getPosX (sint32 x, sint32 y);
	uint8				getPosY (sint32 x, sint32 y);
	uint8				getRot (sint32 x, sint32 y);
	uint8				getFlip (sint32 x, sint32 y);

	sint32				getMinX () { return _MinX; }
	sint32				getMaxX () { return _MaxX; }
	sint32				getMinY () { return _MinY; }
	sint32				getMaxY () { return _MaxY; }
	bool				getMustAskSave () { return _MustAskSave; }
};

#endif // BUILDERZONEREGION_H