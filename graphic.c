#include "graphic.h"

/***********************************************************************
* @brief    Renders the graphic in a loop
*
***********************************************************************/
static void* Render_Loop();

/***********************************************************************
* @brief    Renders the info banner
*
***********************************************************************/
static void Render_Info_Banner();

/***********************************************************************
* @brief    Renders the volume icon
*
***********************************************************************/
static void Render_Volume();

/***********************************************************************
* @brief    Adds the second strign to the first
* 
* @param	[in] firstStr - pointer to first string
* @param	[in] firstStr - pointer to second string
* 
* @return   i - number of added characters to the first string
*
***********************************************************************/
static int32_t Add_Strings(char* firstStr, char* secondStr);

/* Structure to be used by the main thread */
static graphicElements graphic;
/* Structure to be read by the render thread */
static graphicElements graphicLocal;
/* Signal for exiting render loop */ 
static int32_t graphicInit = 0;  
/* DFB basic variables */
static IDirectFBSurface *primary = NULL;
IDirectFB *dfbInterface = NULL;
/* Default 1920x1080 */
static int screenWidth = 0;
static int screenHeight = 0;
DFBSurfaceDescription surfaceDesc;
	
static pthread_t renderLoopThread;
static pthread_mutex_t mutex;
static pthread_mutex_t volumeMutex;
static pthread_mutex_t infoBannerMutex;

int32_t Graphic_Init()
{
	int32_t ret;
	struct sigevent signalEvent;

	/* Erase structures */
	memset(&graphic, 0, sizeof(graphicElements));
	memset(&graphicLocal, 0, sizeof(graphicElements));
	
	/* Tell the OS to call a specified function */
	signalEvent.sigev_notify = SIGEV_THREAD;
	/* Function to be called when timer runs out */
    signalEvent.sigev_notify_function = Hide_Info_Banner; 
    /* Thread arguments */
    signalEvent.sigev_value.sival_ptr = NULL;
    /* Thread attributes - if NULL default attributes are applied */
    signalEvent.sigev_notify_attributes = NULL; 
    /* Create Info Banner timer */
    ret = timer_create(CLOCK_REALTIME, &signalEvent, &graphic.timerInfoBanner);
	if (ret == ERROR)
	{
		printf("Error in info banner timer\n");
		return EXIT_FAILURE;
	}
	
	/* Function to be called when timer runs out */
    signalEvent.sigev_notify_function = Hide_Volume; 
	/* Create Volume timer */
    ret = timer_create(CLOCK_REALTIME, &signalEvent, &graphic.timerVolume);
	if (ret == ERROR)
	{
		printf("Error in volume timer\n");
		return EXIT_FAILURE;
	}
	
	/* Initialize DirectFB */
	DFBCHECK(DirectFBInit(NULL, NULL));
	/* Fetch the DirectFB interface */
	DFBCHECK(DirectFBCreate(&dfbInterface));
	/* Tell the DirectFB to take the full screen for this application */
	DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));

	/* Create primary surface with double buffering enabled */
	surfaceDesc.flags = DSDESC_CAPS;
	surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
	DFBCHECK (dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary));

	/* Fetch the screen size */
    DFBCHECK (primary->GetSize(primary, &screenWidth, &screenHeight));
	
	graphicInit = 1;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&volumeMutex, NULL);
	pthread_mutex_init(&infoBannerMutex, NULL);
	pthread_create(&renderLoopThread, NULL, Render_Loop, NULL);
	return EXIT_SUCCESS;
}

int32_t Graphic_Deinit()
{
	pthread_mutex_lock(&mutex);
	graphicInit = 0;
	pthread_mutex_unlock(&mutex);
	
	pthread_join(renderLoopThread, NULL);
	
	/* Clean up */
	primary->Release(primary);
	dfbInterface->Release(dfbInterface);
	
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&volumeMutex);
	pthread_mutex_destroy(&infoBannerMutex);
	
	timer_delete(graphic.timerInfoBanner);
	timer_delete(graphic.timerVolume);
	return EXIT_SUCCESS;
}

int32_t Show_Info_Banner(infoElements inputInfoBanner)
{
	int32_t ret;
	struct itimerspec timerSpec;
	
	pthread_mutex_lock(&infoBannerMutex);
	graphic.infoBanner = SHOW;
	graphic.infoBannerValue.channel = inputInfoBanner.channel;
	graphic.infoBannerValue.teletext = inputInfoBanner.teletext;
	graphic.infoBannerValue.audioPID = inputInfoBanner.audioPID;
	graphic.infoBannerValue.videoPID = inputInfoBanner.videoPID;
	pthread_mutex_unlock(&infoBannerMutex);
	
	/* Erase timer */
	memset(&timerSpec,0,sizeof(timerSpec));

	/* Specify the timer timeout time */
	timerSpec.it_value.tv_sec = 3;
	timerSpec.it_value.tv_nsec = 0;
	
	/* Set the new timer specificationss */
	ret = timer_settime(graphic.timerInfoBanner,0,&timerSpec,NULL);
	if (ret == ERROR)
	{
		printf("Error in info banner timer\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void Hide_Info_Banner(union sigval value)
{
	pthread_mutex_lock(&infoBannerMutex);
	graphic.infoBanner = HIDE;
	pthread_mutex_unlock(&infoBannerMutex);
}

int32_t Show_Volume(uint8_t volume)
{
	int32_t ret;
	struct itimerspec timerSpec;
	
	pthread_mutex_lock(&volumeMutex);
	graphic.volume = SHOW;
	graphic.volumeValue = volume;
	pthread_mutex_unlock(&volumeMutex);
		
	memset(&timerSpec,0,sizeof(timerSpec));

	/* Specify the timer timeout time */
	timerSpec.it_value.tv_sec = 3;
	timerSpec.it_value.tv_nsec = 0;
	
	/* Set the new timer specificationss */
	ret = timer_settime(graphic.timerVolume,0,&timerSpec,NULL);
	if (ret == ERROR)
	{
		printf("Error in volume timer\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void Hide_Volume(union sigval value)
{
	pthread_mutex_lock(&volumeMutex);
	graphic.volume = HIDE;
	pthread_mutex_unlock(&volumeMutex);
}  

void* Render_Loop()
{	
	while (NON_STOP)
    {
		pthread_mutex_lock(&mutex);
		if (graphicInit == 0)
		{
			pthread_mutex_unlock(&mutex);
			return;
		}
		pthread_mutex_unlock(&mutex);
		
		pthread_mutex_lock(&infoBannerMutex);
		pthread_mutex_lock(&volumeMutex);
		/* Check if local and global graphic structures are different */
		if (graphicLocal.infoBanner != graphic.infoBanner
			|| graphicLocal.volume != graphic.volume
			|| graphicLocal.volumeValue != graphic.volumeValue
			|| graphicLocal.infoBannerValue.channel != graphic.infoBannerValue.channel)
		{
			graphicLocal = graphic;
			pthread_mutex_unlock(&infoBannerMutex);
			pthread_mutex_unlock(&volumeMutex);
			
			/* Clear the screen before drawing anything */
			DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
			DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
			
			if (graphicLocal.infoBanner == SHOW)
			{
				Render_Info_Banner();
			}
			
			if(graphicLocal.volume == SHOW)
			{
				Render_Volume();
			}
			
			DFBCHECK(primary->Flip(primary, NULL, 0));
		}
		else
		{
			pthread_mutex_unlock(&infoBannerMutex);
			pthread_mutex_unlock(&volumeMutex);
		}
	}
}

void Render_Info_Banner()
{
	/* Outer rectangle drawing */
	DFBCHECK(primary->SetColor(primary, 0x00, 0x88, 0x44, 0xff));
	DFBCHECK(primary->FillRectangle(primary, screenWidth/4, 4*screenHeight/5, screenWidth/2, screenHeight/6));
	/* Inner rectangle drawing */
	DFBCHECK(primary->SetColor(primary, 0x00, 0xCE, 0x67, 0xff));
	DFBCHECK(primary->FillRectangle(primary, screenWidth/4+10, 4*screenHeight/5+10, screenWidth/2-20, screenHeight/6-20));
	
	/* Draw text */
	char channel[20] = "Channel: ";
	char audioPID[20] = "Audio PID: ";
	char videoPID[20] = "Video PID: ";
	char teletext[] = "TXT";
	char temp[6];
	
	IDirectFBFont *fontInterface = NULL;
	DFBFontDescription fontDesc;
	
	DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));

	/* Specify the height of the font by raising the appropriate flag and setting the height value */
	fontDesc.flags = DFDESC_HEIGHT;
	fontDesc.height = 48;
	/* Create the font and set the created font for primary surface text drawing */
	DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
	DFBCHECK(primary->SetFont(primary, fontInterface));
	/* Draw the text */
	sprintf(temp, "%d", graphicLocal.infoBannerValue.channel);
	Add_Strings(channel, temp);
	DFBCHECK(primary->DrawString(primary, channel, -1,
								 /*x coordinate of the top left corner of the resulting text*/ screenWidth/4+20,
								 /*y coordinate of the top left corner of the resulting text*/ 4*screenHeight/5+10,
								 /*in case of multiple lines, allign text to left*/ DSTF_TOPLEFT));
	
	fontDesc.flags = DFDESC_HEIGHT;
	fontDesc.height = 30;
	DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
	DFBCHECK(primary->SetFont(primary, fontInterface));
	
	sprintf(temp, "%d", graphicLocal.infoBannerValue.audioPID);
	Add_Strings(audioPID, temp);
	DFBCHECK(primary->DrawString(primary, audioPID, -1, screenWidth/4+20, 4*screenHeight/5+100, DSTF_TOPLEFT));
	
	sprintf(temp, "%d", graphicLocal.infoBannerValue.videoPID);
	Add_Strings(videoPID, temp);
	DFBCHECK(primary->DrawString(primary, videoPID, -1, screenWidth/4+20, 4*screenHeight/5+130, DSTF_TOPLEFT));
	if (graphicLocal.infoBannerValue.teletext == SHOW)
	{
		DFBCHECK(primary->DrawString(primary, teletext, -1, screenWidth-screenWidth/4-20, 4*screenHeight/5+10, DSTF_TOPRIGHT));
	}
}

void Render_Volume()
{
	/* Draw image from file */
	IDirectFBImageProvider *provider;
	IDirectFBSurface *logoSurface = NULL;
	int32_t logoHeight, logoWidth;
	char volumeFileName[] = "volume_0.png";

	/* Create the image provider for the specified file */
	if (graphicLocal.volumeValue >= 1 && graphicLocal.volumeValue <= 9)
	{
		volumeFileName[7] = graphicLocal.volumeValue + '0';
		DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, volumeFileName, &provider));
	}
	else if (graphicLocal.volumeValue == 10)
	{
		DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_10.png", &provider));
	}
	else
	{
		DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_0.png", &provider));
	}
	
	/* Get surface descriptor for the surface where the image will be rendered */
	DFBCHECK(provider->GetSurfaceDescription(provider, &surfaceDesc));
	/* Create the surface for the image */
	DFBCHECK(dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &logoSurface));
	/* Render the image to the surface */
	DFBCHECK(provider->RenderTo(provider, logoSurface, NULL));

	/* Cleanup the provider after rendering the image to the surface */
	provider->Release(provider);

	/* Fetch the logo size and add (blit) it to the screen */
	DFBCHECK(logoSurface->GetSize(logoSurface, &logoWidth, &logoHeight));
	DFBCHECK(primary->Blit(primary, logoSurface, NULL,
						   /* Destination x coordinate of the upper left corner of the image */40,
						   /* Destination y coordinate of the upper left corner of the image */40));
}

int32_t Add_Strings(char* firstStr, char* secondStr)
{
	int i = 0;
	int j = 0;
	
	while (firstStr[j] != '\0')
	{
		j++;
	}
	
	while (secondStr[i] != '\0')
	{
		firstStr[j + i] = secondStr[i];
		i++;
	}

	firstStr[j + i] = '\0';
	return i;
}




