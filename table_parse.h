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

#endif
