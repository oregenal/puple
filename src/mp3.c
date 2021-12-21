/* http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm 
 * https://habr.com/ru/post/103635 
 * https://www.fcreyf.com/article/mp3-decoding-in-c++
 * http://gabriel.mp3-tech.org/mp3infotag.html
 * */

#define _DEFAULT_SOURCE

#include "mp3.h"
#include "id3.h"
#include "header.h"
#include "side.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int search_frame(const char* file_buffer, int size)
{
	for(int i = 0; i < size; ++i) {
		if((unsigned char)file_buffer[i] == 0xff 
			&& (unsigned char)file_buffer[i + 1] >= 0xf0)
				return i;
	}
	return -1;
}


static void play_frame(const char *file_buffer, frame_t *frame_props)
{
	printf("Frame position: %d\n", frame_props->location);
	printf("Frame length: %d.\n", frame_props->length);
	printf("Mpeg version: %d.\n", frame_props->mpeg_id);
	printf("Layer version: %d.\n", frame_props->layer_discription);
	printf("Bitrate: %d.\n", frame_props->bitrate);
	printf("Samplerate: %d.\n", frame_props->samplerate);

	switch(frame_props->channel_mode) {
		case STEREO:
			printf("Stereo.\n");
			break;
		case JOINT_STEREO:
			printf("Joint stereo.\n");
			break;
		case DUAL_CHANNEL:
			printf("Dual channel.\n");
			break;
		case SINGLE_CHANNEL:
			printf("Single channel.\n");
			break;
		default: {}
	}

	if(frame_props->channel_mode == JOINT_STEREO) {
		switch(frame_props->intensity_stereo) {
			case INTENSITY_OFF:
				printf("Intensity stereo: Off.\n");
				break;
			case INTENSITY_ON:
				printf("Intensity stereo: On.\n");
				break;
			default: {}
		}

		switch(frame_props->ms_stereo) {
			case MS_OFF:
				printf("M/S stereo: Off.\n");
				break;
			case MS_ON:
				printf("M/S stereo: On.\n");
				break;
			default: {}
		}
	}

	printf("Padded: %d.\n", frame_props->padding_bit);
	printf("Protection: %d.\n", frame_props->protection_bit);
	printf("Main data negative offset: %d.\n", frame_props->main_data_begin);
	for(int ch = 0; 
			ch < (frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2)
			; ++ch)
		printf("Scfsi for channel %d: 0x%x.\n", ch + 1, frame_props->scfsi[ch]);
	putchar('\n');
}


void play_mp3_file(const char *file_name)
{
	struct stat file_stat;
	frame_t frame_props;
	if(stat(file_name, &file_stat)) {
		perror("File error");
		return;
	}

	if(!S_ISREG(file_stat.st_mode)) {
		fprintf(stderr, "Not a file.\n");
		return;
	}

	FILE *audio_file = fopen(file_name, "rb");
	if(!audio_file) {
		perror("File open error.");
		return;
	}
	char *file_buffer = malloc(sizeof(char) * file_stat.st_size);
	if(fread((void*)file_buffer, 1, file_stat.st_size, audio_file) 
			!= file_stat.st_size) {
		fprintf(stderr, "File read error.\n");
		return;
	}

	frame_props.location = read_id3(file_buffer);

	if(frame_props.location < 0) {
		fprintf(stderr, "Unsupported file format.\n");
		return;
	}

	int frame_start = 0;
	while((frame_start = search_frame(file_buffer 
									+ frame_props.location + frame_start, 
									file_stat.st_size - frame_props.location)) 
																	>= 0) {
		frame_props.location += frame_start;

		read_frame_header(file_buffer, &frame_props);

		if(frame_props.status == OK) {
			read_side_info(file_buffer, &frame_props);
			play_frame(file_buffer, &frame_props);
		}

		frame_props.location += frame_props.length;
		frame_start = 0;
	}

	printf("Not implemented.\n");

	free(file_buffer);
	fclose(audio_file);
}
