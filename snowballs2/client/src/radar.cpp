/** \file radar.cpp
 * 
 *
 * $Id: radar.cpp,v 1.1 2001/07/17 17:20:45 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>

#include "camera.h"
#include "client.h"
#include "mouse_listener.h"
#include "entities.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

uint RadarDistance = 500;
uint RadarMinDistance = 100;
uint RadarMaxDistance = 1000;

uint RadarState = 0;


struct ParticularPlace
{
	float x, y;
	char name[128];
};

ParticularPlace ParticularPlaces[] = {
	{ 2300.0f, -776.0f, "Vagimount" },
	{ 770.0f, -640.0f, "Manhatan" },
	{ 1135.0f, -840.0f, "ThePill" },
	{ 2140.0f, -500.0f, "Circus" },
	{ 1033.0f, -944.0f, "End WuDoor" },
	{ 564.0f, -1033.0f, "Start WuDoor" },
	{ 2125.0f, -764.0f, "BigHole" },
	{ 2169.0f, -1258.0f, "GapCave" },
	{ 1840.0f, -970.0f, "StartPosition" },
	{ 2350.0f, -1158.0f, "BackWay" },
	{ 2440.0f, -1302.0f, "WorldRoof" },
};



/*********************************************************\
					displayRadar()
\*********************************************************/
void displayRadar ()
{
	float xLeft = 0.5f-0.4f*3.f/4.f;
	float xRight = 0.5f+0.4f*3.f/4.f;
	float yTop = 0.9f;
	float yBottom = 0.1f;

	// Background
	Driver->drawQuad (xLeft,yBottom,xRight,yTop,CRGBA(0,255,0,50));

	// Print radar's range
	TextContext->setHotSpot(UTextContext::TopRight);
	TextContext->setColor(CRGBA(0,255,0));
	TextContext->setFontSize(14);
	TextContext->printfAt(0.5f+0.39f*3.f/4.f,0.89f,"%d m",RadarDistance);
		
	// Radar unit
	float stepV = 50.0f;
	float stepH = stepV*3.f/4.f;
	// Changing scale
	stepV = 0.8f*stepV/RadarDistance;
	stepH = 0.8f*stepH/RadarDistance;

	// Drawing radar's lines
	float gapV = stepV/2;
	float gapH = stepH/2;
	
	while(gapV<=0.4f)
	{
		// v lines
		Driver->drawLine(0.5f+gapH,yTop,0.5f+gapH,yBottom,CRGBA(0,255,0,100));
		Driver->drawLine (0.5f-gapH,yTop,0.5f-gapH,yBottom,CRGBA(0,255,0,100));
		
		// h lines
		Driver->drawLine (xLeft,0.5f+gapV,xRight,0.5f+gapV,CRGBA(0,255,0,100));
		Driver->drawLine (xLeft,0.5f-gapV,xRight,0.5f-gapV,CRGBA(0,255,0,100));

		gapV += stepV;
		gapH += stepH;
	}

	float scale = 1.0f;

	float xscreen = 0.5f;
	float yscreen = 0.5f;

	Driver->setFrustum (CFrustum(0.f, 1.0f, 0.f, 1.f, -1.f, 1.f, false));
	
	// distance between user and neighbour player
	float myPosx = Self->Position.x;
	float myPosy = Self->Position.y;

	// Quads size
	float radius = 0.006f;

	// Arrow in center (user)
	Driver->drawTriangle(xscreen-2*radius,yscreen-2*radius, xscreen,yscreen-radius, xscreen,yscreen+2*radius, CRGBA(0,255,0));
	Driver->drawTriangle(xscreen,yscreen-radius, xscreen+2*radius,yscreen-2*radius, xscreen,yscreen+2*radius, CRGBA(0,255,0));

	TextContext->setColor(CRGBA(255,100,0));

	for(EIT eit=Entities.begin(); eit!=Entities.end(); eit++)
	{
		if((*eit).second.Type == CEntity::Other)
		{
			CVector playerPos = (*eit).second.Position;

			// position of neighbour
			float posx = playerPos.x;
			float posy = playerPos.y;

			// relative position
			posx = (posx-myPosx)*0.4f/RadarDistance;
			posy = (posy-myPosy)*0.4f/RadarDistance;

			float dist = (float) sqrt((posx*posx)+(posy*posy));

			// Display a quad to show a player
			float an;
			float az;
			float x;
			float y;
			az = MouseListener->getOrientation ();
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

			TextContext->setColor(CRGBA(255,100,0));

			Driver->drawQuad (x-radius,y-radius, x+radius,y+radius,CRGBA(255,100,0));

			// Print his name
			TextContext->setFontSize(10);
			if(x>=0.5f)
			{
				if(y>=0.5f)
				{
					TextContext->setHotSpot(UTextContext::BottomLeft);
					TextContext->printfAt(x+2*radius, y+2*radius, (*eit).second.Name.c_str());
				}
				else
				{
					TextContext->setHotSpot(UTextContext::TopLeft);
					TextContext->printfAt(x+2*radius, y-2*radius, (*eit).second.Name.c_str());
				}
			}
			else
			{
				if(y>=0.5f)
				{
					TextContext->setHotSpot(UTextContext::BottomRight);
					TextContext->printfAt(x-2*radius, y+2*radius, (*eit).second.Name.c_str());
				}
				else
				{
					TextContext->setHotSpot(UTextContext::TopRight);
					TextContext->printfAt(x-2*radius, y-2*radius, (*eit).second.Name.c_str());
				}
			}
		}
	}

	// display particular places
	for(uint i = 0; i < sizeof(ParticularPlaces)/sizeof(ParticularPlaces[0]); i++)
	{
		// relative position
		float posx = (ParticularPlaces[i].x-myPosx)*0.4f/RadarDistance;
		float posy = (ParticularPlaces[i].y-myPosy)*0.4f/RadarDistance;
		
		float dist = (float) sqrt((posx*posx)+(posy*posy));

		// Display a quad to show a player
		float an;
		float az;
		float x;
		float y;
		az = MouseListener->getOrientation ();
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

		Driver->drawTriangle(x-radius,y-radius, x+radius,y-radius, x,y+radius, CRGBA(0,0,255));

		TextContext->setFontSize(10);
		TextContext->setColor(CRGBA(0,0,255));

		if(x>=0.5f)
		{
			if(y>=0.5f)
			{
				TextContext->setHotSpot(UTextContext::BottomLeft);
				TextContext->printfAt(x+2*radius, y+2*radius, ParticularPlaces[i].name);
			}
			else
			{
				TextContext->setHotSpot(UTextContext::TopLeft);
				TextContext->printfAt(x+2*radius, y-2*radius, ParticularPlaces[i].name);
			}
		}
		else
		{
			if(y>=0.5f)
			{
				TextContext->setHotSpot(UTextContext::BottomRight);
				TextContext->printfAt(x-2*radius, y+2*radius, ParticularPlaces[i].name);
			}
			else
			{
				TextContext->setHotSpot(UTextContext::TopRight);
				TextContext->printfAt(x-2*radius, y-2*radius, ParticularPlaces[i].name);
			}
		}
	}
}



/*********************************************************\
					displayLittleRadar()
\*********************************************************/
void displayLittleRadar()
{
	float width = 0.1f;
	float xLeft = 0.9f-width*3.f/4.f;
	float xRight = 0.9f+width*3.f/4.f;
	float yTop = 0.3f+width;
	float yBottom = 0.3f - width;

	// Background
	Driver->drawQuad (xLeft,yBottom,xRight,yTop,CRGBA(0,255,0,50));

	// Print radar's range
	TextContext->setHotSpot(UTextContext::MiddleBottom);
	TextContext->setColor(CRGBA(0,255,0));
	TextContext->setFontSize(12);
	TextContext->printfAt(xLeft+width*3.f/4.f,yTop+0.01f,"%d m",RadarDistance);
		
	// Radar unit
	float stepV = 50.0f;
	float stepH = stepV*3.f/4.f;
	// Changing scale
	stepV = width*stepV/RadarDistance;
	stepH = width*stepH/RadarDistance;

	// Drawing radar's lines
	// h lines
	Driver->drawLine (xLeft,yTop,xRight,yTop,CRGBA(0,255,0,100));
	Driver->drawLine (xLeft,yBottom+width,xRight,yBottom+width,CRGBA(0,255,0,100));
	Driver->drawLine (xLeft,yBottom,xRight,yBottom,CRGBA(0,255,0,100));	
	
	// v lines
	Driver->drawLine (xLeft,yTop,xLeft,yBottom,CRGBA(0,255,0,100));	
	Driver->drawLine (xLeft+width*3.f/4.f,yTop,xLeft+width*3.f/4.f,yBottom,CRGBA(0,255,0,100));	
	Driver->drawLine (xRight,yTop,xRight,yBottom,CRGBA(0,255,0,100));	

	float scale = 1.0f;

	float xscreen = xLeft + width*3.f/4.f;
	float yscreen = yBottom + width;

	Driver->setFrustum (CFrustum(0.f, 1.0f, 0.f, 1.f, -1.f, 1.f, false));
	
	// distance between user and neighbour player
	float myPosx = Self->Position.x;
	float myPosy = Self->Position.y;

	// Quads size
	float radius = 0.003f;

	TextContext->setColor(CRGBA(255,100,0));

	for(EIT eit=Entities.begin(); eit!=Entities.end(); eit++)
	{
		if((*eit).second.Type == CEntity::Other)
		{
			CVector playerPos = (*eit).second.Position;

			// position of neighbour
			float posx = playerPos.x;
			float posy = playerPos.y;

			// relative position
			posx = (posx-myPosx)*width/RadarDistance;
			posy = (posy-myPosy)*width/RadarDistance;

			float dist = (float) sqrt((posx*posx)+(posy*posy));

			// Display a quad to show a player
			float an;
			float az;
			float x;
			float y;
			az = MouseListener->getOrientation ();
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

			TextContext->setColor(CRGBA(255,100,0));

			Driver->drawQuad (x-radius,y-radius,x+radius,y+radius,CRGBA(255,100,0));
		}
	}


	// display particular places
	for(uint i = 0; i < sizeof(ParticularPlaces)/sizeof(ParticularPlaces[0]); i++)
	{
		// relative position
		float posx = (ParticularPlaces[i].x-myPosx)*width/RadarDistance;
		float posy = (ParticularPlaces[i].y-myPosy)*width/RadarDistance;

		float dist = (float) sqrt((posx*posx)+(posy*posy));

		// Display a quad to show a player
		float an;
		float az;
		float x;
		float y;
		az = MouseListener->getOrientation ();
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

		Driver->drawTriangle(x-radius,y-radius, x+radius,y-radius, x,y+radius, CRGBA(0,0,255));
	}
}


void initRadar ()
{
	RadarState = ConfigFile.getVar("ShowRadar").asInt ();

	// todo metre les milliard de var pour le radar dans le ficheir de config
}

void updateRadar ()
{
	if (RadarDistance > RadarMaxDistance) RadarDistance = RadarMaxDistance;
	else if (RadarDistance < RadarMinDistance) RadarDistance = RadarMinDistance;

	switch (RadarState)
	{
	case 0:
		break;
	case 1:
		displayRadar ();
		break;
	case 2:
		displayLittleRadar();
		break;
	}
}

void releaseRadar ()
{
}
