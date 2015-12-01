#include "remote.h"

/***********************************************************************
* @brief    Reads the events on the remote and call callback function
*
***********************************************************************/
static void* Read_Input_Events();

/***********************************************************************
* @brief    Gets the keys that were pressed
* 
* @param	[in] count - maximum number of events to read
* @param	[out] buf - array where the events will be
* @param	[out] eventsRead - number of read events
* 
* @return   EXIT_SUCCESS - no error
* @return   EXIT_FAILURE - error
*
***********************************************************************/
static int32_t Get_Keys(int32_t count, uint8_t* buf, int32_t* eventsRead);

Remote_Events_Callback RemoteEventsCallback = NULL;

static int32_t inputFileDesc;
static struct input_event* eventBuf;
static int32_t remoteInit = 0;  
static pthread_t readInputEventsThread;
static pthread_mutex_t mutex;

int32_t Remote_Init()
{
	const char* dev = "/dev/input/event0";
	char deviceName[20];

	inputFileDesc = open(dev, O_RDWR);
    if (inputFileDesc == ERROR)
    {
        printf("Error while opening device (%s)!\n", dev);
	    return EXIT_FAILURE;
    }
    
    ioctl(inputFileDesc, EVIOCGNAME(sizeof(deviceName)), deviceName);
	printf("RC device opened succesfully [%s]\n", deviceName);
	
	eventBuf = malloc(NUM_EVENTS * sizeof(struct input_event));
    if (!eventBuf)
    {
        printf("Error allocating memory!\n");
        return EXIT_FAILURE;
    }
    
    remoteInit = 1;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&readInputEventsThread, NULL, Read_Input_Events, NULL);
	return EXIT_SUCCESS;
}

int32_t Remote_Deinit()
{
	pthread_mutex_lock(&mutex);
	remoteInit = 0;
	pthread_mutex_unlock(&mutex);
	
	pthread_join(readInputEventsThread, NULL);
	
	free(eventBuf);
	
	if(close(inputFileDesc) == ERROR)
	{
		printf("Error while closing device (%d)!\n", strerror(errno));
	    return EXIT_FAILURE;
	}
	
	pthread_mutex_destroy(&mutex);
	return EXIT_SUCCESS;
}

int32_t Remote_Register_Events_Callback(Remote_Events_Callback remoteEventsCallback)
{
	if(RemoteEventsCallback != NULL)
	{
		printf("%s(%d): Remote_Register_Events_Callback failed, callback already registered!\n", __FUNCTION__, __LINE__);
		return EXIT_FAILURE;
	}
	
	RemoteEventsCallback = remoteEventsCallback;
	
	return EXIT_SUCCESS;
}

int32_t Remote_Unregister_Events_Callback(Remote_Events_Callback remoteEventsCallback)
{
	if(RemoteEventsCallback == NULL)
	{
		printf("%s(%d): Remote_Unregister_Events_Callback failed, callback already unregistered!\n", __FUNCTION__, __LINE__);
		return EXIT_FAILURE;
	}
	
	if(RemoteEventsCallback != remoteEventsCallback)
	{
		printf("%s(%d): Remote_Unregister_Events_Callback failed, wrong callback function!\n", __FUNCTION__, __LINE__);
		return EXIT_FAILURE;
	}
	
	RemoteEventsCallback = NULL;
	
	return EXIT_SUCCESS;
}

void* Read_Input_Events()
{
	uint32_t eventCnt;
	
	while (NON_STOP)
    {
		pthread_mutex_lock(&mutex);
		if (remoteInit == 0)
		{
			pthread_mutex_unlock(&mutex);
			return;
		}
		pthread_mutex_unlock(&mutex);
		
        /* Read input events */
        if (Get_Keys(NUM_EVENTS, (uint8_t*)eventBuf, &eventCnt))
        {
			printf("Error while reading input events!");
			return;
		}
		
		if (RemoteEventsCallback != NULL)
		{
			RemoteEventsCallback(eventBuf, eventCnt);
		}
	}
	
}

int32_t Get_Keys(int32_t count, uint8_t* buf, int32_t* eventsRead)
{
    int32_t ret = 0;
    
    /* Read input events and put them in buffer */
    ret = read(inputFileDesc, buf, (size_t)(count * (int)sizeof(struct input_event)));
    if (ret <= 0)
    {
        printf("Error code %d\n", ret);
        return EXIT_FAILURE;
    }
    
    /* Calculate number of read events */
    *eventsRead = ret / (int)sizeof(struct input_event);
    
    return EXIT_SUCCESS;
}
