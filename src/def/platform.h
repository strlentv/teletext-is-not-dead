/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file platform.h
*
* \brief Compiler and platform specific codes
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Definitions and macros
//--------------------------------------------------------------------------------------------------------/

// Static assertion macro
#ifdef __GNUC__              // GNU C compiler
  #define STATIC_ASSERT(X)  //TODO: get it to work
//  #define STATIC_ASSERT(X) ({ extern int __attribute__((error("assertion failure: '" #X "' not true"))) compile_time_check(); ((X)?0:compile_time_check()),0; })

  #define PACKED_STRUCT
  #define PACKED_TYPES_BEGIN              _Pragma( "pack(push,1)" )
  #define PACKED_TYPES_END                _Pragma( "pack(pop)" )

#elif __IAR_SYSTEMS_ICC__    // IAR C compiler
  #define STATIC_ASSERT(predicate) _impl_CASSERT_LINE(predicate,__LINE__,__FILE__)
  #define _impl_PASTE(a,b) a##b
  #define _impl_CASSERT_LINE(predicate, line, file) typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1];

  #define PACKED_STRUCT                     __packed
  #define PACKED_TYPES_BEGIN                _Pragma( "pack(push,1)" )
  #define PACKED_TYPES_END                  _Pragma( "pack(pop)" )
#endif


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/



#endif  // PLATFORM_H

//-----------------------------------------------< EOF >--------------------------------------------------/
