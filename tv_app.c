#include <stdio.h>
#include <pthread.h>
#include "remote.h"

#define NON_STOP 1

#define CH_UP 62
#define CH_DOWN 61
#define RM_EXIT 102

int32_t Remote_Callback(struct input_event* buffer, uint32_t eventCnt);
static uint8_t channel;
static uint8_t exitStatus;
static pthread_mutex_t mutex;

int32_t main()
{	
	channel = 0;
	exitStatus = 0;
	
	Remote_Init();
	Remote_Register_Events_Callback(Remote_Callback);
	pthread_mutex_init(&mutex, NULL);
	
	while(NON_STOP)
	{
		pthread_mutex_lock(&mutex);
		if (exitStatus == 1)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
	}
	
	Remote_Unregister_Events_Callback(Remote_Callback);
	Remote_Deinit();
	pthread_mutex_destroy(&mutex);
	
	return 0;
}

int32_t Remote_Callback(struct input_event* buffer, uint32_t eventCnt)
{
	uint8_t i;
	for (i = 0; i < eventCnt; i++)
	{
		if (buffer[i].type == EV_KEY && (buffer[i].value == 1 || buffer[i].value == 2))
		{
			switch (buffer[i].code)
			{
				case CH_UP:
					channel++;
					printf("Channel:\t%d\n", channel);
					break;
				case CH_DOWN:
					channel--;
					printf("Channel:\t%d\n", channel);
					break;
				case RM_EXIT:
					pthread_mutex_lock(&mutex);
					exitStatus = 1;
					pthread_mutex_unlock(&mutex);
					printf("Exit\n");
					break;
				default:
					break;
			}
		}
	}
	return 0;
}

