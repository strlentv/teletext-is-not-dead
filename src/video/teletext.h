/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file teletext.h
*
* \brief Teletext implementation
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef TELETEXT_H
#define TELETEXT_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "consts.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#ifndef TELETEXT_LINES
  #define TELETEXT_LINES (0u)  //!< Default value
#endif

#define TELETEXT_PIXELS_X              (39u*2u)  //!< X resolution in teletext graphics mode
#define TELETEXT_PIXELS_Y   (TELETEXT_LINES*3u)  //!< Y resolution in teletext graphics mode


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Color indexes
//! \note  These values act as indices for the gcaColorTable[] array!
typedef enum
{
  TXT_BLACK = 0u,
  TXT_BLUE,
  TXT_RED,
  TXT_MAGENTA,
  TXT_GREEN,
  TXT_CYAN,
  TXT_YELLOW,
  TXT_WHITE
} E_TXT_COLOR;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
extern volatile U8 gau8TeletextLineBuffer[ TELETEXT_LINES + 1u ][ 46u ];
extern volatile U8 gu8TeletextLine;
extern const U8 gcau8AlphaColorTable[ 8u ];
extern const U8 gcau8GraphicsColorTable[ 8u ];


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
void Teletext_Init( void );
void Teletext_ScanLineCallback( void );
void Teletext_FieldEndCallback( void );

// API
void Teletext_SynchronizeField( void );
void Teletext_PutChar( U8 u8PosX, U8 u8PosY, U8 u8Char );
void Teletext_TextMode( void );
void Teletext_GraphicsMode( E_TXT_COLOR eColor );
void Teletext_Pixel( U8 u8PosX, U8 u8PosY, BOOL bIsOn );
void Teletext_DrawLine( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, BOOL bIsOn );


#endif  // TELETEXT_H

//-----------------------------------------------< EOF >--------------------------------------------------/
