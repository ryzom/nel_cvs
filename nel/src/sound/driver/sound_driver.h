/** \file sound_driver.h
 * ISoundDriver: sound driver interface
 *
 * $Id: sound_driver.h,v 1.10 2002/08/26 09:36:28 lecroart Exp $
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

#ifndef NL_SOUND_DRIVER_H
#define NL_SOUND_DRIVER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"

/// This namespace contains the sound classes
namespace NLSOUND {


class IBuffer;
class IListener;
class ISource;


#ifdef NL_OS_WINDOWS

#if _MSC_VER >= 1300	// visual .NET, use different dll name
// must test it first, because NL_DEBUG_FAST and NL_DEBUG are declared at same time.
#ifdef NL_DEBUG_FAST
#define NLSOUND_DLL_NAME "nldriver_openal_df.dll"
#elif defined (NL_DEBUG)
#define NLSOUND_DLL_NAME "nldriver_openal_d.dll"
#elif defined (NL_RELEASE_DEBUG)
#define NLSOUND_DLL_NAME "nldriver_openal_rd.dll"
#elif defined (NL_RELEASE)
#define NLSOUND_DLL_NAME "nldriver_openal_r.dll"
#else
#error "Unknown dll name"
#endif

#else

// must test it first, because NL_DEBUG_FAST and NL_DEBUG are declared at same time.
#ifdef NL_DEBUG_FAST
#define NLSOUND_DLL_NAME "nel_drv_dsound_win_df.dll"
#elif defined (NL_DEBUG)
#define NLSOUND_DLL_NAME "nel_drv_dsound_win_d.dll"
#elif defined (NL_RELEASE_DEBUG)
#define NLSOUND_DLL_NAME "nel_drv_dsound_win_rd.dll"
#elif defined (NL_RELEASE)
#define NLSOUND_DLL_NAME "nel_drv_dsound_win_r.dll"
#else
#error "Unknown dll name"
#endif

#endif 

#elif defined (NL_OS_UNIX)
#define NLSOUND_DLL_NAME "libnel_drv_openal.so"
#else
#error "Unknown system"
#endif



/**
 * Abstract sound driver (implemented in sound driver dynamic library)
 *
 * The caller of the create methods is responsible for the deletion of the created objects.
 * These objects must be deleted before deleting the ISoundDriver instance.
 *
 * The driver is a singleton. To access, only the pointer returned by createDriver()
 * is provided.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class ISoundDriver
{
public:

	/// Version of the driver interface. To increment when the interface change.
	static const uint32		InterfaceVersion;

	/** The static method which builds the sound driver instance
	 * In case of failure, can throw one of these ESoundDriver exception objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion
	 */
	static	ISoundDriver	*createDriver();

	/// Create a sound buffer
	virtual	IBuffer			*createBuffer() = 0;

	/// Create the listener instance
	virtual	IListener		*createListener() = 0;

	/// Return the maximum number of sources that can created
	virtual uint			countMaxSources() = 0;

	/// Create a source
	virtual	ISource			*createSource() = 0;

	/// Temp
	virtual bool			loadWavFile( IBuffer *destbuffer, const char *filename ) = 0;


	/// Commit all the changes made to 3D settings of listener and sources
	virtual void			commit3DChanges() = 0;

	/// Write information about the driver to the output stream.
	virtual void			writeProfile(std::ostream& out) = 0;

	// Does not create a sound loader

	/// Destructor
	virtual	~ISoundDriver() {}

protected:

	/// Constructor
	ISoundDriver() {}

	/// Remove a buffer (should be called by the friend destructor of the buffer class)
	virtual void			removeBuffer( IBuffer *buffer ) = 0;

	/// Remove a source (should be called by the friend destructor of the source class)
	virtual void			removeSource( ISource *source ) = 0;
};


/**
 * Sound driver exceptions
 */
class ESoundDriver : public NLMISC::Exception
{
public:
	ESoundDriver() : NLMISC::Exception( "Sound driver error" ) {}
	ESoundDriver( const char *reason ) : NLMISC::Exception( reason ) {}
};


/**
 * ESoundDriverNotFound
 */
class ESoundDriverNotFound : public ESoundDriver
{
public:
	ESoundDriverNotFound() : ESoundDriver( NLSOUND_DLL_NAME " or third-party library not found" ) {}
};


/**
 * ESoundDriverCorrupted
 */
class ESoundDriverCorrupted : public ESoundDriver
{
public:
	ESoundDriverCorrupted() : ESoundDriver( "Can't get NLSOUND_createISoundDriverInstance from " NLSOUND_DLL_NAME " (Bad dll?)" ) {}
};


/**
 * ESoundDriverOldVersion
 */
class ESoundDriverOldVersion : public ESoundDriver
{
public:
	ESoundDriverOldVersion() : ESoundDriver( NLSOUND_DLL_NAME " is a too old version. Ask for a more recent file" ) {}
};


/**
 * ESoundDriverUnknownVersion
 */
class ESoundDriverUnknownVersion : public ESoundDriver
{
public:
	ESoundDriverUnknownVersion() : ESoundDriver( NLSOUND_DLL_NAME " is more recent than the application" ) {}
};


/**
 * ESoundDriverCantCreateDriver
 */
class ESoundDriverCantCreateDriver : public ESoundDriver
{
public:
	ESoundDriverCantCreateDriver() : ESoundDriver( NLSOUND_DLL_NAME " can't create driver" ) {}
};

  
/**
 * ESoundDriverGenBuf
 */
class ESoundDriverGenBuf : public ESoundDriver
{
public:
	ESoundDriverGenBuf() : ESoundDriver( "Unable to generate sound buffers" ) {}
};


/**
 * ESoundDriverGenBuf
 */
class ESoundDriverGenSrc : public ESoundDriver
{
public:
	ESoundDriverGenSrc() : ESoundDriver( "Unable to generate sound sources" ) {}
};

  
/**
 * ESoundDriverNotSupp
 */
class ESoundDriverNotSupp : public ESoundDriver
{
public:
	ESoundDriverNotSupp() : ESoundDriver( "Operation not supported by sound driver" ) {}
};


} // NLSOUND


#endif // NL_SOUND_DRIVER_H

/* End of sound_driver.h */
