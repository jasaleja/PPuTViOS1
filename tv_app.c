#include <stdio.h>
#include <time.h>
#include "graphic.h"

int32_t main()
{	
	infoElements input;
	
	input.channel = 2;
	input.teletext = 1;
	input.audioPID = 201;
	input.videoPID = 0;
	
	Graphic_Init();
	
	Show_Info_Banner(input);
	Show_Volume(2);
	
	sleep(5);
	
	Graphic_Deinit();
	return 0;
}

