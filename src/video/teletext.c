/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file teletext.c
*
* \brief Teletext implementation
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <stdlib.h>
#include <string.h>  // memcpy, etc.
#include "types.h"

// Own include
#include "teletext.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
// Parameters
#define MAGAZINE    (1u)  //!< Default magazine number

// Macros
//! \brief Macro for computing Hamming 8/4 coded data
#define Hamming84(x) gcau8Hamming84Table[(x)&0xFu]

//! \brief Macro for adding parity
//! \note  Use only for constants!
#define p(x) ((x|((((x^(x>>4u))^((x^(x>>4u))>>2u))^(((x^(x>>4u))^((x^(x>>4u))>>2u))>>1u)^1u)<<7u))&0xFFu)


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Teletext line buffer -- sent out in each scanline
_Pragma("data_alignment=4") volatile U8 gau8TeletextLineBuffer[ TELETEXT_LINES + 1u ][ 46u ];  // note: teletext packets are 45 bytes long (including clock run-in), the last byte is needed for definite level on SPI

//! \brief Helper variable for double-buffering teletext lines. Shows what buffer can be modified.//FIXME: <= the description of this variable needs to be changed!
volatile U8 gu8TeletextLine;

//! \brief Variable for synchronizing for field ends
volatile static BOOL gbWaitForFieldEnd;

//! \brief Helper table for computing Hamming 8/4 code
static const U8 gcau8Hamming84Table[ 16u ] = { 0x15u, 0x02u, 0x49u, 0x5Eu, 0x64u, 0x73u, 0x38u, 0x2Fu, 0xD0u, 0xC7u, 0x8Cu, 0x9Bu, 0xA1u, 0xB6u, 0xFDu, 0xEAu };

//! \brief Lookup table for color-modifying alpha space characters
//! \note  In order: black, blue, red, magenta, green, cyan, yellow, white (corresponds to ZX Spectrum color table)
const U8 gcau8AlphaColorTable[ 8u ] = { 0x80u, 0x04u, 0x01u, 0x85u, 0x02u, 0x86u, 0x83u, 0x07u };

//! \brief Lookup table for color-modifying graphics space characters
//! \note  In order: black, blue, red, magenta, green, cyan, yellow, white (corresponds to ZX Spectrum color table)
const U8 gcau8GraphicsColorTable[ 8u ] = { 0x10u, 0x94u, 0x91u, 0x15u, 0x92u, 0x16u, 0x13u, 0x97u };

//! \brief Translation lookup table for graphics characters
static const U8 gcau8GraphicsLUT[ 3u ][ 2u ] = { { 33u, 34u }, { 36u, 40u }, { 48u, 96u } };


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/
static U8 AddParity( U8 u8Val );
static void TestPattern( void );
static void PlotLineLow( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, BOOL bIsOn );
static void PlotLineHigh( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, BOOL bIsOn );


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Calculates and appends odd parity to a given 8-bit value
 * \param  u8Val: given 8-bit value
 * \return The given value with parity appended on MSB.
 *********************************************************************/
static U8 AddParity( U8 u8Val )
{
  U8 u8Temp;
  u8Temp = u8Val ^ ( u8Val >> 4u );
  u8Temp ^= u8Temp >> 2u;
  u8Temp ^= u8Temp >> 1u;
  u8Temp ^= 1u;
  return( u8Val | ( u8Temp << 7u ) );
}

/*! *******************************************************************
 * \brief  Puts a test pattern on the teletext screen
 * \param  -
 * \return -
 *********************************************************************/
static void TestPattern( void )
{
  //Note: ¡¡ this is just a 'hello world' program on Teletext Page 100
  //NOTE: bytes are transmitted LSB first!
  U8* au8LineBuffer;
  for( U8 u8Line = 0u; u8Line <= TELETEXT_LINES; u8Line++ )
  {
    au8LineBuffer = (U8*)gau8TeletextLineBuffer[ u8Line ];
    if( u8Line == 0u )  // page header
    {
      // Text to be displayed in page header
      au8LineBuffer[ 13u ] = AddParity( 'S' );
      au8LineBuffer[ 14u ] = AddParity( 't' );
      au8LineBuffer[ 15u ] = AddParity( 'r' );
      au8LineBuffer[ 16u ] = AddParity( 'l' );
      au8LineBuffer[ 17u ] = AddParity( 'e' );
      au8LineBuffer[ 18u ] = AddParity( 'n' );
      au8LineBuffer[ 19u ] = AddParity( ' ' );
      au8LineBuffer[ 20u ] = AddParity( 'p' );
      au8LineBuffer[ 21u ] = AddParity( 'r' );
      au8LineBuffer[ 22u ] = AddParity( 'e' );
      au8LineBuffer[ 23u ] = AddParity( 's' );
      au8LineBuffer[ 24u ] = AddParity( 'e' );
      au8LineBuffer[ 25u ] = AddParity( 'n' );
      au8LineBuffer[ 26u ] = AddParity( 't' );
      au8LineBuffer[ 27u ] = AddParity( 's' );
      au8LineBuffer[ 28u ] = AddParity( ':' );
      au8LineBuffer[ 29u ] = gcau8AlphaColorTable[ TXT_MAGENTA ];
      au8LineBuffer[ 30u ] = AddParity( 'A' );
      au8LineBuffer[ 31u ] = AddParity( ' ' );
      au8LineBuffer[ 32u ] = AddParity( 't' );
      au8LineBuffer[ 33u ] = AddParity( 'e' );
      au8LineBuffer[ 34u ] = AddParity( 'l' );
      au8LineBuffer[ 35u ] = AddParity( 'e' );
      au8LineBuffer[ 36u ] = AddParity( 't' );
      au8LineBuffer[ 37u ] = AddParity( 'e' );
      au8LineBuffer[ 38u ] = AddParity( 'x' );
      au8LineBuffer[ 39u ] = AddParity( 't' );
      au8LineBuffer[ 40u ] = AddParity( ' ' );
      au8LineBuffer[ 41u ] = AddParity( 'd' );
      au8LineBuffer[ 42u ] = AddParity( 'e' );
      au8LineBuffer[ 43u ] = AddParity( 'm' );
      au8LineBuffer[ 44u ] = AddParity( 'o' );
    }
    else if( u8Line == 1u )
    {
      // this is a 0 magazine, 1 row packet --> contains only text information
      au8LineBuffer[  5u ] = AddParity( 'H' );
      au8LineBuffer[  6u ] = AddParity( 'e' );
      au8LineBuffer[  7u ] = AddParity( 'l' );
      au8LineBuffer[  8u ] = AddParity( 'l' );
      au8LineBuffer[  9u ] = AddParity( 'o' );
      au8LineBuffer[ 10u ] = AddParity( ' ' );
      au8LineBuffer[ 11u ] = AddParity( 'W' );
      au8LineBuffer[ 12u ] = AddParity( 'o' );
      au8LineBuffer[ 13u ] = AddParity( 'r' );
      au8LineBuffer[ 14u ] = AddParity( 'l' );
      au8LineBuffer[ 15u ] = AddParity( 'd' );
      au8LineBuffer[ 16u ] = AddParity( '!' );
      au8LineBuffer[ 17u ] = AddParity( ' ' );
      au8LineBuffer[ 18u ] = gcau8AlphaColorTable[ TXT_RED ];
      au8LineBuffer[ 19u ] = AddParity( 'R' );
      au8LineBuffer[ 20u ] = AddParity( 'e' );
      au8LineBuffer[ 21u ] = AddParity( 'd' );
      au8LineBuffer[ 22u ] = gcau8AlphaColorTable[ TXT_GREEN ];
      au8LineBuffer[ 23u ] = AddParity( 'G' );
      au8LineBuffer[ 24u ] = AddParity( 'r' );
      au8LineBuffer[ 25u ] = AddParity( 'e' );
      au8LineBuffer[ 26u ] = AddParity( 'e' );
      au8LineBuffer[ 27u ] = AddParity( 'n' );
      au8LineBuffer[ 28u ] = gcau8AlphaColorTable[ TXT_BLUE ];
      au8LineBuffer[ 29u ] = AddParity( 'B' );
      au8LineBuffer[ 30u ] = AddParity( 'l' );
      au8LineBuffer[ 31u ] = AddParity( 'u' );
      au8LineBuffer[ 32u ] = AddParity( 'e' );
      au8LineBuffer[ 33u ] = gcau8AlphaColorTable[ TXT_WHITE ];
      au8LineBuffer[ 34u ] = AddParity( 'W' );
      au8LineBuffer[ 35u ] = AddParity( 'h' );
      au8LineBuffer[ 36u ] = AddParity( 'i' );
      au8LineBuffer[ 37u ] = AddParity( 't' );
      au8LineBuffer[ 38u ] = AddParity( 'e' );
      au8LineBuffer[ 39u ] = AddParity( ' ' );
      au8LineBuffer[ 40u ] = AddParity( ' ' );
      au8LineBuffer[ 41u ] = AddParity( ' ' );
      au8LineBuffer[ 42u ] = AddParity( ' ' );
      au8LineBuffer[ 43u ] = AddParity( ' ' );
      au8LineBuffer[ 44u ] = AddParity( ' ' );
    }
    else if( u8Line == 2u )
    {
      // this is a 0 magazine, 2 row packet --> contains only text information
      au8LineBuffer[  5u ] = gcau8AlphaColorTable[ TXT_MAGENTA ];
      au8LineBuffer[  6u ] = AddParity( 'M' );
      au8LineBuffer[  7u ] = AddParity( 'a' );
      au8LineBuffer[  8u ] = AddParity( 'g' );
      au8LineBuffer[  9u ] = AddParity( 'e' );
      au8LineBuffer[ 10u ] = AddParity( 'n' );
      au8LineBuffer[ 11u ] = AddParity( 't' );
      au8LineBuffer[ 12u ] = AddParity( 'a' );
      au8LineBuffer[ 13u ] = gcau8AlphaColorTable[ TXT_CYAN ];
      au8LineBuffer[ 14u ] = AddParity( 'C' );
      au8LineBuffer[ 15u ] = AddParity( 'y' );
      au8LineBuffer[ 16u ] = AddParity( 'a' );
      au8LineBuffer[ 17u ] = AddParity( 'n' );
      au8LineBuffer[ 18u ] = gcau8AlphaColorTable[ TXT_YELLOW ];
      au8LineBuffer[ 19u ] = AddParity( 'Y' );
      au8LineBuffer[ 20u ] = AddParity( 'e' );
      au8LineBuffer[ 21u ] = AddParity( 'l' );
      au8LineBuffer[ 22u ] = AddParity( 'l' );
      au8LineBuffer[ 23u ] = AddParity( 'o' );
      au8LineBuffer[ 24u ] = AddParity( 'w' );
      au8LineBuffer[ 25u ] = AddParity( 0x08u );  // flash attribute
      au8LineBuffer[ 26u ] = AddParity( 'F' );
      au8LineBuffer[ 27u ] = AddParity( 'l' );
      au8LineBuffer[ 28u ] = AddParity( 'a' );
      au8LineBuffer[ 29u ] = AddParity( 's' );
      au8LineBuffer[ 30u ] = AddParity( 'h' );
      au8LineBuffer[ 31u ] = gcau8AlphaColorTable[ TXT_WHITE ];
      au8LineBuffer[ 32u ] = AddParity( ' ' );
      au8LineBuffer[ 33u ] = AddParity( ' ' );
      au8LineBuffer[ 34u ] = AddParity( ' ' );
      au8LineBuffer[ 35u ] = AddParity( ' ' );
      au8LineBuffer[ 36u ] = AddParity( ' ' );
      au8LineBuffer[ 37u ] = AddParity( ' ' );
      au8LineBuffer[ 38u ] = AddParity( ' ' );
      au8LineBuffer[ 39u ] = AddParity( ' ' );
      au8LineBuffer[ 40u ] = AddParity( ' ' );
      au8LineBuffer[ 41u ] = AddParity( ' ' );
      au8LineBuffer[ 42u ] = AddParity( ' ' );
      au8LineBuffer[ 43u ] = AddParity( ' ' );
      au8LineBuffer[ 44u ] = AddParity( ' ' );
    }
    else if( u8Line == TELETEXT_LINES )
    {
      au8LineBuffer[  5u ] = AddParity( 'B' );
      au8LineBuffer[  6u ] = AddParity( 'o' );
      au8LineBuffer[  7u ] = AddParity( 't' );
      au8LineBuffer[  8u ] = AddParity( 't' );
      au8LineBuffer[  9u ] = AddParity( 'o' );
      au8LineBuffer[ 10u ] = AddParity( 'm' );
      au8LineBuffer[ 11u ] = AddParity( ' ' );
      au8LineBuffer[ 12u ] = AddParity( 'l' );
      au8LineBuffer[ 13u ] = AddParity( 'i' );
      au8LineBuffer[ 14u ] = AddParity( 'n' );
      au8LineBuffer[ 15u ] = AddParity( 'e' );
      au8LineBuffer[ 16u ] = AddParity( ' ' );
      au8LineBuffer[ 17u ] = AddParity( 't' );
      au8LineBuffer[ 18u ] = AddParity( 'e' );
      au8LineBuffer[ 19u ] = AddParity( 'x' );
      au8LineBuffer[ 20u ] = AddParity( 't' );
      au8LineBuffer[ 21u ] = AddParity( ' ' );
      au8LineBuffer[ 22u ] = AddParity( ' ' );
      au8LineBuffer[ 23u ] = AddParity( ' ' );
      au8LineBuffer[ 24u ] = AddParity( ' ' );
      au8LineBuffer[ 25u ] = AddParity( ' ' );
      au8LineBuffer[ 26u ] = AddParity( ' ' );
      au8LineBuffer[ 27u ] = AddParity( ' ' );
      au8LineBuffer[ 28u ] = AddParity( ' ' );
      au8LineBuffer[ 29u ] = AddParity( ' ' );
      au8LineBuffer[ 30u ] = AddParity( ' ' );
      au8LineBuffer[ 31u ] = AddParity( ' ' );
      au8LineBuffer[ 32u ] = AddParity( ' ' );
      au8LineBuffer[ 33u ] = AddParity( ' ' );
      au8LineBuffer[ 34u ] = AddParity( ' ' );
      au8LineBuffer[ 35u ] = AddParity( ' ' );
      au8LineBuffer[ 36u ] = AddParity( ' ' );
      au8LineBuffer[ 37u ] = AddParity( ' ' );
      au8LineBuffer[ 38u ] = AddParity( ' ' );
      au8LineBuffer[ 39u ] = AddParity( ' ' );
      au8LineBuffer[ 40u ] = AddParity( ' ' );
      au8LineBuffer[ 41u ] = AddParity( ' ' );
      au8LineBuffer[ 42u ] = AddParity( ' ' );
      au8LineBuffer[ 43u ] = AddParity( ' ' );
      au8LineBuffer[ 44u ] = AddParity( ' ' );
    }
    else if( u8Line == 3u )
    {
      // Graphics test
      au8LineBuffer[  5u ] = gcau8GraphicsColorTable[7];
      au8LineBuffer[  6u ] = AddParity( gcau8GraphicsLUT[0][0] );
      au8LineBuffer[  7u ] = gcau8GraphicsColorTable[1];
      au8LineBuffer[  8u ] = AddParity( gcau8GraphicsLUT[0][1] );
      au8LineBuffer[  9u ] = gcau8GraphicsColorTable[2];
      au8LineBuffer[ 10u ] = AddParity( gcau8GraphicsLUT[1][0] );
      au8LineBuffer[ 11u ] = gcau8GraphicsColorTable[3];
      au8LineBuffer[ 12u ] = AddParity( gcau8GraphicsLUT[1][1] );
      au8LineBuffer[ 13u ] = gcau8GraphicsColorTable[4];
      au8LineBuffer[ 14u ] = AddParity( gcau8GraphicsLUT[2][0] );
      au8LineBuffer[ 15u ] = gcau8GraphicsColorTable[5];
      au8LineBuffer[ 16u ] = AddParity( gcau8GraphicsLUT[2][1] );
      au8LineBuffer[ 17u ] = gcau8GraphicsColorTable[6];
      au8LineBuffer[ 18u ] = AddParity( gcau8GraphicsLUT[0][0] );
      au8LineBuffer[ 19u ] = gcau8GraphicsColorTable[0];
      au8LineBuffer[ 20u ] = AddParity( gcau8GraphicsLUT[0][0] );
      au8LineBuffer[ 21u ] = AddParity( ' ' );
      au8LineBuffer[ 20u ] = AddParity( ' ' );
      au8LineBuffer[ 23u ] = AddParity( ' ' );
      au8LineBuffer[ 24u ] = AddParity( ' ' );
      au8LineBuffer[ 25u ] = AddParity( ' ' );
      au8LineBuffer[ 26u ] = AddParity( ' ' );
      au8LineBuffer[ 27u ] = AddParity( ' ' );
      au8LineBuffer[ 28u ] = AddParity( ' ' );
      au8LineBuffer[ 29u ] = AddParity( ' ' );
      au8LineBuffer[ 30u ] = AddParity( ' ' );
      au8LineBuffer[ 31u ] = AddParity( ' ' );
      au8LineBuffer[ 32u ] = AddParity( ' ' );
      au8LineBuffer[ 33u ] = AddParity( ' ' );
      au8LineBuffer[ 34u ] = AddParity( ' ' );
      au8LineBuffer[ 35u ] = AddParity( ' ' );
      au8LineBuffer[ 36u ] = AddParity( ' ' );
      au8LineBuffer[ 37u ] = AddParity( ' ' );
      au8LineBuffer[ 38u ] = AddParity( ' ' );
      au8LineBuffer[ 39u ] = AddParity( ' ' );
      au8LineBuffer[ 40u ] = AddParity( ' ' );
      au8LineBuffer[ 41u ] = AddParity( ' ' );
      au8LineBuffer[ 42u ] = AddParity( ' ' );
      au8LineBuffer[ 43u ] = AddParity( ' ' );
      au8LineBuffer[ 44u ] = AddParity( ' ' );
    }
    else
    {
      au8LineBuffer[  5u ] = AddParity( '0'+ (u8Line/10u) );
      au8LineBuffer[  6u ] = AddParity( '0'+ (u8Line%10u) );
    }
  }
  // end of 'Hello World' program
}

/*! *******************************************************************
 * \brief  Line drawing algorithm for low gradients
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  bIsOn: If TRUE: line will be set; if FALSE: line will be cleared
 * \return -
 *********************************************************************/
static void PlotLineLow( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, BOOL bIsOn )
{
  I8  i8dx, i8dy, i8yi;
  I16 i16D;
  U8  u8X, u8Y;
  
  i8dx = u8X1 - u8X0;
  i8dy = u8Y1 - u8Y0;

  i8yi = 1;
  if( i8dy < 0 )
  {
    i8yi = -1;
    i8dy = -i8dy;
  }
  i16D = ( i8dy * 2 ) - i8dx;
  
  u8Y = u8Y0;
  for( u8X = u8X0; u8X <= u8X1; u8X++ )
  {
    Teletext_Pixel( u8X, u8Y, bIsOn );
    if( i16D > 0 )
    {
      u8Y += i8yi;
      i16D = i16D - ( i8dx * 2 );
    }
    i16D += ( i8dy * 2 );
  }
}

/*! *******************************************************************
 * \brief  Line drawing algorithm for high gradients
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  bIsOn: If TRUE: line will be set; if FALSE: line will be cleared
 * \return -
 *********************************************************************/
static void PlotLineHigh( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, BOOL bIsOn )
{
  I8 i8dx, i8dy, i8xi;
  I16 i16D;
  U8 u8X, u8Y;
  
  i8dx = u8X1 - u8X0;
  i8dy = u8Y1 - u8Y0;

  i8xi = 1;
  if( i8dx < 0 )
  {
    i8xi = -1;
    i8dx = -i8dx;
  }
  i16D = ( i8dx * 2 ) - i8dy;
  
  u8X = u8X0;
  for( u8Y = u8Y0; u8Y <= u8Y1; u8Y++ )
  {
    Teletext_Pixel( u8X, u8Y, bIsOn );
    if( i16D > 0 )
    {
      u8X += i8xi;
      i16D = i16D - ( i8dy * 2 );
    }
    i16D += ( i8dx * 2 );
  }
}

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
 * \brief  Init teletext line buffers.
 * \param  -
 * \return -
 *********************************************************************/
void Teletext_Init( void )
{
  U8 u8Line;
  
  // the 0 index line buffer will be transmitted first
  gu8TeletextLine = 0u;

  gbWaitForFieldEnd = FALSE;
  
  // Teletext lines
  // Note: bytes are transmitted LSB first!
  for( u8Line = 0u; u8Line <= TELETEXT_LINES; u8Line++ )
  {
    // Obligatory fields for all lines
    gau8TeletextLineBuffer[ u8Line ][  0u ] = 0x55u;  // clock run-in first byte
    gau8TeletextLineBuffer[ u8Line ][  1u ] = 0x55u;  // clock run-in second byte
    gau8TeletextLineBuffer[ u8Line ][  2u ] = 0x27u;  // framing code
    gau8TeletextLineBuffer[ u8Line ][ 45u ] = 0x00u;  // this will force SPI MOSI to low state
    // Packet address
    gau8TeletextLineBuffer[ u8Line ][  3u ] = Hamming84( MAGAZINE | ( ( u8Line & 1u ) << 3u ) );  // packet address: magazine number, packet number
    gau8TeletextLineBuffer[ u8Line ][  4u ] = Hamming84( u8Line >> 1u );  // packet address: packet number
  }
  
  // Header packet (0 magazine, 0 row packet) --> contains critical information about the page
  // page number
  gau8TeletextLineBuffer[ 0u ][  5u ] = Hamming84( 0u );  // units 0-F
  gau8TeletextLineBuffer[ 0u ][  6u ] = Hamming84( 0u );  // tens 0-F
  // Subcodes
  gau8TeletextLineBuffer[ 0u ][  7u ] = Hamming84( 0u );  // S1, 0-F
  gau8TeletextLineBuffer[ 0u ][  8u ] = Hamming84( 0u );  // S2, 0-7
  gau8TeletextLineBuffer[ 0u ][  9u ] = Hamming84( 0u );  // S3, 0-F
  gau8TeletextLineBuffer[ 0u ][ 10u ] = Hamming84( 0u );  // S4, 0-3
  // Control bits
  gau8TeletextLineBuffer[ 0u ][ 11u ] = Hamming84( 2u );  // data changed indicator
  gau8TeletextLineBuffer[ 0u ][ 12u ] = Hamming84( 0u );  // default codepage
  
  // Init visible area
  Teletext_TextMode();
  
#ifdef TELETEXT_TESTPATTERN
  // Displaying default test pattern
  TestPattern();
#else
  const U8 cau8HeaderLine[] = { p('S'), p('t'), p('r'), p('l'), p('e'), p('n'), p('.'),
                                p('T'), p('V'), p(' '), p(' '), p(' '), p(' '), p('1'), p('9'), p('9'), p('5'), p('/'), p('0'), p('9'), p('/'), p('1'), p('4'), p(' '), p(' '),
                                p(' '), p('2'), p('1'), p(':'), p('1'), p('6'), p(' ') };
  (void)memcpy( (U8*)&gau8TeletextLineBuffer[ 0u ][ 13u ], cau8HeaderLine, sizeof( cau8HeaderLine ) );
#endif
}

/*! *******************************************************************
 * \brief  Function to be called on each teletext-enabled scanline
 * \param  -
 * \return -
 *********************************************************************/
void Teletext_ScanLineCallback( void )
{
  //TODO: fill next scanline to be sent
}

/*! *******************************************************************
 * \brief  Function to be called after the end of teletext scanlines
 * \param  -
 * \return -
 *********************************************************************/
void Teletext_FieldEndCallback( void )
{
  gbWaitForFieldEnd = FALSE;
}

/*! *******************************************************************
 * \brief  Wait for field synchronization
 * \param  -
 * \return -
 *********************************************************************/
void Teletext_SynchronizeField( void )
{
  gbWaitForFieldEnd = TRUE;
  while( TRUE == gbWaitForFieldEnd );
}

/*! *******************************************************************
 * \brief  In text mode, writes character to given position
 * \param  u8PosX: Character X coordinate
 * \param  u8PosY: Character Y coordinate 
 * \param  u8Char: Character to be displayed
 * \return -
 *********************************************************************/
void Teletext_PutChar( U8 u8PosX, U8 u8PosY, U8 u8Char )
{
  // function parameter validation -- only for debug mode!
#ifdef DEBUG
  // function parameter validation -- only for debug mode!
  if( ( u8PosX >= 40u )
   || ( u8PosY >= TELETEXT_LINES ) )
  {
    return;
  }
#endif
  
  gau8TeletextLineBuffer[ u8PosY + 1u ][ u8PosX + 5u ] = AddParity( u8Char );
}

/*! *******************************************************************
 * \brief  Reinitializes teletext screen to text mode
 * \param  -
 * \return -
 *********************************************************************/
void Teletext_TextMode( void )
{
  U8 u8Line, u8Pos;
  
  // Note: header is not updated!
  for( u8Line = 1u; u8Line <= TELETEXT_LINES; u8Line++ )
  {
    // Default text transmitted: space
    for( u8Pos = 5u; u8Pos < 45u; u8Pos++ )
    {
      gau8TeletextLineBuffer[ u8Line ][ u8Pos ] = AddParity( ' ' );
    }
  }
}

/*! *******************************************************************
 * \brief  Switch display mode to graphics
 * \param  eColor: pixel color
 * \return -
 *********************************************************************/
void Teletext_GraphicsMode( E_TXT_COLOR eColor )
{
  U8 u8Line, u8Pos;
  
  // Note: header is not updated!
  for( u8Line = 1u; u8Line <= TELETEXT_LINES; u8Line++ )
  {
    gau8TeletextLineBuffer[ u8Line ][ 5u ] = gcau8GraphicsColorTable[ (U8)eColor ];
    // Default text transmitted: space
    for( u8Pos = 6u; u8Pos < 45u; u8Pos++ )
    {
      gau8TeletextLineBuffer[ u8Line ][ u8Pos ] = AddParity( ' ' );
    }
  }
}

/*! *******************************************************************
 * \brief  Turns the addressed pixel on/off
 * \param  u8PosX: Character X coordinate
 * \param  u8PosY: Character Y coordinate 
 * \param  bIsOn: If TRUE: pixel will be set; if FALSE: pixel will be cleared
 * \return -
 *********************************************************************/
void Teletext_Pixel( U8 u8PosX, U8 u8PosY, BOOL bIsOn )
{
  U8 u8Line = ( (U16)u8PosY*(U16)86u )>>8u;  // fixed-point division by 3
  U8 u8SubPixel = u8PosY - (u8Line*3u);  // fixed-point modulo by 3
  U8 u8Pos  = u8PosX>>1u;  // division by 2
  U8 u8Prev;

#ifdef DEBUG
  // function parameter validation -- only for debug mode!
  if( ( u8PosX >= TELETEXT_PIXELS_X )
   || ( u8PosY >= TELETEXT_PIXELS_Y ) )
  {
    return;
  }
#endif
  
  u8Prev = gau8TeletextLineBuffer[ u8Line + 1u ][ u8Pos + 6u ] & 0x7Fu;  // parity bit omitted
  if( TRUE == bIsOn )
  {
    gau8TeletextLineBuffer[ u8Line + 1u ][ u8Pos + 6u ] = AddParity( u8Prev | gcau8GraphicsLUT[ u8SubPixel ][ u8PosX & 0x01u ] );
  }
  else
  {
    gau8TeletextLineBuffer[ u8Line + 1u ][ u8Pos + 6u ] = AddParity( u8Prev & ~gcau8GraphicsLUT[ u8SubPixel ][ u8PosX & 0x01u ] );
  }
}

/*! *******************************************************************
 * \brief  Draws a line on graphics screen using Bresenham's line algorithm
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  bIsOn: If TRUE: line will be set; if FALSE: line will be cleared
 * \return -
 *********************************************************************/
void Teletext_DrawLine( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, BOOL bIsOn )
{
  //TODO: function parameter validation -- only for debug mode!
  
  // Note: this can be optimized!
  if( abs( u8Y1 - u8Y0 ) < abs( u8X1 - u8X0 ) )
  {
    if( u8X0 > u8X1 )
    {
      PlotLineLow( u8X1, u8Y1, u8X0, u8Y0, bIsOn );
    }
    else
    {
      PlotLineLow( u8X0, u8Y0, u8X1, u8Y1, bIsOn );
    }
  }
  else
  {
    if( u8Y0 > u8Y1 )
    {
      PlotLineHigh( u8X1, u8Y1, u8X0, u8Y0, bIsOn );
    }
    else
    {
      PlotLineHigh( u8X0, u8Y0, u8X1, u8Y1, bIsOn );
    }
  }
}

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


//-----------------------------------------------< EOF >--------------------------------------------------/
