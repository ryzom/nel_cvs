#ifndef WATER_HEIGHT_MAP_H
#define WATER_HEIGHT_MAP_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"




namespace NL3D
{

/**
  * This class is a portion of water, it encodes its height, and simulates its propagation.
  * It contains datas about the eight around the user position
  *
  *
  *			size x 2
  *     |--------|
  *	   s| (x,y)  |
  *	   i|   |---||
  *	   z|   |   ||
  *	   e|   |---||
  *	   x|        |
  *    2|--------|
  */

class CWaterHeightMap
{
public:
	// ctor
	CWaterHeightMap();
	/** Set this quad dimension. It is given as a power of 2
	  * This also reset the eightField values
	  */
	void					setSize(uint size);

	/// return the size of the height map
	uint					getSize(void) const { return _Size; }

	/// return the power of 2 used for this quad size

	/** Set the userPos (relative to the height map coordinates). This is needed because a height map can't be used with large surface (such as a sea).
	  * As a consequence, the height map is only valid below the user (e.g from user.x - 0.5 * size to user.x + 0.5 *size).
	  * When setPos is called, and if a move has occured, new area of the height field are set to 0
	  */
	void					setUserPos(sint x, sint y);


	/// retrieve the use pos
	void					getUserPos(sint &x, sint &y) const;

	/** Perform water propagation on this quad.
	  * You should call swapBuffers after this, or after calling filter.
	  * \param damping The attenuation factor used for propagation.
	  */
	void					propagate();

	/// create a perturbation in the height map.
	void					perturbate(sint x, sint y, sint radius, float intensity);


	/// create a point perturbation in the height map.
	void					perturbatePoint(sint x, sint y, float intensity);

	/// apply a filter on the height field
	void					filter();

	/// apply a filter on the height field, and also compute the gradient at the same time
	void					filterNStoreGradient();

	/// get a pointer on the current buffer.
	float					*getPointer(void) { return &(_Map[_CurrMap][0]); }

	/// get a pointer on the gradient buffer
	NLMISC::CVector2f		*getGradPointer(void) { return &_Grad[0]; }

	/// swap the height maps. It must be called once propagation and filtering have been performed
	void					swapBuffers(void);

	/// damping
	void  setDamping(float damping) { _Damping = damping; }
	float getDamping() const { return _Damping; }

	/// filter weight
	void  setFilterWeight(float filterWeight) { _FilterWeight = filterWeight; }
	float getFilterWeight() const { return _FilterWeight; }

	/// water unit size
	void  setUnitSize(float unitSize) { _UnitSize = unitSize; }
	float getUnitSize() const { return _UnitSize; }

	


	sint64					Date;
private:
	float					   _Damping;
	float					   _FilterWeight;
	float					   _UnitSize;

	uint					   _X, _Y;	
	typedef std::vector<float>				TFloatVect;
	typedef std::vector<NLMISC::CVector2f > TFloat2Vect;

	TFloatVect				   _Map[2]; // the 2 maps used for propagation
	TFloat2Vect				   _Grad;   // used to store the gradient 
	uint8					   _CurrMap;	
	uint					   _Size;	

	/// clear an area of the water height map (has clipping, but no wrapping)	
	void						clearArea(uint8 currMap, sint x, sint y, sint width, sint height);
	// same than clearArea, but perform on both maps
	void						clearZone(sint x, sint y, sint width, sint height);
	/// displace the height map, when a boundary has been reached
	void						makeCpy(uint buffer, uint dX, uint dY, uint sX, uint sY, uint width, uint height);

	/// clear the borders
	void						clearBorder(uint currMap);


};

} // NL3D

#endif