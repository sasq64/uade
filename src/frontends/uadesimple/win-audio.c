#include "audio.h"

#include <uade/uade.h>

#include <stdlib.h>
#include <stdio.h>


void audio_close(void)
{
}


int total = 0;

int audio_init(int frequency)
{
	return 1;
}


int audio_play(char *samples, int bytes)
{
	for(int i=0; i<bytes; i++)
		total += samples[i];
	return bytes;
}
