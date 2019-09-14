/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file types.h
*
* \brief General types
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef TYPES_H
#define TYPES_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <stdint.h>
#include "platform.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
// Unsigned types
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

// Signed types
typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

// Fractional types (fixed-point arithmetic)
typedef int16_t Q15;
typedef int32_t Q31;

#ifndef _WINDEF_H
// Bool type
typedef enum
{
  TRUE  = 0xA3,
  FALSE = 0x00
} BOOL;
#endif // BOOL

// Limits
#define U16MAX  0xFFFFu

// Special types
typedef I8  SNDTYPE;    //!< Number format used by the synthesizer for representing sound samples
typedef I16 MIXERTYPE;  //!< Number format used by the sound mixer

PACKED_TYPES_BEGIN

typedef PACKED_STRUCT struct  //!< Pixel format
{
  U8 u8Color : 4;
} PIXEL;

PACKED_TYPES_END

//--------------------------------------------------------------------------------------------------------/
// Static assertions
//--------------------------------------------------------------------------------------------------------/
STATIC_ASSERT( sizeof( U8   ) == 1u );
STATIC_ASSERT( sizeof( U16  ) == 2u );
STATIC_ASSERT( sizeof( U32  ) == 4u );
STATIC_ASSERT( sizeof( U64  ) == 8u );
STATIC_ASSERT( sizeof( I8   ) == 1u );
STATIC_ASSERT( sizeof( I16  ) == 2u );
STATIC_ASSERT( sizeof( I32  ) == 4u );
STATIC_ASSERT( sizeof( I64  ) == 8u );
STATIC_ASSERT( sizeof( BOOL ) == 1u );


#endif  // TYPES_H

//-----------------------------------------------< EOF >--------------------------------------------------/
