#ifndef _TABLE_PARSE_H_
#define _TABLE_PARSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Descriptor code in PMT table */
#define TELETEXT	0x56

typedef struct PATTable {
	uint16_t programNumber;
	uint16_t programMapPID;
} PATTable;

typedef struct PMTTable {
	uint16_t videoPID;
	uint16_t audioPID;
	uint8_t teletext;
} PMTTable;

/***********************************************************************
* @brief    Parses the PAT table and saves the program numbers and their
* 			network PID
* 
* @param    [in] buffer - pointer to array with PAT table
* @param    [in] programTable - pointer to array of type PATTable where 
* 								to save program number and network PID
*
* @return   numOfPrograms - number of channels
*
***********************************************************************/
uint32_t PAT_Parse(uint8_t* buffer, PATTable* programTable);

/***********************************************************************
* @brief    Parses the PMT table and saves the audio and video PID of
* 			the streams, and information if there is teletext,
* 			if videoPID is 0, then the channel is audio only
* 
* @param    [in] buffer - pointer to array with PMT table
* @param    [out] returnValues - pointer to structure which contains the
* 								 audio and video PID of the channel and
* 								 information if there is teletext 
*
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t PMT_Parse(uint8_t* buffer, PMTTable* returnValues);

/***********************************************************************
* @brief    Takes two 8 bit numbers from buffer and makes combines them
* 			into 16 bit
* 
* @param    [in] buffer - pointer to array with PMT table
* @param    [in] firstIndex - index of more significant 8 bit
* @param    [in] secondIndex - index of more significant 8 bit
* @param    [in] mask - default is 0xFFFF for 16 bit numbers, for less 
* 						then 16 bit use adequate mask
*
* @return   number - 16 bit number
*
***********************************************************************/
uint16_t Make_16bit_Number(uint8_t* buffer, uint16_t firstIndex, uint16_t secondIndex, uint16_t mask);

#endif
