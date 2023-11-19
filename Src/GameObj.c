/*****************************************************************************
 @Project		: 
 @File 			: GameObj.h
 @Details  	:                  
 @Author		: 
 @Hardware	: 
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  Name     XXXX-XX-XX  		Initial Release
   
******************************************************************************/

#include "GameObj.h"

/*****************************************************************************
 Define
******************************************************************************/

/*****************************************************************************
 Type definition
******************************************************************************/

/*****************************************************************************
 Global Variables
******************************************************************************/

/*****************************************************************************
 Local Variables
******************************************************************************/

/*****************************************************************************
 Implementation
******************************************************************************/

void Print_GameObject(GameObject* g_Obj, BOOL hitbox)
{           
    // Draws the sprite at the GameObject's position
    GUI_DrawBitmap(g_Obj->sprite, g_Obj->pos.x - (g_Obj->size.x / 2), g_Obj->pos.y - g_Obj->size.y);
    
    // Draws hitbox if set to true
    if (hitbox)
    {
        GUI_SetColor(ClrWhite);
        
        // Calculate the exact left, top, right, and bottom coordinates of the sprite
        int left = g_Obj->pos.x - (g_Obj->size.x / 2);
        int top = g_Obj->pos.y - g_Obj->size.y;
        int right = left + g_Obj->size.x;
        int bottom = top + g_Obj->size.y;

        GUI_DrawRect(
            left,   // left
            top,    // top
            right,  // right
            bottom  // bottom
        );
    }
}


