/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file renderer.c
*
* \brief 3D rendering routines
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "types.h"
#include "teletext.h"

// Own include
#include "renderer.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define SIN_OWN(x)  gcaq15SinTable[(x)&0xFFu]
#define COS_OWN(x)  gcaq15SinTable[((x)+64u)&0xFFu]

#define CLIP_INSIDE 0u
#define CLIP_LEFT   1u
#define CLIP_RIGHT  2u
#define CLIP_BOTTOM 4u
#define CLIP_TOP    8u


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Lookup-table for calculating Sin(x)
//! \note  Contains a full sine wave!
const Q15 gcaq15SinTable[ 256u ] =
{
      0,   807,  1614,  2420,  3224,  4027,  4827,  5624,  6417,  7207,  7992,  8773,  9548, 10317, 11080, 11837,
  12586, 13328, 14061, 14786, 15502, 16208, 16905, 17592, 18267, 18932, 19585, 20226, 20855, 21472, 22075, 22665,
  23241, 23803, 24351, 24883, 25401, 25903, 26390, 26860, 27315, 27752, 28173, 28577, 28963, 29332, 29683, 30016,
  30330, 30627, 30904, 31163, 31403, 31624, 31826, 32008, 32171, 32315, 32439, 32543, 32627, 32692, 32737, 32761,
  32766, 32751, 32717, 32662, 32587, 32493, 32379, 32246, 32092, 31920, 31728, 31516, 31286, 31036, 30768, 30481, 
  30175, 29851, 29510, 29150, 28772, 28377, 27965, 27536, 27090, 26627, 26149, 25654, 25144, 24619, 24079, 23524,
  22955, 22372, 21775, 21165, 20542, 19907, 19260, 18601, 17931, 17250, 16558, 15856, 15145, 14425, 13695, 12958,
  12212, 11459, 10700,  9933,  9161,  8383,  7600,  6813,  6021,  5226,  4427,  3626,  2822,  2017,  1211,   404,
   -404, -1211, -2017, -2822, -3626, -4427, -5226, -6021, -6813, -7600, -8383, -9161, -9933,-10700,-11459,-12212,
 -12958,-13695,-14425,-15145,-15856,-16558,-17250,-17931,-18601,-19260,-19907,-20542,-21165,-21775,-22372,-22955,
 -23524,-24079,-24619,-25144,-25654,-26149,-26627,-27090,-27536,-27965,-28377,-28772,-29150,-29510,-29851,-30175,
 -30481,-30768,-31036,-31286,-31516,-31728,-31920,-32092,-32246,-32379,-32493,-32587,-32662,-32717,-32751,-32766,
 -32761,-32737,-32692,-32627,-32543,-32439,-32315,-32171,-32008,-31826,-31624,-31403,-31163,-30904,-30627,-30330,
 -30016,-29683,-29332,-28963,-28577,-28173,-27752,-27315,-26860,-26390,-25903,-25401,-24883,-24351,-23803,-23241,
 -22665,-22075,-21472,-20855,-20226,-19585,-18932,-18267,-17592,-16905,-16208,-15502,-14786,-14061,-13328,-12586,
 -11837,-11080,-10317, -9548, -8773, -7992, -7207, -6417, -5624, -4827, -4027, -3224, -2420, -1614,  -807,     0
};


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/
static U8 ComputeOutCode( Q15 q15X, Q15 q15Y );


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Computes the clip code for Cohen–Sutherland algorithm 
 * \param  q15X, q15Y: coordinates
 * \return Clip code
 *********************************************************************/
static U8 ComputeOutCode( Q15 q15X, Q15 q15Y )
{
  U8 u8OutCode = CLIP_INSIDE;          // initialised as being inside of [[clip window]]
  
  if( q15X < 0 )                       // to the left of clip window
  {
    u8OutCode |= CLIP_LEFT;
  }
  else if( q15X > TELETEXT_PIXELS_X )  // to the right of clip window
  {
    u8OutCode |= CLIP_RIGHT;
  }
  if( q15Y < 0 )                       // below the clip window
  {
    u8OutCode |= CLIP_TOP;
  }
  else if( q15Y > TELETEXT_PIXELS_Y )  // above the clip window
  {
    u8OutCode |= CLIP_BOTTOM;
  }

  return u8OutCode;
}

 /*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Rotates a vector over the X axis
 * \param  vect: vector to be rotated
 * \param  dPhi: angle of rotation
 * \return Rotated vector
 *********************************************************************/
S_VECTOR Renderer_RotateX( S_VECTOR vect, DEG dPhi )
{
  S_VECTOR sReturn;
  
  // fixed-point arithmetic -- it's faster with unsigned operations
  sReturn.x = vect.x;
  sReturn.y = ( (U32)vect.y*COS_OWN(dPhi) - (U32)vect.z*SIN_OWN(dPhi) )>>15u;
  sReturn.z = ( (U32)vect.y*SIN_OWN(dPhi) + (U32)vect.z*COS_OWN(dPhi) )>>15u;
  
  return sReturn;
}

/*! *******************************************************************
 * \brief  Rotates a vector over the Y axis
 * \param  vect: vector to be rotated
 * \param  dPhi: angle of rotation
 * \return Rotated vector
 *********************************************************************/
S_VECTOR Renderer_RotateY( S_VECTOR vect, DEG dPhi )
{
  S_VECTOR sReturn;
  
  // fixed-point arithmetic -- it's faster with unsigned operations
  sReturn.x = ( (U32)vect.x*COS_OWN(dPhi) + (U32)vect.z*SIN_OWN(dPhi) )>>15u;
  sReturn.y = vect.y;
  sReturn.z = ( 0 -(U32)vect.x*SIN_OWN(dPhi) + (U32)vect.z*COS_OWN(dPhi) )>>15u;
  
  return sReturn;
}

/*! *******************************************************************
 * \brief  Rotates a vector over the Z axis
 * \param  vect: vector to be rotated
 * \param  dPhi: angle of rotation
 * \return Rotated vector
 *********************************************************************/
S_VECTOR Renderer_RotateZ( S_VECTOR vect, DEG dPhi )
{
  S_VECTOR sReturn;
  
  // fixed-point arithmetic -- it's faster with unsigned operations
  sReturn.x = ( (U32)vect.x * COS_OWN(dPhi) - (U32)vect.y * SIN_OWN(dPhi) )>>15u;
  sReturn.y = ( (U32)vect.x * SIN_OWN(dPhi) + (U32)vect.y * COS_OWN(dPhi) )>>15u;
  sReturn.z = vect.z;
  
  return sReturn;
}

/*! *******************************************************************
 * \brief  Projects a vector to the screen
 * \param  vect: vector to be projected
 * \param  q15CameraDistZ: distance of the camera from the object
 * \return Projected vector
 * \note   Uses division, so it can be slow on processors without HW divisor.
 *********************************************************************/
S_VECTOR Renderer_Project( S_VECTOR vect, Q15 q15CameraDistZ, U8 u8Zoom )
{
  S_VECTOR sReturn = vect;
  
  // fixed-point arithmetic
  sReturn.x = ( (Q31)u8Zoom*( (Q31)vect.x * (Q31)q15CameraDistZ )/( (Q31)q15CameraDistZ + (Q31)vect.z ) )/128;  //note: there is no hw divisor in STM32F030
  sReturn.y = ( (Q31)u8Zoom*( (Q31)vect.y * (Q31)q15CameraDistZ )/( (Q31)q15CameraDistZ + (Q31)vect.z ) )/128;
  sReturn.z = ( (Q31)u8Zoom*vect.z )/128;

  return sReturn;
}

/*! *******************************************************************
 * \brief  Projects a vector to the screen -- using an approximate method that is faster
 * \param  vect: vector to be projected
 * \return Projected vector
 * \note   Coordinates must be in <<128 to produce sufficient results.
 *********************************************************************/
S_VECTOR Renderer_ProjectFast( S_VECTOR vect )
{
  S_VECTOR sReturn = vect;
  
  // fixed-point arithmetic -- it's faster with unsigned operations
  sReturn.x += ( (U32)vect.x * (U32)vect.z )>>6u;
  sReturn.y += ( (U32)vect.y * (U32)vect.z )>>6u;
  sReturn.z = vect.z;
  
  return sReturn;
}

/*! *******************************************************************
 * \brief  Renders the object to display device
 * \param  asVertexes[]: table of object vertexes
 * \param  au8Edges[][2u]: table of object edges
 * \param  u8OffsetX: X offset of the camera
 * \param  u8OffsetY: Y offset of the camera
 * \param  u32NumberOfEdges: number of edges in the table
 * \return -
 *********************************************************************/
void Renderer_RenderObject( const S_VECTOR asVertexes[], const U8 au8Edges[][2u], U8 u8OffsetX, U8 u8OffsetY, U32 u32NumberOfEdges )
{
  U8 u8Index;
  Q15 q15X0, q15Y0, q15X1, q15Y1, q15X, q15Y;
  U8 u8Clip0, u8Clip1;
  U8 u8ClipOut;
  
  // Draw previously computed edges
  for( u8Index = 0u; u8Index < u32NumberOfEdges; u8Index++ )
  {
    q15X0 = ( (Q15)u8OffsetX + asVertexes[ au8Edges[ u8Index ][ 0u ] ].x );
    q15Y0 = ( (Q15)u8OffsetY + asVertexes[ au8Edges[ u8Index ][ 0u ] ].y );
    q15X1 = ( (Q15)u8OffsetX + asVertexes[ au8Edges[ u8Index ][ 1u ] ].x );
    q15Y1 = ( (Q15)u8OffsetY + asVertexes[ au8Edges[ u8Index ][ 1u ] ].y );
    // Simple Cohen–Sutherland line clipping algorithm implementation
    while( 1u )
    {
      u8Clip0 = ComputeOutCode( q15X0, q15Y0 );
      u8Clip1 = ComputeOutCode( q15X1, q15Y1 );
      if( 0u == ( u8Clip0 | u8Clip1 ) )  // both clipped endpoints are in window range
      {
        // Draw the clipped line
        Teletext_DrawLine( q15X0, q15Y0, q15X1, q15Y1, TRUE );
        break;  // exit loop
      }
      else if( 0u != ( u8Clip0 & u8Clip1 ) )
      {
        // both endpoints are outside of window range -- the line should not be rendered
        break;
      }
      else  // clipping should be performed
      {
        u8ClipOut = u8Clip0 ? u8Clip0 : u8Clip1;
        if( u8ClipOut & CLIP_BOTTOM )
        {
          q15X = q15X0 + (q15X1 - q15X0) * (TELETEXT_PIXELS_Y - q15Y0) / (q15Y1 - q15Y0);
          q15Y = TELETEXT_PIXELS_Y;
        }
        else if( u8ClipOut & CLIP_TOP )
        {
          q15X = q15X0 + (q15X1 - q15X0) * (0 - q15Y0) / (q15Y1 - q15Y0);
          q15Y = 0;
        }
        else if( u8ClipOut & CLIP_RIGHT )
        {
          q15Y = q15Y0 + (q15Y1 - q15Y0) * (TELETEXT_PIXELS_X - q15X0) / (q15X1 - q15X0);
          q15X = TELETEXT_PIXELS_X;
        }
        else if( u8ClipOut & CLIP_LEFT )
        {
          q15Y = q15Y0 + (q15Y1 - q15Y0) * (0 - q15X0) / (q15X1 - q15X0);
          q15X = 0;
        }
	if( u8ClipOut == u8Clip0)
        {
          q15X0 = q15X;
          q15Y0 = q15Y;
        }
        else
        {
          q15X1 = q15X;
          q15Y1 = q15Y;
        }
      }
    }

  }
}

/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//-----------------------------------------------< EOF >--------------------------------------------------/
