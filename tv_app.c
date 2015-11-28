#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "tdp_api.h"
#include "table_parse.h"

#define NO_ERROR 		0
#define ERROR			1

#define DESIRED_FREQUENCY 754000000	/* Tune frequency in Hz */
#define BANDWIDTH 8    				/* Bandwidth in Mhz */

int32_t Tuner_Callback(t_LockStatus status);
int32_t PAT_Callback(uint8_t* buffer);
int32_t PMT_Callback(uint8_t* buffer);

static pthread_mutex_t mutex;
static pthread_cond_t conditionVariable;

static PATTable programTable[10];
static PMTTable returnOfPMT;
static uint16_t numOfPrograms;

static uint16_t audioPID;
static uint16_t videoPID;

static uint32_t playerHandle;
static uint32_t sourceHandle;
static uint32_t filterHandle;

int32_t main()
{	
	struct timespec timeToWait;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&conditionVariable, NULL);
	
	{ /* Tuner setup */
		if(Tuner_Init() == ERROR)
		{
			printf("Tuner_Init returned ERROR\n");
			return ERROR;
		}
		
		if(Tuner_Register_Status_Callback(Tuner_Callback) == ERROR)
		{
			printf("Tuner_Register_Status_Callback returned ERROR\n");
			goto tuner_deinit;
		}
		
		if(Tuner_Lock_To_Frequency(DESIRED_FREQUENCY, BANDWIDTH, DVB_T) == ERROR)
		{
			printf("Tuner_Lock_To_Frequency ERROR\n");
			goto tuner_deinit;
		}
		
		clock_gettime(CLOCK_REALTIME, &timeToWait);
		timeToWait.tv_sec += 5;
		
		pthread_mutex_lock(&mutex);
		pthread_cond_timedwait(&conditionVariable, &mutex, &timeToWait);
		pthread_mutex_unlock(&mutex);
	}

	{ /* Player setup */
		if(Player_Init(&playerHandle) == ERROR)
		{
			printf("Player_Init returned ERROR\n");
			goto tuner_deinit;
		}
		
		if(Player_Source_Open(playerHandle, &sourceHandle) == ERROR)
		{
			printf("Player_Source_Open returned ERROR\n");
			goto player_deinit;
		}
	}

	{ /* Set filter for PAT table */
		if(Demux_Set_Filter(playerHandle, 0, 0, &filterHandle) == ERROR)
		{
			printf("Demux_Set_Filter returned ERROR\n");
			goto player_source_close;
		}
		/* Register callback for PAT */
		if(Demux_Register_Section_Filter_Callback(PAT_Callback) == ERROR)
		{
			printf("Demux_Set_Filter returned ERROR\n");
			goto demux_free_filter;
		}
		
		//clock_gettime(CLOCK_REALTIME, &timeToWait);
		//timeToWait.tv_sec += 5;
		
		pthread_mutex_lock(&mutex);
		pthread_cond_timedwait(&conditionVariable, &mutex, &timeToWait);
		pthread_mutex_unlock(&mutex);

		if(Demux_Unregister_Section_Filter_Callback(PAT_Callback) == ERROR)
		{
			printf("Demux_Set_Filter returned ERROR\n");
			goto demux_free_filter;
		}
		
		if(Demux_Free_Filter(playerHandle, filterHandle) == ERROR)
		{
			printf("Demux_Free_Filter returned ERROR\n");
			goto player_source_close;
		}
	}
	
	{ /* Set filter for PMT table */
		if(Demux_Set_Filter(playerHandle, programTable[0].programMapPID, 2, &filterHandle) == ERROR)
		{
			printf("Demux_Set_Filter returned ERROR\n");
			goto player_source_close;
		}
		/* Register callback for PMT */
		if(Demux_Register_Section_Filter_Callback(PMT_Callback) == ERROR)
		{
			printf("Demux_Set_Filter returned ERROR\n");
			goto demux_free_filter;
		}
		
		//clock_gettime(CLOCK_REALTIME, &timeToWait);
		//timeToWait.tv_sec += 5;
		
		pthread_mutex_lock(&mutex);
		pthread_cond_timedwait(&conditionVariable, &mutex, &timeToWait);
		pthread_mutex_unlock(&mutex);

		if(Demux_Unregister_Section_Filter_Callback(PMT_Callback) == ERROR)
		{
			printf("Demux_Set_Filter returned ERROR\n");
			goto demux_free_filter;
		}
		
		if(Demux_Free_Filter(playerHandle, filterHandle) == ERROR)
		{
			printf("Demux_Free_Filter returned ERROR\n");
			goto player_source_close;
		}
	}

	goto player_source_close;
	{ /* Deinitialization */
		demux_free_filter:
		if(Demux_Free_Filter(playerHandle, filterHandle) == ERROR)
		{
			printf("Demux_Free_Filter returned ERROR\n");
		}
		player_source_close:
		if(Player_Source_Close(playerHandle, sourceHandle) == ERROR)
		{
			printf("Player_Source_Close returned ERROR\n");
		}
		player_deinit:
		if(Player_Deinit(playerHandle) == ERROR)
		{
			printf("Player_Deinit returned ERROR\n");
		}
		tuner_deinit:
		if(Tuner_Deinit() == ERROR)
		{
			printf("Tuner_Deinit returned ERROR\n");
		}
	}
}

int32_t Tuner_Callback(t_LockStatus status)
{
	if(status == STATUS_ERROR)
	{
		printf("Tuner didn't lock to frequency\n");
	}
	else
	{
		printf("Tuner locked to frequency\n");
	}
	pthread_cond_signal(&conditionVariable);
	return 0;
}

int32_t PAT_Callback(uint8_t* buffer)
{
	numOfPrograms = PAT_Parse(buffer, programTable);
	pthread_cond_signal(&conditionVariable);
	return 0;
}

int32_t PMT_Callback(uint8_t* buffer)
{
	numOfPrograms = PMT_Parse(buffer, &returnOfPMT);
	printf("videoPID: %d\n", returnOfPMT.videoPID);
	printf("audioPID: %d\n", returnOfPMT.audioPID);
	printf("teletext: %d\n", returnOfPMT.teletext);
	pthread_cond_signal(&conditionVariable);
	return 0;
}
