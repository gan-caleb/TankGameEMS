/*****************************************************************************
 @Project		: MET2304 - Mini Project 
 @File 			: main.c
 @Details  	:	2 Player Tank Game
 @Author		: Caleb Gan / Koven Lui
 @Hardware	: Tiva Launchpad
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0       			19 Jun 20  		Initial Release

******************************************************************************/

#include "Common.h"
#include "Hal.h"
#include "BSP.h"
#include "LED.h"
#include "IRQ.h"
#include "spim.h"
#include "LCD_ST7735R.h"
#include "gui.h"
#include "UART.h"
#include <math.h>
#include "GameObj.h"
#include "GamePool.h"
#include "GameSprites.h"

/*****************************************************************************
 Define
******************************************************************************/

#define LCD_BUF_SIZE			4096 
#define LCD_UPDATE_MS			30U
#define KEYPAD_UPDATE_MS	50U
#define BUZZER_MS					100U // Buzzer volume/duration

/*****************************************************************************
 Type definition
******************************************************************************/

#define PLAYER_X_SIZE						30U
#define PLAYER_Y_SIZE						30U
#define DEBOUNCE_DELAY 					20000
#define FLOOR_LEVEL							160U
#define CEILING_LEVEL						10U
#define SCREEN_WIDTH						128U
#define SCREEN_HEIGHT						160U

// Define tank directions

//#define TANK_DIRECTION_WEST 3									// WEST not needed for P1
#define TANK_DIRECTION_NORTHWEST 1
#define TANK_DIRECTION_NORTH 2
#define TANK_DIRECTION_NORTHEAST 3
#define TANK_DIRECTION_EAST 4										// default position for P1
#define TANK_DIRECTION_SOUTHEAST 5
#define TANK_DIRECTION_SOUTH 6
#define TANK_DIRECTION_SOUTHWEST 7

// Define tank movement speed
#define TANK_MOVE_INCREMENT 1

// Potentiometer Threshhold
#define POTENTIOMETER_THRESHOLD 50

/*****************************************************************************
 Global Variables
******************************************************************************/


/*****************************************************************************
 Local const Variables
******************************************************************************/
/**		Keypad Array		**/
static char KEY_DECODE_TABLE[4][3] = 
{
	{ '1', '2', '3' },
	{ '4', '5', '6' },
	{ '7', '8', '9' },
	{ '*', '0', '#' }
};

/*****************************************************************************
 Local Variables
******************************************************************************/

/**************************		Systick Variables		****************************/
static volatile BOOL			g_bSystemTick = FALSE;
static volatile BOOL			g_bSecTick = FALSE;
static volatile BOOL			g_bToggle = FALSE;
static unsigned int				g_nTimeSec = 0;
static int            		g_nCount = 0;

/**********************		SPI Interface & LCD 	******************************/
static volatile int 			g_bSpiDone = FALSE;
static SPIM_HANDLE				g_SpimHandle;
static GUI_DATA						g_aBuf[LCD_BUF_SIZE];
static GUI_MEMDEV					g_MemDev;
static volatile BOOL 			g_bLCDUpdate = FALSE;
static volatile BOOL			g_bBacklightOn = TRUE;
static volatile BOOL 			g_bLcdFree  = TRUE;
static volatile BOOL			IN_FLAG = FALSE;
static volatile int 			g_nLCD = LCD_UPDATE_MS;
static unsigned int				HIST_Deg;
static unsigned char 			HIST_Dir;

/**************************		Keypad	 ***************************************/
static volatile BOOL 			g_bKeypadScan = FALSE;
static volatile BOOL 			bKeyPressed = FALSE;
static unsigned char 			g_cKey = '-';
static unsigned char 			g_cDir = ' ';
static volatile int 			g_nKeypad = KEYPAD_UPDATE_MS;
static volatile int   		hist_row, hist_col;
static unsigned int 			g_nAngle = 0;
static volatile 					BOOL hashPress = FALSE;

/*************************		Buttons		**************************/
static volatile BOOL			g_bSW1 = FALSE;  
static volatile BOOL			g_bSW2 = FALSE; 
static volatile BOOL			SW1_press = FALSE;
static volatile BOOL			SW2_press = FALSE;
volatile uint32_t 				SW1_delay = 0;
volatile uint32_t 				SW2_delay = 0;
static int				 				timer = 0;
volatile uint32_t					SW1_debounce = 0;
volatile uint32_t					SW2_debounce = 0;



/********************		Timer 0A TESTPIN		**********************/
static BOOL								g_bTESTPIN = FALSE;

/*************************		Buzzer		**************************/
static volatile BOOL  		buzz_flag;

/**************************		UART		*****************************/
// TODO add variables

/**************************		Potentiometer		********************************/
// TODO add variables

/************************** Sprite Animation *********************************/

/*	Fire Sprite Variables  */
static volatile int					g_FireCounter = 150;
static volatile BOOL				g_FireFlip = FALSE;
static int 									s_FireToggleCounter = 0;

/* Explosion Sprite Variables */
static volatile int					g_ExplosionCounter = 5;
static volatile int					g_ExplosionType = 1;
static 											BOOL explosionInProgress = FALSE;

/** Tank  Parameters **/
static volatile int 				g_TankLifeCounter = 3;
static volatile int					TankOri = 1;
static volatile int					TankCounter = 200;
static int 									g_nTankSpeed = 1; // Adjust speed as needed


/****************************** Game Objects *********************************/
		
/* Tank Sprite Directional Variables */	

static GameObject *currentTankSprite;
static volatile int g_nTankDirection = 5; // Initialize at East for P1
static volatile BOOL g_bTankMoving = FALSE;

static GameObject gObj_p1_NorthBluetank = { //NORTH
		bmpP1tankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P1NWtank = { //NORTH-WEST
		bmpP1tankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

		static GameObject gObj_P1NWtank2 = { //NORTH-WEST
		bmpP1tankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };
		
static GameObject gObj_p1_WestBluetank = { //WEST
		bmpP1tankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1SWtank = { // SOUTH-WEST
		bmpP1tankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_p1_SouthBluetank = { //SOUTH
		bmpP1tankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1SEtank = { // SOUTH-EAST
		bmpP1tankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_p1_EastBluetank = { //EAST
		bmpP1tankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1NEtank = { // NORTH-EAST
		bmpP1tankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

		
/* Fire Sprite Variables */

static GameObject gObj_Fire1 = {
		bmpFire1Sprite,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{40, 40} };	
static GameObject gObj_Fire2 = {
		bmpFire2Sprite,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{40, 40} };	

		
/* Misc Sprite Variables */

static GameObject gObj_Cactus = {
bmpCactusSprite,
		{MISC_X_SIZE, MISC_Y_SIZE}, 
		{128, 128} };	


	/* Explosion Sprite Variables */

static GameObject gObj_Explosion1 = {
		bmpExplosion1Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion2 = {
		bmpExplosion2Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion3 = {
		bmpExplosion3Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion4 = {
		bmpExplosion4Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion5 = {
		bmpExplosion5Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{20, 20} };

		
	/* Bullet Sprite Variables */
		
static volatile	BOOL bulletActive = FALSE;
		// Global variable to store bullet direction
static int g_nBulletDirection = 5;
		
static GameObject gObj_Bullet = {
		bmpBulletSprite,
		{BULLET_X_SIZE, BULLET_Y_SIZE},
		{0,0},
		{0,0} };


/*****************************************************************************
 Local Functions
******************************************************************************/
		
static void main_LcdInit( void );
static void main_KeyScan( void );
static void main_KeypadOutput(void);
int CHAR_TO_INT (char); 
void GUI_AppDraw( BOOL bFrameStart );
extern void GPIOF_Button_IRQHandler ( uint32_t Status);
static void main_AdcUpdate(void);
static void main_AdcInit(void);
void UpdateTankPosition(void);
void UpdateTankSprite();
void InitBullet();
void UpdateBulletPosition();
		
/*****************************************************************************
 Implementation
******************************************************************************/
		
int main()
{
	Port_Init(); // Initialize all Ports
	SystemCoreClockUpdate ();
	SysTick_Config( SystemCoreClock/1000 );  
	
	/* SSI initialization */
	NVIC_SetPriority( SSI0_IRQn, 0 );
	
	SpimInit(
		&g_SpimHandle,
		0U,
		25000000U,
		SPI_CLK_INACT_LOW,
		SPI_CLK_RISING_EDGE,
		SPI_DATA_SIZE_8 );
	
	main_LcdInit(); // Initialize LCD
	
	IRQ_Init(); // Initialize Interrupt handler
	
	main_AdcInit();

  currentTankSprite = &gObj_p1_EastBluetank;

	
	
	/*	Infinite FOR Loop, all functions that need to be looped, place here		*/
	for(;;)
  { //////////////////////// START OF FOR LOOP //////////////////////////////////////////////
		main_AdcUpdate();

		IN_FLAG = FALSE;
		/* LCD update */
		if( FALSE != g_bLCDUpdate )
		{
			if( 0 != g_bLcdFree )
			{
				g_bLCDUpdate = FALSE;
				g_bLcdFree = FALSE;
			  
				/* Draw every block. Consumes less time  */
				GUI_Draw_Exe(); 
			}
		}
		
		/* SW1 Debounce Function */
		
		if (SW1_press == TRUE)
		{
			SW1_delay = 0;
			SW1_debounce++;
			if ((SW1_debounce > DEBOUNCE_DELAY) && SW1_press == TRUE)
			{
				g_TankLifeCounter--;
				SW1_debounce = 0;
			}
			
//				if (g_TankLifeCounter < 0) // If Tank lives drop below 0
//				{
//					g_TankLifeCounter = 3; // Reset to 3
//				}
		}
		
		/* SW2 Debounce Function */
		
		if (SW2_press == TRUE)
		{
			SW2_delay = 0;
			SW2_debounce++;
			if ((SW2_debounce > DEBOUNCE_DELAY) && SW2_press == TRUE)
			{
				SW2_debounce = 0;
			}
		}
	
		
	/* Keypad Functions */
	
		if( FALSE != g_bKeypadScan )
		{
			g_bKeypadScan = FALSE;
			main_KeyScan();   // TODO add directional input to TANK here
			}
		}			/////////////////////// END OF FOR LOOP//////////////////////////////////////////
	} 
		
	


/*****************************************************************************
 Callback functions
******************************************************************************/
void SysTick_Handler( void )  
{

	g_bSystemTick = TRUE;
		
	/* Provide system tick */
  g_nCount++;
	
  if (g_nCount == 1000)
  {
    g_bSecTick = TRUE;
		g_nCount=0;
		
		/* Keep track of time based on 1 sec interval */ 
		g_nTimeSec++;
		if(g_nTimeSec > 24*60*60)
		{
			g_nTimeSec = 0;
		}
  }
	
/**************************		System Flags		************************/
	
  /** LCD flag		**/	
	if( 0 != g_nLCD )
	{
		g_nLCD--;
		if( 0 == g_nLCD )
		{
			g_nLCD = LCD_UPDATE_MS;
			g_bLCDUpdate = TRUE;
		}
	}
	
	/**  Keypad flag	**/
	if( 0 != g_nKeypad )
	{
		g_nKeypad--;
		if( 0 == g_nKeypad )
		{
			g_nKeypad = KEYPAD_UPDATE_MS;
			g_bKeypadScan = TRUE;
		}
	}
	
	/** Buzzer Flag **/
	
	if(buzz_flag){
		int buzz_count=BUZZER_MS;
		while(buzz_count){
			BUZZER_ON();
			buzz_count--;
		}
		BUZZER_OFF();
		buzz_flag=FALSE;
	}
	
	/** Explosion Animation Flags **/

	
	if (g_TankLifeCounter != 0) // if player still has lives
	{
    if (SW1_press == TRUE || explosionInProgress) // Enter the condition even if SW1_press becomes false    
		{
        g_ExplosionCounter--;
			
        if (g_ExplosionCounter == 0)        
				{
            g_ExplosionType++;            
						g_ExplosionCounter = 200;
					
            if (g_ExplosionType > 5)
						{
                g_ExplosionType = 1;
                explosionInProgress = FALSE; // Reset explosion state when > 5            
						}
            else            
						{
                explosionInProgress = TRUE; // Set explosion state when SW1_press is true            
            }
					}
				}
			}

	/** TESTING for Fire animation when Life drops to 0 **/

if (SW1_press == FALSE && g_TankLifeCounter == 0) // When player dead

    {
        
        if (s_FireToggleCounter >= 10) 
        {
            g_FireFlip = !g_FireFlip;
            s_FireToggleCounter = 0;
        }
        
        s_FireToggleCounter++;
    }
		 else
    {
        // Reset the toggle counter when TankLifeCounter is not 0
        s_FireToggleCounter = 0;

		}
		
		 // Handle tank movement
    if (SW1_press)
    {
        g_bTankMoving = TRUE; // Tank is moving as long as SW1 is pressed
        UpdateTankPosition(); // Update tank position based on direction
    }
    else
    {
        g_bTankMoving = FALSE; // Tank stops moving when SW1 is released
    }
		
					 if (bulletActive) {
        UpdateBulletPosition();
    }
		}
	
		

			


//////////////////////////////end of Systick Handler//////////////////////////////////////


/**************************** Printing to LCD ******************************************/

void GUI_AppDraw( BOOL bFrameStart )
{
	
	/** Initialization Screen **/
	char buf[128];
	GUI_Clear( ClrBlack ); /* Set background colour */
	
	//GUI_SetColor(ClrYellow);	// Screen Border
	//GUI_DrawRect(0,0,159,127);
	
	GUI_SetFont (&g_FontComic16);

//	sprintf(buf, "LIVES: %d", g_TankLifeCounter); // Print Life counter to LCD
//		GUI_PrintString(buf, ClrWhite, 10, 60);

Print_GameObject(currentTankSprite, TRUE);
	
	if (bulletActive) {
    Print_GameObject(&gObj_Bullet, FALSE); // TRUE to draw hitbox, if needed
}
	
	/** Print Fire Animation **/

	
//	if (g_FireFlip == FALSE)
//	{
//	Print_GameObject(&gObj_Fire1, FALSE);
//	}
//	else
//	{
//	Print_GameObject(&gObj_Fire2, FALSE);
//	}

	
	/** Print Explosion Animation **/
	
//	switch (g_ExplosionType)
//	{
//		case 1:
//		Print_GameObject(&gObj_Explosion1, FALSE);
//		break;
//		case 2:
//		Print_GameObject(&gObj_Explosion2, FALSE);
//		break;
//		case 3:
//		Print_GameObject(&gObj_Explosion3, FALSE);
//		break;
//		case 4:
//		Print_GameObject(&gObj_Explosion4, FALSE);
//		break;
//		case 5:
//		Print_GameObject(&gObj_Explosion5, FALSE);
//		break;
//	}
	
		/** Print Cactus Object **/

//	Print_GameObject(&gObj_Cactus, FALSE);

		/** Print Tank Sprite **/

switch (TankOri)
	{
		case 1:
		Print_GameObject(&gObj_P1NWtank, TRUE);
		break;
		
		case 2:
		Print_GameObject(&gObj_p1_NorthBluetank, TRUE);
		break;
						
		case 3:
		Print_GameObject(&gObj_P1NEtank, TRUE);
		break;
						
		case 4:
		Print_GameObject(&gObj_p1_EastBluetank, TRUE);
		break;
										
		case 5:
		Print_GameObject(&gObj_P1SEtank, TRUE);
		break;
						
		case 6:
		Print_GameObject(&gObj_p1_SouthBluetank, TRUE);
		break;
														
		case 7:
		Print_GameObject(&gObj_P1SWtank, TRUE);
		break;
														
//		case 3:
//		Print_GameObject(&gObj_p1_WestBluetank, FALSE);
//		break;

	}
}


static void main_cbLcdTransferDone( void )
{
	g_bLcdFree = TRUE;
}

static void main_cbGuiFrameEnd( void )
{
	g_bLcdFree = TRUE;
}

/*****************************************************************************
 Local functions
******************************************************************************/

/****************** Tank Sprite positioning *****************************/

void UpdateTankSprite()
{
    GameObject *prevTankSprite = currentTankSprite;

    switch (TankOri)
    {
			
				case TANK_DIRECTION_NORTHWEST:
            currentTankSprite = &gObj_P1NWtank;
            break;
        case TANK_DIRECTION_NORTH:
            currentTankSprite = &gObj_p1_NorthBluetank;
            break;
				case TANK_DIRECTION_NORTHEAST:
            currentTankSprite = &gObj_P1NEtank;
            break;
				case TANK_DIRECTION_EAST:
            currentTankSprite = &gObj_p1_EastBluetank;
            break;
				case TANK_DIRECTION_SOUTHEAST:
            currentTankSprite = &gObj_P1SEtank;
            break;	
				case TANK_DIRECTION_SOUTH:
            currentTankSprite = &gObj_p1_SouthBluetank;
            break;
				case TANK_DIRECTION_SOUTHWEST:
            currentTankSprite = &gObj_P1SWtank;
            break;

//        case TANK_DIRECTION_WEST:																// not needed for P1
//            currentTankSprite = &gObj_p1_WestBluetank;
//            break;
//        default:
//            // Default sprite when direction is not recognized
//            currentTankSprite = &gObj_p1_NorthBluetank;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite != currentTankSprite) {
        currentTankSprite->pos.x = prevTankSprite->pos.x;
        currentTankSprite->pos.y = prevTankSprite->pos.y;
    }
}


void UpdateTankPosition()
{
    static int tickCount = 0;
    tickCount++;
		static int pixelSpeed = 2; // Movement speed

    // Only move the tank once every 500 ticks
    if (tickCount >= 200 && g_bTankMoving)
    {
        tickCount = 0; // Reset tick counter

        switch (TankOri)
        {
            
						case TANK_DIRECTION_NORTHWEST:
								currentTankSprite->pos.x -= pixelSpeed;
                currentTankSprite->pos.y -= pixelSpeed;
                break;						
        
            case TANK_DIRECTION_NORTH:
               currentTankSprite->pos.y -= pixelSpeed;
						        break;
						 case TANK_DIRECTION_NORTHEAST:
                currentTankSprite->pos.x += pixelSpeed;
                currentTankSprite->pos.y -= pixelSpeed;
                break;
						  case TANK_DIRECTION_EAST:
                currentTankSprite->pos.x += pixelSpeed;
                break;
							case TANK_DIRECTION_SOUTHEAST:
                currentTankSprite->pos.x += pixelSpeed;
                currentTankSprite->pos.y += pixelSpeed;
                break;
							 case TANK_DIRECTION_SOUTH:
                currentTankSprite->pos.y += pixelSpeed;
                break;
							 case TANK_DIRECTION_SOUTHWEST:
                currentTankSprite->pos.x -= pixelSpeed;
                currentTankSprite->pos.y += pixelSpeed;
                break;
//            case TANK_DIRECTION_WEST:
//                currentTankSprite->pos.x -= pixelSpeed;
//                break;
								default:
                // No movement if the direction is not recognized
                break;
        }
    }
}
		
void InitBullet() {
    gObj_Bullet.sprite = bmpBulletSprite;
    gObj_Bullet.size.x = BULLET_X_SIZE;
    gObj_Bullet.size.y = BULLET_Y_SIZE;

    // Calculate bullet initial position based on the tank's orientation
		// DO NOT ADJUST THESE VALUES!!! THEY HAVE BEEN CALIBRATED TO THE TANK'S FRONT ALREADY
    switch (TankOri) {
			
			  case TANK_DIRECTION_NORTHWEST:
            gObj_Bullet.pos.x = currentTankSprite->pos.x + 2;
            gObj_Bullet.pos.y = currentTankSprite->pos.y ;
            break;
        case TANK_DIRECTION_NORTH:
            gObj_Bullet.pos.x = currentTankSprite->pos.x + 1;
            gObj_Bullet.pos.y = currentTankSprite->pos.y - 5;
            break;
        case TANK_DIRECTION_NORTHEAST:
            gObj_Bullet.pos.x = currentTankSprite->pos.x + 5;
            gObj_Bullet.pos.y = currentTankSprite->pos.y - 6;
            break;
        case TANK_DIRECTION_EAST:
            gObj_Bullet.pos.x = currentTankSprite->pos.x + 5;
            gObj_Bullet.pos.y = currentTankSprite->pos.y - 3;
            break;
        case TANK_DIRECTION_SOUTHEAST:
            gObj_Bullet.pos.x = currentTankSprite->pos.x;
            gObj_Bullet.pos.y = currentTankSprite->pos.y - 4;
            break;
        case TANK_DIRECTION_SOUTH:
            gObj_Bullet.pos.x = currentTankSprite->pos.x + 1;
            gObj_Bullet.pos.y = currentTankSprite->pos.y + 3;
            break;
        case TANK_DIRECTION_SOUTHWEST:
            gObj_Bullet.pos.x = currentTankSprite->pos.x - 1;
            gObj_Bullet.pos.y = currentTankSprite->pos.y - 1 ;
            break;

    }

    bulletActive = TRUE; // Activate the bullet
}

void UpdateBulletPosition() {
    if (!bulletActive) return;

     static int tickCountBullet = 0;
    tickCountBullet++;
		static int BulletSpeed = 6; // Movement speed

    // Only move the tank once every 500 ticks
    if (tickCountBullet >= 200 && g_bTankMoving)
    {
        tickCountBullet = 0; // Reset tick counter

    switch(g_nBulletDirection) {
			        case TANK_DIRECTION_NORTHWEST:
            gObj_Bullet.pos.x -= BulletSpeed;
            gObj_Bullet.pos.y -= BulletSpeed;
            break;
        case TANK_DIRECTION_NORTH:
            gObj_Bullet.pos.y -= BulletSpeed;
            break;
				        case TANK_DIRECTION_NORTHEAST:
            gObj_Bullet.pos.x += BulletSpeed;
            gObj_Bullet.pos.y -= BulletSpeed;
            break;
								        case TANK_DIRECTION_EAST:
            gObj_Bullet.pos.x += BulletSpeed;
            break;
												        case TANK_DIRECTION_SOUTHEAST:
            gObj_Bullet.pos.x += BulletSpeed;
            gObj_Bullet.pos.y += BulletSpeed;
            break;
																        case TANK_DIRECTION_SOUTH:
            gObj_Bullet.pos.y += BulletSpeed;
            break;

        case TANK_DIRECTION_SOUTHWEST:
            gObj_Bullet.pos.x -= BulletSpeed;
            gObj_Bullet.pos.y += BulletSpeed;
            break;




    }

    // Check for border collision
   if (gObj_Bullet.pos.y <= 0 || gObj_Bullet.pos.y >= SCREEN_HEIGHT 
        || gObj_Bullet.pos.x <= 0 || gObj_Bullet.pos.x >= SCREEN_WIDTH) {
        bulletActive = FALSE; // Deactivate the bullet
    }
	}
}

/*************************** Keypad Parameters ******************************/

static void main_KeyScan( void )
{
  int nRow, nCol, input;
  /* Set all rows to high so that if any key is pressed, 
     a falling edge on the column line can be detected */
  KEYPAD_ALL_ROWS_ON();
	
	if(bKeyPressed==FALSE){ //only scan if button is released
		for( nRow=0; nRow<4; nRow++ )
		{
			/* Pull row by row low to determined which button is pressed */
			KEPAD_ROW_MASKED &= ~(1U << nRow);
			/* Short delay to stabalize row that has just been pulled low */
			__NOP(); __NOP(); __NOP();
			
			input = KEYPAD_COL_IN();	
			
			/* If a column is asserted, then look for the corresponding key 
				 Make use of the KEY_DECODE_TABLE, exit loop once key found!  */

				for (nCol = 0; nCol < 3; nCol++)
				{
						if (input & (1U << nCol)) //HIGH = OFF
						{
							bKeyPressed = FALSE;
							g_cKey='-';
						}
						else{											//LOW = ON
							g_cKey = KEY_DECODE_TABLE[nRow][nCol];
							hist_row=nRow;
							hist_col=nCol;
							bKeyPressed=TRUE;
							buzz_flag = TRUE;
							printf("INPUT: %c", g_cKey);
							
							  if (g_cKey == '#') 
    {
        hashPress = !hashPress;  // Toggle the hashPress flag
    }
            }
            return;
						}
				}		
			
		} //end of for loop
	

  /* Check if key is released */
	else{	//(bKeyPressed==TRUE)
		KEPAD_ROW_MASKED &= ~(1U << hist_row);
		__NOP(); __NOP(); __NOP();
		input = KEYPAD_COL_IN();	

		if (input & (1U << hist_col)) //HIGH = OFF
		{
			bKeyPressed = FALSE;
			g_cKey='-';
		}
	}		
  /* Reset all rows for next key detection */
  KEYPAD_ALL_ROWS_OFF();
	main_KeypadOutput();

}

/************************ LCD Parameters ***************************/

static void main_LcdInit( void )
{
	int screenx;
	int screeny;
	
	/* g_SpimHandle shall be itializaed before use */
	
	/* Choosing a landscape orientation */
	LcdInit( &g_SpimHandle, LCD_LANDSCAPE_180 );
	
	/* Get physical LCD size in pixels */
	LCD_GetSize( &screenx, &screeny );
	
	/* Initialize GUI */
	GUI_Init(
		&g_MemDev,
		screenx,
		screeny,
		g_aBuf,
		sizeof(g_aBuf) );
	
	/* Switch to transfer word for faster performance */
	SpimSetDataSize( &g_SpimHandle, SPI_DATA_SIZE_16 );
	GUI_16BitPerPixel( TRUE );
	
	/* Clear LCD screen to Blue */
	GUI_Clear( ClrBlue );

  /* set font color background */
  GUI_SetFontBackColor( ClrBlue );
    
  /* Set font */
	GUI_SetFont( &g_sFontCalibri10 );
	
	LCD_AddCallback( main_cbLcdTransferDone );
	
	GUI_AddCbFrameEnd( main_cbGuiFrameEnd );
	
	/* Backlight ON */
	LCD_BL_ON();
}

/*******************  ADC Parameters ****************************/

static void main_AdcInit(void)
{
    /* Disable Sample Sequencer 0 (SS0) for initialization */
    ADC0->ACTSS &= ~ADC_ACTSS_ASEN0;
    ADC1->ACTSS &= ~ADC_ACTSS_ASEN0;

    /* Configure SS0 to read from AIN8 and AIN9 */
    ADC0->SSMUX0 = AIN8; // Configure both AIN9 and AIN8
    ADC1->SSMUX0 = AIN9; // Configure both AIN9 and AIN8

    /* Configure SS0 step 0 and step 1 to be the end of the sequence */
    ADC0->SSCTL0 |= ADC_SSCTL0_END0 | ADC_SSCTL0_END1;
    ADC1->SSCTL0 |= ADC_SSCTL0_END0 | ADC_SSCTL0_END1;
    /* Set hardware averaging to 32X */
    ADC0->SAC |= ADC_SAC_AVG_32X;
    ADC1->SAC |= ADC_SAC_AVG_32X;

    /* Enable SS0 */
    ADC0->ACTSS |= ADC_ACTSS_ASEN0;
    ADC1->ACTSS |= ADC_ACTSS_ASEN0;

    /* Start SS0 */
    ADC0_SS0_Start();
    ADC1_SS0_Start();
}


void main_AdcUpdate(void)
{
    static uint16_t lastResult_MUX0 = 0;
    uint16_t Result_MUX0;
    Result_MUX0 = ADC0_GET_MUX0_FIFO();

    // Simple filtering: Only update if the change is significant
    if (abs(Result_MUX0 - lastResult_MUX0) > POTENTIOMETER_THRESHOLD) {
        TankOri = (Result_MUX0 * 7) / 4096 + 1; // Assuming 12-bit ADC resolution
        if (TankOri > 7) TankOri = 7;
        if (TankOri < 1) TankOri = 1;

        lastResult_MUX0 = Result_MUX0;
        UpdateTankSprite();
    }

    ADC0_SS0_Start();
}


/*************************** Keypad Output Parameters ****************************/

static void main_KeypadOutput(void)
{
	static char cKey;
	uint8_t nKey = 0;
	uint8_t nRGB;
		
	if(cKey != g_cKey)
	{
		cKey = g_cKey;
	
		switch(cKey)
		{
			case '*':
				nKey = 10;
				break;
			case '0':
				nKey = 11;
				break;
			case '#':
				nKey = 12;
				break;
			default:
				if(cKey > '0' && cKey <= '9')
				{
					nKey = cKey - '0';
				}
		}
			
		nRGB = LED_RGB_GET(); 
		
		VERIFY_ON();
		LED_RGB_SET(RGB_RED);
		LED_RGB_SET(RGB_OFF);
		while(nKey-- > 0)
		{
			LED_RGB_SET(RGB_BLUE);
			LED_RGB_SET(RGB_OFF);
		}
		LED_RGB_SET(RGB_BLUE);
		LED_RGB_SET(RGB_OFF);
		LED_RGB_SET(RGB_RED);
		LED_RGB_SET(RGB_OFF);
		
		VERIFY_OFF();
		LED_RGB_SET(nRGB);
	}
}
	void GPIOF_Button_IRQHandler(uint32_t Status) {
    /* Check if it is SW2 (PF0) interrupt */
    if (0 != (Status & BIT(PF_SW2))) {
        GPIOF->ICR = BIT(PF_SW2); // Clear interrupt

        // Fire a bullet regardless of the previous bullet's state
        g_nBulletDirection = TankOri; // Set bullet direction to current tank orientation
        InitBullet(); // Initialize bullet position and set it active
    }

    /* Check if it is SW1 (PF4) interrupt */
    if (0 != (Status & BIT(PF_SW1))) {
        GPIOF->ICR = BIT(PF_SW1); // Clear interrupt
        SW1_press = !SW1_press;
        g_bTankMoving = SW1_press; // Continue tank movement
    }
}


/*****************************************************************************
 Interrupt functions
******************************************************************************/

/*	Timer Interrupts	*/
extern void TIMER0A_IRQHandler( uint32_t Status )
{
	if( 0 != (Status & TIMER_RIS_TATORIS) )
	{
		TIMER0->ICR |= TIMER_ICR_TATOCINT;
	}
}
/*****************************************************************************
 Other functions
******************************************************************************/
int CHAR_TO_INT (char c)
{
	return c - '0';
}


