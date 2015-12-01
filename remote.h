#ifndef _REMOTE_H_
#define _REMOTE_H_

#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

#define NUM_EVENTS 5

#define ERROR -1
#define NON_STOP 1

typedef int32_t(*Remote_Events_Callback)(struct input_event* buffer, uint32_t eventCnt);

/***********************************************************************
* @brief    Remote initialization function
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Remote_Init();

/***********************************************************************
* @brief    Remote deinitialization function
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Remote_Deinit();

/***********************************************************************
* @brief    Registers the remote callback
* 
* @param	[in] remoteEventsCallback - callback function
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Remote_Register_Events_Callback(Remote_Events_Callback remoteEventsCallback);

/***********************************************************************
* @brief    Unregisters the remote callback
* 
* @param	[in] remoteEventsCallback - callback function
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Remote_Unregister_Events_Callback(Remote_Events_Callback remoteEventsCallback);

#endif
