// Counter.h: interface for the CCounter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COUNTER_H__0C20D48D_E90E_4B32_B53C_2C91974411DB__INCLUDED_)
#define AFX_COUNTER_H__0C20D48D_E90E_4B32_B53C_2C91974411DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
* Classe CCounter : represent counters in .LOGIC files
*/
class CCounter  
{
public:
	CCounter( const CString &name = "");

	virtual ~CCounter();

	inline const CString &name() const { return m_sName; }
	inline const CString &way() const { return m_sWay; }
	inline const CString &mode() const { return m_sMode; }
	inline long upperLimit() const { return m_nUpperLimit; }
	inline long lowerLimit() const { return m_nLowerLimit; }

	inline void name( const CString &name) { m_sName = name; }
	inline void way( const CString &way) { m_sWay = way; }
	inline void mode( const CString &mode) { m_sMode = mode; }
	inline void lowerLimit( long min) { m_nLowerLimit = min; }
	inline void upperLimit( long max) { m_nUpperLimit = max; }

//attributes
private:
	/// counter name
	CString	m_sName;

	/// mode (shuttle/loop/stop_on_arrival)
	CString	m_sMode;

	/// way (up/down)
	CString	m_sWay;

	/// lower limit
	long	m_nLowerLimit;

	/// upper limit
	long	m_nUpperLimit;
};

#endif // !defined(AFX_COUNTER_H__0C20D48D_E90E_4B32_B53C_2C91974411DB__INCLUDED_)
