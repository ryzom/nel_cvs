/** \file sound_driver.h
 * ISoundDriver: sound driver interface
 *
 * $Id: sound_driver.h,v 1.18 2004/02/23 14:38:44 lecroart Exp $
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
#include "nel/misc/string_mapper.h"
#include "nel/misc/common.h"

/// This namespace contains the sound classes
namespace NLSOUND {


class IBuffer;
class IListener;
class ISource;

/** Configuration for compiling with or without EAX support.
 *	Set to 0 if you don't have EAX library or don't whant EAX support.
 *	This definition impact on code generation for driver AND sound lib.
 * NO EAX on linux
 */
#ifdef NL_OS_WINDOWS
#define EAX_AVAILABLE	1
#endif
/** Configuration to compile with manual or API (directx or open AL) rolloff factor.
 *	Set it to 1 for manual rolloff, 0 for API rolloff.
*/
#define MANUAL_ROLLOFF	0


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


/*
 * Sound sample format
 */
enum TSampleFormat { Mono8, Mono16ADPCM, Mono16, Stereo8, Stereo16 };



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

	/** The interface must be implemented and provided to the driver
	 *	in order to have a coherent string mapping.
	 *	The driver must not call directly CStringMapper method because
	 *	the static map container are located in a lib, so the main
	 *	code and the driver have theire own version of the static
	 *	container !
	 */
	class IStringMapperProvider
	{
	public:
		/// map a string
		virtual const NLMISC::TStringId map(const std::string &str) =0;
		/// unmap a string
		virtual const std::string &unmap(const NLMISC::TStringId &stringId) =0;
	};

	/// Version of the driver interface. To increment when the interface change.
	static const uint32		InterfaceVersion;

	/** The static method which builds the sound driver instance
	 * In case of failure, can throw one of these ESoundDriver exception objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion
	 *
	 * You can request support for EAX. If EAX is requested and if there is enougth hardware
	 * buffer replay, then only hardware buffer are created when calling createBuffer.
	 * If the number of available hardware buffer is less than 10, then EAX is ignored.
	 */
	static	ISoundDriver	*createDriver(bool useEax, IStringMapperProvider *stringMapper);

	/// Create a sound buffer
	virtual	IBuffer			*createBuffer() = 0;

	/// Create the listener instance
	virtual	IListener		*createListener() = 0;

	/// Return the maximum number of sources that can created
	virtual uint			countMaxSources() = 0;

	/// Create a source
	virtual	ISource			*createSource() = 0;

	/// Temp
//	virtual bool			loadWavFile( IBuffer *destbuffer, const char *filename ) = 0;
	virtual bool			readWavBuffer( IBuffer *destbuffer, const std::string &name, uint8 *wavData, uint dataSize) = 0;

	virtual bool			readRawBuffer( IBuffer *destbuffer, const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat format, uint32 frequency) = 0;

	/// Commit all the changes made to 3D settings of listener and sources
	virtual void			commit3DChanges() = 0;

	/// Write information about the driver to the output stream.
	virtual void			writeProfile(std::ostream& out) = 0;

	// Does not create a sound loader

	virtual void	startBench() =0;
	virtual void	endBench() =0;
	virtual void	displayBench(NLMISC::CLog *log) =0;


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
