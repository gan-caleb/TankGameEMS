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
//#include "i2c.h"
//#include "MCP23017.h"
//#include "AHT10.h"
#include "UART.h"
#include <math.h>
#include "GameObj.h"
#include "GamePool.h"
#include "GameSprites.h"
#include <stdlib.h> // For rand() and srand()
#include <time.h>   // For time()

/*****************************************************************************
 Hardware definitions
******************************************************************************/

#define LCD_BUF_SIZE			4096 
#define LCD_UPDATE_MS			30U
#define KEYPAD_UPDATE_MS	50U
#define BUZZER_MS					100U // Buzzer volume/duration

/*****************************************************************************
 Game definitions
******************************************************************************/

#define DEBOUNCE_DELAY 					20000
#define SCREEN_WIDTH						159U
#define SCREEN_HEIGHT						127U

/***************** Tank Directions *******************/

/* Direction of P1 Tank */

#define TANK1_DIRECTION_NORTHWEST 1
#define TANK1_DIRECTION_NORTH 2
#define TANK1_DIRECTION_NORTHEAST 3
#define TANK1_DIRECTION_EAST 4										// default position for P1
#define TANK1_DIRECTION_SOUTHEAST 5
#define TANK1_DIRECTION_SOUTH 6
#define TANK1_DIRECTION_SOUTHWEST 7

/* Directions for P2 Tank */
#define TANK2_DIRECTION_SOUTHEAST 1
#define TANK2_DIRECTION_SOUTH 2
#define TANK2_DIRECTION_SOUTHWEST 3
#define TANK2_DIRECTION_WEST 4										// default position for P2
#define TANK2_DIRECTION_NORTHWEST 5
#define TANK2_DIRECTION_NORTH 6
#define TANK2_DIRECTION_NORTHEAST 7

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
/************************* Skin Select Stage ********************************/
static void P1selectColor(void);
static void P2selectColor(void);

/**************************		Systick Variables		****************************/
static volatile BOOL			g_bSystemTick = FALSE;
static volatile BOOL			g_bSecTick = FALSE;
static volatile BOOL			g_bToggle = FALSE;
static unsigned int				g_nTimeSec = 0;
static int            		g_nCount = 0;
static volatile int				TIMER_MS = 75000;

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

/*************************		Buttons		**************************/
static volatile BOOL			g_bSW1 = FALSE;  
static volatile BOOL			g_bSW2 = FALSE; 
static volatile BOOL			P1_SW1_press = FALSE;
static volatile BOOL			P2_SW1_press = FALSE;
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

/*************************		Stages		**************************/
static volatile int 			Stage = 1; // intro screen
static volatile BOOL			Game_In_Progress = FALSE; // for Timer

/*********************** tank color select ***********************/
volatile uint8_t 					player1Color = 7; // number bound to a color
volatile uint8_t 					player2Color = 0;
volatile BOOL 						P1colorChosen = FALSE; // P1 selected flag
volatile BOOL 						P2colorChosen = FALSE; // P2 selected flag
static volatile	BOOL			Cleared_Once = FALSE; // for updating screen when selecting color
static volatile int 			P1SelectedColor = 0;  
static volatile int 			P2SelectedColor = 0;

/************************** Sprite Animation *********************************/

/*	Fire Sprite Variables  */
static volatile int					g_FireCounter = 150;
static volatile int					g_FireFlip = FALSE;
static int 									s_FireToggleCounter = 0;

/* Explosion Sprite Variables */
static volatile int					g_ExplosionCounter1 = 200;
static volatile int					g_ExplosionCounter2 = 200;
static volatile int					g_ExplosionType = 1;
static 											BOOL explosionInProgress1 = FALSE;
static 											BOOL explosionInProgress2 = FALSE;
static volatile BOOL 				CollisionCheck1;
static volatile BOOL 				CollisionCheck2;

/** Tank  Parameters **/
static volatile int 				g_TankLifeCounter1 = 3;
static volatile int					g_TankLifeCounter2 = 3;
static volatile int					TankOriP1;
static volatile int 				TankOriP2;
static volatile int					TankCounter = 200;
static int 									g_nTankSpeed = 1; // Adjust speed as needed

// TANK ORI P1
static volatile int					TankOri_P1_BLUE= 0;
static volatile int					TankOri_P1_RED = 0;
static volatile int					TankOri_P1_GREEN = 0;
static volatile int					TankOri_P1_PINK = 0;
static volatile int					TankOri_P1_PURPLE = 0;
static volatile int					TankOri_P1_CYAN = 0;
// TANK ORI P2
static volatile int					TankOri_P2_BLUE= 0;
static volatile int					TankOri_P2_RED = 0;
static volatile int					TankOri_P2_GREEN = 0;
static volatile int					TankOri_P2_PINK = 0;
static volatile int					TankOri_P2_PURPLE = 0;
static volatile int					TankOri_P2_CYAN = 0;


/****************************** Game Objects *********************************/
		
/*****************Title Screen*****************/		
static GameObject gObj_TitleScreen = { 
		bmpTitleScreenSprite,
		{TITLE_X_SIZE, TITLE_Y_SIZE}, 
		{10, 10} };

/* Tank Sprite Directional Variables */
static GameObject *currentTankSprite1;
static GameObject *currentTankSprite2;
static volatile int g_nTankDirection1 = 4; // Initialize at East for P1
static volatile BOOL g_bTankMoving1 = FALSE;
static volatile BOOL g_bTankMoving2 = FALSE;
static volatile BOOL TankHitCheck1 = FALSE;
static volatile BOOL TankHitCheck2 = FALSE;

/***************** P1 TANK SPRITES *****************/			
// BLUE
static GameObject gObj_P1_blue_N_tank = { //NORTH
		bmpBLUEtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P1_blue_NW_tank = { //NORTH-WEST
		bmpBLUEtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P1_blue_W_tank = { //WEST
		bmpBLUEtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_blue_SW_tank = { // SOUTH-WEST
		bmpBLUEtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_blue_S_tank = { //SOUTH
		bmpBLUEtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_blue_SE_tank = { // SOUTH-EAST
		bmpBLUEtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_blue_E_tank = { //EAST
		bmpBLUEtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_blue_NE_tank = { // NORTH-EAST
		bmpBLUEtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

// PINK
static GameObject gObj_P1_pink_N_tank = { //NORTH
		bmpPINKtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };			

static GameObject gObj_P1_pink_NW_tank = { //NORTH-WEST
		bmpPINKtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		
	
static GameObject gObj_P1_pink_W_tank = { //WEST
		bmpPINKtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_pink_SW_tank = { // SOUTH-WEST
		bmpPINKtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_pink_S_tank = { //SOUTH
		bmpPINKtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		

static GameObject gObj_P1_pink_SE_tank = { // SOUTH-EAST
		bmpPINKtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_pink_E_tank = { //EAST
		bmpPINKtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_pink_NE_tank = { // NORTH-EAST
		bmpPINKtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };

// PURPLE
static GameObject gObj_P1_purple_N_tank = { //NORTH
		bmpPURPLEtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P1_purple_NW_tank = { //NORTH-WEST
		bmpPURPLEtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P1_purple_W_tank = { //WEST
		bmpPURPLEtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_purple_SW_tank = { // SOUTH-WEST
		bmpPURPLEtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_purple_S_tank = { //SOUTH
		bmpPURPLEtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_purple_SE_tank = { // SOUTH-EAST
		bmpPURPLEtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_purple_E_tank = { //EAST
		bmpPURPLEtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_purple_NE_tank = { // NORTH-EAST
		bmpPURPLEtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

// CYAN
static GameObject gObj_P1_cyan_N_tank = { //NORTH
		bmpCYANtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P1_cyan_NW_tank = { //NORTH-WEST
		bmpCYANtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P1_cyan_W_tank = { //WEST
		bmpCYANtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_cyan_SW_tank = { // SOUTH-WEST
		bmpCYANtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_cyan_S_tank = { //SOUTH
		bmpCYANtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_cyan_SE_tank = { // SOUTH-EAST
		bmpCYANtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_cyan_E_tank = { //EAST
		bmpCYANtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_cyan_NE_tank = { // NORTH-EAST
		bmpCYANtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

// GREEN
static GameObject gObj_P1_green_N_tank = { //NORTH
		bmpGREENtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P1_green_NW_tank = { //NORTH-WEST
		bmpGREENtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P1_green_W_tank = { //WEST
		bmpGREENtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_green_SW_tank = { // SOUTH-WEST
		bmpGREENtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_green_S_tank = { //SOUTH
		bmpGREENtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1_green_SE_tank = { // SOUTH-EAST
		bmpGREENtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_green_E_tank = { //EAST
		bmpGREENtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1_green_NE_tank = { // NORTH-EAST
		bmpGREENtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

// RED
static GameObject gObj_P1_red_N_tank = { //NORTH
		bmpREDtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };			

static GameObject gObj_P1_red_NW_tank = { //NORTH-WEST
		bmpREDtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		
	
static GameObject gObj_P1_red_W_tank = { //WEST
		bmpREDtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_red_SW_tank = { // SOUTH-WEST
		bmpREDtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_red_S_tank = { //SOUTH
		bmpREDtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		

static GameObject gObj_P1_red_SE_tank = { // SOUTH-EAST
		bmpREDtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_red_E_tank = { //EAST
		bmpREDtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P1_red_NE_tank = { // NORTH-EAST
	
		bmpREDtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	
/***************** P2 TANK SPRITES *****************/			
// BLUE
static GameObject gObj_P2_blue_N_tank = { //NORTH
		bmpBLUEtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P2_blue_NW_tank = { //NORTH-WEST
		bmpBLUEtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P2_blue_W_tank = { //WEST
		bmpBLUEtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_blue_SW_tank = { // SOUTH-WEST
		bmpBLUEtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_blue_S_tank = { //SOUTH
		bmpBLUEtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_blue_SE_tank = { // SOUTH-EAST
		bmpBLUEtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_blue_E_tank = { //EAST
		bmpBLUEtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_blue_NE_tank = { // NORTH-EAST
		bmpBLUEtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

// PINK
static GameObject gObj_P2_pink_N_tank = { //NORTH
		bmpPINKtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };			

static GameObject gObj_P2_pink_NW_tank = { //NORTH-WEST
		bmpPINKtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		
	
static GameObject gObj_P2_pink_W_tank = { //WEST
		bmpPINKtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_pink_SW_tank = { // SOUTH-WEST
		bmpPINKtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_pink_S_tank = { //SOUTH
		bmpPINKtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		

static GameObject gObj_P2_pink_SE_tank = { // SOUTH-EAST
		bmpPINKtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_pink_E_tank = { //EAST
		bmpPINKtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_pink_NE_tank = { // NORTH-EAST
		bmpPINKtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };

// PURPLE
static GameObject gObj_P2_purple_N_tank = { //NORTH
		bmpPURPLEtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P2_purple_NW_tank = { //NORTH-WEST
		bmpPURPLEtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P2_purple_W_tank = { //WEST
		bmpPURPLEtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_purple_SW_tank = { // SOUTH-WEST
		bmpPURPLEtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_purple_S_tank = { //SOUTH
		bmpPURPLEtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_purple_SE_tank = { // SOUTH-EAST
		bmpPURPLEtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_purple_E_tank = { //EAST
		bmpPURPLEtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_purple_NE_tank = { // NORTH-EAST
		bmpPURPLEtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

// CYAN
static GameObject gObj_P2_cyan_N_tank = { //NORTH
		bmpCYANtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P2_cyan_NW_tank = { //NORTH-WEST
		bmpCYANtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P2_cyan_W_tank = { //WEST
		bmpCYANtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_cyan_SW_tank = { // SOUTH-WEST
		bmpCYANtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_cyan_S_tank = { //SOUTH
		bmpCYANtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_cyan_SE_tank = { // SOUTH-EAST
		bmpCYANtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_cyan_E_tank = { //EAST
		bmpCYANtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_cyan_NE_tank = { // NORTH-EAST
		bmpCYANtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

// GREEN
static GameObject gObj_P2_green_N_tank = { //NORTH
		bmpGREENtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P2_green_NW_tank = { //NORTH-WEST
		bmpGREENtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	
		
static GameObject gObj_P2_green_W_tank = { //WEST
		bmpGREENtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_green_SW_tank = { // SOUTH-WEST
		bmpGREENtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_green_S_tank = { //SOUTH
		bmpGREENtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P2_green_SE_tank = { // SOUTH-EAST
		bmpGREENtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_green_E_tank = { //EAST
		bmpGREENtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P2_green_NE_tank = { // NORTH-EAST
		bmpGREENtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 50} };		

// RED
static GameObject gObj_P2_red_N_tank = { //NORTH
		bmpREDtankSprite1,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };			

static GameObject gObj_P2_red_NW_tank = { //NORTH-WEST
		bmpREDtankSprite2,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		
	
static GameObject gObj_P2_red_W_tank = { //WEST
		bmpREDtankSprite3,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_red_SW_tank = { // SOUTH-WEST
		bmpREDtankSprite4,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_red_S_tank = { //SOUTH
		bmpREDtankSprite5,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };		

static GameObject gObj_P2_red_SE_tank = { // SOUTH-EAST
		bmpREDtankSprite6,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_red_E_tank = { //EAST
		bmpREDtankSprite7,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	

static GameObject gObj_P2_red_NE_tank = { // NORTH-EAST
	
		bmpREDtankSprite8,
		{TANK_X_SIZE, TANK_Y_SIZE}, 
		{100, 100} };	


		
/* Fire Sprite Variables */

static GameObject gObj_Fire1 = {
		bmpFire1Sprite,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{140, 70} };	
static GameObject gObj_Fire2 = {
		bmpFire2Sprite,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{140, 70} };	

		
/* Misc Sprite Variables */

static GameObject gObj_Cactus = {
bmpCactusSprite,
		{MISC_X_SIZE, MISC_Y_SIZE}, 
		{128, 128} };	


	/* Explosion Sprite Variables */

static GameObject gObj_Explosion1 = {
		bmpExplosion1Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{0, 0} };	
static GameObject gObj_Explosion2 = {
		bmpExplosion2Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{0, 0} };	
static GameObject gObj_Explosion3 = {
		bmpExplosion3Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{0, 0} };	
static GameObject gObj_Explosion4 = {
		bmpExplosion4Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{0, 0} };	
static GameObject gObj_Explosion5 = {
		bmpExplosion5Sprite,
		{EXP_X_SIZE, EXP_Y_SIZE}, 
		{0, 0} };

		
	/* Bullet Sprite Variables */
		
static volatile	BOOL bulletActive1 = FALSE;
// Global variable to store bullet direction
static int g_nBulletDirection1 = 5;
		
static GameObject gObj_Bullet1 = {
		bmpBulletSprite,
		{BULLET_X_SIZE, BULLET_Y_SIZE},
		{0,0},
		{0,0} };

		static volatile	BOOL bulletActive2 = FALSE;
// Global variable to store bullet direction
static int g_nBulletDirection2 = 5;
		
static GameObject gObj_Bullet2 = {
		bmpBulletSprite,
		{BULLET_X_SIZE, BULLET_Y_SIZE},
		{0,0},
		{0,0} };


	/* Blank Sprite */ 
		static GameObject *blankSprite;
static GameObject gObj_Blank = {
  bmpBlankSprite,  {TANK_X_SIZE, TANK_Y_SIZE}, 
  {0, 0} };

	
	/* Border Sprite Variables */		
static GameObject gObj_HorizontalBorder_1 = {
		bmpBorderSprite1,
		{FLATBORDER_X_SIZE, FLATBORDER_Y_SIZE}, 
		{10, 60 } };			

static GameObject gObj_HorizontalBorder_2 = {
		bmpBorderSprite1,
		{FLATBORDER_X_SIZE, FLATBORDER_Y_SIZE}, 
		{10, 145} };	

static GameObject gObj_VerticalBorder_1 = {
		bmpBorderSprite2,
		{SIDEBORDER_X_SIZE, SIDEBORDER_Y_SIZE}, 
		{9,60 } };	

static GameObject gObj_VerticalBorder_2 = {
		bmpBorderSprite2,
		{SIDEBORDER_X_SIZE, SIDEBORDER_Y_SIZE}, 
		{167,60 } };	// GOD POSITION
	

/* Wall Sprite Variables */
static GameObject gObj_LongWall_1 = {
		bmpLongWallSprite,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{50, 55} };		

static GameObject gObj_ShortWall_1 = {
		bmpShortWallSprite1,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{40, 40} };			

static GameObject gObj_ShortWall_2 = {
		bmpShortWallSprite2,
		{FIRE_X_SIZE, FIRE_Y_SIZE}, 
		{100, 100} };	

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
void UpdateTankPositionP1(void);
void UpdateTankPositionP2 (void);
void UpdateTankSprite(void);
void InitBulletP1(void);
void InitBulletP2(void);
void UpdateBulletPosition1(void);
void UpdateBulletPosition2(void);
static volatile BOOL CheckCollision1(GameObject *bullet2, GameObject *tank1);
static volatile BOOL CheckCollision2(GameObject *bullet1, GameObject *tank2);
static void ExplosionAnimate1 (void);
static void ExplosionAnimate2 (void);
int getRandomNumber(int min, int max);
static void selectColorP1(void);	
static void selectColorP2(void);
static void P2TankColor(void);
static void P1TankColor(void);
 
/********* P1 select color functions *********************/		
static void P1_Blue_Config (void);
static void P1_Red_Config (void);
static void P1_Pink_Config (void);
static void P1_Purple_Config (void);
static void P1_Green_Config (void);
static void P1_Cyan_Config (void);

/********* P2 select color functions *********************/		
static void P2_Blue_Config (void);
static void P2_Red_Config (void);
static void P2_Pink_Config (void);
static void P2_Purple_Config (void);
static void P2_Green_Config (void);
static void P2_Cyan_Config (void);	

/********* P1 Sprite Functions ***************************/
void UpdateP1TankSpriteALL(void);
void UpdateP1TankSprite_BLUE(void);
void UpdateP1TankSprite_RED(void);
void UpdateP1TankSprite_PINK(void);
void UpdateP1TankSprite_PURPLE(void);
void UpdateP1TankSprite_GREEN(void);
void UpdateP1TankSprite_CYAN(void);

/********* P2 Sprite Functions ***************************/
void UpdateP2TankSpriteALL(void);
void UpdateP2TankSprite_BLUE(void);
void UpdateP2TankSprite_RED(void);
void UpdateP2TankSprite_PINK(void);
void UpdateP2TankSprite_PURPLE(void);
void UpdateP2TankSprite_GREEN(void);
void UpdateP2TankSprite_CYAN(void);
/*****************************************************************************
 Implementation
******************************************************************************/
		
int main()
{
	Port_Init(); // Initialize all Ports
	SystemCoreClockUpdate ();
	SysTick_Config( SystemCoreClock/ 2000 );  
	
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

  currentTankSprite1 = &gObj_P1_blue_E_tank;
	currentTankSprite2 = &gObj_P2_red_W_tank;
	blankSprite				 = &gObj_Blank;
	
//	srand(time(NULL)); // Seed the random number generator


	
	
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
		
		/* Keypad Functions */
	
		if( FALSE != g_bKeypadScan )
		{
			g_bKeypadScan = FALSE;
			main_KeyScan();   // TODO add directional input to TANK here
			}
		
		/************************ stage 1 (Title Screen) *******************/		
	if (P1_SW1_press == TRUE) // Press SW 1 go tank select
		{
			SW1_debounce++;
			if ((SW1_debounce > DEBOUNCE_DELAY) && P1_SW1_press == TRUE)
			{
				P1_SW1_press = FALSE;
				SW1_debounce = 0;
				Stage = 2; 
			}	
		}		
		
		/************************ stage 2 (Skin Select) *******************/	
	if (Stage == 2)	
{
	if (P1colorChosen == FALSE) 
	{
	P1selectColor();		// P1 choose color
	P1TankColor();
	}
	
	if (P1colorChosen == TRUE && P2colorChosen == FALSE)
	{
		P2selectColor(); // P2 choose color
		P2TankColor();
	}
	
	if (P1colorChosen == TRUE && P2colorChosen == TRUE)
	{
		if (player1Color == player2Color) // P2 choose again if both same color
		{
		P2colorChosen = FALSE;	
		
		Stage = 2;
		}
	
		if (player1Color != player2Color) // else move onto gameplay 
	{
		Stage = 3;
	}
	
	}
}	

/************************ stage 3 (Gameplay) *******************/	
		if (Stage == 3)
{		
	
		Game_In_Progress = TRUE; // starts timer
		
		/* SW1 Debounce Function */
		
		if (P1_SW1_press == TRUE)
		{
			SW1_delay = 0;
			SW1_debounce++;
			if ((SW1_debounce > DEBOUNCE_DELAY) && P1_SW1_press == TRUE)
			{
				SW1_debounce = 0;
			}
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
		
			if (g_TankLifeCounter1 == 0) // If P1 lives drop to 0
				{
					buzz_flag = TRUE;
					Game_In_Progress = FALSE;
				}
		 else if (g_TankLifeCounter2 == 0) // If P2 lives drop to 0
				{
					buzz_flag = TRUE;
					Game_In_Progress = FALSE;
				}
	//	else if (Game_In_Progress == FALSE) // TIMEOUT
//				{
//					buzz_flag = TRUE;
//					Game_In_Progress = FALSE;
//				}
				
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
	
		/** Game Timer Flag **/
	if (Game_In_Progress) {
		int Round_Timer = TIMER_MS;
		while(Round_Timer) {
			Round_Timer--;
		}
		Game_In_Progress=FALSE; // END GAME after timer over
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

	 if (bulletActive1) {
        UpdateBulletPosition1();
		 
		 		 if (CheckCollision1(&gObj_Bullet2, currentTankSprite1) || CollisionCheck1 == TRUE) {
				
				
				TankHitCheck1 = TRUE;    // Tank is confirm hit

			 
			if (TankHitCheck1 == TRUE && g_TankLifeCounter1 == 2 ) {
        // Tank 1 explode and respawn
				 
				explosionInProgress1 = TRUE;
				ExplosionAnimate1();
				 
        int new_x = 120;
				int new_y = 90;

        currentTankSprite1->pos.x = new_x;
        currentTankSprite1->pos.y = new_y;
				TankHitCheck1 = FALSE;
			 
			}
			 
				if (TankHitCheck1 == TRUE && g_TankLifeCounter1 == 1 ) {
        // Tank 2 explode and respawn
				explosionInProgress1 = TRUE;
				ExplosionAnimate1();
        int new_x = 50;
				int new_y = 20;

        currentTankSprite2->pos.x = new_x;
        currentTankSprite2->pos.y = new_y;
				TankHitCheck1 = FALSE;
			 
			}
			
						
			 if (TankHitCheck1 == TRUE && g_TankLifeCounter1 == 0) {
        // Blank out tank 1 and TP to spawn
				 
				explosionInProgress1 = TRUE;
				ExplosionAnimate1();
				 
        int new_x = 140;
				int new_y = 40;
				
				currentTankSprite1 = &gObj_Blank; // replace with blank sprite
					
				currentTankSprite1->pos.x = new_x;
        currentTankSprite1->pos.y = new_y;
				
				 /////////////////////// END OF COLLISION MAP 1 /////////////////////////
		}
	}
}
	
		 
		 
	if (bulletActive2) {
				UpdateBulletPosition2();
		 
		 ///////////////////////// COLLISION MAP /////////////////////////////////////
						 
			 // CHECK WHETHER TANK IS HIT FOR P2
     if (CheckCollision2(&gObj_Bullet1, currentTankSprite2) || CollisionCheck2 == TRUE) {
			 
			  TankHitCheck2 = TRUE;    // Tank is confirm hit
			 
			  if (TankHitCheck2 == TRUE && g_TankLifeCounter2 == 2 ) {
        // Tank 2 explode and respawn 
				 
				explosionInProgress2 = TRUE;
				ExplosionAnimate2();
				 
        int new_x = 120;
				int new_y = 90;

        currentTankSprite2->pos.x = new_x;
        currentTankSprite2->pos.y = new_y;
				TankHitCheck2 = FALSE;
					
				}
				
				 if (TankHitCheck2 == TRUE && g_TankLifeCounter2 == 1 ) {
        // Tank 2 explode and respawn 
				 
				explosionInProgress2 = TRUE;
				ExplosionAnimate2();
				 
        int new_x = 120;
				int new_y = 90;

       currentTankSprite2->pos.x = new_x;
        currentTankSprite2->pos.y = new_y;
				TankHitCheck2 = FALSE;
			 
			}
				  if (TankHitCheck2 == TRUE && g_TankLifeCounter2 == 0) {
        // Blank out tank 2 and TP to spawn
				explosionInProgress2 = TRUE;
				ExplosionAnimate2();
						
				
        int new_x = 140;
				int new_y = 40;
				
				currentTankSprite2 = &gObj_Blank; // replace with blank sprite
					
				currentTankSprite2->pos.x = new_x;
        currentTankSprite2->pos.y = new_y;
			 
			}
		}
	}

			
				 ///////////// END OF COLLISION MAP FOR P2///////////////////

			 
			 

			

	/** TESTING for Fire animation when Life drops to 0 **/
	
	
if (g_TankLifeCounter2 == 0) // When player dead

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
	
/////////////////////////////////////// EDIT HERE FOR P1 and P2 UPDATE SPRITE /////////////////////////////////////		
		 // Handle tank movement for P1
    if (P1_SW1_press)
    {
        g_bTankMoving1 = TRUE; // Tank is moving as long as SW1 is pressed
				UpdateTankPositionP1(); // Update tank position based on direction
    }
    else
    {
        g_bTankMoving1 = FALSE; // Tank stops moving when SW1 is released
    }
		
					
		

		  if (P2_SW1_press)
    {
        g_bTankMoving2 = TRUE; // Tank is moving as long as SW1 is pressed
				UpdateTankPositionP2(); // Update tank position based on direction
    }
    else
    {
        g_bTankMoving2 = FALSE; // Tank stops moving when SW1 is released
    }
	}

		
					
		
	


	

//////////////////////////////end of Systick Handler//////////////////////////////////////


/**************************** Printing to LCD ******************************************/

void GUI_AppDraw( BOOL bFrameStart )
{
	
	/** Initialization Screen **/
	char buf[128];
	GUI_Clear( ClrKhaki ); /* Set background colour */
	GUI_SetFont (&g_FontComic16);

	sprintf(buf, "LIVES: %d", g_TankLifeCounter2); // Print Life counter to LCD
		GUI_PrintString(buf, ClrBlack, 10, 60);

Print_GameObject(currentTankSprite1, TRUE);
Print_GameObject(currentTankSprite2, TRUE);
	
	/*********************************** Stage 1 *************************************/
if (Stage == 1) 
{
	GUI_Clear( ClrKhaki );
	Print_GameObject(&gObj_TitleScreen, FALSE);
	
}

/*********************************** Stage 2 *************************************/	
if (Stage == 2) // Skin selection screen
{
	if (P1colorChosen == FALSE && P2colorChosen == FALSE) 
	{
		if (Cleared_Once == FALSE) // if previous screen not wiped yet
		{
		GUI_Clear(ClrKhaki); // wipe previous content ONCE 
		
		GUI_PrintString( "P1 select tank color", ClrBlack, 4, 75 ); // P1 choose tank color
				
		// ADD printing of color options
		Cleared_Once = TRUE;
		}
	}
	
	Cleared_Once = FALSE;
	
	if (P1colorChosen == TRUE && P2colorChosen == FALSE) // prompt P2 to choose after P1 selected
	{
		if (Cleared_Once == FALSE)
		{
		GUI_Clear(ClrKhaki); // wipe previous content ONCE 
		
		GUI_PrintString( "P2 select tank color", ClrBlack, 4, 75 );

	// ADD printing of color options
		Cleared_Once = TRUE;
		}
	}
	
	Cleared_Once = FALSE;
	
	if (player1Color == player2Color) 
	{
		if (Cleared_Once == FALSE)
		{
		GUI_Clear(ClrKhaki); // wipe previous content ONCE 
		GUI_PrintString( "Reselect tank colors", ClrBlack, 4, 75 ); 
		Cleared_Once = TRUE;
		}	
	

	}
		Cleared_Once = FALSE;
	
}
/*********************************** Stage 3 *************************************/



if (Stage == 3) // Skin selection screen
	{
		/** Print Bullet when fired **/
	if (bulletActive1) 
		{
    Print_GameObject(&gObj_Bullet1, TRUE); // TRUE to draw hitbox, if needed
		}
		
	if (bulletActive2) 
		{
    Print_GameObject(&gObj_Bullet2, TRUE); // TRUE to draw hitbox, if needed
		}	
		
	
			/** Print Fire Animation P1 **/
	if (g_TankLifeCounter1 == 0)
	{
		if (g_FireFlip == FALSE)
		{
		Print_GameObject(&gObj_Fire1, FALSE);
		}
		else
		{
		Print_GameObject(&gObj_Fire2, FALSE);
		}
	}
	
		/** Print Fire Animation P2 **/
	if (g_TankLifeCounter2 == 0)
	{
		if (g_FireFlip == FALSE)
		{
		Print_GameObject(&gObj_Fire1, FALSE);
		}
		else
		{
		Print_GameObject(&gObj_Fire2, FALSE);
		}
	}


		/** Print Explosion Animation **/

		//// Explosion for P1 //////////
		 if (explosionInProgress1) {
						// Draw explosion animation based on `g_ExplosionType`
						switch (g_ExplosionType) {
								case 1:
										Print_GameObject(&gObj_Explosion1, FALSE);
								 gObj_Explosion1.pos = currentTankSprite1->pos;
							 
										break;
								case 2:
										Print_GameObject(&gObj_Explosion2, FALSE);
								 gObj_Explosion2.pos = currentTankSprite1->pos;
							 
										break;
								case 3:
										Print_GameObject(&gObj_Explosion3, FALSE);
								 gObj_Explosion3.pos = currentTankSprite1->pos;
								
										break;
								case 4:
										Print_GameObject(&gObj_Explosion4, FALSE);
								gObj_Explosion4.pos = currentTankSprite1->pos;
								
										break;
								case 5:
										Print_GameObject(&gObj_Explosion5, FALSE);
								gObj_Explosion5.pos = currentTankSprite1->pos;
										break;
								// Add more cases if there are more frames in the explosion animation
								default:
										// Optionally, handle cases where the explosion type is unknown
										break;
							}
						} else {
							 Print_GameObject(currentTankSprite1, TRUE); // Draw Tank 2 normally
						}
						
						/////////////// EXPLOSION for P2 ///////////////
					
				 if (explosionInProgress2) {
						// Draw explosion animation based on `g_ExplosionType`
						switch (g_ExplosionType) {
								case 1:
										Print_GameObject(&gObj_Explosion1, FALSE);
								 gObj_Explosion1.pos = currentTankSprite2->pos;
							 
										break;
								case 2:
										Print_GameObject(&gObj_Explosion2, FALSE);
								 gObj_Explosion2.pos = currentTankSprite2->pos;
							 
										break;
								case 3:
										Print_GameObject(&gObj_Explosion3, FALSE);
								 gObj_Explosion3.pos = currentTankSprite2->pos;
								
										break;
								case 4:
										Print_GameObject(&gObj_Explosion4, FALSE);
								gObj_Explosion4.pos = currentTankSprite2->pos;
								
										break;
								case 5:
										Print_GameObject(&gObj_Explosion5, FALSE);
								gObj_Explosion5.pos = currentTankSprite2->pos;
										break;
								// Add more cases if there are more frames in the explosion animation
								default:
										// Optionally, handle cases where the explosion type is unknown
										break;
							}
						} else {
							 Print_GameObject(currentTankSprite2, TRUE); // Draw Tank 2 normally
						}
					


		/************************* P1 TANK ORI (DELETE WEST) **************************/
		switch (TankOri_P1_BLUE)
			{
				case 1:
				Print_GameObject(&gObj_P1_blue_NW_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P1_blue_N_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P1_blue_NE_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P1_blue_E_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P1_blue_SE_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P1_blue_S_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P1_blue_SW_tank, FALSE);
				break;
			}
			
			
			switch (TankOri_P1_RED)
			{
				case 1:
				Print_GameObject(&gObj_P1_red_NW_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P1_red_N_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P1_red_NE_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P1_red_E_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P1_red_SE_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P1_red_S_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P1_red_SW_tank, FALSE);
				break;
			}
			
		switch (TankOri_P1_PINK)
			{
				case 1:
				Print_GameObject(&gObj_P1_pink_NW_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P1_pink_N_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P1_pink_NE_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P1_pink_E_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P1_pink_SE_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P1_pink_S_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P1_pink_SW_tank, FALSE);
				break;
			}
			
			switch (TankOri_P1_PURPLE)
			{
				case 1:
				Print_GameObject(&gObj_P1_purple_NW_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P1_purple_N_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P1_purple_NE_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P1_purple_E_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P1_purple_SE_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P1_purple_S_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P1_purple_SW_tank, FALSE);
				break;
			}
			
		switch (TankOri_P1_GREEN)
			{
				case 1:
				Print_GameObject(&gObj_P1_green_NW_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P1_green_N_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P1_green_NE_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P1_green_E_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P1_green_SE_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P1_green_S_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P1_green_SW_tank, FALSE);
				break;
			}
			
		switch (TankOri_P1_CYAN)
			{
				case 1:
				Print_GameObject(&gObj_P1_cyan_NW_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P1_cyan_N_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P1_cyan_NE_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P1_cyan_E_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P1_cyan_SE_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P1_cyan_S_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P1_cyan_SW_tank, FALSE);
				break;
			}
			
		/************************ P2 Tank Ori (DELETE EAST) **********************/
		switch (TankOri_P2_BLUE)
			{
				case 1:
				Print_GameObject(&gObj_P2_blue_SE_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P2_blue_S_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P2_blue_SW_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P2_blue_W_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P2_blue_NW_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P2_blue_N_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P2_blue_NE_tank, FALSE);
				break;
			}
			
///////////////////// P2 ORIENTATIONS //////////////////////////
			
		switch (TankOri_P2_RED)
			{
				case 1:
				Print_GameObject(&gObj_P2_red_SE_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P2_red_S_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P2_red_SW_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P2_red_W_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P2_red_NW_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P2_red_N_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P2_red_NE_tank, FALSE);
				break;
			}
			
		switch (TankOri_P2_PINK)
			{
				case 1:
				Print_GameObject(&gObj_P2_pink_SE_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P2_pink_S_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P2_pink_SW_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P2_pink_W_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P2_pink_NW_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P2_pink_N_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P2_pink_NE_tank, FALSE);
				break;
			}
			
			switch (TankOri_P2_PURPLE)
			{
				case 1:
				Print_GameObject(&gObj_P2_purple_SE_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P2_purple_S_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P2_purple_SW_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P2_purple_W_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P2_purple_NW_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P2_purple_N_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P2_purple_NE_tank, FALSE);
				break;
			}
			
		switch (TankOri_P2_GREEN)
			{
				case 1:
				Print_GameObject(&gObj_P2_green_SE_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P2_green_S_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P2_green_SW_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P2_green_W_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P2_green_NW_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P2_green_N_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P2_green_NE_tank, FALSE);
				break;
			}
			
		switch (TankOri_P2_CYAN)
			{
				case 1:
				Print_GameObject(&gObj_P2_cyan_SE_tank, FALSE);
				break;
				case 2:
				Print_GameObject(&gObj_P2_cyan_S_tank, FALSE);
				break;
				case 3:
				Print_GameObject(&gObj_P2_cyan_SW_tank, FALSE);
				break;
				case 4:
				Print_GameObject(&gObj_P2_cyan_W_tank, FALSE);
				break;
				case 5:
				Print_GameObject(&gObj_P2_cyan_NW_tank, FALSE);
				break;
				case 6:
				Print_GameObject(&gObj_P2_cyan_N_tank, FALSE);
				break;
				case 7:
				Print_GameObject(&gObj_P2_cyan_NE_tank, FALSE);
				break;
			}
			
			
			
			if (g_TankLifeCounter1 == 0) // P1 LOSE
			{
				GUI_SetFont( &FONT_Arialbold24 );
				GUI_PrintString( "P2 WINS", ClrBlack, 70, 50 );
			}
			
			if (g_TankLifeCounter2 == 0) // P2 LOSE
			{
				GUI_SetFont( &FONT_Arialbold24 );
				GUI_PrintString( "P1 WINS", ClrBlack, 70, 50 );
			}
			
//		 if (Game_In_Progress == FALSE) // TIMEOUT
//		 {
//			 if (g_TankLifeCounter1 > g_TankLifeCounter2) // P1 MORE LIVES
//			 {
//				GUI_SetFont( &FONT_Arialbold24 );
//				GUI_PrintString( "P1 WINS", ClrBlack, 70, 50 );
//			 }
//			 else if (g_TankLifeCounter1 > g_TankLifeCounter2) // P2 MORE LIVES
//			{
//				GUI_SetFont( &FONT_Arialbold24 );
//				GUI_PrintString( "P2 WINS", ClrBlack, 70, 50 );
//			}
//			else if (g_TankLifeCounter1 == g_TankLifeCounter2) // SAME NO. OF LIVES
//			{
//				GUI_SetFont( &FONT_Arialbold24 );
//				GUI_PrintString( "TIE", ClrBlack, 70, 50 );
//			}
//		} // END OF TIMEOUT
	}// END OF STAGE 3
}// END OF APP DRAW



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


void UpdateP1TankSpriteALL(void)
{
	switch (P1SelectedColor)
	{
		case 1:
			UpdateP1TankSprite_BLUE();
		break;
		case 2:
			UpdateP1TankSprite_RED();
		break;
		case 3:
			UpdateP1TankSprite_PINK();
		break;
		case 4:
			UpdateP1TankSprite_PURPLE();
		break;
		case 5:
			UpdateP1TankSprite_GREEN();
		break;
		case 6:
			UpdateP1TankSprite_CYAN();
		break;
	}
}
	
	void UpdateP2TankSpriteALL(void)
{
	switch (P2SelectedColor)
	{
		case 1:
			UpdateP2TankSprite_BLUE();
		break;
		case 2:
			UpdateP2TankSprite_RED();
		break;
		case 3:
			UpdateP2TankSprite_PINK();
		break;
		case 4:
			UpdateP2TankSprite_PURPLE();
		break;
		case 5:
			UpdateP2TankSprite_GREEN();
		break;
		case 6:
			UpdateP2TankSprite_CYAN();
		break;
	}
}
	
/****************** Tank Sprite positioning *****************************/



void UpdateP1TankSprite_BLUE(void)
{
    GameObject *prevTankSprite1 = currentTankSprite1;

    switch (TankOriP1)
    {
      
        case TANK1_DIRECTION_NORTHWEST:
            currentTankSprite1 = &gObj_P1_blue_NW_tank;
            break;
        case TANK1_DIRECTION_NORTH:
            currentTankSprite1 = &gObj_P1_blue_N_tank;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            currentTankSprite1 = &gObj_P1_blue_NE_tank;
            break;
        case TANK1_DIRECTION_EAST:
            currentTankSprite1 = &gObj_P1_blue_E_tank;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            currentTankSprite1 = &gObj_P1_blue_SE_tank;
            break;  
        case TANK1_DIRECTION_SOUTH:
            currentTankSprite1 = &gObj_P1_blue_S_tank;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            currentTankSprite1 = &gObj_P1_blue_SW_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite1 != currentTankSprite1) {
        currentTankSprite1->pos.x = prevTankSprite1->pos.x;
        currentTankSprite1->pos.y = prevTankSprite1->pos.y;
    }
}

void UpdateP1TankSprite_RED(void)
{
    GameObject *prevTankSprite1 = currentTankSprite1;

    switch (TankOriP1)
    {
      
        case TANK1_DIRECTION_NORTHWEST:
            currentTankSprite1 = &gObj_P1_red_NW_tank;
            break;
        case TANK1_DIRECTION_NORTH:
            currentTankSprite1 = &gObj_P1_red_N_tank;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            currentTankSprite1 = &gObj_P1_red_NE_tank;
            break;
        case TANK1_DIRECTION_EAST:
            currentTankSprite1 = &gObj_P1_red_E_tank;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            currentTankSprite1 = &gObj_P1_red_SE_tank;
            break;  
        case TANK1_DIRECTION_SOUTH:
            currentTankSprite1 = &gObj_P1_red_S_tank;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            currentTankSprite1 = &gObj_P1_red_SW_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite1 != currentTankSprite1) {
        currentTankSprite1->pos.x = prevTankSprite1->pos.x;
        currentTankSprite1->pos.y = prevTankSprite1->pos.y;
    }
}

void UpdateP1TankSprite_PINK(void)
{
    GameObject *prevTankSprite1 = currentTankSprite1;

    switch (TankOriP1)
    {
      
        case TANK1_DIRECTION_NORTHWEST:
            currentTankSprite1 = &gObj_P1_pink_NW_tank;
            break;
        case TANK1_DIRECTION_NORTH:
            currentTankSprite1 = &gObj_P1_pink_N_tank;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            currentTankSprite1 = &gObj_P1_pink_NE_tank;
            break;
        case TANK1_DIRECTION_EAST:
            currentTankSprite1 = &gObj_P1_pink_E_tank;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            currentTankSprite1 = &gObj_P1_pink_SE_tank;
            break;  
        case TANK1_DIRECTION_SOUTH:
            currentTankSprite1 = &gObj_P1_pink_S_tank;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            currentTankSprite1 = &gObj_P1_pink_SW_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite1 != currentTankSprite1) {
        currentTankSprite1->pos.x = prevTankSprite1->pos.x;
        currentTankSprite1->pos.y = prevTankSprite1->pos.y;
    }
}

void UpdateP1TankSprite_PURPLE(void)
{
    GameObject *prevTankSprite1 = currentTankSprite1;

    switch (TankOriP1)
    {
      
        case TANK1_DIRECTION_NORTHWEST:
            currentTankSprite1 = &gObj_P1_purple_NW_tank;
            break;
        case TANK1_DIRECTION_NORTH:
            currentTankSprite1 = &gObj_P1_purple_N_tank;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            currentTankSprite1 = &gObj_P1_purple_NE_tank;
            break;
        case TANK1_DIRECTION_EAST:
            currentTankSprite1 = &gObj_P1_purple_E_tank;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            currentTankSprite1 = &gObj_P1_purple_SE_tank;
            break;  
        case TANK1_DIRECTION_SOUTH:
            currentTankSprite1 = &gObj_P1_purple_S_tank;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            currentTankSprite1 = &gObj_P1_purple_SW_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite1 != currentTankSprite1) {
        currentTankSprite1->pos.x = prevTankSprite1->pos.x;
        currentTankSprite1->pos.y = prevTankSprite1->pos.y;
    }
}

void UpdateP1TankSprite_GREEN(void)
{
    GameObject *prevTankSprite1 = currentTankSprite1;

    switch (TankOriP1)
    {
      
        case TANK1_DIRECTION_NORTHWEST:
            currentTankSprite1 = &gObj_P1_green_NW_tank;
            break;
        case TANK1_DIRECTION_NORTH:
            currentTankSprite1 = &gObj_P1_green_N_tank;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            currentTankSprite1 = &gObj_P1_green_NE_tank;
            break;
        case TANK1_DIRECTION_EAST:
            currentTankSprite1 = &gObj_P1_green_E_tank;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            currentTankSprite1 = &gObj_P1_green_SE_tank;
            break;  
        case TANK1_DIRECTION_SOUTH:
            currentTankSprite1 = &gObj_P1_green_S_tank;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            currentTankSprite1 = &gObj_P1_green_SW_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite1 != currentTankSprite1) {
        currentTankSprite1->pos.x = prevTankSprite1->pos.x;
        currentTankSprite1->pos.y = prevTankSprite1->pos.y;
    }
}

void UpdateP1TankSprite_CYAN(void)
{
    GameObject *prevTankSprite1 = currentTankSprite1;

    switch (TankOriP1)
    {
      
        case TANK1_DIRECTION_NORTHWEST:
            currentTankSprite1 = &gObj_P1_cyan_NW_tank;
            break;
        case TANK1_DIRECTION_NORTH:
            currentTankSprite1 = &gObj_P1_cyan_N_tank;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            currentTankSprite1 = &gObj_P1_cyan_NE_tank;
            break;
        case TANK1_DIRECTION_EAST:
            currentTankSprite1 = &gObj_P1_cyan_E_tank;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            currentTankSprite1 = &gObj_P1_cyan_SE_tank;
            break;  
        case TANK1_DIRECTION_SOUTH:
            currentTankSprite1 = &gObj_P1_cyan_S_tank;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            currentTankSprite1 = &gObj_P1_cyan_SW_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite1 != currentTankSprite1) {
        currentTankSprite1->pos.x = prevTankSprite1->pos.x;
        currentTankSprite1->pos.y = prevTankSprite1->pos.y;
    }
}

/****************************** P2 UPDATE SPRITES ************************************/

void UpdateP2TankSprite_BLUE(void)
{
    GameObject *prevTankSprite2 = currentTankSprite2;

    switch (TankOriP2)
    {
      
        case TANK2_DIRECTION_SOUTHEAST:
            currentTankSprite2 = &gObj_P2_blue_SE_tank;
            break;
        case TANK2_DIRECTION_SOUTH:
            currentTankSprite2 = &gObj_P2_blue_S_tank;
            break;
        case TANK2_DIRECTION_SOUTHWEST:
            currentTankSprite2 = &gObj_P2_blue_SW_tank;
            break;
        case TANK2_DIRECTION_WEST:
            currentTankSprite2 = &gObj_P2_blue_W_tank;
            break;
        case TANK2_DIRECTION_NORTHWEST:
            currentTankSprite2 = &gObj_P2_blue_NW_tank;
            break;  
        case TANK2_DIRECTION_NORTH:
            currentTankSprite2 = &gObj_P2_blue_N_tank;
            break;
        case TANK2_DIRECTION_NORTHEAST:
            currentTankSprite2 = &gObj_P2_blue_NE_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite2 != currentTankSprite2) {
        currentTankSprite2->pos.x = prevTankSprite2->pos.x;
        currentTankSprite2->pos.y = prevTankSprite2->pos.y;
    }
}

void UpdateP2TankSprite_RED(void)
{
    GameObject *prevTankSprite2 = currentTankSprite2;

    switch (TankOriP2)
    {
      
        case TANK2_DIRECTION_SOUTHEAST:
            currentTankSprite2 = &gObj_P2_red_SE_tank;
            break;
        case TANK2_DIRECTION_SOUTH:
            currentTankSprite2 = &gObj_P2_red_S_tank;
            break;
        case TANK2_DIRECTION_SOUTHWEST:
            currentTankSprite2 = &gObj_P2_red_SW_tank;
            break;
        case TANK2_DIRECTION_WEST:
            currentTankSprite2 = &gObj_P2_red_W_tank;
            break;
        case TANK2_DIRECTION_NORTHWEST:
            currentTankSprite2 = &gObj_P2_red_NW_tank;
            break;  
        case TANK2_DIRECTION_NORTH:
            currentTankSprite2 = &gObj_P2_red_N_tank;
            break;
        case TANK2_DIRECTION_NORTHEAST:
            currentTankSprite2 = &gObj_P2_red_NE_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite2 != currentTankSprite2) {
        currentTankSprite2->pos.x = prevTankSprite2->pos.x;
        currentTankSprite2->pos.y = prevTankSprite2->pos.y;
    }
}
void UpdateP2TankSprite_PINK(void)
{
    GameObject *prevTankSprite2 = currentTankSprite2;

    switch (TankOriP2)
    {
      
        case TANK2_DIRECTION_SOUTHEAST:
            currentTankSprite2 = &gObj_P2_pink_SE_tank;
            break;
        case TANK2_DIRECTION_SOUTH:
            currentTankSprite2 = &gObj_P2_pink_S_tank;
            break;
        case TANK2_DIRECTION_SOUTHWEST:
            currentTankSprite2 = &gObj_P2_pink_SW_tank;
            break;
        case TANK2_DIRECTION_WEST:
            currentTankSprite2 = &gObj_P2_pink_W_tank;
            break;
        case TANK2_DIRECTION_NORTHWEST:
            currentTankSprite2 = &gObj_P2_pink_NW_tank;
            break;  
        case TANK2_DIRECTION_NORTH:
            currentTankSprite2 = &gObj_P2_pink_N_tank;
            break;
        case TANK2_DIRECTION_NORTHEAST:
            currentTankSprite2 = &gObj_P2_pink_NE_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite2 != currentTankSprite2) {
        currentTankSprite2->pos.x = prevTankSprite2->pos.x;
        currentTankSprite2->pos.y = prevTankSprite2->pos.y;
    }
}
void UpdateP2TankSprite_PURPLE(void)
{
    GameObject *prevTankSprite2 = currentTankSprite2;

    switch (TankOriP2)
    {
      
        case TANK2_DIRECTION_SOUTHEAST:
            currentTankSprite2 = &gObj_P2_purple_SE_tank;
            break;
        case TANK2_DIRECTION_SOUTH:
            currentTankSprite2 = &gObj_P2_purple_S_tank;
            break;
        case TANK2_DIRECTION_SOUTHWEST:
            currentTankSprite2 = &gObj_P2_purple_SW_tank;
            break;
        case TANK2_DIRECTION_WEST:
            currentTankSprite2 = &gObj_P2_purple_W_tank;
            break;
        case TANK2_DIRECTION_NORTHWEST:
            currentTankSprite2 = &gObj_P2_purple_NW_tank;
            break;  
        case TANK2_DIRECTION_NORTH:
            currentTankSprite2 = &gObj_P2_purple_N_tank;
            break;
        case TANK2_DIRECTION_NORTHEAST:
            currentTankSprite2 = &gObj_P2_purple_NE_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite2 != currentTankSprite2) {
        currentTankSprite2->pos.x = prevTankSprite2->pos.x;
        currentTankSprite2->pos.y = prevTankSprite2->pos.y;
    }
}
void UpdateP2TankSprite_GREEN(void)
{
    GameObject *prevTankSprite2 = currentTankSprite2;

    switch (TankOriP2)
    {
      
        case TANK2_DIRECTION_SOUTHEAST:
            currentTankSprite2 = &gObj_P2_green_SE_tank;
            break;
        case TANK2_DIRECTION_SOUTH:
            currentTankSprite2 = &gObj_P2_green_S_tank;
            break;
        case TANK2_DIRECTION_SOUTHWEST:
            currentTankSprite2 = &gObj_P2_green_SW_tank;
            break;
        case TANK2_DIRECTION_WEST:
            currentTankSprite2 = &gObj_P2_green_W_tank;
            break;
        case TANK2_DIRECTION_NORTHWEST:
            currentTankSprite2 = &gObj_P2_green_NW_tank;
            break;  
        case TANK2_DIRECTION_NORTH:
            currentTankSprite2 = &gObj_P2_green_N_tank;
            break;
        case TANK2_DIRECTION_NORTHEAST:
            currentTankSprite2 = &gObj_P2_green_NE_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite2 != currentTankSprite2) {
        currentTankSprite2->pos.x = prevTankSprite2->pos.x;
        currentTankSprite2->pos.y = prevTankSprite2->pos.y;
    }
}
void UpdateP2TankSprite_CYAN(void)
{
    GameObject *prevTankSprite2 = currentTankSprite2;

    switch (TankOriP2)
    {
      
        case TANK2_DIRECTION_SOUTHEAST:
            currentTankSprite2 = &gObj_P2_cyan_SE_tank;
            break;
        case TANK2_DIRECTION_SOUTH:
            currentTankSprite2 = &gObj_P2_cyan_S_tank;
            break;
        case TANK2_DIRECTION_SOUTHWEST:
            currentTankSprite2 = &gObj_P2_cyan_SW_tank;
            break;
        case TANK2_DIRECTION_WEST:
            currentTankSprite2 = &gObj_P2_cyan_W_tank;
            break;
        case TANK2_DIRECTION_NORTHWEST:
            currentTankSprite2 = &gObj_P2_cyan_NW_tank;
            break;  
        case TANK2_DIRECTION_NORTH:
            currentTankSprite2 = &gObj_P2_cyan_N_tank;
            break;
        case TANK2_DIRECTION_NORTHEAST:
            currentTankSprite2 = &gObj_P2_cyan_NE_tank;
            break;
    }

    // Copy the previous position to the new sprite
    if (prevTankSprite2 != currentTankSprite2) {
        currentTankSprite2->pos.x = prevTankSprite2->pos.x;
        currentTankSprite2->pos.y = prevTankSprite2->pos.y;
    }
}

void UpdateTankPositionP1(void)
{
    static int tickCount1 = 0;
    tickCount1++;
		static int pixelSpeed1 = 2; // Movement speed

    // Only move the tank once every 500 ticks
    if (tickCount1 >= 200 && g_bTankMoving1)
    {
        tickCount1 = 0; // Reset tick counter

        switch (TankOriP1)
        {
            
						case TANK1_DIRECTION_NORTHWEST:
								currentTankSprite1->pos.x -= pixelSpeed1;
                currentTankSprite1->pos.y -= pixelSpeed1;
                break;						
        
            case TANK1_DIRECTION_NORTH:
               currentTankSprite1->pos.y -= pixelSpeed1;
						        break;
						 case TANK1_DIRECTION_NORTHEAST:
                currentTankSprite1->pos.x += pixelSpeed1;
                currentTankSprite1->pos.y -= pixelSpeed1;
                break;
						  case TANK1_DIRECTION_EAST:
                currentTankSprite1->pos.x += pixelSpeed1;
                break;
							case TANK1_DIRECTION_SOUTHEAST:
                currentTankSprite1->pos.x += pixelSpeed1;
                currentTankSprite1->pos.y += pixelSpeed1;
                break;
							 case TANK1_DIRECTION_SOUTH:
                currentTankSprite1->pos.y += pixelSpeed1;
                break;
							 case TANK1_DIRECTION_SOUTHWEST:
                currentTankSprite1->pos.x -= pixelSpeed1;
                currentTankSprite1->pos.y += pixelSpeed1;
                break;
         
        }
    }
}

void UpdateTankPositionP2(void)
{
    static int tickCount2 = 0;
    tickCount2++;
		static int pixelSpeed2 = 2; // Movement speed

    // Only move the tank once every 500 ticks
    if (tickCount2 >= 200 && g_bTankMoving2)
    {
        tickCount2 = 0; // Reset tick counter

        switch (TankOriP2)
        {
            case TANK2_DIRECTION_SOUTHEAST:
                currentTankSprite2->pos.x += pixelSpeed2;
                currentTankSprite2->pos.y += pixelSpeed2;
						case TANK2_DIRECTION_SOUTH:
                currentTankSprite2->pos.y += pixelSpeed2;
                break;
						
						case TANK2_DIRECTION_SOUTHWEST:
                currentTankSprite2->pos.x -= pixelSpeed2;
                currentTankSprite2->pos.y += pixelSpeed2;
                break;
						case TANK2_DIRECTION_WEST:
                currentTankSprite2->pos.x -= pixelSpeed2;
                break;
						case TANK2_DIRECTION_NORTHWEST:
								currentTankSprite2->pos.x -= pixelSpeed2;
                currentTankSprite2->pos.y -= pixelSpeed2;
                break;						
        
            case TANK2_DIRECTION_NORTH:
               currentTankSprite2->pos.y -= pixelSpeed2;
								break;
						case TANK2_DIRECTION_NORTHEAST:
                currentTankSprite2->pos.x += pixelSpeed2;
                currentTankSprite2->pos.y -= pixelSpeed2;
                break;
					
        }
    }
}
		
void InitBulletP1(void) {
    gObj_Bullet1.sprite = bmpBulletSprite;
    gObj_Bullet1.size.x = BULLET_X_SIZE;
    gObj_Bullet1.size.y = BULLET_Y_SIZE;

    // Calculate bullet initial position based on the tank's orientation
		// DO NOT ADJUST THESE VALUES!!! THEY HAVE BEEN CALIBRATED TO THE TANK'S FRONT ALREADY
    switch (TankOriP1) {
			
			  case TANK1_DIRECTION_NORTHWEST:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x + 2;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y ;
            break;
        case TANK1_DIRECTION_NORTH:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x + 1;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y - 5;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x + 5;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y - 6;
            break;
        case TANK1_DIRECTION_EAST:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x + 5;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y - 3;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y - 4;
            break;
        case TANK1_DIRECTION_SOUTH:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x + 1;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y + 3;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            gObj_Bullet1.pos.x = currentTankSprite1->pos.x - 1;
            gObj_Bullet1.pos.y = currentTankSprite1->pos.y - 1 ;
            break;

    }

    bulletActive1 = TRUE; // Activate the bullet
}

void InitBulletP2(void) {
    gObj_Bullet2.sprite = bmpBulletSprite;
    gObj_Bullet2.size.x = BULLET_X_SIZE;
    gObj_Bullet2.size.y = BULLET_Y_SIZE;

    // Calculate bullet initial position based on the tank's orientation
		// DO NOT ADJUST THESE VALUES!!! THEY HAVE BEEN CALIBRATED TO THE TANK'S FRONT ALREADY
    switch (TankOriP2) {
			
			  case TANK1_DIRECTION_NORTHWEST:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x + 2;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y ;
            break;
        case TANK1_DIRECTION_NORTH:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x + 1;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y - 5;
            break;
        case TANK1_DIRECTION_NORTHEAST:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x + 5;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y - 6;
            break;
        case TANK1_DIRECTION_EAST:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x + 5;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y - 3;
            break;
        case TANK1_DIRECTION_SOUTHEAST:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y - 4;
            break;
        case TANK1_DIRECTION_SOUTH:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x + 1;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y + 3;
            break;
        case TANK1_DIRECTION_SOUTHWEST:
            gObj_Bullet2.pos.x = currentTankSprite2->pos.x - 1;
            gObj_Bullet2.pos.y = currentTankSprite2->pos.y - 1 ;
            break;

    }

    bulletActive2 = TRUE; // Activate the bullet
}

void UpdateBulletPosition1(void) {
    if (!bulletActive1) return;

    static int tickCountBullet1 = 0;
    tickCountBullet1++;
		static int BulletSpeed1 = 5; // Movement speed

    // Only move the tank once every 500 ticks
    if (tickCountBullet1 >= 200 && g_bTankMoving1)
    {
        tickCountBullet1 = 0; // Reset tick counter

    switch(g_nBulletDirection1) {
			       case TANK1_DIRECTION_NORTHWEST:
            gObj_Bullet1.pos.x -= BulletSpeed1;
            gObj_Bullet1.pos.y -= BulletSpeed1;
            break;
						case TANK1_DIRECTION_NORTH:
            gObj_Bullet1.pos.y -= BulletSpeed1;
            break;
				    case TANK1_DIRECTION_NORTHEAST:
            gObj_Bullet1.pos.x += BulletSpeed1;
            gObj_Bullet1.pos.y -= BulletSpeed1;
            break;
						case TANK1_DIRECTION_EAST:
            gObj_Bullet1.pos.x += BulletSpeed1;
            break;
						case TANK1_DIRECTION_SOUTHEAST:
            gObj_Bullet1.pos.x += BulletSpeed1;
            gObj_Bullet1.pos.y += BulletSpeed1;
            break;
						case TANK1_DIRECTION_SOUTH:
            gObj_Bullet1.pos.y += BulletSpeed1;
            break;
						case TANK1_DIRECTION_SOUTHWEST:
            gObj_Bullet1.pos.x -= BulletSpeed1;
            gObj_Bullet1.pos.y += BulletSpeed1;
            break;

    }

    // Check for border collision
   if (gObj_Bullet1.pos.y <= 0 || gObj_Bullet1.pos.y >= SCREEN_HEIGHT 
        || gObj_Bullet1.pos.x <= 0 || gObj_Bullet1.pos.x >= SCREEN_WIDTH) {
        bulletActive1 = FALSE; // Deactivate the bullet
    }
	}
}

void UpdateBulletPosition2(void) {
    if (!bulletActive2) return;

    static int tickCountBullet2 = 0;
    tickCountBullet2++;
		static int BulletSpeed2 = 5; // Movement speed

    // Only move the tank once every 500 ticks
    if (tickCountBullet2 >= 200 && g_bTankMoving2)
    {
        tickCountBullet2 = 0; // Reset tick counter

    switch(g_nBulletDirection2) {
			
						case TANK2_DIRECTION_SOUTHEAST:
            gObj_Bullet2.pos.x += BulletSpeed2;
            gObj_Bullet2.pos.y += BulletSpeed2;
            break;
						case TANK2_DIRECTION_SOUTH:
            gObj_Bullet2.pos.y += BulletSpeed2;
            break;
						case TANK2_DIRECTION_SOUTHWEST:
            gObj_Bullet2.pos.x -= BulletSpeed2;
            gObj_Bullet2.pos.y += BulletSpeed2;
            break;
						case TANK2_DIRECTION_WEST:
            gObj_Bullet2.pos.x += BulletSpeed2;
            break;
			      case TANK2_DIRECTION_NORTHWEST:
            gObj_Bullet2.pos.x -= BulletSpeed2;
            gObj_Bullet2.pos.y -= BulletSpeed2;
            break;
						case TANK2_DIRECTION_NORTH:
            gObj_Bullet2.pos.y -= BulletSpeed2;
            break;
				    case TANK2_DIRECTION_NORTHEAST:
            gObj_Bullet2.pos.x += BulletSpeed2;
            gObj_Bullet2.pos.y -= BulletSpeed2;
            break;

    }

    // Check for border collision
   if (gObj_Bullet2.pos.y <= 0 || gObj_Bullet2.pos.y >= SCREEN_HEIGHT 
        || gObj_Bullet2.pos.x <= 0 || gObj_Bullet2.pos.x >= SCREEN_WIDTH) {
        bulletActive2 = FALSE; // Deactivate the bullet
    }
	}
}

////////////////////////// CHECK BACK HERE TO SEE IF NEED TO UPDATE BULLET FOR INDI PLAYER ////////////

static volatile BOOL CheckCollision2(GameObject *bullet1, GameObject *tank2) {
    int bulletLeft1 = bullet1->pos.x;
    int bulletRight1 = bullet1->pos.x + bullet1->size.x;
    int bulletTop1 = bullet1->pos.y;
    int bulletBottom1 = bullet1->pos.y + bullet1->size.y;

    // Assuming tank->pos represents the top-left corner of the tank
    int tankLeft2 = tank2->pos.x;
    int tankRight2 = tank2->pos.x + tank2->size.x;
    int tankTop2 = tank2->pos.y;
    int tankBottom2 = tank2->pos.y + tank2->size.y;

    // Check if any side of the bullet is inside the tank's hitbox
    if (bulletLeft1 < tankRight2 && bulletRight1 > tankLeft2 &&
        bulletTop1 < tankBottom2 && bulletBottom1 > tankTop2) { 
				
				CollisionCheck2 = TRUE;
			 	g_TankLifeCounter2--;

						
					
        return TRUE;
    }

    return FALSE;
}

static volatile BOOL CheckCollision1(GameObject *bullet2, GameObject *tank1) {
    int bulletLeft2 = bullet2->pos.x;
    int bulletRight2 = bullet2->pos.x + bullet2->size.x;
    int bulletTop2 = bullet2->pos.y;
    int bulletBottom2 = bullet2->pos.y + bullet2->size.y;

    // Assuming tank->pos represents the top-left corner of the tank
    int tankLeft = tank1->pos.x;
    int tankRight = tank1->pos.x + tank1->size.x;
    int tankTop = tank1->pos.y;
    int tankBottom = tank1->pos.y + tank1->size.y;

    // Check if any side of the bullet is inside the tank's hitbox
    if (bulletLeft2 < tankRight && bulletRight2 > tankLeft &&
        bulletTop2 < tankBottom && bulletBottom2 > tankTop) { 
				
				CollisionCheck1 = TRUE;
			 	g_TankLifeCounter1--;

						
					
        return TRUE;
    }

    return FALSE;
}

static void ExplosionAnimate1 (void)
{	
	
	 if (explosionInProgress1) // Enter the condition even if SW1_press becomes false    
		{
        g_ExplosionCounter1--;
			
        if (g_ExplosionCounter1 == 0)        
				{
            g_ExplosionType++;            
						g_ExplosionCounter1 = 200;
					
            if (g_ExplosionType > 5)
						{
                g_ExplosionType = 1;
                explosionInProgress1 = FALSE; // Reset explosion state when > 5 
								CollisionCheck1 = FALSE;
						}
            else            
						{
								CollisionCheck1 = TRUE;
                explosionInProgress1 = TRUE; // Set explosion state when SW1_press is true            
            }
						// Set explosion position to Tank 2's position
           
					}
				
					  
			}
		}

		static void ExplosionAnimate2 (void)
{	
	
	 if (explosionInProgress2) // Enter the condition even if SW1_press becomes false    
		{
        g_ExplosionCounter2--;
			
        if (g_ExplosionCounter2 == 0)        
				{
            g_ExplosionType++;            
						g_ExplosionCounter2 = 200;
					
            if (g_ExplosionType > 5)
						{
                g_ExplosionType = 1;
                explosionInProgress2 = FALSE; // Reset explosion state when > 5 
								CollisionCheck2 = FALSE;
						}
            else            
						{
								CollisionCheck2 = TRUE;
                explosionInProgress2 = TRUE; // Set explosion state when SW1_press is true            
            }
						// Set explosion position to Tank 2's position
           
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


void main_AdcUpdate(void) {
    static uint16_t lastResult_MUX0_P1 = 0;
    static uint16_t lastResult_MUX0_P2 = 0;

    uint16_t Result_MUX0_P1 = ADC0_GET_MUX0_FIFO();
    uint16_t Result_MUX0_P2 = ADC1_GET_MUX0_FIFO();

    // Update for Player 1
    if (abs(Result_MUX0_P1 - lastResult_MUX0_P1) > POTENTIOMETER_THRESHOLD) {
        TankOriP1 = (Result_MUX0_P1 * 7) / 4096 + 1; // Assuming 12-bit ADC resolution
        if (TankOriP1 > 7) TankOriP1 = 7;
        if (TankOriP1 < 1) TankOriP1 = 1;
        lastResult_MUX0_P1 = Result_MUX0_P1;
        UpdateP1TankSpriteALL();
    }

    // Update for Player 2
    if (abs(Result_MUX0_P2 - lastResult_MUX0_P2) > POTENTIOMETER_THRESHOLD) {
        TankOriP2 = (Result_MUX0_P2 * 7) / 4096 + 1; // Assuming 12-bit ADC resolution
        if (TankOriP2 > 7) TankOriP2 = 7;
        if (TankOriP2 < 1) TankOriP2 = 1;
        lastResult_MUX0_P2 = Result_MUX0_P2;
        UpdateP1TankSpriteALL();
    }

    ADC0_SS0_Start();
    ADC1_SS0_Start();
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

/********************** Tank Color Selection ***********************************/


///********************** Tank Color Selection ***********************************/
void P1selectColor(void) // P1 Select color
{ 
	uint8_t P1input;
		
		if(g_cKey != '-' && g_cKey != '0' && g_cKey != '7' && g_cKey != '8' && g_cKey != '9') // Get input from keypad
	{
			if(g_cKey != '*' && g_cKey != '#') // STORE READ VALUE
			{
    P1input = CHAR_TO_INT(g_cKey);
		g_cKey = '-';
			}
		else
		{
				/*  '*' or '#' entered  */
    // Map keypad input to color options

        P1SelectedColor = P1input;
				P1colorChosen = TRUE;
			}
		g_cKey = '-'; // reset to '-'
		}
	}



void P2selectColor(void) // P2 Select color
{ 
	uint8_t P2input; // Get input from keypad
		if(g_cKey != '-' && g_cKey != '0' && g_cKey != '7' && g_cKey != '8' && g_cKey != '9')
		{
			if(g_cKey != '*' && g_cKey != '#') // STORE READ VALUE
			{
    P2input = CHAR_TO_INT(g_cKey);
		g_cKey = '-';
			}
			else
		{
		/*  '*' or '#' entered  */
     P2SelectedColor = P2input;	
		P2colorChosen = TRUE;			
    }
		}
}

static void P1TankColor(void)
	{
	switch (P1SelectedColor) 
		{
		case 1:
		P1_Blue_Config ();
		TankOriP1 = TankOri_P1_BLUE;
		break;
		case 2:
		P1_Red_Config ();
		TankOriP1 = TankOri_P1_RED;
		break;
		case 3:
		P1_Pink_Config ();	
		TankOriP1 = TankOri_P1_PINK;
		break;
		case 4:
		P1_Purple_Config ();
		TankOriP1 = TankOri_P1_PURPLE;
		break;		
		case 5:
		P1_Green_Config ();	
		TankOriP1 = TankOri_P1_GREEN;
		break;
		case 6:
		P1_Cyan_Config ();	
		TankOriP1 = TankOri_P1_CYAN;
		break;
		}
} //END P1TankColor Function
	
static void P2TankColor(void)
	{
	switch (P2SelectedColor) 
		{
		case 1:
		P2_Blue_Config ();
		TankOriP2 = TankOri_P2_BLUE;
		break;
		case 2:
		P2_Red_Config ();
		TankOriP2 = TankOri_P2_RED;
		break;
		case 3:
		P2_Pink_Config ();
		TankOriP2 = TankOri_P2_PINK;	
		break;
		case 4:
		P2_Purple_Config ();
		TankOriP2 = TankOri_P2_PURPLE;
		break;		
		case 5:
		P2_Green_Config ();
		TankOriP2 = TankOri_P2_GREEN;		
		break;
		case 6:
		P2_Cyan_Config ();	
		TankOriP2 = TankOri_P2_CYAN;
		break;
		}
} //END P2TankColor Function
	
/******************** P1 SWITCH CASES FOR TANK MOVEMENT *************/
static void P1_Blue_Config (void)
{
switch (TankOri_P1_BLUE)
	{
		case 1:
		Print_GameObject(&gObj_P1_blue_NW_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1_blue_N_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P1_blue_NE_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1_blue_E_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P1_blue_SE_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1_blue_S_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P1_blue_SW_tank, FALSE);
		break;
	}
} // END P1 BLUE CONFIG

static void P1_Red_Config (void)
{
	switch (TankOri_P1_RED)
	{
			case 1:
		Print_GameObject(&gObj_P1_red_NW_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1_red_N_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P1_red_NE_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1_red_E_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P1_red_SE_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1_red_S_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P1_red_SW_tank, FALSE);
		break;
	}
} // END P1 RED CONFIG

static void P1_Pink_Config (void)
{
switch (TankOri_P1_PINK)
	{
				case 1:
		Print_GameObject(&gObj_P1_pink_NW_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1_pink_N_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P1_pink_NE_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1_pink_E_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P1_pink_SE_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1_pink_S_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P1_pink_SW_tank, FALSE);
		break;
	}
} // END P1 PINK CONFIG

static void P1_Purple_Config (void)
{
switch (TankOri_P1_PURPLE)
	{
				case 1:
		Print_GameObject(&gObj_P1_purple_NW_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1_purple_N_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P1_purple_NE_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1_purple_E_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P1_purple_SE_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1_purple_S_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P1_purple_SW_tank, FALSE);
		break;
	}
} // END P1 PURPLE CONFIG
	

static void P1_Green_Config (void)
{
switch (TankOri_P1_GREEN)
	{
				case 1:
		Print_GameObject(&gObj_P1_green_NW_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1_green_N_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P1_green_NE_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1_green_E_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P1_green_SE_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1_green_S_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P1_green_SW_tank, FALSE);
		break;
	}
} // END P1 GREEN CONFIG


static void P1_Cyan_Config (void)
{
switch (TankOri_P1_CYAN)
	{
			case 1:
		Print_GameObject(&gObj_P1_cyan_NW_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1_cyan_N_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P1_cyan_NE_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1_cyan_E_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P1_cyan_SE_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1_cyan_S_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P1_cyan_SW_tank, FALSE);
		break;
	}
}	// END P1 CYAN CONFIG

/******************** P2 SWITCH CASES FOR TANK MOVEMENT *************/
static void P2_Blue_Config (void)
{	
switch (TankOri_P2_BLUE)
	{
		case 1:
		Print_GameObject(&gObj_P2_blue_SE_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P2_blue_S_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P2_blue_SW_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P2_blue_W_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P2_blue_NW_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P2_blue_N_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P2_blue_NE_tank, FALSE);
		break;
	}
} // END P2_Blue_Config

static void P2_Red_Config (void)
{	
switch (TankOri_P2_RED)
	{
		case 1:
		Print_GameObject(&gObj_P2_red_SE_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P2_red_S_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P2_red_SW_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P2_red_W_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P2_red_NW_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P2_red_N_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P2_red_NE_tank, FALSE);
		break;
	}
} // END P2 RED CONFIG

static void P2_Pink_Config (void)
{
switch (TankOri_P2_PINK)
	{
		case 1:
		Print_GameObject(&gObj_P2_pink_SE_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P2_pink_S_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P2_pink_SW_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P2_pink_W_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P2_pink_NW_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P2_pink_N_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P2_pink_NE_tank, FALSE);
		break;
	}
} // END P2 PINK CONFIG

static void P2_Purple_Config (void)
{
	switch (TankOri_P2_PURPLE)
	{
			case 1:
		Print_GameObject(&gObj_P2_purple_SE_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P2_purple_S_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P2_purple_SW_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P2_purple_W_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P2_purple_NW_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P2_purple_N_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P2_purple_NE_tank, FALSE);
		break;
	}
} // END P2 PURPLE CONFIG

static void P2_Green_Config (void)
{
switch (TankOri_P2_GREEN)
	{
		case 1:
		Print_GameObject(&gObj_P2_green_SE_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P2_green_S_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P2_green_SW_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P2_green_W_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P2_green_NW_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P2_green_N_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P2_green_NE_tank, FALSE);
		break;
	}
}// END P2 GREEN CONFIG

static void P2_Cyan_Config (void)
{
switch (TankOri_P2_CYAN)
	{
		case 1:
		Print_GameObject(&gObj_P2_cyan_SE_tank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P2_cyan_S_tank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_P2_cyan_SW_tank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P2_cyan_W_tank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_P2_cyan_NW_tank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P2_cyan_N_tank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_P2_cyan_NE_tank, FALSE);
		break;
	}
} // END P2 CYAN CONFIG

/*****************************************************************************
 Interrupt functions
******************************************************************************/

void GPIOF_Button_IRQHandler(uint32_t Status) {
	
	////////////// PLAYER 1 INTERRUPTS ////////////////////
	
    /* Check if it is SW2 (PF0) interrupt */
    if (0 != (Status & BIT(PF_SW2))) {
        GPIOF->ICR = BIT(PF_SW2); // Clear interrupt

        // Fire a bullet regardless of the previous bullet's state
        g_nBulletDirection1 = TankOriP1; // Set bullet direction to current tank orientation
        InitBulletP1(); // Initialize bullet position and set it active
    }

    /* Check if it is SW1 (PF4) interrupt */
    if (0 != (Status & BIT(PF_SW1))) {
        GPIOF->ICR = BIT(PF_SW1); // Clear interrupt
        P1_SW1_press = !P1_SW1_press;
        g_bTankMoving1 = P1_SW1_press; // Continue tank movement
    }
		
	////////////// PLAYER 2 INTERRUPTS /////////////////
		
		/* Check if it is ext SW2 (PF2) interrupt */
    if (0 != (Status & BIT(PF_SW2))) {
        GPIOF->ICR = BIT(PF_SW2); // Clear interrupt

        // Fire a bullet regardless of the previous bullet's state
        g_nBulletDirection2 = TankOriP1; // Set bullet direction to current tank orientation
        InitBulletP2(); // Initialize bullet position and set it active
    }

    /* Check if it is ext SW1 (PF1) interrupt */
    if (0 != (Status & BIT(PF_SW1))) {
        GPIOF->ICR = BIT(PF_SW1); // Clear interrupt
        P2_SW1_press = !P2_SW1_press;
        g_bTankMoving2 = P2_SW1_press; // Continue tank movement
    }
}

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


