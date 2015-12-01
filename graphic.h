#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <directfb.h>
#include <pthread.h>

/* Helper macro for error checking */
#define DFBCHECK(x...)										\
{															\
DFBResult err = x;											\
															\
if (err != DFB_OK)											\
  {															\
    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );	\
    DirectFBErrorFatal( #x, err );							\
  }															\
}

#define SHOW 1
#define HIDE 0

#define ERROR -1
#define NON_STOP 1

typedef struct infoElements {
	uint8_t channel;
	uint8_t teletext;
	uint16_t audioPID;
	uint16_t videoPID;
	/* TODO: Add infomation for personal exercise */
} infoElements;

typedef struct graphicElements {
	uint8_t infoBanner;
	infoElements infoBannerValue;
	timer_t timerInfoBanner;
	uint8_t volume;
	uint8_t volumeValue;
	timer_t timerVolume;
} graphicElements;

/***********************************************************************
* @brief    Graphic module initialization function
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Graphic_Init();

/***********************************************************************
* @brief    Graphic module deinitialization function
* 
* @return   EXIT_SUCCESS - no error
*
***********************************************************************/
int32_t Graphic_Deinit();

/***********************************************************************
* @brief    Signal the graphic module to show info banner
* 
* @param	[in] inputInfoBanner - structure with required elements that
* 								   are to be shown on the info banner
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Show_Info_Banner(infoElements inputInfoBanner);

/***********************************************************************
* @brief    Signal the graphic module to hide the info banner
* 
* @param	[in] value - required argument by timer callback
*
***********************************************************************/
void Hide_Info_Banner(union sigval value);

/***********************************************************************
* @brief    Signal the graphic module to show volume icon
* 
* @param	[in] volume - value of volume to be shown by progress bar
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
int32_t Show_Volume(uint8_t volume);

/***********************************************************************
* @brief    Signal the graphic module to hide the volume icon
* 
* @param	[in] value - required argument by timer callback
*
***********************************************************************/
void Hide_Volume(union sigval value);

#endif
