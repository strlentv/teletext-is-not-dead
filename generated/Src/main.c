
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <math.h>
#include "types.h"
#include "drv_video.h"
#include "teletext.h"
#include "unlz4.h"
#include "renderer.h"
//#include "drv_sound.h"
//#include "synth_main.h"
//#include "instrument.h"
//#include "synth_gen.h"
//#include "tracker.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void LL_Init(void);
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void SlideShow( U8* pu8SlidesPacked, const U16* pu16TimingTableMs );
static void ObjectShow( void );

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
U32 gu32TimeNow;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
//  volatile DMA_TypeDef* pDMA1 = DMA1;
//  volatile DMA_Channel_TypeDef* pDMA1Ch4 = DMA1_Channel4;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  //MX_TIM1_Init();
  MX_SPI1_Init();
  //MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

  // Init time
  gu32TimeNow = 0u;
  LL_SYSTICK_EnableIT();
  
  // Init sound driver
  //DrvSound_Init();
  
  // Init synth
  //Synth_Main_Init();
  
  // Init internal software variables
  Teletext_Init();
  
  // Initializing hardware and starting operation
  DrvVideo_Init();

  // Init tracker
  //Tracker_Init( 23000u );  // music will start 23 seconds after this
  
  LL_mDelay( 4000u );
  
  /*
  Teletext_GraphicsMode( TXT_WHITE );
  // Box
  Teletext_DrawLine( 0, 0, TELETEXT_PIXELS_X-1, 0, TRUE );
  Teletext_DrawLine( 0, 0, 0, TELETEXT_PIXELS_Y-1, TRUE );
  Teletext_DrawLine( 0, TELETEXT_PIXELS_Y-1, TELETEXT_PIXELS_X-1, TELETEXT_PIXELS_Y-1, TRUE );
  Teletext_DrawLine( TELETEXT_PIXELS_X-1, 0, TELETEXT_PIXELS_X-1, TELETEXT_PIXELS_Y-1, TRUE );
  // Cross
  Teletext_DrawLine( 0, 0, TELETEXT_PIXELS_X-1, TELETEXT_PIXELS_Y-1, TRUE );
  Teletext_DrawLine( 0, TELETEXT_PIXELS_Y-1, TELETEXT_PIXELS_X-1, 0, TRUE );

  LL_mDelay( 1000u );
*/
  
  // Show the first block of teletext "slides"
  extern U8 gau8SlidesPacked[];
  static const U16 cau16TimingTable1Ms[] = { 6000u, 7000u, 8000u, 6500u, 6500u, 5000u, 10000u, 8000u, 7000u, 6000u, 13000u, 5000u, 5000u };  //NOTE: this must be maintained!
  SlideShow( gau8SlidesPacked, cau16TimingTable1Ms );
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

    // Show the 3D objects
    ObjectShow();

    // Show the second block of teletext "slides"
    extern U8 gau8Slides2Packed[];
    static const U16 cau16TimingTable2Ms[] = { 4500u, 4000u, 20000u };  //NOTE: this must be maintained!
    SlideShow( gau8Slides2Packed, cau16TimingTable2Ms );
    
    // Non-timecritical synth section
    //Synth_Main_Cycle();
    
    // Process music sheet
    //Tracker_Play( gu32TimeNow );
    //if( gu32TimeNow > 55000u )
    {
      NVIC_SystemReset();
    }
  }
  /* USER CODE END 3 */
}

static void LL_Init(void)
{
  

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);

  /* System interrupt init*/
  /* SVC_IRQn interrupt configuration */
  NVIC_SetPriority(SVC_IRQn, 0);
  /* PendSV_IRQn interrupt configuration */
  NVIC_SetPriority(PendSV_IRQn, 0);
  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  Error_Handler();  
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
    
  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_2);

  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
    
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  
  }
  LL_Init1msTick(27750000);

  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

  LL_SetSystemCoreClock(27750000);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);
}

/* USER CODE BEGIN 4 */
//! \brief Displays the teletext slides on screen
//! \param pu8SlidesPacked: pointer to the packed slides
//! \param pu16TimingTableMs: pointer to the timing array
//! \note  High stack usage!
static void SlideShow( U8* pu8SlidesPacked, const U16* pu16TimingTableMs )
{
  U8  au8Unpacked[ PACKER_BUFFER_SIZE ];
  U16 u16PackLen, u16PackOffset;
  U16 u16PackedFileSize;
  U8  u8Slide = 0u;
  
  U8  u8PosX, u8PosY, u8Block;
  U16 u16IndexInBlock;
  BOOL bEndOfPicture;
  u16PackedFileSize = (U16)pu8SlidesPacked[ 0u ] + ( (U16)pu8SlidesPacked[ 1u ]<<8u );
  u16PackOffset = 2u;  // first two bytes contain the size of the compressed file
  while( u16PackOffset < u16PackedFileSize )
  {
    u8PosX = 0u;
    u8PosY = 0u;
    bEndOfPicture = FALSE;
    for( u8Block = 0u; ( FALSE == bEndOfPicture ) && ( u16PackOffset < u16PackedFileSize ); u8Block++ )
    {
      u16PackLen = (U16)pu8SlidesPacked[ 0u + u16PackOffset ] + ( (U16)pu8SlidesPacked[ 1u + u16PackOffset ]<<8u );
      unlz4_len( &pu8SlidesPacked[ 2u + u16PackOffset ], au8Unpacked, u16PackLen );
      u16PackOffset += u16PackLen + 2u;
      for( u16IndexInBlock = 40u; u16IndexInBlock < PACKER_BUFFER_SIZE; u16IndexInBlock++ )
      {
        Teletext_PutChar( u8PosX, u8PosY, au8Unpacked[ u16IndexInBlock ] );
        u8PosX++;
        if( 40u == u8PosX )
        {
          u8PosX = 0u;
          u8PosY++;
          if( TELETEXT_LINES == u8PosY )
          {
            bEndOfPicture = TRUE;
            break;
          }
        }
      }
    }
    LL_mDelay( pu16TimingTableMs[ u8Slide ] );
/*    if( u8Slide == 9u )  // freeze
    {
      while(1);
    }*/
    u8Slide++;
  }
}

//! \brief Displays the 3D objects on screen
//! \note  High stack usage!
static void ObjectShow( void )
{
  extern U8 gau8ObjectsPacked[];
  U8 au8Object[ PACKER_BUFFER_SIZE ];
  U16 u16PackLen;
  U16 u16PackedFileSize;
  U16 u16PackOffset = 2u;  // first two bytes are the size of the package
  //
  S_VECTOR* asVertexes;
  U16       u16NumberOfVertexes;
  U8        (*au8Edges)[2u];
  U16       u16NumberOfEdges;
  //
  U32 u32StartTime;
  U8 u8Index;
  DEG degRotate = 0u;
  U32 u32ColorChangeMs = 0u;
  U8  u8ColorStripeOffset = 0u;
  
  // Unpack and render
  u16PackedFileSize = (U16)gau8ObjectsPacked[ 0u ] + ( (U16)gau8ObjectsPacked[ 1u ]<<8u );
    
  // iterate through 3D objects
  while( u16PackedFileSize > u16PackOffset )
  {
    u32StartTime = gu32TimeNow;
    u32ColorChangeMs = gu32TimeNow;
    while( gu32TimeNow < ( u32StartTime + 6000u ) )  // each objects will be displayed for 6 secs
    {
      // Decompress 3D object
      u16PackLen = (U16)gau8ObjectsPacked[ 0u + u16PackOffset ] + ( (U16)gau8ObjectsPacked[ 1u + u16PackOffset ]<<8u );
      unlz4_len( &gau8ObjectsPacked[ 2u + u16PackOffset ], au8Object, u16PackLen );

      // Set pointers to the object
      asVertexes = (S_VECTOR*)&( au8Object[ 2u ] );
      u16NumberOfVertexes = (U16)au8Object[ 0u ] + ( (U16)au8Object[ 1u ]<<8u );
      au8Edges = (U8(*)[2u])&( au8Object[ 2u + u16NumberOfVertexes*sizeof( S_VECTOR ) + 2u ] );
      u16NumberOfEdges = (U16)au8Object[ 2u + u16NumberOfVertexes*sizeof( S_VECTOR ) ] + ( (U16)au8Object[ 2u + u16NumberOfVertexes*sizeof( S_VECTOR ) + 1u ]<<8u );
      
      // Rotate vertex vectors
      degRotate += 2u;
      for( u8Index = 0u; u8Index < u16NumberOfVertexes; u8Index++ )
      {
        asVertexes[ u8Index ] = Renderer_RotateX( asVertexes[ u8Index ], degRotate );
        asVertexes[ u8Index ] = Renderer_RotateY( asVertexes[ u8Index ], degRotate );
        asVertexes[ u8Index ] = Renderer_RotateZ( asVertexes[ u8Index ], degRotate );
        //asVertexes[ u8Index ] = Renderer_Project( asVertexes[ u8Index ], 180, 35u+( ( gu32TimeNow - u32StartTime )>>6u ) );
        asVertexes[ u8Index ] = Renderer_Project( asVertexes[ u8Index ], 180, 75u+( ( gu32TimeNow - u32StartTime )>>7u ) );
        //asVertexes[ u8Index ] = Renderer_ProjectFast( asVertexes[ u8Index ] );
      }

      // Wait for TV screen to end
      Teletext_SynchronizeField();
      // clear screen
      Teletext_GraphicsMode( TXT_WHITE );
      // Box
      Teletext_DrawLine( 0, 0, TELETEXT_PIXELS_X-1, 0, TRUE );
      Teletext_DrawLine( 0, 0, 0, TELETEXT_PIXELS_Y-1, TRUE );
      Teletext_DrawLine( 0, TELETEXT_PIXELS_Y-1, TELETEXT_PIXELS_X-1, TELETEXT_PIXELS_Y-1, TRUE );
      Teletext_DrawLine( TELETEXT_PIXELS_X-1, 0, TELETEXT_PIXELS_X-1, TELETEXT_PIXELS_Y-1, TRUE );

      // Color stripes
      for( u8Index = 1u; u8Index < TELETEXT_LINES; u8Index += 2u )
      {
        Teletext_PutChar( 0u, u8Index, gcau8GraphicsColorTable[ ( ( u8Index>>1u) + u8ColorStripeOffset ) % 0x05u + 2u ] );
      }
      
      // Render the screen
      Renderer_RenderObject( asVertexes, au8Edges, 40u, 35u, u16NumberOfEdges );

      // In each second, increment color stripe offset
      if( ( gu32TimeNow - u32ColorChangeMs ) > 1000u )
      {
        u8ColorStripeOffset++;
        u32ColorChangeMs = gu32TimeNow;
      }
    }
    // Next object
    u16PackOffset += 2u + u16PackLen;
    degRotate = 0u;
  }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
