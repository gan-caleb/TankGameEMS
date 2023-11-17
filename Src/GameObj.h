/*****************************************************************************
 @Project		: 
 @File 			: GameObj.h
 @Details  	: structure definitions for game objects and function declarations 
							game objects. It defines the basic properties and methods that 
							game objects share, like position, size, and rendering.                   
 @Author		: 
 @Hardware	: 
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  Name     XXXX-XX-XX  		Initial Release
   
******************************************************************************/

#ifndef GAMEOBJ_DOT_H
#define GAMEOBJ_DOT_H

#include <stdint.h>
#include "Common.h"
#include "gui.h"

typedef struct coord_tag
{
    uint8_t x;
    uint8_t y;
} coordinates;

typedef struct GO_tag
{
    const unsigned short* sprite; // bitmap of the sprite
    coordinates size;             // dimensions of the game object
    coordinates pos;              // x,y position of the game object (middle-bottom of object)
    coordinates prevPos;          // previous position of the game object
} GameObject;

void Print_GameObject(GameObject*, BOOL hitbox);

#endif
