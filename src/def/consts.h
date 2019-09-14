/*! *******************************************************************************************************
* Copyright (c) 2018-2019 K. Sz. Horvath
*
* All rights reserved
*
* \file consts.h
*
* \brief Global constants -- these act as parameters!
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef CONSTS_H
#define CONSTS_H

//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/

//=======================
// Global parameters
//=======================
#define DEBUG  //!< Comment if you want to have faster, smaller code

//=======================
// Packer/unpacker parameters
//=======================
#define PACKER_BUFFER_SIZE    (1024u)  //!< Size of the unpacked block

//=======================
// Synthesizer parameters
//=======================
#define SAMPLE_RATE         (54199u)  //!< Sample rate of audio output
#define SYNTH_CHANNELS          (4u)  //!< Number of channels synthesized

//===========================
// Video generator parameters
//=======================
// Teletext parameters
#define TELETEXT_OUTPUT                          //!< Comment out, if you don't want teletext output
#define TELETEXT_LINES                    (23u)  //!< Number of visible lines on teletext screen (excluding the header)
//#define TELETEXT_TESTPATTERN                     //!< Comment in, if you want a teletext test pattern in init

// Video picture parameters
//#define PICTURE_OUTPUT                           //!< Comment out, if you don't want picture output
#define ROW_DOUBLING                             //!< Each rows will be displayed twice
#define HORIZONTAL_RESOLUTION            (160u)  //!< Number of pixels in one row
#define VERTICAL_RESOLUTION              (128u)  //!< Number of rows
#define VISIBLE_ROWS    (2*VERTICAL_RESOLUTION)  //!< Number of visible rows on display


#endif  // CONSTS_H

//-----------------------------------------------< EOF >--------------------------------------------------/
