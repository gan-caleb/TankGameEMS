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
#define BUZZER_MS					100U

/*****************************************************************************
 Type definition
******************************************************************************/
#define PLAYER_X_SIZE						30U
#define PLAYER_Y_SIZE						30U
#define DEBOUNCE_DELAY 					100000
#define FLOOR_LEVEL							160U
#define CEILING_LEVEL						10U

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
/**		Systick Variables		**/
static volatile BOOL			g_bSystemTick = FALSE;
static volatile BOOL			g_bSecTick = FALSE;
static volatile BOOL			g_bToggle = FALSE;
static unsigned int				g_nTimeSec = 0;
static int            		g_nCount = 0;

/**		SPI Interface & LCD 	**/
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

/**		Keypad	 **/
static volatile BOOL 			g_bKeypadScan = FALSE;
static volatile BOOL 			bKeyPressed = FALSE;
static unsigned char 			g_cKey = '-';
static unsigned char 			g_cDir = ' ';
static volatile int 			g_nKeypad = KEYPAD_UPDATE_MS;
static volatile int   		hist_row, hist_col;
static unsigned int 			g_nAngle = 0;

/**		Buttons		**/
static volatile BOOL			g_bSW1 = FALSE;  
static volatile BOOL			g_bSW2 = FALSE; 

/**		Timer 0A TESTPIN		**/
static BOOL								g_bTESTPIN = FALSE;

/**		Buzzer		**/
static volatile BOOL  		buzz_flag;

/**		UART		**/

/**		Potentiometer		**/

/** Sprite Animation **/
static volatile int					g_FireCounter = 150;
static volatile BOOL				g_FireFlip = FALSE;

static volatile int					g_ExplosionCounter = 75;
static volatile int					g_ExplosionType = 1;


/** Tank  Parameters **/

static volatile int 				g_TankLifeCounter = 3;
static volatile int					TankOri = 1;
static volatile int					TankCounter = 200;



/** Game Objects **/
		
/** Tanks **/	
/********************************************/
static GameObject gObj_p1_NorthBluetank = { //NORTH
		bmpP1tankSprite1,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };		

static GameObject gObj_P1NWtank = { //NORTH-WEST
		bmpP1tankSprite2,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };	

		static GameObject gObj_P1NWtank2 = { //NORTH-WEST
		bmpP1tankSprite2,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 100} };
		
static GameObject gObj_p1_WestBluetank = { //WEST
		bmpP1tankSprite3,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1SWtank = { // SOUTH-WEST
		bmpP1tankSprite4,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_p1_SouthBluetank = { //SOUTH
		bmpP1tankSprite5,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };	

static GameObject gObj_P1SEtank = { // SOUTH-EAST
		bmpP1tankSprite6,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_p1_EastBluetank = { //EAST
		bmpP1tankSprite7,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };

static GameObject gObj_P1NEtank = { // NORTH-EAST
		bmpP1tankSprite8,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{100, 50} };		
	
/********************************************/
	
		
/** Lit **/
/********************************************/
static GameObject gObj_Fire1 = {
		bmpFire1Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{40, 40} };	
static GameObject gObj_Fire2 = {
		bmpFire2Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{40, 40} };	

static GameObject gObj_Cactus = {
bmpCactusSprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{128, 128} };	
/********************************************/

	/** Boom Boom **/
/********************************************/
static GameObject gObj_Explosion1 = {
		bmpExplosion1Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion2 = {
		bmpExplosion2Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion3 = {
		bmpExplosion3Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion4 = {
		bmpExplosion4Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{20, 20} };	
static GameObject gObj_Explosion5 = {
		bmpExplosion5Sprite,
		{BALL_X_SIZE, BALL_Y_SIZE}, 
		{20, 20} };	
/********************************************/
		
//* testing **//
static volatile BOOL	SW1_press = FALSE;
static volatile BOOL	SW2_press = FALSE;
volatile uint32_t 		delay = 0;
static int				 		timer = 0;
volatile uint32_t			SW1_debounce = 0;
volatile uint32_t			SW2_debounce = 0;


/*****************************************************************************
 Local Functions
******************************************************************************/
static void main_LcdInit( void );
static void main_KeyScan( void );
static void main_KeypadOutput(void);
int CHAR_TO_INT (char); 
void GUI_AppDraw( BOOL bFrameStart );
extern void GPIOF_Button_IRQHandler ( uint32_t Status);
/*****************************************************************************
 Implementation
******************************************************************************/
int main()
{
	Port_Init();
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
	
	main_LcdInit();
	
	IRQ_Init();
	
	/*	Infinite FOR Loop		*/
	for(;;)
  {
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
		
		if (SW1_press == TRUE)
		{
			delay = 0;
			SW1_debounce++;
			if ((SW1_debounce > DEBOUNCE_DELAY) && SW1_press == TRUE)
			{
				g_TankLifeCounter--;
				SW1_press = FALSE;
				SW1_debounce = 0;
			
				if (g_TankLifeCounter == 0)
				{
					g_TankLifeCounter = 3;
				}
		}
	}
		if( FALSE != g_bKeypadScan )
		{
			g_bKeypadScan = FALSE;
			main_KeyScan();
		}
		// TODO: Update tank position and game logic
	}
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
	//LED_RGB_SET( RGB_RED*(g_nCount & 1U) );		/* Toggle LED every 1 sec based on time LSB */
	
/**		System Flags		**/
	
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
	
	if(buzz_flag){
		int buzz_count=BUZZER_MS;
		while(buzz_count){
			BUZZER_ON();
			buzz_count--;
		}
		BUZZER_OFF();
		buzz_flag=FALSE;
	}

if (g_TankLifeCounter-- ) // when player loses a life
{
if(g_TankLifeCounter != 0) //if player still has lives
{
		 g_ExplosionCounter--;
	if (g_ExplosionCounter == 0 )
	{
		g_ExplosionType++;
		g_ExplosionCounter = 200;
		if (g_ExplosionType > 5 )
		{
			g_ExplosionType = 1;
		}
	}
}
else if (g_TankLifeCounter == 0) // When player dead
{
	if( g_FireCounter >= 75)
	{
		g_FireFlip = FALSE;
	}
	 if (g_FireCounter < 75)
	{
		g_FireFlip = TRUE;
	}
	
	g_FireCounter--;
		 if (g_FireCounter == 0)
		 {
	g_FireCounter = 150;
		g_FireFlip = FALSE;
		 }
}
}
	 TankCounter--;
	if (TankCounter == 0 )
	{
		TankOri++;
		TankCounter = 200;
		if (TankOri > 8 )
		{
			TankOri = 1;
	}

}
}//end of Systick Handler 

void GUI_AppDraw( BOOL bFrameStart )
{
	
	/** Initialization Screen **/
	char buf[128];
	GUI_Clear( ClrBlack ); /* Set background colour */
	
	//GUI_SetColor(ClrYellow);	// Screen Border
	//GUI_DrawRect(0,0,159,127);

Print_GameObject(&gObj_P1NWtank2, FALSE);
	
	/** Fireflip **/
	if (g_FireFlip == FALSE)
	{
	Print_GameObject(&gObj_Fire1, FALSE);
	}
	else
	{
	Print_GameObject(&gObj_Fire2, FALSE);
	}
	
	
	/** Cactus **/
/********************************************/
	Print_GameObject(&gObj_Cactus, FALSE);
/********************************************/


	
	/** Explosion **/
/********************************************/
	switch (g_ExplosionType)
	{
		case 1:
		Print_GameObject(&gObj_Explosion1, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_Explosion2, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_Explosion3, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_Explosion4, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_Explosion5, FALSE);
		break;
	}
/********************************************/

/** Tank **/
/********************************************/

switch (TankOri)
	{
		case 1:
		Print_GameObject(&gObj_p1_NorthBluetank, FALSE);
		break;
		case 2:
		Print_GameObject(&gObj_P1NWtank, FALSE);
		break;
		case 3:
		Print_GameObject(&gObj_p1_WestBluetank, FALSE);
		break;
		case 4:
		Print_GameObject(&gObj_P1SWtank, FALSE);
		break;
		case 5:
		Print_GameObject(&gObj_p1_SouthBluetank, FALSE);
		break;
		case 6:
		Print_GameObject(&gObj_P1SEtank, FALSE);
		break;
		case 7:
		Print_GameObject(&gObj_p1_EastBluetank, FALSE);
		break;
		case 8:
		Print_GameObject(&gObj_P1NEtank, FALSE);
		break;
	}
}
/********************************************/


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
/*----------------------------------------------------------------------------*/
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
							return;
						}
				}		
			/*----------------------------------------------------------------------------*/
		} //end of for loop
	}

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
	void GPIOF_Button_IRQHandler( uint32_t Status )
{
/* check if it is SW2 (PF0) intr */
	
	if( 0 != (Status & BIT(PF_SW2 ) )){
	GPIOF->ICR = BIT(PF_SW2); /* clear intr */
	SW2_press = TRUE;
	}
/* check if it is SW1 (PF4) interrupt */
	if( 0 != (Status & BIT(PF_SW1) )){
	GPIOF->ICR = BIT(PF_SW1); /* clear intr */
	SW1_press = TRUE;
	
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