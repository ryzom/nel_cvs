/** \file client.cpp
 * Client prototype
 *
 * $Id: client.cpp,v 1.1 2001/07/11 15:17:40 legros Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/types_nl.h"

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
#include <windows.h>
#endif

#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"

#include "nel/net/msg_socket.h"
#include "nel/net/naming_client.h"
#include "nel/net/unitime.h"
#include "nel/net/net_displayer.h"
#include "nel/net/net_log.h"
#include "nel/net/local_area.h"

#include "nel/3d/driver.h"
#include "nel/3d/camera.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/landscape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/text_context.h"
#include "nel/3d/mini_col.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_mem.h"

#include "nel/3d/landscape_profile.h"


#include "move_listener.h"
#include "language_interface.h"
#include "login_interface.h"
#include "shards_list_interface.h"
#include "character_interface.h"
#include "player_view.h"
#include "nel/3d/zone_manager.h"
#include "sight.h"
#include "lens_flare.h"


#include <string>
#include <deque>

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace NL3D;


#undef NETWORK_LOGGING



//temporary, for using thread zone loading around a player, 
//#define THREAD_ZONE_LOADING

// just only for unix...
#include "nel/net/service.h"
const char NLNET::IService::_Name[] = "client";
const uint16 NLNET::IService::_DefaultPort = 12345;
sint16 CallbackArraySize = 0;
NLNET::TCallbackItem CallbackArray [] = { NULL, };
// end of just...

CMsgSocket						*ClientSocket = NULL;
CLocalArea						*LocalArea = NULL;

// Create a Landscape
CLandscapeModel	*Landscape = NULL;

// Zone manager use TaskManager for background loading of zone
#ifdef THREAD_ZONE_LOADING
CZoneManager	ZoneManager;
#endif

CLog CommandExecutionLog;

bool commandLine (const ucstring &str);


/** 
 * Chat text input
 */

class CChatListener : public IEventListener
{
	virtual void	operator() ( const CEvent& event )
	{
		CEventChar &ec = (CEventChar&)event;

		switch ( ec.Char )
		{
		case 13 : // RETURN : Send the chat message
			if ( _Line.size() == 0 )
				break;
	
			// if, it s a command, execute it and don't send the command
			if ( ! commandLine( _Line ) )
			{
				if ( (ClientSocket != NULL) && ClientSocket->connected() )
				{
					CMessage msgout( "CHAT" );
					ucstring totalline = ucstring( LocalArea->User.name() + "> " ) + _Line;
					TEntityId id = LocalArea->User.id();
					msgout.serial( totalline );
					msgout.serial( id );
					ClientSocket->send( msgout );
				}
			}
			_Line = "";
			_MaxWidthReached = false;
			break;

		case 8 : // BACKSPACE
			if ( _Line.size() != 0 )
			{
				_Line.erase( _Line.end()-1 );
				// _MaxWidthReached = false; // no need
			}
			break;
		case 9 : // TAB
			{
				if (!_Line.empty() && _Line[0] == '/')
				{
					string command = _Line.toString().substr(1);
					ICommand::expand(command);
					_Line = '/' + command;
				}
			}
			break;
		case 27 : // ESCAPE
			break;

		default: 
			if ( ! _MaxWidthReached )
			{
				_Line += ec.Char;
			}
		}
	}

public:
	CChatListener() : _MaxWidthReached( false )
	{}

	const ucstring&	line() const
	{
		return _Line;
	}

	void			setMaxWidthReached( bool b )
	{
		_MaxWidthReached = b;
	}

private:
	ucstring		_Line;
	bool			_MaxWidthReached;
};


/**
 * CClientConfig
 */
struct CClientConfig
{
	bool			Windowed;
	uint			Width;
	uint			Height;
	uint			Depth;
	CVector			Position;
	CVector			Heading;
	CRGBA			Background;
	string			DataPath;
	string			FontName;
	sint			LanguageIndex;
	string			LSHost;
	string			Login;
	string			Password;
	string			ShardIP;
	CFontManager	FontManager;
	CTextContext	TextContext;
	char**			ZonesPath;
	uint32			NbZonesPath;
	sint			CharacterIndex;
	float			LandscapeTileNear;
	float			LandscapeThreshold;

	CClientConfig()
	{
		Windowed = false;
		Width = 800;
		Height = 600;
		Depth = 32;
		Position = CVector( 1840.0f, -970.0f, -23.520323f );
		Heading = CVector( -0.122154f, 1.0f, 0.0f );
		Background = CRGBA(100,100,255);
		//DataPath = "\\\\server\\gamedata\\data/";
		DataPath = "data/";
		FontName = "n019003l.pfb";
		LanguageIndex = 0;
		LSHost = "itsalive.nevrax.org";
		ZonesPath = NULL;
		NbZonesPath = 0;
		CharacterIndex = 0;
		LandscapeTileNear = 50.0f;
		LandscapeThreshold = 0.001f;
	}
};

CClientConfig					ClientCfg;


typedef deque<uint32> CStringIndexes;

/**
 * Chat display
 */
class CChatOutput
{
public:

	static const uint ChatNbLinesSaved;
	static const uint ChatNbLinesDisplayedShort;
	static const uint ChatNbLinesDisplayedLong;
	static const float ChatX;
	static const float ChatY;
	static const float ChatLineHeight;

	/// Constructor
	CChatOutput() : _ChatNbLinesDisplayed( CChatOutput::ChatNbLinesDisplayedShort )
	{}

	/// Adds a line to the chat output (unicode)
	void	addLine( const ucstring& us, TEntityId id )
	{
		CRGBA linecolor;
		if ( id == LocalArea->User.id() ) // local players
		{
			linecolor.set( 128, 0, 128, 255 );
		}
		else if ( id == 0 ) // system messages
		{
			linecolor.set( 32, 32, 32, 255 );
		}
		else // other players
		{
			linecolor.set( 0, 128, 128, 255 );
		}
		ClientCfg.TextContext.setColor( linecolor );
		ClientCfg.TextContext.setFontSize( 11 );
		_StringIndexes.push_back( ClientCfg.TextContext.textPush( us ) );
		
		if ( _StringIndexes.size() > ChatNbLinesSaved )
		{
			ClientCfg.TextContext.erase( *_StringIndexes.begin() );
			_StringIndexes.pop_front();
		}
	}

	/// Draws the chat on the screen
	void	renderChat()
	{
		ClientCfg.TextContext.setHotSpot(CComputedString::BottomLeft);
		sint i;
		for ( i=_StringIndexes.size()-1; i>=max((sint)0,(sint)_StringIndexes.size()-(sint)_ChatNbLinesDisplayed); --i )
		{
			ClientCfg.TextContext.printAt( ChatX, ChatY+(_StringIndexes.size()-1-i)*ChatLineHeight, _StringIndexes[i] );
		}
	}

	/// Toggle between short and long display
	void	switchDisplay()
	{
		if ( _ChatNbLinesDisplayed == ChatNbLinesDisplayedShort )
		{
			_ChatNbLinesDisplayed = CChatOutput::ChatNbLinesDisplayedLong;
		}
		else
		{
			_ChatNbLinesDisplayed = CChatOutput::ChatNbLinesDisplayedShort;
		}
	}

	void clear()
	{
		sint i;
		for(i=0; i<(sint)_StringIndexes.size(); i++)
		{
			ClientCfg.TextContext.erase( _StringIndexes[i] );
		}
		_StringIndexes.clear();
	}

private:

	CStringIndexes	_StringIndexes;
	uint			_ChatNbLinesDisplayed;
	
};

const uint CChatOutput::ChatNbLinesSaved = 28;
const uint CChatOutput::ChatNbLinesDisplayedShort = 5;
const uint CChatOutput::ChatNbLinesDisplayedLong = 28;
const float CChatOutput::ChatX = 0.05f;
const float CChatOutput::ChatY = 0.08f;
const float CChatOutput::ChatLineHeight = 0.025f;



/**
 *	CClientEntity
 */
struct CClientEntity
{
	vector<CTransformShape *>	Shapes;
	vector<bool>				ShowShapes;
	uint						CurrentShapeIndex;
	bool						SnappedToGround;
	CRemoteEntity				*Entity;

	CClientEntity( CRemoteEntity *entity )
	{
		CurrentShapeIndex = 0;
		SnappedToGround = false;
		Entity = entity;
	}
};

map<TEntityId,CClientEntity*>	ClientEntities;




/*
 * Globals
 */

CMoveListener					MoveListener;
CChatListener					ChatListener;
CChatOutput						ChatOutput;

CPlayerView						playerView;

map<TEntityId,sint32>			ScoresMap;
map<TEntityId,sint64>			ShotMap; // time remaining until end of invulnerability
//map<TEntityId,bool>				ShowMap; // player'
//map<TEntityId,CTransformShape*>	InstancesMap;
CMiniCol						CollisionManager;
const CVector					EyesHeight[2] = {CVector( 0.0f, 0.0f, 1.80f ),CVector( 0.0f, 0.0f, 3.0f )};
const TTime						InvincibilityDuration = 5000; // 5 s after being shot
TTime							ShootTime = 0;

vector<string>					Shapes;
TTime							PingTime = 0;

enum TMEType {
	MEPingoo = 0,
	MEGnu = 1,
	MEWeapon = 2
};

sint64							ShakeTime = 0;
sint64							ShakeDuration = 500;
float							ShakeAmplitude = (float)Pi/4.f;
uint							RollProjectile = 0;

TTime							FirstRollTime = 0;


bool							Sight = false;
CSight							SelectedSight;
bool							ShowVectors = false;

vector<pair<CVector,string> >	ParticularPlaces;
CLensFlare * LensFlare; 

/*
 * CChatDisplayer
 *
 * Log format : "<LogType>: <Msg>"
 */
class CChatDisplayer : public IDisplayer
{
	virtual void doDisplay (time_t date, NLMISC::CLog::TLogType logType, const std::string &processName, const char *fileName, sint line, const char *message)
	{
		bool needSpace = false;
		stringstream ss;

		if (logType != CLog::LOG_NO)
		{
			ss << logTypeToString(logType);
			needSpace = true;
		}

		if (needSpace) { ss << ": "; needSpace = false; }

		ss << message;

		ChatOutput.addLine( ucstring(ss.str()), 0 );
	}
};


CChatDisplayer					ChatDisplayer;



// Forward declaration
void DisplayClientDump();


void displayNevrax()
{
	static CSmartPtr<ITexture> logoTexture = new CTextureFile("nel128.tga");
	float	x= 8/800.0f;
	float	y= 8/600.0f;
	float	w=128/800.0f;
	float	h=128/600.0f;
	CDRU::drawBitmap (x, 1-(h+y), w, h, *logoTexture, *CNELU::Driver );

	ClientCfg.TextContext.setColor(CRGBA(255,255,255));
	ClientCfg.TextContext.setFontSize(12);
	ClientCfg.TextContext.setHotSpot(CComputedString::MiddleTop);
	h=(128-42)/600.0f;
	ClientCfg.TextContext.printfAt(x+w/2.0f,1.0f-h-y-0.01f,"Snowballs version 0.2");
}


// Callback for remote entity creation
void createNewEntityInstance( CRemoteEntity *entity )
{
	CVector pos = entity->pos();

	if ( LocalArea->inRadius( pos ) )
	{
		// Snap new entity to ground (only in the local area)
		if ( entity->full3d() )
		{
			CollisionManager.snapToGround( pos );
		}
		else
		{
			// Guess remote altitude altitude using user's altitude
			entity->setAltitude( LocalArea->User.pos().z );
			CollisionManager.snapToGround( pos, 100.0f );
		}
		entity->setPos( pos );
	}

	// Create 3d instance
	CClientEntity * ce = new CClientEntity( entity );
	
	switch(entity->type())
	{
		case MEPingoo :
		{
			ce->Shapes.resize(2);
			ce->ShowShapes.resize(2);
			ce->Shapes[0] = CNELU::Scene.createInstance("pingoo.shape");
			ce->Shapes[0]->setTransformMode(CTransform::DirectMatrix);
			ce->ShowShapes[0] = true;
			ce->Shapes[0]->show();
			ce->Shapes[1] = CNELU::Scene.createInstance("pingoo_fighted.shape");
			ce->Shapes[1]->setTransformMode(CTransform::DirectMatrix);
			ce->ShowShapes[1] = false;
			ce->Shapes[1]->hide();
			ucstring str = entity->name() + CI18N::get(" joined the game");
			ChatOutput.addLine( str, 0 );
		}
		break;
		case MEGnu :
		{
			ce->Shapes.resize(2);
			ce->ShowShapes.resize(2);
			ce->Shapes[0] = CNELU::Scene.createInstance("gnu_fly.shape");
			ce->Shapes[0]->setTransformMode(CTransform::DirectMatrix);
			ce->ShowShapes[0] = true;
			ce->Shapes[0]->show();
			ce->Shapes[1] = CNELU::Scene.createInstance("gnu_fighted.shape");
			ce->Shapes[1]->setTransformMode(CTransform::DirectMatrix);
			ce->ShowShapes[1] = false;
			ce->Shapes[1]->hide();
			ucstring str = entity->name()+CI18N::get(" joined the game");
			ChatOutput.addLine( str, 0 );
		}
		break;
		case MEWeapon :
		{
			ce->Shapes.resize(1);
			ce->ShowShapes.resize(1);
			ce->Shapes[0] = CNELU::Scene.createInstance("snowball_01.shape");
			ce->Shapes[0]->setTransformMode(CTransform::DirectMatrix);
			ucstring str = entity->name()+CI18N::get(" joined the game");
			ce->ShowShapes[0] = true;
			ce->Shapes[0]->show();
		}
		break;
		default:
			break;
	}

	CMatrix transform;
	transform.identity();
	transform.setPos( entity->pos() );
	transform.setRot( CVector(0.0f,0.0f,entity->angleAroundZ()+(float)Pi/2.0f), CMatrix::XYZ ); // to change (?)
	vector<CTransformShape *>::iterator	itsh;
	for(itsh = ce->Shapes.begin(); itsh!=ce->Shapes.end();itsh++)
	{
		(*itsh)->setMatrix( transform );
	}
	nldebug( "Instance created for entity %u",  entity->id() );

	ClientEntities.insert(make_pair( entity->id(), ce ));
	
	// Note: ScoresMap is appended in cbAddNewScore()
}


// Callback for remote entity move
void moveEntityInstance( CRemoteEntity *entity )
{
	CVector pos = entity->pos();
	if ( LocalArea->inRadius( pos ) )
	{
		if ( entity->full3d() )
		{
			CollisionManager.snapToGround( pos );
		}
		else
		{
			CollisionManager.testMove( entity->previousPos(), pos );
		}
		entity->setPos( pos );
	}
	

	// Compute Vector up.
	CVector vup(0,0,1);
	CollisionManager.getGroundNormal(pos, vup);
	float teta = vup * CVector(0,0,1);
	float pitch = 0;
	CVector pp = entity->previousPos();
	float difz = pp.z - pos.z;
	if(teta!=0)
	{
		if(difz>0)
			pitch = (float)acos(teta);
		else
		if(difz<0)
			pitch = -(float)acos(teta);
		else
			pitch = 0;
	}


	float roll = (float)RollProjectile*(float)Pi/180.f;

	CMatrix transform;
	transform.identity();
	
	// Compute transform with vup.
	if(entity->type() == MEWeapon)
	{
		transform.rotateZ(entity->angleAroundZ()+(float)Pi/2.0f);
		transform.rotateX(pitch);
		transform.rotateY(roll);
	}
	else
	{
		transform.setRot( CVector(0.0f,0,entity->angleAroundZ()+(float)Pi/2.0f), CMatrix::XYZ ); // to change (?)
		CVector i,j,k;
		transform.getRot(i,j,k);
		transform.setRot(i,j,vup);
		transform.normalize( CMatrix::ZYX );
	}
		
	
	// Set the good pos.
	transform.setPos( pos ); // human height
	if(entity->type() == MEWeapon)
	{
		transform.setPos( pos + EyesHeight[LocalArea->User.type()] );
	}
	//InstancesMap[entity->id()]->setMatrix( transform );
	vector<CTransformShape *>::iterator	itsh;
	for(itsh = ClientEntities[entity->id()]->Shapes.begin(); itsh!=ClientEntities[entity->id()]->Shapes.end();itsh++)
	{
		(*itsh)->setMatrix( transform );
	}
}





// Callback for remote entity deletion
void deleteEntityInstance( TEntityId id )
{
	vector<CTransformShape *>::iterator	itsh;
	for(itsh = ClientEntities[id]->Shapes.begin(); itsh!=ClientEntities[id]->Shapes.end();itsh++)
	{
		CNELU::Scene.deleteInstance( *itsh );
	}
	ClientEntities.erase( id );

	CRemoteEntities::const_iterator ire = LocalArea->neighbors().find(id);
	if ( ire != LocalArea->neighbors().end() )
	{
		if ( (*ire).second->type() != MEWeapon )
		{
			ucstring str = (*ire).second->name()+CI18N::get(" left the game");
			ChatOutput.addLine( str, 0 );
			ScoresMap.erase( id );
		}
	}

	ShotMap.erase( id );

	//map<TEntityId,CClientEntity*>::iterator	itce = ClientEntities.find(id);
	ClientEntities.erase(id);

	nldebug( "Instance of entity %u deleted", id );
}


// Resets a player
void ResetUser()
{
	// Reset position
	CVector pos = ClientCfg.Position;
	CollisionManager.snapToGround( pos );
	LocalArea->User.resetPos( pos );
	LocalArea->User.resetBodyHeading( ClientCfg.Heading );
	LocalArea->User.setFrontVelocity( 0 );
	LocalArea->User.setStrafeVelocity( 0 );
}


// Sends a Remove Weapon command to the server
void RemoveWeapon( TEntityId wid )
{
	if ( (ClientSocket != NULL) && ClientSocket->connected() )
	{
		CMessage msgout( "RMW" );
		msgout.serial( wid );
		ClientSocket->send( msgout );
	}
}


// Shoots a player
void Shoot( TEntityId idweapon, TEntityId idshooter )
{
	if ( (CTime::getLocalTime() > ShootTime + InvincibilityDuration)
	  && (idshooter != LocalArea->User.id()) ) // can't kill oneself
	{
		if ( (ClientSocket != NULL) && ClientSocket->connected() )
		{
			CMessage msgout( "SHO" );
			TEntityId idshot = LocalArea->User.id();
			nlassert( idshooter != 0 );
			msgout.serial( idshot );
			msgout.serial( idweapon );
			msgout.serial( idshooter );
			ClientSocket->send( msgout );
		}

		ShootTime = CTime::getLocalTime();
		MoveListener.setShot(true);
		ShakeTime = ShakeDuration;
		
		ucstring str = CI18N::get("You were shot down by ");
		str += *LocalArea->nameFromId(idshooter);
		ChatOutput.addLine( str, 0 );
		nlinfo( "You were shot down by %s !", LocalArea->neighbors()[idshooter]->name().c_str() );
		ScoresMap[idshooter]++;
	}
	else
	{
		// Either the player shot itself or the player is invincible
		if ( (ClientSocket != NULL) && ClientSocket->connected() )
		{
			RemoveWeapon( idweapon );
		}
		nldebug( "You destroyed the weapon %u", idweapon );
	}
}


// Tests the collisions between weapons and players
void TestCollisions()
{
	CRemoteEntities::const_iterator ipr1;
	for ( ipr1=LocalArea->neighbors().begin(); ipr1!=LocalArea->neighbors().end(); ++ipr1 )
	{
		if ( (*ipr1).second->type() == MEWeapon )
		{
			// Prevent from detecting several times a single collision (using Tag as idshooter or 0)
			if ( (*ipr1).second->Tag != 0 )
			{
				// Test collision between weapons and entities
				if ( (LocalArea->User.pos()-(*ipr1).second->pos()).norm() < 2.0f )
				{
					nldebug( "You are hit by a weapon" );
					Shoot( (*ipr1).second->id(), (*ipr1).second->Tag );
					(*ipr1).second->Tag = 0; // reset Tag to 0
					continue; // no need to test the following predicate
				}

				// Test if the weapons fired by *this player* are blocked or too far (> 100 meters)
				if ( ( (*ipr1).second->Tag == LocalArea->User.id() )
				  && ( ( ((*ipr1).second->pos()-LocalArea->User.pos()).norm() > 100 )
				    || ( (*ipr1).second->pos() == (*ipr1).second->previousPos() ) ) )
				{
					nldebug( "Removing weapon" );
					/*nldebug( "Previous: %f\t%f\t%f", (*ipr1).second->previousPos().x, (*ipr1).second->previousPos().y, (*ipr1).second->previousPos().z );
					nldebug( "Current:  %f\t%f\t%f", (*ipr1).second->pos().x, (*ipr1).second->pos().y, (*ipr1).second->pos().z );*/
					RemoveWeapon( (*ipr1).second->id() );
					(*ipr1).second->Tag = 0; // reset Tag to 0
				}
			}
		}
	}
}


/*********************************************************\
			Yoyo is sorry: drawDummyShadow()
\*********************************************************/
void	drawDummyShadow(const CVector &pos)
{
	static	vector<CTriangle>	Triangles;
	static	vector<CTriangleUV>	Faces;
	static	bool	textInit= false;
	static	CSmartPtr<ITexture>		text; 
	static	CMaterial				mat;

	// Init the Material/Texture.
	if(!textInit)
	{
		textInit= true;
		text= new CTextureFile("shadow_yoyo.tga");
		text->setWrapS(ITexture::Clamp);
		text->setWrapT(ITexture::Clamp);
		text->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
		mat.initUnlit();
		mat.setBlend(true);
		mat.setBlendFunc(CMaterial::zero, CMaterial::srccolor);
		mat.setTexture(0, text);
	}

	// Get the faces just under the entity.
	const	float	radius= 1;
	CAABBox		bbox;
	bbox.setCenter(pos);
	bbox.setHalfSize(CVector(radius,radius,radius*2));
	CollisionManager.getFaces(Triangles, bbox);

	// Map them, from the plane XY.
	Faces.resize(Triangles.size());
	for(sint i=0;i<(sint)Faces.size();i++)
	{
		CVector		v;
		Faces[i]= Triangles[i];
		// Because tesselation is not EXACTLY the same (geomorph, ???), and because tesselation is not the same,
		// in minicol, and in landscape, must add 5cm. This is a trick, but this is a test.
		Faces[i].V0.z+=0.05f;
		Faces[i].V1.z+=0.05f;
		Faces[i].V2.z+=0.05f;
		v= (Faces[i].V0-pos)/(radius*2);
		Faces[i].Uv0.U= v.x+0.5f; Faces[i].Uv0.V= v.y+0.5f;
		v= (Faces[i].V1-pos)/(radius*2);
		Faces[i].Uv1.U= v.x+0.5f; Faces[i].Uv1.V= v.y+0.5f;
		v= (Faces[i].V2-pos)/(radius*2);
		Faces[i].Uv2.U= v.x+0.5f; Faces[i].Uv2.V= v.y+0.5f;
	}

	// Render them!!
	CDRU::drawTrianglesUnlit(Faces, mat, *CNELU::Driver);

}


// Call it just after render of the scene.
void displayOtherPlayersShadow()
{
	CRemoteEntities::const_iterator it;

	// Reset model matrix.
	CMatrix matrix;
	matrix.identity();
	CNELU::Driver->setupModelMatrix(matrix);


	for(it=LocalArea->neighbors().begin(); it!=LocalArea->neighbors().end(); it++)
	{
		if((*it).second->type() != MEWeapon)
		{
			if(LocalArea->inRadius( (*it).second->pos() ) )
			{
				CVector pos = (*it).second->pos();

				drawDummyShadow(pos);
			}
		}
	}
}



/*********************************************************\
					hitShake()
\*********************************************************/
void hitShake()
{
	CVector v2 = LocalArea->User.bodyHeading();
	CVector v1 = v2 ^ CVector(0,0,1);
	CVector v3 = v1 ^ v2;
	CMatrix viewmatrix;
	viewmatrix.identity();
	viewmatrix.setPos(LocalArea->User.pos() + EyesHeight[LocalArea->User.type()]);
	viewmatrix.setRot( v1, v2, v3, true );
	
	viewmatrix.rotateX( LocalArea->User.ViewPitch );
	viewmatrix.rotateX( -ShakeAmplitude/2+frand(ShakeAmplitude) );
	/*
	float freq = 10.f*(float)Pi/1000;
	float x = ShakeAmplitude*ShakeTime/ShakeDuration;
	float y = (ShakeDuration-ShakeTime)*freq;
	viewmatrix.rotateX( x*cos(y) );
	*/
	viewmatrix.rotateY( LocalArea->User.ViewRoll );
	viewmatrix.rotateY( -ShakeAmplitude/2+frand(ShakeAmplitude) );
	
	CNELU::Camera->setMatrix( viewmatrix );
}


/*********************************************************\
					displayPenaltyTime()
\*********************************************************/
void displayPenaltyTime()
{
	TTime time = CTime::getLocalTime();
	if(time > ShootTime + InvincibilityDuration)
	{
		ShootTime = 0;
		MoveListener.setShot(false);
	}

	if(ShootTime>0)
	{
		// Display penalty time remaining
		ClientCfg.TextContext.setHotSpot(CComputedString::MiddleBottom);
		ClientCfg.TextContext.setColor(CRGBA(255,255,0));
		ClientCfg.TextContext.setFontSize(24);
		ClientCfg.TextContext.printfAt(0.5f,0.90f,"%d s",
			(sint)(InvincibilityDuration/1000-(time-ShootTime)/1000)-1);
	}
}



/*********************************************************\
					displayRadar()
\*********************************************************/
void displayRadar(float angle, uint maxDistance)
{
	float xLeft = 0.5f-0.4f*3.f/4.f;
	float xRight = 0.5f+0.4f*3.f/4.f;
	float yTop = 0.9f;
	float yBottom = 0.1f;

	// Background
	CDRU::drawQuad(xLeft,yBottom,xRight,yTop,*CNELU::Driver,CRGBA(0,255,0,50),CNELU::Scene.getViewport());

	// Print radar's range
	ClientCfg.TextContext.setHotSpot(CComputedString::TopRight);
	ClientCfg.TextContext.setColor(CRGBA(0,255,0));
	ClientCfg.TextContext.setFontSize(14);
	ClientCfg.TextContext.printfAt(0.5f+0.39f*3.f/4.f,0.89f,"%d m",maxDistance);
		
	// Radar unit
	float stepV = 50.0f;
	float stepH = stepV*3.f/4.f;
	// Changing scale
	stepV = 0.8f*stepV/maxDistance;
	stepH = 0.8f*stepH/maxDistance;

	// Drawing radar's lines
	float gapV = stepV/2;
	float gapH = stepH/2;
	
	while(gapV<=0.4f)
	{
		// v lines
		CDRU::drawLine(0.5f+gapH,yTop,0.5f+gapH,yBottom,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	
		CDRU::drawLine(0.5f-gapH,yTop,0.5f-gapH,yBottom,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	
		
		// h lines
		CDRU::drawLine(xLeft,0.5f+gapV,xRight,0.5f+gapV,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	
		CDRU::drawLine(xLeft,0.5f-gapV,xRight,0.5f-gapV,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	

		gapV += stepV;
		gapH += stepH;
	}
	
	

	float scale = 1.0f;

	float xscreen = 0.5f;
	float yscreen = 0.5f;

	CNELU::Driver->setFrustum (0.f, 4.0f/3.0f, 0.f, 1.f, -1.f, 1.f, false);
	
	// distance between user and neighbour player
	float myPosx = LocalArea->User.pos().x;
	float myPosy = LocalArea->User.pos().y;

	// Quads size
	float radius = 0.006f;

	// Arrow in center (user)
	CDRU::drawTriangle(xscreen-2*radius,yscreen-2*radius, 
					   xscreen,yscreen-radius, 
					   xscreen,yscreen+2*radius, 
					   *CNELU::Driver,CRGBA(0,255,0),CNELU::Scene.getViewport());
	CDRU::drawTriangle(xscreen,yscreen-radius, 
					   xscreen+2*radius,yscreen-2*radius, 
					   xscreen,yscreen+2*radius, 
					   *CNELU::Driver,CRGBA(0,255,0),CNELU::Scene.getViewport());

	ClientCfg.TextContext.setColor(CRGBA(255,100,0));

	CRemoteEntities::const_iterator it;

	for(it=LocalArea->neighbors().begin(); it!=LocalArea->neighbors().end(); it++)
	{
		if((*it).second->type() != MEWeapon)
		{
			CVector playerPos = (*it).second->pos();

			// position of neighbour
			float posx = playerPos.x;
			float posy = playerPos.y;

			// relative position
			posx = (posx-myPosx)*0.4f/maxDistance;
			posy = (posy-myPosy)*0.4f/maxDistance;

			float dist = (float) sqrt((posx*posx)+(posy*posy));

			// Display a quad to show a player
			float an;
			float az;
			float x;
			float y;
			az = LocalArea->User.angleAroundZ();
			if(posx==0)
			{
				if(posy==0)
				{
					x = xscreen;
					y = yscreen;
				}
				else
				{
					if(posy>0)
					{
						x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(Pi-az));
					}
					else
					{
						x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(-az));
					}
				}
			}
			else
			{
				an = (float) atan(posy/posx);
				if(posx<0) an = an + (float)Pi;
				x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
				y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
			}

			// Players out of range are not displayed
			if(x<xLeft || x>xRight || y>yTop || y<yBottom) continue;

			

			ClientCfg.TextContext.setColor(CRGBA(255,100,0));

			CDRU::drawQuad(x-radius,y-radius,
							x+radius,y+radius,
							*CNELU::Driver,CRGBA(255,100,0),CNELU::Scene.getViewport());

			// Print his name
			ClientCfg.TextContext.setFontSize(10);
			if(x>=0.5f)
			{
				if(y>=0.5f)
				{
					ClientCfg.TextContext.setHotSpot(CComputedString::BottomLeft);
					ClientCfg.TextContext.printfAt(x+2*radius, y+2*radius, (*it).second->name().c_str());
				}
				else
				{
					ClientCfg.TextContext.setHotSpot(CComputedString::TopLeft);
					ClientCfg.TextContext.printfAt(x+2*radius, y-2*radius, (*it).second->name().c_str());
				}
			}
			else
			{
				if(y>=0.5f)
				{
					ClientCfg.TextContext.setHotSpot(CComputedString::BottomRight);
					ClientCfg.TextContext.printfAt(x-2*radius, y+2*radius, (*it).second->name().c_str());
				}
				else
				{
					ClientCfg.TextContext.setHotSpot(CComputedString::TopRight);
					ClientCfg.TextContext.printfAt(x-2*radius, y-2*radius, (*it).second->name().c_str());
				}
			}
		}
	}


	// display particular places
	vector<pair<CVector,string> >::iterator itpp;
	for(itpp = ParticularPlaces.begin(); itpp != ParticularPlaces.end(); itpp++)
	{
		// relative position
		float posx = ((*itpp).first.x-myPosx)*0.4f/maxDistance;
		float posy = ((*itpp).first.y-myPosy)*0.4f/maxDistance;
		
		float dist = (float) sqrt((posx*posx)+(posy*posy));

		// Display a quad to show a player
		float an;
		float az;
		float x;
		float y;
		az = LocalArea->User.angleAroundZ();
		if(posx==0)
		{
			if(posy==0)
			{
				x = xscreen;
				y = yscreen;
			}
			else
			{
				if(posy>0)
				{
					x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(Pi-az));
				}
				else
				{
					x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(-az));
				}
			}
		}
		else
		{
			an = (float) atan(posy/posx);
			if(posx<0) an = an + (float)Pi;
			x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
			y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
		}


		if(x<xLeft || x>xRight || y>yTop || y<yBottom) 
		{
			continue;
		}

		CDRU::drawTriangle(x-radius,y-radius, 
							x+radius,y-radius, 
							x,y+radius, 
							*CNELU::Driver,
							CRGBA(0,0,255),
							CNELU::Scene.getViewport());

		ClientCfg.TextContext.setFontSize(10);
		ClientCfg.TextContext.setColor(CRGBA(0,0,255));

		if(x>=0.5f)
		{
			if(y>=0.5f)
			{
				ClientCfg.TextContext.setHotSpot(CComputedString::BottomLeft);
				ClientCfg.TextContext.printfAt(x+2*radius, y+2*radius, 
												(*itpp).second.c_str());
			}
			else
			{
				ClientCfg.TextContext.setHotSpot(CComputedString::TopLeft);
				ClientCfg.TextContext.printfAt(x+2*radius, y-2*radius, 
												(*itpp).second.c_str());
			}
		}
		else
		{
			if(y>=0.5f)
			{
				ClientCfg.TextContext.setHotSpot(CComputedString::BottomRight);
				ClientCfg.TextContext.printfAt(x-2*radius, y+2*radius, 
												(*itpp).second.c_str());
			}
			else
			{
				ClientCfg.TextContext.setHotSpot(CComputedString::TopRight);
				ClientCfg.TextContext.printfAt(x-2*radius, y-2*radius, 
												(*itpp).second.c_str());
			}
		}
	}

}



/*********************************************************\
					displayLittleRadar()
\*********************************************************/
void displayLittleRadar(float angle, uint maxDistance)
{
	float width = 0.1f;
	float xLeft = 0.9f-width*3.f/4.f;
	float xRight = 0.9f+width*3.f/4.f;
	float yTop = 0.3f+width;
	float yBottom = 0.3f - width;

	// Background
	CDRU::drawQuad(xLeft,yBottom,xRight,yTop,*CNELU::Driver,CRGBA(0,255,0,50),CNELU::Scene.getViewport());

	// Print radar's range
	ClientCfg.TextContext.setHotSpot(CComputedString::MiddleBottom);
	ClientCfg.TextContext.setColor(CRGBA(0,255,0));
	ClientCfg.TextContext.setFontSize(12);
	ClientCfg.TextContext.printfAt(xLeft+width*3.f/4.f,yTop+0.01f,"%d m",maxDistance);
		
	// Radar unit
	float stepV = 50.0f;
	float stepH = stepV*3.f/4.f;
	// Changing scale
	stepV = width*stepV/maxDistance;
	stepH = width*stepH/maxDistance;

	// Drawing radar's lines
	// h lines
	CDRU::drawLine(xLeft,yTop,xRight,yTop,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());
	CDRU::drawLine(xLeft,yBottom+width,xRight,yBottom+width,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());
	CDRU::drawLine(xLeft,yBottom,xRight,yBottom,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	
	
	// v lines
	CDRU::drawLine(xLeft,yTop,xLeft,yBottom,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	
	CDRU::drawLine(xLeft+width*3.f/4.f,yTop,xLeft+width*3.f/4.f,yBottom,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	
	CDRU::drawLine(xRight,yTop,xRight,yBottom,*CNELU::Driver,CRGBA(0,255,0,100),CNELU::Scene.getViewport());	


	float scale = 1.0f;

	float xscreen = xLeft + width*3.f/4.f;
	float yscreen = yBottom + width;

	CNELU::Driver->setFrustum (0.f, 4.0f/3.0f, 0.f, 1.f, -1.f, 1.f, false);
	
	// distance between user and neighbour player
	float myPosx = LocalArea->User.pos().x;
	float myPosy = LocalArea->User.pos().y;

	// Quads size
	float radius = 0.003f;

	

	ClientCfg.TextContext.setColor(CRGBA(255,100,0));

	CRemoteEntities::const_iterator it;

	for(it=LocalArea->neighbors().begin(); it!=LocalArea->neighbors().end(); it++)
	{
		if((*it).second->type() != MEWeapon)
		{
			CVector playerPos = (*it).second->pos();

			// position of neighbour
			float posx = playerPos.x;
			float posy = playerPos.y;

			// relative position
			posx = (posx-myPosx)*width/maxDistance;
			posy = (posy-myPosy)*width/maxDistance;

			float dist = (float) sqrt((posx*posx)+(posy*posy));

			// Display a quad to show a player
			float an;
			float az;
			float x;
			float y;
			az = LocalArea->User.angleAroundZ();
			if(posx==0)
			{
				if(posy==0)
				{
					x = xscreen;
					y = yscreen;
				}
				else
				{
					if(posy>0)
					{
						x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(Pi-az));
					}
					else
					{
						x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(-az));
					}
				}
			}
			else
			{
				an = (float) atan(posy/posx);
				if(posx<0) an = an + (float)Pi;
				x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
				y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
			}

			// Players out of range are not displayed
			if(x<xLeft || x>xRight || y>yTop || y<yBottom) continue;

			

			ClientCfg.TextContext.setColor(CRGBA(255,100,0));

			CDRU::drawQuad(x-radius,y-radius,
							x+radius,y+radius,
							*CNELU::Driver,CRGBA(255,100,0),CNELU::Scene.getViewport());
		}
	}


	// display particular places
	vector<pair<CVector,string> >::iterator itpp;
	for(itpp = ParticularPlaces.begin(); itpp != ParticularPlaces.end(); itpp++)
	{
		// relative position
		float posx = ((*itpp).first.x-myPosx)*width/maxDistance;
		float posy = ((*itpp).first.y-myPosy)*width/maxDistance;
		
		float dist = (float) sqrt((posx*posx)+(posy*posy));

		// Display a quad to show a player
		float an;
		float az;
		float x;
		float y;
		az = LocalArea->User.angleAroundZ();
		if(posx==0)
		{
			if(posy==0)
			{
				x = xscreen;
				y = yscreen;
			}
			else
			{
				if(posy>0)
				{
					x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(Pi-az));
				}
				else
				{
					x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(-az));
				}
			}
		}
		else
		{
			an = (float) atan(posy/posx);
			if(posx<0) an = an + (float)Pi;
			x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
			y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
		}


		if(x<xLeft || x>xRight || y>yTop || y<yBottom) 
		{
			continue;
		}

		CDRU::drawTriangle(x-radius,y-radius, 
							x+radius,y-radius, 
							x,y+radius, 
							*CNELU::Driver,
							CRGBA(0,0,255),
							CNELU::Scene.getViewport());

	}

}



/*********************************************************\
					displayInvadesScanner()
\*********************************************************/
void displayInvadesScanner()
{
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	
	// Background
	CDRU::drawQuad(0,0,1,1,*CNELU::Driver,CRGBA(255,0,0,50),CNELU::Scene.getViewport());

	// Black border
	//=============
	// v-left
	CDRU::drawQuad(0,0,0.1f,1,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());
	// v-right
	CDRU::drawQuad(1-0.1f,0,1,1,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());
	// h-up
	CDRU::drawQuad(0,0.9f,1,1,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());
	// h-down
	CDRU::drawQuad(0,0,1,0.1f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());

	// Black wired quad
	//=================
	/*
	// v-left
	CDRU::drawLine(0.2f,0.2f,0.2f,0.8f ,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());	
	// v-right
	CDRU::drawLine(0.8f,0.2f,0.8f,0.8f ,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());	
	// h-up
	CDRU::drawLine(0.2f,0.8f,0.8f,0.8f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());	
	// h-bottom
	CDRU::drawLine(0.2f,0.2f,0.8f,0.2f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());
	*/
	CDRU::drawWiredQuad(0.5f,0.5f,0.3f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());

	// Black triangles
	//================
	// h-left
	CDRU::drawTriangle(0.17f,0.48f, 0.27f,0.5f, 0.17f,0.52f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());
	// h-right
	CDRU::drawTriangle(0.73f,0.5f, 0.83f,0.48f, 0.83f,0.52f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene.getViewport());

	// Red sight
	//==========
	// h
	CDRU::drawLine(0.48f,0.5f,0.52f,0.5f ,*CNELU::Driver,CRGBA(255,0,0,100),CNELU::Scene.getViewport());	
	// v
	CDRU::drawLine(0.5f,0.52f,0.5f,0.48f ,*CNELU::Driver,CRGBA(255,0,0,100),CNELU::Scene.getViewport());	

	CNELU::Driver->setFrustum (0.f, 4.0f/3.0f, 0.f, 1.f, -1.f, 1.f, false);
}




/*********************************************************\
					displayOrientation()
\*********************************************************/
void displayOrientation()
{
	float x = 0.9f*4.f/3.f;
	float y = 0.1f;
	float radius = 0.015f;

	// Triangle
	CMaterial mat;
	mat.initUnlit();
	mat.setSrcBlend(CMaterial::srcalpha);
	mat.setDstBlend(CMaterial::invsrcalpha);
	mat.setBlend(true);
	
	CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ);
	vb.setNumVertices (7);
	
	// tri
	vb.setVertexCoord (0, CVector (-radius, 0, 0));
	vb.setVertexCoord (1, CVector (radius, 0, 0));
	vb.setVertexCoord (2, CVector (0, 0, 3*radius));

	// quad
	vb.setVertexCoord (3, CVector (-radius, 0, -radius));
	vb.setVertexCoord (4, CVector (radius, 0, -radius));
	vb.setVertexCoord (5, CVector (radius, 0, radius));
	vb.setVertexCoord (6, CVector (-radius, 0, radius));
	
	CNELU::Driver->activeVertexBuffer(vb);

	CPrimitiveBlock pbTri;
	pbTri.setNumTri (1);
	pbTri.setTri (0, 0, 1, 2);
	
	CPrimitiveBlock pbQuad;
	pbQuad.setNumQuad (1);
	pbQuad.setQuad (0, 3, 4, 5, 6);
	

	CNELU::Driver->setFrustum (0.f, 4.f/3.f, 0.f, 1.f, -1.f, 1.f, false);
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewMatrix (mtx);

	mat.setColor(CRGBA(50,255,255,150));

	// up
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(LocalArea->User.angleAroundZ() - (float)Pi/2);
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeVertexBuffer(vb);
	CNELU::Driver->render(pbTri, mat);

	mat.setColor(CRGBA(50,50,255,150));

	// down
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(LocalArea->User.angleAroundZ() + (float)Pi/2);
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeVertexBuffer(vb);
	CNELU::Driver->render(pbTri, mat);

	// left
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(LocalArea->User.angleAroundZ());
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeVertexBuffer(vb);
	CNELU::Driver->render(pbTri, mat);

	// right
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(LocalArea->User.angleAroundZ() - (float)Pi);
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeVertexBuffer(vb);
	CNELU::Driver->render(pbTri, mat);

	// center
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(LocalArea->User.angleAroundZ());
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeVertexBuffer(vb);
	CNELU::Driver->render(pbQuad, mat);
	
}


/*********************************************************\
					setupLensFlare()
\*********************************************************/
CLensFlare * createLensFlare()
{
	CVector sunVector = 100000*CVector(0.075f, -1.0f, 0.25f);
	CLensFlare * lensFlare = new CLensFlare(CNELU::Driver,CNELU::Camera,sunVector);

	ITexture * flareTexture1 = new CTextureFile("flare01.tga");
	ITexture * flareTexture3 = new CTextureFile("flare03.tga");
	ITexture * flareTexture4 = new CTextureFile("flare04.tga");
	ITexture * flareTexture5 = new CTextureFile("flare05.tga");
	ITexture * flareTexture6 = new CTextureFile("flare06.tga");
	ITexture * flareTexture7 = new CTextureFile("flare07.tga");
	
	float w = 30/800.0f;
	float h = 30/600.0f;

	// shine
	lensFlare->addFlare( flareTexture3, w, h, 1.f, 16.f);

	lensFlare->addFlare( flareTexture1, w, h, 1.f, 6.f );
	lensFlare->addFlare( flareTexture6, w, h, 1.3f, 1.2f );
	lensFlare->addFlare( flareTexture7, w, h, 1.0f, 3.f );
	lensFlare->addFlare( flareTexture6, w, h, 0.5f, 4.f );
	lensFlare->addFlare( flareTexture5, w, h, 0.2f, 2.f );
	lensFlare->addFlare( flareTexture7, w, h, 0.0f, 0.8f );
	lensFlare->addFlare( flareTexture7, w, h, -0.25f, 2.f );
	lensFlare->addFlare( flareTexture1, w, h, -0.4f, 1.f );
	lensFlare->addFlare( flareTexture4, w, h, -1.0f, 12.f );
	lensFlare->addFlare( flareTexture5, w, h, -0.6f, 6.f );

	return lensFlare;
}


/*********************************************************\
					displayDazzle()
\*********************************************************/
void displayDazzle()
{
	// vector to sun
	//==============
	CVector sunVector = CVector(0.075f, -1.0f, 0.25f);

	// look's vector
	//==============
	CVector v2 = LocalArea->User.bodyHeading();
	CVector v1 = v2 ^ CVector(0,0,1);
	CVector v3 = v1 ^ v2;
	CMatrix viewmatrix;
	viewmatrix.identity();
	viewmatrix.setRot( v1, v2, v3, true );
	viewmatrix.rotateX( LocalArea->User.ViewPitch );
	CVector userLook = viewmatrix.getJ();

	// cosinus between the two previous vectors
	//=========================================
	float cosAngle = sunVector*userLook/sunVector.norm();	
	
	// alpha
	//======
	float alphaf;
	if(cosAngle<0) 
	{
		alphaf = 0;
	}
	else
	{
		alphaf = 255*(float)(pow(cosAngle,20));
	}
	
	// landscape's masking sun ?
	//==========================
	CMatrix camMatrix;
	camMatrix = CNELU::Camera->getMatrix();
	camMatrix.setPos(CVector::Null);
	camMatrix.invert();
	CVector tmp = camMatrix * sunVector;
	tmp = CNELU::Camera->getFrustum().project(tmp);
	uint32	w,h;
	CNELU::Driver->getWindowSize(w,h);
	float sunRadius = 24;
	CRect rect((uint32)(tmp.x*w)-(uint32)sunRadius,(uint32)(tmp.y*h)-(uint32)sunRadius,2*(uint32)sunRadius,2*(uint32)sunRadius);
	vector<float> zbuff;
	CNELU::Driver->getZBufferPart(zbuff, rect);
	float view = 0.f;
	float sum = 0;
	sint i;
	for(i=0; i<(sint)zbuff.size(); i++)
	{
		if(zbuff[i]>=0.99999f) sum ++;
	}
	view = sum/(sunRadius*2*sunRadius*2);

	// quad for dazzle 
	//================
	uint8 alpha = (uint8)(alphaf*view/2.0f);
	if(alpha!=0)
	{
		CDRU::drawQuad(0,0,1,1,*CNELU::Driver,CRGBA(255,255,255,alpha),CNELU::Scene.getViewport());
	}

	// Display lens-flare
	LensFlare->setAlphaCoef( 1.f - (float)cos(alphaf*view*Pi/(2.f*255.f)) );
	LensFlare->show();
}



/*********************************************************\
					displayScores()
\*********************************************************/
void displayScores()
{
	uint columnMaxH = 40;
	uint nbPlayers = ScoresMap.size();
	uint nbColumn;
	if(nbPlayers%columnMaxH != 0)
	{
		nbColumn = nbPlayers/columnMaxH + 1;
	}
	else
	{
		nbColumn = nbPlayers/columnMaxH;
	}
	

	// Determining optimal font size
	const uint fontSizeMin = 10;
	const uint fontSizeMax = 20;
	uint fontSize = fontSizeMax;
	do
	{
		float h = nbPlayers*0.04f*fontSize/20.f;
		if(h<=0.9f)
		{
			break;
		}
		else
			fontSize--;
	}
	while(fontSize>fontSizeMin);

	// Text properties
	ClientCfg.TextContext.setColor(CRGBA(250,200,60));
	ClientCfg.TextContext.setFontSize(fontSize);
	ClientCfg.TextContext.setHotSpot(CComputedString::MiddleMiddle);

	// Horizontal space between two columns
	float interH = 0.04f;
	
	// Vertical space between two print locations
	float interV = 0.04f * fontSize / 20.f;

	// Background dimensions
	float backW = 0.3f * fontSize / 20.f;
	float backH; 
	
	float columnEltCountf;

	if(nbPlayers>columnMaxH)
	{
		backH = (columnMaxH + 1) * interV;
		columnEltCountf = (float)columnMaxH;
	}
	else
	{
		backH = (nbPlayers + 1) * interV;
		columnEltCountf = (float)nbPlayers;
	}

	float xcenter = 0.5f;
	float ycenter = 0.5f;

	sint i;
	for(i=0; i<(sint)nbColumn; i++)
	{
		float midColumnCountf = (float)nbColumn/2;
		uint midColumnCounti = nbColumn/2;

		float x0 = xcenter + (i - midColumnCountf)*backW - midColumnCounti*interH;
		float y0 = ycenter - columnEltCountf/2*interV; 
		CDRU::drawQuad(x0,y0,x0+backW,y0+backH,*CNELU::Driver,CRGBA(128,128,128,200),CNELU::Scene.getViewport());
	}

	ClientCfg.TextContext.setShaded(true);
 	i = 0;
	map<TEntityId,sint32>::iterator it = ScoresMap.begin();
	while(it!=ScoresMap.end())
	{
 		TEntityId id = (*it).first;
 		sint score = (*it).second;
 		const string * s = LocalArea->nameFromId(id);
		string name;
		if ( s == NULL )
		{
			name = "*NEW*"; // to localize
		}
		else
		{
			name = *s;
		}

		uint column = i/columnMaxH;
		float midColumnCountf = (float)nbColumn/2;
		uint midColumnCounti = nbColumn/2;
	
		float x = xcenter + backW/2 + (column - midColumnCountf)*backW - midColumnCounti*interH;
		float y = ycenter + (columnEltCountf/2 - i%columnMaxH)*interV;
		
		ClientCfg.TextContext.printfAt(x,y,"%s - %d pts",name.c_str(),score);
		
		i++;
		it++;
	}
	ClientCfg.TextContext.setShaded(false);

}



/*********************************************************\
					displayOtherPlayersName()
\*********************************************************/
void displayOtherPlayersName()
{
	ClientCfg.TextContext.setHotSpot(CComputedString::MiddleMiddle);
	ClientCfg.TextContext.setColor(CRGBA(255,0,0));
	ClientCfg.TextContext.setFontSize(20);

	CRemoteEntities::const_iterator it;

	for(it=LocalArea->neighbors().begin(); it!=LocalArea->neighbors().end(); it++)
	{
		if((*it).second->type() != MEWeapon)
		{
			if(LocalArea->inRadius( (*it).second->pos() ) )
			{
				CComputedString computedStr;
				ClientCfg.TextContext.computeString((*it).second->name(),computedStr);
				
				CMatrix matrix;
				CVector namePos = (*it).second->pos();
				namePos.z += 1.5f*EyesHeight[(*it).second->type()].z;
				matrix.setPos( namePos );
				CVector v2 = LocalArea->User.bodyHeading();
				CVector v1 = v2 ^ CVector(0,0,1);
				CVector v3 = v1 ^ v2;
				matrix.setRot( v1, v2, v3, true );
				matrix.scale(10.0f);
							
				computedStr.render3D(*CNELU::Driver,matrix);
			}
		}
	}
}


/*
 * setCamera
 */
void setCamera()
{
	CVector v2 = LocalArea->User.bodyHeading();
	CVector v1 = v2 ^ CVector(0,0,1);
	CVector v3 = v1 ^ v2;
	CMatrix viewmatrix;
	viewmatrix.identity();
	viewmatrix.setPos( LocalArea->User.pos() + EyesHeight[LocalArea->User.type()] );
	viewmatrix.setRot( v1, v2, v3, true );
	viewmatrix.rotateX( LocalArea->User.ViewPitch );
	viewmatrix.rotateY( LocalArea->User.ViewRoll );
	CNELU::Camera->setMatrix( viewmatrix );
}


/*********************************************************\
					displayZone()
\*********************************************************/
void displayZone(char **zones, uint32 nbzones)
{
	nlinfo( "Loading data..." );

	LensFlare = createLensFlare();

	Landscape = (CLandscapeModel*)CNELU::Scene.createModel(LandscapeModelId);

	// Init TileBank.
	CIFile bankFile(CPath::lookup("bank.bank"));
	Landscape->Landscape.TileBank.serial(bankFile);
	Landscape->Landscape.TileBank.makeAllPathRelative();
	// Use DDS!!!
	Landscape->Landscape.TileBank.makeAllExtensionDDS();
	CIFile farbankFile(CPath::lookup("bank.farbank"));
	Landscape->Landscape.TileFarBank.serial(farbankFile);
	if ( ! Landscape->Landscape.initTileBanks() )
	{
		nlwarning( "You need to recompute bank.farbank for the far textures" );
	}
	bankFile.close();
	farbankFile.close();
 
	// Init light color
	CRGBA diffuse (241, 226, 244);
	CRGBA ambiant  (17, 54, 100);
	Landscape->Landscape.setupStaticLight (diffuse, ambiant, 1.1f);

	// Preload of TileBank
	for (int ts=0; ts<Landscape->Landscape.TileBank.getTileSetCount (); ts++)
	{
		CTileSet *tileSet=Landscape->Landscape.TileBank.getTileSet (ts);
		sint tl;
		for (tl=0; tl<tileSet->getNumTile128(); tl++)
			Landscape->Landscape.flushTiles (CNELU::Scene.getDriver(), (uint16)tileSet->getTile128(tl), 1);
		for (tl=0; tl<tileSet->getNumTile256(); tl++)
			Landscape->Landscape.flushTiles (CNELU::Scene.getDriver(), (uint16)tileSet->getTile256(tl), 1);
		for (tl=0; tl<CTileSet::count; tl++)
			Landscape->Landscape.flushTiles (CNELU::Scene.getDriver(), (uint16)tileSet->getTransition(tl)->getTile (), 1);
	}

	// Init collision Manager.
	CollisionManager.init( &(Landscape->Landscape), 200);

	// Build zones.
#ifndef THREAD_ZONE_LOADING
	for(uint32 i =0; i<nbzones; i++)
	{
		CZone zone;
		CIFile file(CPath::lookup(zones[i]));
		zone.serial(file);
		file.close();

		// Add it to landscape.
		Landscape->Landscape.addZone(zone);

		// Add it to collision manager.
		CollisionManager.addZone(zone.getZoneId());
	}
#else	
	// Load all zone in area for init
	ZoneManager.loadAllZonesAround((uint)ClientCfg.Position.x, (uint)(-ClientCfg.Position.y), 1500, true);
	while(ZoneManager.getTaskListSize() != 0)
	{
		if(!ZoneManager.ZoneAdded)
		{
			Landscape->Landscape.addZone(*ZoneManager.Zone);
			CollisionManager.addZone(ZoneManager.Zone->getZoneId());
			delete ZoneManager.Zone;
			ZoneManager.ZoneAdded = true;
		}
		else
		{
			Sleep(0);
		}
	}
#endif	

	// Yoyo: check zone binds.
	Landscape->Landscape.checkBinds();

	Landscape->Landscape.setTileNear(ClientCfg.LandscapeTileNear);
	Landscape->Landscape.setThreshold(ClientCfg.LandscapeThreshold);

	// Start collision manager. Must do it first.
	CollisionManager.setCenter(ClientCfg.Position);

	// Snap user to ground.
	float snaphup;
	if ( ClientCfg.Position.z == -1234.0f ) // user bidouillage in config file 
	{
		ClientCfg.Position.z = 0.0f;
		snaphup = 1000.0f;
	}
	else
	{
		snaphup = 0.5f;
	}
	CollisionManager.snapToGround( ClientCfg.Position, snaphup );

	// Setup local area
	LocalArea = new CLocalArea( ClientSocket, ClientCfg.Position, ClientCfg.Heading );
	LocalArea->User.setName( ClientCfg.Login );
	LocalArea->User.setType( ClientCfg.CharacterIndex );
	LocalArea->setNewEntityCallback( createNewEntityInstance );
	LocalArea->setEntityMovedCallback( moveEntityInstance );
	LocalArea->setEntityRemovedCallback( deleteEntityInstance );

	// hide mouse cursor
	CNELU::Driver->showCursor(false);
#ifdef NL_RELEASE
	CNELU::Driver->setCapture(true);
#endif



	// Load meshes
	/*
	vector<string>::iterator itshp;
	for(itshp = EntityShapes.begin(); itshp != EntityShapes.end(); itshp++)
	{
		CShapeStream entity_mesh;
		CIFile meshfile(CPath::lookup(*itshp));
		meshfile.serial( entity_mesh );
		meshfile.close();
		CNELU::Scene.addShape( *itshp, entity_mesh.getShapePointer() );
	}
	for(itshp = WeaponShapes.begin(); itshp != WeaponShapes.end(); itshp++)
	{
		CShapeStream weapon_mesh;
		CIFile meshfile(CPath::lookup(*itshp));
		meshfile.serial( weapon_mesh );
		meshfile.close();
		CNELU::Scene.addShape( "weapon", weapon_mesh.getShapePointer() );
	}
	*/


	/*// String for moving mode
	ClientCfg.TextContext.setColor(CRGBA(255,0,0));
	ClientCfg.TextContext.setFontSize(12);
	ClientCfg.TextContext.textPush("Free look mode");
	ClientCfg.TextContext.textPush("Walk mode");*/
		
	// Events management
	CNELU::EventServer.addEmitter(CNELU::Driver->getEventEmitter());
	CNELU::AsyncListener.addToServer(CNELU::EventServer);
	CNELU::EventServer.addListener( EventCharId, &ChatListener );

	MoveListener.init(&CNELU::Scene, ClientCfg.Width, ClientCfg.Height);
	MoveListener.addToServer(CNELU::EventServer);
	

	// Init camera
//	CNELU::Camera->setFrustum(0.120f, 0.090f, 0.1f, 1000.0f);
	//CNELU::Camera->setPerspective (float(80.0*Pi/180.0), 1.33f, 0.1f, 1000.0f);
	

	// Init sky's scene and sky's camera.
	CScene skyScene;
	CShapeBank skyShapeBank;
	skyScene.setShapeBank(&skyShapeBank);
	skyScene.initDefaultTravs();
	skyScene.initDefaultRoots();
	skyScene.setDriver(CNELU::Driver);
	CNELU::Scene.setViewport (CNELU::Scene.getViewport());
	CCamera * skyCamera = (CCamera*)skyScene.createModel(NL3D::CameraId);
	skyCamera->setTransformMode(CTransform::DirectMatrix);
	skyScene.setCam(skyCamera);
	
	// Init the sky shape.
	CTransformShape * skyShape= skyScene.createInstance( "sky_pingoo.shape" );
	skyShape->setTransformMode(CTransform::DirectMatrix);


	bool showInfos = true;
	nlinfo( "Data loaded." );

	// Init local area
	LocalArea->init();

	uint8 radar = 0;
	uint radarDistance = 500;
	uint radarMinDistance = 100;
	uint radarMaxDistance = 1000;

	bool flash = false;
	sint64 flashTime = 0;
	sint64 flashDuration = 3000;
	float flashScaleAmplitude = (float)flashDuration/(255.f*255.f);


	// Period when an hit player has the shape _fighted
	sint64 hitDuration = 1000;

	ClientCfg.TextContext.setFontSize(20);
	
	ClientCfg.TextContext.setColor(CRGBA(255,0,0));
	uint32 CantShotTextIndex = ClientCfg.TextContext.textPush (CI18N::get("Reloading snowball"));
	ClientCfg.TextContext.setColor(CRGBA(0,0,0));
	ClientCfg.TextContext.textPush (CI18N::get("Reloading snowball"));
	
	ClientCfg.TextContext.setColor(CRGBA(0,255,0));
	uint32 CanShotTextIndex = ClientCfg.TextContext.textPush (CI18N::get("Snowball loaded"));
	ClientCfg.TextContext.setColor(CRGBA(0,0,0));
	ClientCfg.TextContext.textPush (CI18N::get("Snowball loaded"));


	// Setup fog.
	CNELU::Driver->setupFog(800,1000, CRGBA(80,77,118,255));


	// Debug
	//InfoLog->addDisplayer( &ChatDisplayer );

	// connect the command execution to the chat displayer
	CommandExecutionLog.addDisplayer (&ChatDisplayer);

	float zoomAngle = 80.0f;


	do
	{
		// Time mgt.
		//==========
		static sint64 t0 = (sint64)CTime::getLocalTime();
		static sint64 t1 = (sint64)CTime::getLocalTime();
		static sint64 ts = 0;

		t0 = t1;
		t1 = (sint64)CTime::getLocalTime();
		sint64 dt64 = t1-t0;
		ts += dt64;
		float	dt= ((float)dt64)*0.001f;

		
		// Hit players flash
		//==================
		map<TEntityId,sint64>::iterator ism;
		for ( ism=ShotMap.begin(); ism!=ShotMap.end(); ++ism )
		{
			// if this player has been shot
			if(ShotMap[(*ism).first]>0) 
			{
				// we change hide/show only for visible players
				CVector posP = LocalArea->neighbors()[(*ism).first]->pos();
				if ( LocalArea->inRadius( posP ) )
				{
					// switch show/hide state
					uint shapeIndx =  ClientEntities[(*ism).first]->CurrentShapeIndex;
					if(ClientEntities[(*ism).first]->ShowShapes[shapeIndx])
					{
						ClientEntities[(*ism).first]->Shapes[shapeIndx]->show();
					}
					else
					{
						ClientEntities[(*ism).first]->Shapes[shapeIndx]->hide();
					}
					ClientEntities[(*ism).first]->ShowShapes[shapeIndx] = !ClientEntities[(*ism).first]->ShowShapes[shapeIndx];	
				}
				
				// decrease time
				ShotMap[(*ism).first] -= (t1- t0);
				uint _dt = (uint)(t1-t0);

				// if invincibility duration finished
				if(ShotMap[(*ism).first]<=0)
				{
					uint shapeIndx =  ClientEntities[(*ism).first]->CurrentShapeIndex;
					ClientEntities[(*ism).first]->ShowShapes[shapeIndx] = true;
				}
				
				// 
				if((sint64)InvincibilityDuration-ShotMap[(*ism).first]>hitDuration
					&& ClientEntities[(*ism).first]->CurrentShapeIndex!=0)
				{
					ClientEntities[(*ism).first]->Shapes[1]->hide();
					ClientEntities[(*ism).first]->ShowShapes[1] = false;
					ClientEntities[(*ism).first]->CurrentShapeIndex = 0;
					ClientEntities[(*ism).first]->Shapes[0]->show();
					ClientEntities[(*ism).first]->ShowShapes[0] = true;
					
				}
			}
		}

		// Remote entities too far are not displayed + snapping
		//=====================================================
		CRemoteEntities::iterator ire;
		for(ire=LocalArea->neighbors().begin(); ire!=LocalArea->neighbors().end(); ire++)
		{
			map<TEntityId,CClientEntity*>::iterator imc;
			imc = ClientEntities.find( (*ire).second->id() );
			if ( imc != ClientEntities.end() )
			{
				CClientEntity *ce = (*imc).second;

				// current shape of this entity is shown or hidden
				uint shapeIndx =  ce->CurrentShapeIndex;
				if ( LocalArea->inRadius( (*ire).second->pos() ) && ce->ShowShapes[shapeIndx])
				{
					ce->Shapes[shapeIndx]->show();
				}
				else
				{
					ce->Shapes[shapeIndx]->hide();
				}

				// 1st snapping
				if ( (!ce->SnappedToGround) && LocalArea->inRadius( ce->Entity->pos()) )
				{
					CVector pos = ce->Entity->pos();
					CollisionManager.snapToGround( pos );
					ce->Entity->setPos( pos );
					ce->SnappedToGround = true;
					nldebug( "1st snapping for entity %u", ce->Entity->id() );
				}
			}
		}



		// User input, updating entities and collision detection
		//MoveListener.setLocalTime(CTime::getLocalTime());
		CVector oldpos = LocalArea->User.pos();
		MoveListener.processInput();
		LocalArea->update();
		CVector pos = LocalArea->User.pos();
		CollisionManager.testMove( oldpos, pos );
		LocalArea->User.commitPos( pos );
		setCamera();

		// Update collision.
		CollisionManager.setCenter(LocalArea->User.pos());


		CNELU::EventServer.pump();



		// Zoom
		//=====
		if(CNELU::AsyncListener.isKeyDown(KeyNEXT))
		{
			zoomAngle += 5.f;
		}
		if(CNELU::AsyncListener.isKeyDown(KeyPRIOR))
		{
			zoomAngle -= 5.f;
		}
		if(CNELU::AsyncListener.isKeyDown(KeyHOME))
		{
			zoomAngle = 80.f;
		}
		if(zoomAngle<10.0f) 
		{
			zoomAngle = 10.0f;
		}
		if(zoomAngle>80.0f) 
		{
			zoomAngle = 80.0f;
		}
		CNELU::Camera->setPerspective (float(zoomAngle*Pi/180.0), 1.33f, 0.1f, 1000.0f);






		if(CNELU::AsyncListener.isKeyPushed(KeyF1))
		{
			showInfos = !showInfos;
		}


		if(CNELU::AsyncListener.isKeyPushed(KeyF2))
		{
			if (CNELU::AsyncListener.isKeyDown(KeySHIFT))
			{
				ChatOutput.clear();
			}
			else
			{
				ChatOutput.switchDisplay();
			}
		}

		if(CNELU::AsyncListener.isKeyPushed(KeyF3))
		{
			if (CNELU::Driver->getPolygonMode ()==IDriver::Filled)
				CNELU::Driver->setPolygonMode (IDriver::Line);
			else
				CNELU::Driver->setPolygonMode (IDriver::Filled);
		}


		// Switch between mouse move and keyboard-only move
		if(CNELU::AsyncListener.isKeyPushed(KeyF9))
			MoveListener.changeControlMode();

		// Server state dump
		if(CNELU::AsyncListener.isKeyPushed(KeyF5))
		{
			CMessage msgout( "DUMP" );
			if ( (ClientSocket != NULL) && ClientSocket->connected() )
			{
				ClientSocket->send( msgout );
			}
			DisplayClientDump();
		}

		/*// Reset player
		if(CNELU::AsyncListener.isKeyPushed(KeyR))
			ResetUser();*/

		// Receive messages		
		if (ClientSocket != NULL)
		{
			for ( uint nbupdates=0; ClientSocket->connected() && ClientSocket->update() && nbupdates<4; nbupdates++ ) // 4updates*1msg*40bytes*25fps = 4000 Bps le compte est bon
				;
		}
/*
		// User input, updating entities and collision detection
		//MoveListener.setLocalTime(CTime::getLocalTime());
		CVector oldpos = LocalArea->User.pos();
		MoveListener.processInput();
		LocalArea->update();
		CVector pos = LocalArea->User.pos();
		CollisionManager.testMove( oldpos, pos );
		LocalArea->User.commitPos( pos );
		setCamera();

		// Update collision.
		CollisionManager.setCenter(LocalArea->User.pos());
*/
		// Test collisions for shooting
		TestCollisions();

		// Roll angle for projectiles. one loop in 1 seconds
		if (FirstRollTime==0)
		{
			FirstRollTime = CTime::getLocalTime();
		}
		RollProjectile = (uint32)(((CTime::getLocalTime()-FirstRollTime)*360/1000)%360);

		/*
		playerView.target(LocalArea->User.pos()+CVector( 0, 0, 1.75 ), LocalArea->User.bodyHeading());
		if(CNELU::AsyncListener.isKeyDown(KeyNUMPAD8))
			playerView.forward();
		if(CNELU::AsyncListener.isKeyDown(KeyNUMPAD2))
			playerView.backward();
		if(CNELU::AsyncListener.isKeyDown(KeyNUMPAD4))
			playerView.rotateLeft();
		if(CNELU::AsyncListener.isKeyDown(KeyNUMPAD6))
			playerView.rotateRight();
		if(CNELU::AsyncListener.isKeyDown(KeyPRIOR))
			playerView.rotateUp();
		if(CNELU::AsyncListener.isKeyDown(KeyNEXT))
			playerView.rotateDown();
		playerView.update();
		*/

		// Check if new zone must be added to landscape
#ifdef THREAD_ZONE_LOADING
		if(!ZoneManager.ZoneAdded)
		{
			Landscape->Landscape.addZone(*ZoneManager.Zone);
			CollisionManager.addZone(ZoneManager.Zone->getZoneId());
			delete ZoneManager.Zone;
			ZoneManager.ZoneAdded = true;
		}

		// Check if a zone must be removed from landscape
		if(!ZoneManager.ZoneRemoved)
		{
			CollisionManager.addZone(ZoneManager.IdZoneToRemove);
			Landscape->Landscape.removeZone(ZoneManager.IdZoneToRemove);
			ZoneManager.ZoneRemoved = true;
		}

		// Call loadAllZoneAround for add load task for all new zone in area and remove task for all zone out area
		ZoneManager.loadAllZonesAround((uint)LocalArea->User.pos().x, (uint)(-LocalArea->User.pos().y), 1500);
#endif		
		
		if(ShakeTime>0)
		{
			hitShake();
		}
		ShakeTime -= (t1- t0);

		/*
		// shaking test
		if(CNELU::AsyncListener.isKeyDown(KeyF8))
		{
			ShakeTime = ShakeDuration;
		}
		*/
		
		/*
		if(CNELU::AsyncListener.isKeyPushed(KeyF12,false))
		{
			CNELU::clearBuffers(CRGBA(255,255,255));
			CNELU::swapBuffers();
			flash = true;
		}
		*/
		
		

		// Render
		//=======
		CNELU::clearBuffers(ClientCfg.Background);

		// Update/Render the sky, only if not Wired.
		if (CNELU::Driver->getPolygonMode ()==IDriver::Filled)
		{
			CFrustum	frust;
			frust= CNELU::Camera->getFrustum();
			skyCamera->setFrustum(frust);
			CMatrix skyCameraMatrix;
			skyCameraMatrix.identity();
			skyCameraMatrix= CNELU::Camera->getMatrix();
			skyCameraMatrix.setPos(CVector::Null);
			skyCamera->setMatrix(skyCameraMatrix);
			skyScene.render();
			//Must clear ZBuffer For incoming rendering.
			CNELU::Driver->clearZBuffer();
		}

		// render the main scene.
		CNELU::Driver->enableFog(true);
		CNELU::Scene.render();
		CNELU::Driver->enableFog(false);

		// Just after render of scene (camera setuped)
		displayOtherPlayersShadow();
		drawDummyShadow(CNELU::Camera->getMatrix().getPos()-CVector(0,0,1));

		// Vectors / Log into chat output
		if(CNELU::AsyncListener.isKeyPushed(KeyF11))
		{
			if ( CNELU::AsyncListener.isKeyDown(KeySHIFT) )
			{
				// Info and warnings
				if ( InfoLog->attached( &ChatDisplayer ) )
				{
					InfoLog->removeDisplayer( &ChatDisplayer );
					WarningLog->removeDisplayer( &ChatDisplayer );
				}
				else
				{
					InfoLog->addDisplayer( &ChatDisplayer );
					WarningLog->addDisplayer( &ChatDisplayer );
				}
			}
			else if ( CNELU::AsyncListener.isKeyDown(KeyCONTROL) )
			{
				// Debug
				if ( DebugLog->attached( &ChatDisplayer ) )
				{
					DebugLog->removeDisplayer( &ChatDisplayer );
				}
				else
				{
					DebugLog->addDisplayer( &ChatDisplayer );
				}
			}
			else
			{
				ShowVectors = ! ShowVectors;
			}
		}

		if ( ShowVectors )
		{
			// Display line for vector
			CRemoteEntities::const_iterator ire;
			for ( ire=LocalArea->neighbors().begin(); ire!=LocalArea->neighbors().end(); ++ire )
			{
				CDRU::drawLine( (*ire).second->pos()+CVector(0.0f,0.0f,1.80f), (*ire).second->pos()+(*ire).second->trajVector()+CVector(0.0f,0.0f,1.80f), CRGBA(0,64,0), *CNELU::Driver );
			}
		}

		// Display other players name
		displayOtherPlayersName();

		if(CNELU::AsyncListener.isKeyDown(KeyTAB))
		{
			displayScores();
		}

		// Radar
		if(zoomAngle==80.f)
		{
			if(CNELU::AsyncListener.isKeyPushed(KeyF6))
			{
				radar = (radar+1)%3;
			}
			if(CNELU::AsyncListener.isKeyDown(KeyF7))
			{
				radarDistance += 50;
				if(radarDistance>radarMaxDistance)
				{
					radarDistance = radarMaxDistance;
				}
			}
			if(CNELU::AsyncListener.isKeyDown(KeyF8))
			{
				radarDistance -= 50;
				if(radarDistance<radarMinDistance)
				{
					radarDistance = radarMinDistance;
				}
			}
			if(radar==1)
			{
				displayRadar(LocalArea->User.angleAroundZ(),radarDistance);
			}
			if(radar==2)
			{
				displayLittleRadar(LocalArea->User.angleAroundZ(),radarDistance);
			}
		}

		/*if(CNELU::AsyncListener.isKeyDown(KeyF11))
		{
			TPosUnit r = LocalArea->radius();
			r += 100.0f;
			if ( r > 1001.0f )
			{
				r = 100.0f;
			}
			LocalArea->setRadius( r );
			nlinfo( "Radius of local area is now %f", r );
		}*/

		if(zoomAngle!=80.f)
		{
			displayInvadesScanner();	
		}
		
				
		TTime nt = CTime::getLocalTime();
		MoveListener.CanShot = (nt - MoveListener.LastShotTime > 2000);

		if(showInfos && zoomAngle==80.f)
		{

			// Display fps.
			ClientCfg.TextContext.setHotSpot(CComputedString::BottomLeft);
			ClientCfg.TextContext.setColor(CRGBA(255,255,255));
			ClientCfg.TextContext.setFontSize(12);
			ClientCfg.TextContext.printfAt(0.225f,0.97f,"%.1f fps",1/dt);

//			ClientCfg.TextContext.setFontSize(12);
//			ClientCfg.TextContext.printfAt(0.05f,0.90f,ClientCfg.TextContext.getCacheInformation().c_str());

			// Display penalty time remaining
			displayPenaltyTime();
	
			// Display shot status
			ClientCfg.TextContext.setHotSpot(CComputedString::MiddleBottom);
			if(MoveListener.CanShot)
			{
				ClientCfg.TextContext.printAt(0.5015f,0.9485f,CanShotTextIndex+1);
				ClientCfg.TextContext.printAt(0.5f,0.95f,CanShotTextIndex);
			}
			else
			{
				ClientCfg.TextContext.printAt(0.5015f,0.9485f,CantShotTextIndex+1);
				ClientCfg.TextContext.printAt(0.5f,0.95f,CantShotTextIndex);
			}
			
			// Display player's score
			ClientCfg.TextContext.setFontSize(20);
			ClientCfg.TextContext.setHotSpot(CComputedString::BottomRight);
			ClientCfg.TextContext.setColor(CRGBA(0,0,0));
			ClientCfg.TextContext.printfAt(0.9515f,0.9485f,"%d Pts",ScoresMap[LocalArea->User.id()]);
			ClientCfg.TextContext.setColor(CRGBA(255,255,255));
			ClientCfg.TextContext.printfAt(0.95f,0.95f,"%d Pts",ScoresMap[LocalArea->User.id()]);

			// Display the cool compass.
			displayOrientation();

			// Display the gun's sight
			if(Sight && radar!=1)
			{
			  //displaySight();
			  SelectedSight.displaySight ();
			}

			// Time
			ClientCfg.TextContext.setHotSpot(CComputedString::BottomLeft);
			ClientCfg.TextContext.setColor(CRGBA(50,50,255));
			ClientCfg.TextContext.setFontSize(10);
			ClientCfg.TextContext.printfAt(0.01f,0.01f,"%s",CUniTime::getStringUniTime());

			// Position
			CVector postmp = LocalArea->User.pos();
			ClientCfg.TextContext.setHotSpot(CComputedString::MiddleBottom);
			ClientCfg.TextContext.printfAt(0.9f,0.01f,"Position : %d %d %d",(int)postmp.x,(int)postmp.y,(int)postmp.z);

			// Network rates
			static uint sibps=0;
			static uint sobps=0;
			if ( ts > 999 )
			{
				ts = ts - 1000;
				if ( (ClientSocket != NULL) && ClientSocket->connected() )
				{
					sibps = ClientSocket->newBytesReceivedFromHost();
					sobps = ClientSocket->newBytesSentToHost();
				}
			}
			ClientCfg.TextContext.setFontSize( 10 );
			ClientCfg.TextContext.setHotSpot(CComputedString::MiddleBottom);
			ClientCfg.TextContext.printfAt(0.45f,0.01f,"In: %u Bps", sibps );
			ClientCfg.TextContext.printfAt(0.55f,0.01f,"Out: %u Bps", sobps );
		

			// Display chat input message
			ClientCfg.TextContext.setHotSpot( CComputedString::BottomLeft );
			ClientCfg.TextContext.setColor( CRGBA(128,0,128) );
			ClientCfg.TextContext.setFontSize( 12 );
			ClientCfg.TextContext.printAt( 0.05f,0.05f, ucstring(ucstring(string("> "))+ChatListener.line()) );
			ChatListener.setMaxWidthReached( ClientCfg.TextContext.getLastXBound() > 1.2 ); // max is 1.33=4/3

			// Display Yoyo's profile.
			/*ClientCfg.TextContext.setColor( CRGBA(255,255,255) );
			ClientCfg.TextContext.setFontSize( 18 );
			ClientCfg.TextContext.printfAt(0.10f,0.70f,"NFaces: %d. NFar0: %d. NFar1: %d", ProfNTessFace, ProfNRdrFar0, ProfNRdrFar1 );
			ClientCfg.TextContext.printfAt(0.10f,0.67f,"NTile0: %d. NTile1: %d. NTile2: %d", ProfNRdrTile[0], ProfNRdrTile[1],  ProfNRdrTile[2]);
			ClientCfg.TextContext.printfAt(0.10f,0.64f,"NRefineFaces: %d. NRefineLeaves: %d. NNeedCompute: %d", ProfNRefineFaces, ProfNRefineLeaves, ProfNRefineComputeFaces);
			ClientCfg.TextContext.printfAt(0.10f,0.61f,"%d Splits. %d Merges.", ProfNSplits, ProfNMerges);
			{
				const	sint	NPrec= 16;
				static	float	tab[NPrec];
				static	bool	init= false;
				static	sint	curTab=0;
				sint			i;

				if(!init)
				{
					init =true;
					for(i=0;i<NPrec;i++)
						tab[i]=0;
				}
				tab[curTab]= (1000*dt);
				curTab= (curTab+1)%NPrec;
				float	f=0;
				for(i=0;i<NPrec;i++)
					f+= tab[i];
				f/=NPrec;
				ClientCfg.TextContext.printfAt(0.10f,0.58f,"%d ms", (int)f);
			}*/


			// Display chat output
			ChatOutput.renderChat();

		}

		// Dazzle due to sun
		displayDazzle();

		// display nevrax logo
		displayNevrax();	

		/*
		if(flash)
		{
			flashTime += dt64;
			if(flashTime>flashDuration)
			{
				flash = false;
				flashTime = 0;
			}
			else
			{
				uint8 alpha = (uint8) (170 + flashTime*85/flashDuration);
				CDRU::drawQuad(0,0,1,1,*CNELU::Driver,CRGBA(255,255,255,255-alpha),CNELU::Scene.getViewport());
			}
		}
		*/

		
		CNELU::swapBuffers();
		CNELU::screenshot();
		
		/*if ( TheId != 0 )
		{
			nldebug( "Player: %f %f %f", LocalArea->User.pos().x, LocalArea->User.pos().y, LocalArea->User.pos().z );
			nldebug( "W: %f %f %f", LocalArea->neighbors()[TheId]->pos().x, LocalArea->neighbors()[TheId]->pos().y, LocalArea->neighbors()[TheId]->pos().z );
		}*/
	}
	while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));

	CNELU::AsyncListener.removeFromServer(CNELU::EventServer);
	MoveListener.removeFromServer(CNELU::EventServer);
}





/*********************************************************\
						cbFire()
\*********************************************************/
// Callback
void cbFire( CMessage& msgin, TSenderId idfrom )
{
	IMovingEntity weapon;
	TEntityId idshooter;
	msgin.serial( weapon );
	msgin.serial( idshooter );

	createRemoteEntity( weapon ); // in localarea.cpp
	LocalArea->neighbors()[weapon.id()]->Tag = idshooter;
	//TheId = weapon.id();
}


/*********************************************************\
					cbChangeScores()
\*********************************************************/
// Callback
void cbChangeScores( CMessage& msgin, TSenderId idfrom )
{
	TEntityId idshot, idshooter;
	msgin.serial( idshot );
	msgin.serial( idshooter );
	if ( idshooter != 0 )
	{
		ScoresMap[idshooter]++;
		ShotMap[idshot] = InvincibilityDuration;
				
		// Player hit, so we change shape
		uint shapeIndx =  ClientEntities[idshot]->CurrentShapeIndex;
		ClientEntities[idshot]->ShowShapes[shapeIndx] = false;
		ClientEntities[idshot]->Shapes[shapeIndx]->hide();
		ClientEntities[idshot]->CurrentShapeIndex = 1;
		ClientEntities[idshot]->ShowShapes[1] = true;
		ClientEntities[idshot]->Shapes[1]->show();
		
		nlinfo( "%s shot %s", LocalArea->nameFromId(idshooter)->c_str(), LocalArea->nameFromId(idshot)->c_str() );
		if ( idshooter == LocalArea->User.id() )
		{
			ucstring str = CI18N::get("You shot ");
			str += LocalArea->nameFromId(idshot)->c_str();
			ChatOutput.addLine( str, 0 );
			nlinfo( "Your score is now %d", ScoresMap[idshooter] );
		}


	}

}


/*********************************************************\
					cbCreateScores()
\*********************************************************/
void cbCreateScores( CMessage& msgin, TSenderId idfrom )
{
	msgin.serialMap( ScoresMap );

	/*map<TEntityId,sint32>::iterator ism;
	for ( ism=ScoresMap.begin(); ism!=ScoresMap.end(); ++ism )
	{
		nldebug( "Score for %d: %d", (*ism).first, (*ism).second );
	}*/

	map<TEntityId,sint64>::iterator ism;
	for ( ism=ShotMap.begin(); ism!=ShotMap.end(); ++ism )
	{
		ShotMap.insert( make_pair((*ism).first,0) );
	}
}


/*********************************************************\
					cbAddNewScore()
\*********************************************************/
void cbAddNewScore( CMessage& msgin, TSenderId idfrom )
{
	TEntityId id;
	uint32 sco;
	msgin.serial( id );
	msgin.serial( sco );
	ScoresMap.insert( make_pair(id,sco) );
	ShotMap.insert( make_pair(id,0) );
}


/*********************************************************\
					DisplayClientDump()
\*********************************************************/
void DisplayClientDump()
{
	InfoLog->displayNL( "DUMP of client local information:");
	InfoLog->displayRawNL( "Player:");
	InfoLog->displayRawNL( "   Id: %u", LocalArea->User.id() );
	InfoLog->displayRawNL( "   name:     %s", LocalArea->User.name().c_str() );
	InfoLog->displayRawNL( "   position: %f, %f, %f", LocalArea->User.pos().x, LocalArea->User.pos().y, LocalArea->User.pos().z );
	InfoLog->displayRawNL( "   prev pos: %f, %f, %f", LocalArea->User.previousPos().x, LocalArea->User.previousPos().y, LocalArea->User.previousPos().z );
	InfoLog->displayRawNL( "   heading:  %d deg", (sint)(LocalArea->User.angleAroundZ()*180.0f/(float)Pi) );
	InfoLog->displayRawNL( "   velocity: %f", LocalArea->User.trajVector().norm() );
	CRemoteEntities::const_iterator ire;
	for ( ire=LocalArea->neighbors().begin(); ire!=LocalArea->neighbors().end(); ++ire )
	{
		InfoLog->displayRaw( "Entity %u", (*ire).second->id() );
		switch ( (*ire).second->type() )
		{
			case MEPingoo:
			case MEGnu:
					InfoLog->displayRawNL( " (player)" ); break;
			case MEWeapon:	InfoLog->displayRawNL( " (weapon)" ); break;
			default:	InfoLog->displayRawNL( " (unknown)" );
		}
		InfoLog->displayRawNL( "          name:     %s", (*ire).second->name().c_str() );
		InfoLog->displayRawNL( "          position: %f, %f, %f", (*ire).second->pos().x, (*ire).second->pos().y, (*ire).second->pos().z );
		InfoLog->displayRawNL( "          prev pos: %f, %f, %f", (*ire).second->previousPos().x, (*ire).second->previousPos().y, (*ire).second->previousPos().z );
		InfoLog->displayRawNL( "          heading:  %d deg", (sint)((*ire).second->angleAroundZ()*180.0f/(float)Pi) );
		InfoLog->displayRawNL( "          velocity: %f", (*ire).second->trajVector().norm() );
	}
	InfoLog->displayRawNL( "Local scores:" );
	map<TEntityId,sint32>::iterator ism;
	for ( ism=ScoresMap.begin(); ism!=ScoresMap.end(); ++ism )
	{
		InfoLog->displayRawNL( "Entity %u score: %d", (*ism).first, (*ism).second );
	}
}


/*********************************************************\
					cbDisplayServerDump()
\*********************************************************/
void cbDisplayServerDump( CMessage& msgin, TSenderId idfrom )
{
	string s;
	msgin.serial( s );
	nlinfo( s.c_str() );
}


/*********************************************************\
					cbDisplayChatMsg()
\*********************************************************/
void cbDisplayChatMsg( CMessage& msgin, TSenderId idfrom )
{
	ucstring us;
	TEntityId id;
	msgin.serial( us );
	msgin.serial( id );
	ChatOutput.addLine( us, id );
	//nlinfo( us.toString().c_str() );
}


/*********************************************************\
					cbPing()
\*********************************************************/
void cbPing( CMessage& msginout, TSenderId idfrom )
{
	msginout.invert();
	msginout.setType( "PONG" );
	CMsgSocket::send( msginout, idfrom );
}


/*********************************************************\
					cbPong()
\*********************************************************/
void cbPong( CMessage& msgin, TSenderId idfrom )
{
	//nlinfo( "Ping: %u ms", (sint)((sint64)CTime::getLocalTime()-(sint64)PingTime) );
	char str[1024];
	smprintf(str,1024, "pong: %u ms", (sint)((sint64)CTime::getLocalTime()-(sint64)PingTime) );
	ChatOutput.addLine( ucstring(str), 0 );
	PingTime = 0;
}


/*********************************************************\
					cbHandleDisconnection()
\*********************************************************/
void cbHandleDisconnection( CMessage& msgin, TSenderId idfrom )
{
	ChatOutput.addLine( CI18N::get("Disconnected from the server"), 0 );
}


/****************************************************************\
							writeConfigFile
\****************************************************************/
void writeConfigFile(const char * configFileName)
{
	FILE * f = fopen(configFileName,"wt");

	if(f==NULL)
	{
		nlerror("can't open file '%s'\n",configFileName);
	}

	fprintf(f,"\n// CLIENT CONFIG FILE\n\n");
	fprintf(f,"FullScreen = %d;\n",ClientCfg.Windowed?0:1);
	fprintf(f,"Width = %d;\n",ClientCfg.Width);
	fprintf(f,"Height = %d;\n",ClientCfg.Height);
	fprintf(f,"Depth = %d;\n",ClientCfg.Depth);
	fprintf(f,"Position = { %f, %f, %f };\n", ClientCfg.Position.x,ClientCfg.Position.y,ClientCfg.Position.z);
	fprintf(f,"Heading = { %f, %f, 0.0 };\n", ClientCfg.Heading.x,ClientCfg.Heading.y);
	fprintf(f,"Background = { %d, %d, %d };\n", ClientCfg.Background.R,ClientCfg.Background.G,ClientCfg.Background.B);
	fprintf(f,"DataPath = \"%s\";\n",ClientCfg.DataPath.c_str());
	fprintf(f,"FontName = \"%s\";\n",ClientCfg.FontName.c_str());
	fprintf(f,"LanguageIndex = %d;\n",ClientCfg.LanguageIndex);
	fprintf(f,"LSHost = \"%s\";\n",ClientCfg.LSHost.c_str());
	fprintf(f,"Login = \"%s\";\n",ClientCfg.Login.c_str());
	fprintf(f,"Password = \"%s\";\n",ClientCfg.Password.c_str());
	fprintf(f,"ShardIP = \"%s\";\n",ClientCfg.ShardIP.c_str());
	fprintf(f,"CharacterIndex = %d;\n",ClientCfg.CharacterIndex);
	fprintf(f,"LandscapeTileNear = %f;\n",ClientCfg.LandscapeTileNear);
	fprintf(f,"LandscapeThreshold = %f;\n",ClientCfg.LandscapeThreshold);

	fclose(f);
}


/*******************************************************************\
    Chat command lines
\*******************************************************************/
bool commandLine (const ucstring &str)
{
	string command = "";

	if (str[0]=='/')
	{
		command = str.toString().substr(1);

		// add the string in to the chat
		ChatOutput.addLine("> " + str, 0 );

		ICommand::execute (command, CommandExecutionLog);
	}
	else
		return false;

	return true;

}




/****************************************************************\
						init()
\****************************************************************/
void init(const char * configFileName)
{
	FILE * f = fopen(configFileName,"rt");
	if(f==NULL)
	{
		nlwarning("'%s' not found, default values used", configFileName);
		writeConfigFile(configFileName);
	}
	else fclose (f);
	
	CConfigFile cf;

	cf.load(configFileName);

	CConfigFile::CVar &cvFullScreen = cf.getVar("FullScreen");
	ClientCfg.Windowed = cvFullScreen.asInt() ? false : true;
	
	CConfigFile::CVar &cvWidth = cf.getVar("Width");
	ClientCfg.Width = cvWidth.asInt();

	CConfigFile::CVar &cvHeight = cf.getVar("Height");
	ClientCfg.Height = cvHeight.asInt();

	CConfigFile::CVar &cvDepth = cf.getVar("Depth");
	ClientCfg.Depth = cvDepth.asInt();

	CConfigFile::CVar &cvPosition = cf.getVar("Position");
	nlassert(cvPosition.size()==3);
	ClientCfg.Position.x = cvPosition.asFloat(0);
	ClientCfg.Position.y = cvPosition.asFloat(1);
	ClientCfg.Position.z = cvPosition.asFloat(2);

	CConfigFile::CVar &cvHeading = cf.getVar("Heading");
	nlassert(cvHeading.size()==3);
	ClientCfg.Heading.x = cvHeading.asFloat(0);
	ClientCfg.Heading.y = cvHeading.asFloat(1);
	ClientCfg.Heading.z = cvHeading.asFloat(2);

	CConfigFile::CVar &cvBackColor = cf.getVar("Background");
	nlassert(cvBackColor.size()==3);
	ClientCfg.Background.R = cvBackColor.asInt(0);
	ClientCfg.Background.G = cvBackColor.asInt(1);
	ClientCfg.Background.B = cvBackColor.asInt(2);

	CConfigFile::CVar &cvDataPath = cf.getVar("DataPath");
	ClientCfg.DataPath = cvDataPath.asString();
	
	CConfigFile::CVar &cvFontName = cf.getVar("FontName");
	ClientCfg.FontName = cvFontName.asString();

	CConfigFile::CVar &cvLanguageIndex = cf.getVar("LanguageIndex");
	ClientCfg.LanguageIndex = cvLanguageIndex.asInt();

	CConfigFile::CVar &cvLSHost = cf.getVar("LSHost");
	ClientCfg.LSHost = cvLSHost.asString();

	CConfigFile::CVar &cvLogin = cf.getVar("Login");
	ClientCfg.Login = cvLogin.asString();

	CConfigFile::CVar &cvPassword = cf.getVar("Password");
	ClientCfg.Password = cvPassword.asString();

	CConfigFile::CVar &cvShardIP = cf.getVar("ShardIP");
	ClientCfg.ShardIP = cvShardIP.asString();

	CConfigFile::CVar &cvCharacterIndex = cf.getVar("CharacterIndex");
	ClientCfg.CharacterIndex = cvCharacterIndex.asInt();

	CConfigFile::CVar &cvLandscapeTileNear = cf.getVar("LandscapeTileNear");
	ClientCfg.LandscapeTileNear = cvLandscapeTileNear.asFloat();

	CConfigFile::CVar &cvLandscapeThreshold = cf.getVar("LandscapeThreshold");
	ClientCfg.LandscapeThreshold = cvLandscapeThreshold.asFloat();
}


TCallbackItem UserCbArray [] =
{
	{ "FIR", cbFire },
	{ "SCO", cbChangeScores },
	{ "SCM", cbCreateScores },
	{ "NSC", cbAddNewScore },
	{ "DUMP", cbDisplayServerDump },
	{ "CHAT", cbDisplayChatMsg },
	{ "PING", cbPing },
	{ "PONG", cbPong },
	{ "D", cbHandleDisconnection }
};


// display stuffs in client.log file, clearing the log file each run
CFileDisplayer fd("client.log", true);
CMsgBoxDisplayer mbd;


/****************************************************************\
							MAIN
\****************************************************************/
#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	nlinfo ("Starting Snowballs");

	enum TState
	{
		LANGUAGE,
		LOGIN,
		SHARD,
		CHARACTER
	};


	char * zonesPath[] = { "3_AC.zonel","3_AD.zonel","3_AE.zonel","3_AF.zonel",
						"3_AG.zonel","3_AH.zonel","3_AI.zonel","3_AJ.zonel",
						"3_AK.zonel","3_AL.zonel","3_AM.zonel","3_AN.zonel", 
						"3_AO.zonel","3_AP.zonel","3_AQ.zonel","3_AR.zonel", 
						"4_AC.zonel","4_AD.zonel","4_AE.zonel","4_AF.zonel",
						"4_AG.zonel","4_AH.zonel","4_AI.zonel","4_AJ.zonel",
						"4_AK.zonel","4_AL.zonel","4_AM.zonel","4_AN.zonel",
						"4_AO.zonel","4_AP.zonel","4_AQ.zonel","4_AR.zonel",
						"5_AR.zonel","5_AD.zonel","5_AE.zonel","5_AF.zonel",
						"5_AG.zonel","5_AH.zonel","5_AI.zonel","5_AJ.zonel",
						"5_AK.zonel","5_AL.zonel","5_AM.zonel","5_AN.zonel",
						"5_AO.zonel","5_AP.zonel","5_AQ.zonel","6_AR.zonel",
						"6_AJ.zonel","6_AD.zonel","6_AE.zonel","6_AF.zonel",
						"6_AG.zonel","6_AH.zonel","6_AI.zonel","6_AK.zonel",
						"6_AL.zonel","6_AM.zonel","6_AN.zonel","6_AO.zonel",
						"6_AP.zonel","6_AQ.zonel","5_AC.zonel","6_AC.zonel",
						"7_AC.zonel","7_AD.zonel","7_AE.zonel","7_AF.zonel",
						"7_AG.zonel","7_AH.zonel","7_AI.zonel","7_AJ.zonel",
						"7_AK.zonel","7_AL.zonel","7_AM.zonel","7_AN.zonel",
						"7_AO.zonel","7_AP.zonel","7_AQ.zonel","8_AJ.zonel",
						"7_AR.zonel","8_AC.zonel","8_AD.zonel","8_AE.zonel",
						"8_AF.zonel","8_AG.zonel","8_AH.zonel","8_AI.zonel",
						"8_AK.zonel","8_AL.zonel","8_AM.zonel","8_AN.zonel",
						"8_AO.zonel","8_AP.zonel","8_AQ.zonel","8_AR.zonel"
					  };
	
	ClientCfg.ZonesPath = zonesPath;
	ClientCfg.NbZonesPath = sizeof (zonesPath) / sizeof (char *);

	vector<string> EntityShapes;
	EntityShapes.push_back("Pingoo");
	EntityShapes.push_back("Gnu");

	const char * configFileName = "client.cfg";
	bool launch = true;

	// main try / catch block
	try
	{
		// must be the first thing to do if you want to have information
//		initDebug ();
		DebugLog->addDisplayer (&fd);
		ErrorLog->addDisplayer (&fd);
		WarningLog->addDisplayer (&fd);
		InfoLog->addDisplayer (&fd);
		AssertLog->addDisplayer (&fd);
#ifdef NL_RELEASE
		ErrorLog->addDisplayer (&mbd);
		AssertLog->addDisplayer (&mbd);
#endif // NL_RELEASE

		// load config file
		init(configFileName);

		ParticularPlaces.push_back(pair<CVector,string>(CVector(2300.0f, -776.0f, .0f),"Vagimount"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(770.0f, -640.0f, .0f),"Manhatan"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(1135.0f, -840.0f, .0f),"ThePill"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(2140.0f, -500.0f, .0f),"Circus"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(1033.0f, -944.0f, .0f),"End WuDoor"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(564.0f, -1033.0f, .0f),"Start WuDoor"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(2125.0f, -764.0f, .0f),"BigHole"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(2169.0f, -1258.0f, .0f),"GapCave"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(1840.0f, -970.0f, .0f),"StartPosition"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(2350.0f, -1158.0f, .0f),"BackWay"));
		ParticularPlaces.push_back(pair<CVector,string>(CVector(2440.0f, -1302.0f, .0f),"WorldRoof"));

		CPath::addSearchPath(ClientCfg.DataPath);
		CPath::addSearchPath(ClientCfg.DataPath + "maps/");
		CPath::addSearchPath(ClientCfg.DataPath + "shapes/");
		CPath::addSearchPath(ClientCfg.DataPath + "tiles/");
		CPath::addSearchPath(ClientCfg.DataPath + "zones/");
		CPath::addSearchPath(ClientCfg.DataPath + "fonts/");

		// sooo baddddddd, I know...
		string path2 = ClientCfg.FontName;
		if (path2[path2.size()-4]=='.' && path2[path2.size()-3]=='p' && path2[path2.size()-2]=='f' && path2[path2.size()-1]=='b')
		{
			path2[path2.size()-3] = 'a';
			path2[path2.size()-2] = 'f';
			path2[path2.size()-1] = 'm';
		}
		else
		{
			path2 = "";
		}

		string MyShardAddress, MyAddress;
		uint32 MyKey;

		// init driver and scene
		NL3D::CNELU::init(ClientCfg.Width, ClientCfg.Height, CViewport(), ClientCfg.Depth, ClientCfg.Windowed); 
		// set old directmatrix mode.
		NL3D::CNELU::Camera->setTransformMode(CTransform::DirectMatrix);


		// This client use landscape, and so require at least 2 textures.
		if(NL3D::CNELU::Driver->getNbTextureStages()<2)
		{
			throw NL3D::EBadDisplay("Client require at least 2 textures stages!!");
		}

		// init textcontext.
		ClientCfg.TextContext.init(CNELU::Driver, &ClientCfg.FontManager);
		ClientCfg.TextContext.setFontGenerator(CPath::lookup(ClientCfg.FontName), CPath::lookup(path2));
		ClientCfg.TextContext.setFontSize(12);
		ClientCfg.FontManager.setMaxMemory(2000000);


#ifdef NETWORK_LOGGING
		// Network logging
		CNetDisplayer *nd = new CNetDisplayer();
		if ( nd->connected() )
		{
			//DebugLog->addDisplayer( nd );
			NetLog->addDisplayer( nd );
		}
#endif

		// synchronize time with server
		CUniTime::syncUniTimeFromService();

		// user id (login & password)
		CLogUserId id;

		//shard's name list	
		vector<string> shardsAddress, shardsName;
		
		CI18N::setPath (ClientCfg.DataPath.c_str());
		
		CSocket *lscnx = NULL;

		TState clientState = LANGUAGE;
		//bool launch = false;
		//bool quit = true;
		bool quit = false;
		while(!quit)
		{
			switch(clientState)
			{
				// language choice
				case LANGUAGE:	
				{
					CLanguageInterface languageScreen(ClientCfg.Width, 
													  ClientCfg.Height, 
													  &ClientCfg.TextContext, 
													  ClientCfg.LanguageIndex);
					quit = !languageScreen.choose();
					ClientCfg.LanguageIndex = languageScreen.getLanguageIndex();
					CI18N::load(ClientCfg.LanguageIndex);
					if(quit) 
					{
						launch = false;
					}
					else
					{
						clientState = CHARACTER;
					}

					if(languageScreen.Local == true)	// TEMP !!! (local play)
					{
						launch = true;
					}
				}
				break;

				// character choice
				case CHARACTER:
				{
					CCharacterInterface characterScreen(ClientCfg.Width, 
														ClientCfg.Height, 
														&ClientCfg.TextContext,
														EntityShapes,
														ClientCfg.CharacterIndex);
					// the shard interface destroy the connection
					if(characterScreen.choose())
					{
						ClientCfg.CharacterIndex = characterScreen.getCharacterIndex();
						clientState = LOGIN;
					}
					else
					{
						clientState = LANGUAGE;
					}
				}
				break;

				// get user's login and password
				case LOGIN: 
				{
					CLoginInterface logscreen(ClientCfg.Width, 
											  ClientCfg.Height, 
											  &ClientCfg.TextContext,
											  ClientCfg.LSHost,
											  ClientCfg.Login, 
											  ClientCfg.Password);
					if(logscreen.log(lscnx, shardsAddress, shardsName))
						clientState = SHARD;
					else
					{
						clientState = CHARACTER;
					}
					id = logscreen.getUserId();
					ClientCfg.Login = id.Login;
					ClientCfg.Password = id.Password;
					
				}
				break;
								
				// shard choice
				case SHARD:
				{
					CShardsListInterface shardListScreen(ClientCfg.Width, 
														 ClientCfg.Height, 
														 &ClientCfg.TextContext, 
														 shardsAddress, 
														 shardsName, 
														 ClientCfg.ShardIP);
					// the shard interface destroy the connection
					if(shardListScreen.choose(lscnx, MyShardAddress, MyAddress, MyKey))
					{
						quit = true;
					}
					else
					{
						clientState = LOGIN;
					}
					ClientCfg.ShardIP = shardListScreen.getSelectedShardIP();	
				}
				break;
			}
		}

		if (launch)
		{
			// Connect to the shard
			nlinfo( "Connecting to the shard...");
			ClientSocket = new CMsgSocket( UserCbArray, 
				sizeof(UserCbArray)/sizeof(TCallbackItem), CInetAddress(ClientCfg.ShardIP,50010) ); // using fixed port for now
			ClientSocket->setTimeout( 0 );

			// Authentification
			CMessage msgout( "AUT" );
			msgout.serial( MyAddress );
			msgout.serial( MyKey );
			ClientSocket->send( msgout );
		}

		// Debug
//		DebugLog->addDisplayer( new CStdDisplayer() );

		if (launch)
		{
			// Start the simulation
			displayZone(ClientCfg.ZonesPath, ClientCfg.NbZonesPath);
		}
	}
	catch( EFatalError &)
	{
		// nothing to do
	}
	catch( Exception &e )
	{
		try
		{
			nlerror ("Error in the client: %s", e.what ());
		}
		catch( EFatalError &)
		{
			// nothing to do
		}
	}

	// release try / catch block
	try
	{
		if (launch)
		{
			// Detach logs from chat displayer
			if ( DebugLog->attached( &ChatDisplayer ) )
			{
				DebugLog->removeDisplayer( &ChatDisplayer );
			}
			if ( InfoLog->attached( &ChatDisplayer ) )
			{
				InfoLog->removeDisplayer( &ChatDisplayer );
				WarningLog->removeDisplayer( &ChatDisplayer );
			}

			if ( LocalArea != NULL )
			{
				ClientCfg.Position = LocalArea->User.pos ();
				ClientCfg.Heading = LocalArea->User.bodyHeading ();
			}

			if (CNELU::Driver != NULL)
			{
				CNELU::Driver->showCursor(true);
#ifdef NL_RELEASE
				CNELU::Driver->setCapture(false);
#endif
			}
		}

		// release video
		NL3D::CNELU::release();
		
		// store use information
		writeConfigFile(configFileName);

		// release network stuffs
		if ( LocalArea != NULL )
		{
			delete LocalArea;
		}
	
		if ( ClientSocket != NULL )
		{
			delete ClientSocket;
		}
	}
	catch( Exception &e )
	{
		try
		{
			nlerror( "Error when releasing the client: %s", e.what () );
		}
		catch( EFatalError &)
		{
			// nothing to do
		}
	
	}

	return EXIT_SUCCESS;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// IN GAME COMMAND ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

NLMISC_DYNVARIABLE(float,tilenear,"landscape tile near")
{
	if (get)
		*pointer = Landscape->Landscape.getTileNear();
	else
		Landscape->Landscape.setTileNear(*pointer);
}



NLMISC_DYNVARIABLE(float,threshold,"landscape threshold")
{
	if (get)
		*pointer = Landscape->Landscape.getThreshold();
	else
		Landscape->Landscape.setThreshold(*pointer);
}


NLMISC_COMMAND(boost,"switch landscape parameters between high speed and high quality","0|1")
{
	if (args.size() != 1 ) return false;
	if ( args[0]=="1" )
	{
		ICommand::execute( "tilenear 5", log );
		ICommand::execute( "threshold 1", log );
	}
	else
	{
		ICommand::execute( "tilenear 100", log );
		ICommand::execute( "threshold 0.01", log );
	}
	return true;
}


NLMISC_COMMAND(ping,"send a ping","")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 0) return false;
	CMessage msgout( "PING" );
	if ( (ClientSocket != NULL) && ClientSocket->connected() && (PingTime == 0) )
	{
		PingTime = CTime::getLocalTime();
		ClientSocket->send( msgout );
	}
	return true;
}

NLMISC_COMMAND(clear,"clear the chat history","")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 0) return false;
	ChatOutput.clear();
	return true;
}

NLMISC_COMMAND(speed,"player speed","<speed in km/h>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	float speed = min( max( (float)atof(args[0].c_str()), 0.1f ), 200.0f ); // speed range in km/h
	MoveListener.setSpeed( speed / 3.6f );
	return true;
}


NLMISC_COMMAND(sight,"show/hide/change the sight","<sight_number>")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() == 0)
	{
		Sight = !Sight;
	}
	else if (args.size() == 1)
	{
		sint num = atoi(args[0].c_str());
		if (num == 0) Sight = !Sight;
		else
		{
			Sight = true;
			SelectedSight.setSightType(num-1);
		}
	}
	else
		return false;

	return true;
}

NLMISC_COMMAND(setpos,"change position of the player with a 2D coordinate","<x> <y>")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() == 2)
	{
		float x = (float) atof(args[0].c_str());
		float y = (float) atof(args[1].c_str());
		
		CVector userPos = CVector(x,y,0.0f);
		
		CollisionManager.setCenter(userPos);
		CollisionManager.snapToGround( userPos, 1000.0f );
		LocalArea->User.setPos(userPos);
		
		LocalArea->User.setFull3d(true);
		LocalArea->User.propagateState();
	}
	else
		return false;

	return true;
}

NLMISC_COMMAND(go,"change position of the player with a player name or location","<player_name>|<location>")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() == 1)
	{
		bool gotoplayer = true;

		vector<pair<CVector,string> >::iterator itpp;
		for(itpp = ParticularPlaces.begin(); itpp != ParticularPlaces.end(); itpp++)
		{
			if((*itpp).second==args[0])
			{
				CVector userPos = (*itpp).first;
			
				CollisionManager.setCenter(userPos);
				CollisionManager.snapToGround( userPos, 1000.0f );
				LocalArea->User.setPos(userPos);
				
				LocalArea->User.setFull3d(true);
				LocalArea->User.propagateState();

				gotoplayer = false;
				break;
			}
		}
		
		if(gotoplayer)
		{
			CRemoteEntities::const_iterator itre;
			for(itre=LocalArea->neighbors().begin(); itre!=LocalArea->neighbors().end(); itre++)
			{
				if((*itre).second->type() != MEWeapon)
				{
					if((*itre).second->name() == args[0])
					{
						CVector userPos = (*itre).second->pos();
							
						CollisionManager.setCenter(userPos);
						CollisionManager.snapToGround( userPos, 1000.0f );
						LocalArea->User.setPos(userPos);
						
						LocalArea->User.setFull3d(true);
						LocalArea->User.propagateState();

						break;
					}
				}
			}
		}
	}
	else
		return false;


	return true;
}
