/* log.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: log.h,v 1.1 2000/09/21 12:31:16 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_LOG_H
#define NL_LOG_H

#include <vector>

namespace NLMISC
{

	class IDisplayer;

/**
 * <Replace this by a description of the class>
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CLog
{
public:

	/// Constructor
	CLog() {}

	/// Add a new displayer in the log. You have to create the displayer, remove it and delete it when you have finnish with it.
	/// For example, in a 3dDisplayer, you can add the displayer when you want, and the displayer displayer the string if the 3d
	/// screen is available and do nothing otherwise. In this case, if you want, you could leave the displayer all the time.
	void addDisplayer (IDisplayer *displayer);

	/// Remove a displayer. If the displayer doesn't work, you could remove it.
	void removeDisplayer (IDisplayer *displayer);

	/// Display a string to all displayer available.
	void display (const std::string format, ...);

private:

	std::vector<IDisplayer *> _Displayers;
};

} // NLMISC

#endif // NL_LOG_H

/* End of log.h */
