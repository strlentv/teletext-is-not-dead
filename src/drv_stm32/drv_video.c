/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file drv_video.c
*
* \brief Low-level video driver for STM32
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <string.h>
#include "types.h"
#include "consts.h"
#include "video.h"
#include "teletext.h"

// Drivers
#include "tim.h"
#include "dma.h"
#include "spi.h"
#include "main.h"

// Own include
#include "drv_video.h"

//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
// Hardware used for SYNC generation
#define SYNC_TIMER    TIM3                            //!< Timer used for time base
#define SET_SYNCTIMER_PWM(x)  (SYNC_TIMER->CCR4 = x)  //!< This macro used for modifying PWM value on Channel 4

// Hardware register values -- 27.75 MHz clock input
#define NORMAL_LINE     (1776u)  //!< 64 usec long scanline
#define SHORT_LINE      (888u)   //!< 32 usec long half scanline (used at VSYNC)
#define NORMAL_SYNC     (131u)   //!< 4.7 usec long (normal) sync pulse at the beginning of the line
#define SHORT_SYNC      (65u)    //!< 2.35 usec long (short) sync pulse at the beginning of the line
#define LONG_SYNC       (757u)   //!< 27.3 usec long sync pulse at the beginning of the line
//#define TXT_START       (280u)   //!< ~10 usec after the falling edge of sync, teletext data will be displayed
//#define TXT_STOP        (1777u)  //!< No teletext data will be displayed

/*
// Hardware register values -- 48 MHz clock input
#define NORMAL_LINE     (3072u)  //!< 64 usec long line
#define SHORT_LINE      (1536u)  //!< 32 usec long line
#define NORMAL_SYNC     (192u)   //!< 4 usec long sync pulse at the beginning of the line
#define SHORT_SYNC      (96u)    //!< 2 usec long sync pulse at the beginning of the line
#define LONG_SYNC       (1440u)  //!< 30 usec long sync pulse at the beginning of the line
#define TXT_START       (480u)   //!< 10 usec after the falling edge of sync, teletext data will be displayed
#define TXT_STOP        (U16MAX) //!< No teletext data will be displayed
*/

//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
#ifdef DEBUG
__root volatile DMA_Channel_TypeDef *pDMACH3 = (DMA_Channel_TypeDef*)DMA1_Channel3_BASE;
__root volatile TIM_TypeDef *pTIM3 = (TIM_TypeDef *)TIM3;
#endif

#ifdef PICTURE_OUTPUT
//! \brief Line buffer used by DMA to push pixels out
//! \note  Uses double-buffering (hence the 2); the +1 byte at the end is the horizontal blanking!
_Pragma("data_alignment=16") volatile static U8 gau8LineBuffer[ 2u ][ HORIZONTAL_RESOLUTION+1u ];
#endif

//! \brief Row counter for displaying the correct number of lines
volatile static U16 gu16RowCounter;

//! \brief Helper variable for re-enabling DMA
_Pragma("data_alignment=4") volatile U32 gcu32DmaEnable;


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/

 /*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
 /*! *******************************************************************
 * \brief  Init function
 * \param  -
 * \return -
 *********************************************************************/
void DrvVideo_Init( void )
{
  // Global variables init
#ifdef PICTURE_OUTPUT
  (void)memset( (U8*)gau8LineBuffer, 0, sizeof( gau8LineBuffer ) );  // note: black is represented by 0
#endif
  gu16RowCounter = 0u;

/*
// Local pointers for debugging peripherials -- these will be optimized out by the compiler
volatile SPI_TypeDef *pSPI = SPI1;
volatile DMA_TypeDef *pDMA = DMA1;
volatile DMA_Channel_TypeDef *pDMACH2 = (DMA_Channel_TypeDef*)DMA1_Channel2_BASE;
volatile DMA_Channel_TypeDef *pDMACH3 = (DMA_Channel_TypeDef*)DMA1_Channel3_BASE;
volatile TIM_TypeDef *pTim3 = TIM3;
volatile TIM_TypeDef *pTim1 = TIM1;
*/
  
  // Hardware components init
  // SPI1 -- used for Teletext output
  LL_SPI_EnableDMAReq_TX( SPI1 );
  LL_SPI_SetDMAParity_TX( SPI1, LL_SPI_DMA_PARITY_EVEN );
  LL_SPI_Enable( SPI1 );
  
  // black magic starts here: DMA1 Channel 2 is triggered by TIM3 Channel 3 after each horizontal sync.
  // DMA1 Channel 2 starts DMA1 Channel 3 to initiate a transfer over SPI1 (teletext data).
  // On completion, an interrupt is generated and DMA1 Channel 3 gets re-initialized.
  //
  // DMA1 Channel 2 init -- transfers are triggered by TIM3 Ch3
  LL_DMA_InitTypeDef sDMAInit;
  sDMAInit.PeriphOrM2MSrcAddress  = (U32)&(((DMA_Channel_TypeDef*)DMA1_Channel3_BASE)->CCR);
  sDMAInit.MemoryOrM2MDstAddress  = (U32)&gcu32DmaEnable;
  sDMAInit.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  sDMAInit.Mode                   = LL_DMA_MODE_CIRCULAR;
  sDMAInit.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  sDMAInit.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_NOINCREMENT;
  sDMAInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  sDMAInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  sDMAInit.NbData                 = 1u;
  sDMAInit.Priority               = LL_DMA_PRIORITY_VERYHIGH;
  LL_DMA_Init( DMA1, LL_DMA_CHANNEL_2, &sDMAInit );
  
  // DMA1 Channel 3 init -- transfers teletext data via SPI
  sDMAInit.PeriphOrM2MSrcAddress  = (U32)&(SPI1->DR);
  sDMAInit.MemoryOrM2MDstAddress  = (U32)gau8TeletextLineBuffer[ 0u ];
  sDMAInit.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  sDMAInit.Mode                   = LL_DMA_MODE_NORMAL;
  sDMAInit.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  sDMAInit.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  sDMAInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
  sDMAInit.MemoryOrM2MDstDataSize = LL_DMA_PDATAALIGN_BYTE;
  sDMAInit.NbData                 = sizeof( gau8TeletextLineBuffer[ 0u ] );
  sDMAInit.Priority               = LL_DMA_PRIORITY_VERYHIGH;
  LL_DMA_Init( DMA1, LL_DMA_CHANNEL_3, &sDMAInit );
  LL_DMA_EnableIT_TC( DMA1, LL_DMA_CHANNEL_3 );  // DMA1 Ch3 transfer completed interrupt enable
  // DMA1 Ch3 initialized, saving helper variable
  gcu32DmaEnable = ((DMA_Channel_TypeDef*)DMA1_Channel3_BASE)->CCR | DMA_CCR_EN;
  
  LL_DMA_EnableChannel( DMA1, LL_DMA_CHANNEL_2 );  // only channel 2 is enabled -- channel 3 is enabled by channel 2
  
  // Timer 1 -- pixel DMA trigger
//  HAL_TIM_Base_Start( &htim1 );
//  //HAL_TIM_PWM_Start( &htim1, TIM_CHANNEL_1 );
//  HAL_TIMEx_PWMN_Start( &htim1, TIM_CHANNEL_1 ); //starts PWM on CH1N pin  
  
  // Timer 3 -- sync generation and related
  // note: many parameters are configured via STM32Cube-generated code in tim.c
  LL_TIM_EnableIT_UPDATE( TIM3 );  // interrupt enable on TIM3 update
  TIM3->CCMR2 |= TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;  // passive level on reload, active level after match
  //LL_TIM_CC_EnableChannel( TIM3, LL_TIM_CHANNEL_CH1 );  // this channel triggers TIM1 to display video
  LL_TIM_CC_EnableChannel( TIM3, LL_TIM_CHANNEL_CH3 );  // this channel triggers teletext data transfer
  LL_TIM_OC_EnablePreload( TIM3, LL_TIM_CHANNEL_CH3 );
  // note: DMA request on timer channel 3 event is not enabled here -- it's controlled by the IT routine
  LL_TIM_CC_EnableChannel( TIM3, LL_TIM_CHANNEL_CH4 );  // this channel is used for sync generation
  LL_TIM_EnableCounter( TIM3 );
  
}

/*! *******************************************************************
 * \brief  Horizontal sync interrupt callback
 * \param  -
 * \return -
 *********************************************************************
 * The video signal is organized as follows:
 *   0 <= gu16RowCounter < 305 : first picture field
 * 305 <= gu16RowCounter < 319 : first vsync field
 * 319 <= gu16RowCounter < 624 : second picture field
 * 624 <= gu16RowCounter < 640 : second vsync field
 *********************************************************************/
void DrvVideo_HorizontalSyncIT( void )
{
  // Incrementing row counter
  gu16RowCounter++;  // note: this variable is used for vsync too
  if( 640u == gu16RowCounter )
  {
    gu16RowCounter = 0u;
  }
  
  // VSync
  if( ( 305u <= gu16RowCounter )
   && ( 319u > gu16RowCounter ) )  // first vsync field
  {
    // the first VSYNC field is organized as follows:
    // 305: short sync
    // 306: short sync
    // 307: short sync
    // 308: short sync
    // 309: short sync
    // 310: long sync
    // 311: long sync
    // 312: long sync
    // 313: long sync
    // 314: long sync
    // 315: short sync
    // 316: short sync
    // 317: short sync
    // 318: short sync
    switch( gu16RowCounter )
    {
      case 305u:  // short sync starts here -- 5 pre-equalizing pulses
        // Modify timebase
        SYNC_TIMER->ARR = SHORT_LINE - 1u;
        // Modify PWM generator
        SET_SYNCTIMER_PWM( SHORT_SYNC - 1u );
        break;

      case 310u:  // long sync starts here -- 5 long sync pulses
        // Modify PWM generator
        SET_SYNCTIMER_PWM( LONG_SYNC - 1u);
        break;

      case 315u:  // short sync starts here -- 4 post-equalizing pulses
        // Modify PWM generator
        SET_SYNCTIMER_PWM( SHORT_SYNC - 1u );
        break;

      default:
        // nothing to do
        break;
    }
  }
  else if( ( 624u <= gu16RowCounter )
        && ( 640u > gu16RowCounter ) )  // second vsync field
  {
    // the second VSYNC field is organized as follows:
    // 624: normal sync, short scanline
    // 625: short sync
    // 626: short sync
    // 627: short sync
    // 628: short sync
    // 629: short sync
    // 630: long sync
    // 631: long sync
    // 632: long sync
    // 633: long sync
    // 634: long sync
    // 635: short sync
    // 636: short sync
    // 637: short sync
    // 638: short sync
    // 639: short sync
    switch( gu16RowCounter )
    {
      case 624u:  // short scanlines start here
        // Modify timebase
        SYNC_TIMER->ARR = SHORT_LINE - 1u;
        break;

      case 625u:  // short sync starts here -- 5 pre-equalizing pulses
        // Modify PWM generator
        SET_SYNCTIMER_PWM( SHORT_SYNC - 1u );
        break;

      case 630u:  // long sync starts here -- 5 long sync pulses
        // Modify PWM generator
        SET_SYNCTIMER_PWM( LONG_SYNC - 1u);
        break;

      case 635u:  // short sync starts here -- 5 post-equalizing pulses
        // Modify PWM generator
        SET_SYNCTIMER_PWM( SHORT_SYNC - 1u );
        break;

      default:
        // nothing to do
        break;
    }
  }
  else  // picture field
  {
    // line 0 and 319: reinit timebase and enable normal HSYNC
    if( ( 0u == gu16RowCounter ) || ( 319u == gu16RowCounter ) )
    {
      // Modify timebase
      SYNC_TIMER->ARR = NORMAL_LINE - 1u;
      // enable normal HSYNC
      SET_SYNCTIMER_PWM( NORMAL_SYNC - 1u );
    }
    
#ifdef TELETEXT_OUTPUT
    // Note: some decoders does not support lines 0, 319 and 320, this reduction is needed for compatibility
    if( ( ( 1u <= gu16RowCounter ) && ( 16u >= gu16RowCounter ) )       // teletext-enabled lines in the first picture field
     || ( ( 321u <= gu16RowCounter ) && ( 336u >= gu16RowCounter ) ) )  // teletext-enabled lines in the second picture field
    {
      // build the next teletext packet
      Teletext_ScanLineCallback();
      // enable teletext transfers in the next scanline
      LL_TIM_EnableDMAReq_CC3( TIM3 );  // DMA request on timer channel 3 event
    }
    else  // no teletext-enabled scanline
    {
      if( 337u == gu16RowCounter )
      {
        Teletext_FieldEndCallback();
      }
      // disable teletext transfers in the next scanline
      LL_TIM_DisableDMAReq_CC3( TIM3 );  // DMA request on timer channel 3 event
    }
#endif
    
#ifdef PICTURE_OUTPUT
    static BOOL bFirstLine = TRUE;
    static U8   u8Line = 0u;
    U8 u8Index;
    
    //TODO: enable DMA

    // set DMA pointer
    //TODO: set to gau8LineBuffer[ u8Line ][ 0u ]


#ifdef ROW_DOUBLING
    if( FALSE == bFirstLine )
    {
      u8Line = (u8Line+1u) & 0x01u;
      bFirstLine = TRUE;
    }
    else
    {
      bFirstLine = FALSE;
    }
#else  // if we're not displaying each rows twice
    u8Line = (u8Line+1u) & 0x01u;
#endif  // ROW_DOUBLING

    // if the line needs to be updated
    if( ( TRUE == bFirstLine ) && ( gu16RowCounter < VISIBLE_ROWS ) )
    {
      // build line buffer
      for( u8Index = 0u; u8Index < HORIZONTAL_RESOLUTION; u8Index++ )
      {
        // convert pixel format to 8-bit
        gau8LineBuffer[ u8Line ][ u8Index ] = (U8)gau8VideoScreen[ gu16RowCounter ][ u8Index ].u8Color;  // note: a color lookup can be used here
      }
    }
#endif  // PICTURE_OUTPUT
  }  // picture field
}

/*! *******************************************************************
 * \brief  Teletext send completed by DMA
 * \param  -
 * \return -
 *********************************************************************/
void DrvVideo_TeletextSendCompletedIT( void )
{
  // Next line
  gu8TeletextLine++;
  if( TELETEXT_LINES < gu8TeletextLine )
  {
    gu8TeletextLine = 0u;
  }
  // reinitialize DMA1 Channel 3
  LL_DMA_DisableChannel( DMA1, LL_DMA_CHANNEL_3 );
  ((DMA_Channel_TypeDef*)DMA1_Channel3_BASE)->CMAR = (U32)gau8TeletextLineBuffer[ gu8TeletextLine ];
  ((DMA_Channel_TypeDef*)DMA1_Channel3_BASE)->CNDTR = sizeof( gau8TeletextLineBuffer[ 0u ] );
}

 /*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//-----------------------------------------------< EOF >--------------------------------------------------/
