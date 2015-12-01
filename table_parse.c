#include "table_parse.h"

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
static uint16_t Make_16bit_Number(uint8_t* buffer, uint16_t firstIndex, uint16_t secondIndex, uint16_t mask);

uint32_t PAT_Parse(uint8_t* buffer, PATTable* programTable)
{
	uint16_t sectionLength = 0;
	uint16_t numOfPrograms = 0;
	uint16_t programNumber = 0;
	uint16_t programMapPID = 0;
	uint16_t offset = 0;
	
	printf("PAT receiving started\n");
	
	sectionLength = Make_16bit_Number(buffer, 1, 2, 0x0FFF);
	sectionLength = sectionLength - 9;
	/* 
	 * 9 bytes are:				bit
	 * transport_stream_id		16
	 * reserved					02
	 * version_number			05
	 * current_next_indicator	01
	 * section_number			08
	 * last_section number		08
	 * CRC_32					32
	 */

	while (sectionLength > 0)
	{
		programNumber = Make_16bit_Number(buffer, 8 + offset, 9 + offset, 0xFFFF);
		
		if (programNumber != 0)
		{
			programTable[numOfPrograms].programNumber = programNumber;
			
			programMapPID = Make_16bit_Number(buffer, 10 + offset, 11 + offset, 0x1FFF);
			
			programTable[numOfPrograms].programMapPID = programMapPID;
			numOfPrograms++;
		}
		
		offset += 4;
		sectionLength -= 4;
		/* 
		 * 4 bytes are:		bit
		 * program_number	16
		 * reserved			03
		 * network_PID		13
		 */
	}
	printf("PAT receiving completed\n");
	return numOfPrograms;
}

int32_t PMT_Parse(uint8_t* buffer, PMTTable* returnValues)
{
	uint16_t sectionLength;
	uint16_t programInfoLength;
	uint16_t esInfoLength;
	uint8_t streamType;
	uint16_t offset = 0;
	uint16_t offsetSecond = 0;
	uint8_t descriptorTag;
	uint8_t descriptorLength;
	
	returnValues->videoPID = 0;
	returnValues->audioPID = 0;
	returnValues->teletext = 0;
	
	printf("PMT receiving started\n");
	
	sectionLength = Make_16bit_Number(buffer, 1, 2, 0x0FFF);
	
	programInfoLength = Make_16bit_Number(buffer, 10, 11, 0x0FFF);
	
	sectionLength = sectionLength - 13 - programInfoLength;
	/* 
	 * 13 bytes are:			bit
	 * program_number			16
	 * reserved					02
	 * version_number			05
	 * current_next_indicator	01
	 * section_number			08
	 * last_section number		08
	 * reserved					03
	 * PCR_PID					13
	 * reserved					04
	 * program_info_legth		12
	 * CRC_32					32
	 */
	
	offset = programInfoLength;
	while (sectionLength)
	{	
		if (sectionLength < 0)
		{
		 return EXIT_FAILURE;
		}
		streamType = *(buffer + 12 + offset);
		
		/* Video streams are either stream type 1 or 2 */
		if (streamType == 0x01 || streamType == 0x02)
		{
			returnValues->videoPID  = Make_16bit_Number(buffer, 13 + offset, 14 + programInfoLength + offset, 0x1FFF);
		}
		
		/* Audio streams are either stream type 3 or 4 */
		if (streamType == 0x03 || streamType == 0x04)
		{
			returnValues->audioPID = Make_16bit_Number(buffer, 13 + offset, 14 + programInfoLength + offset, 0x1FFF);
		}
		
		esInfoLength = Make_16bit_Number(buffer, 15 + offset, 16 + programInfoLength + offset, 0x0FFF);
		
		/* offsetSecond is used for goig through descriptors */
		offsetSecond = 0;
		while (esInfoLength - offsetSecond)
		{
			if (esInfoLength - offsetSecond < 0)
			{
				return EXIT_FAILURE;
			}
			descriptorTag = buffer[17 + offset + offsetSecond];
			descriptorLength = buffer[17 + offset + offsetSecond + 1];
			offsetSecond += descriptorLength + 2;
			/*
			 * 2 bytes are:			bit
			 * descriptor_tag		08
			 * descriptor_length	08
			 */
			if (descriptorTag == TELETEXT)
			{
				returnValues->teletext = 1;
			}
		}
		
		offset += (5 + esInfoLength);
		sectionLength -= (5 + esInfoLength);
		/* 
		 * 5 bytes are:		bit
		 * stream_type		08
		 * reserved			03
		 * elementary_PID	13
		 * reserved			04
		 * ES_info_legth	12
		 */
	}
	printf("PMT receiving completed\n");
	return EXIT_SUCCESS;
}

uint16_t Make_16bit_Number(uint8_t* buffer, uint16_t firstIndex, uint16_t secondIndex, uint16_t mask)
{
	uint16_t number;
	number = *(buffer + firstIndex);
	number = number<<8;
	number = number + *(buffer + secondIndex);
	number = number & mask;
	return number;
}
