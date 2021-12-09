/* https://web.archive.org/web/20070821052201/https://www.id3.org/mp3Frame */

#include "mp3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
	int frame_location;
} frame_t;

int search_frame(const char* file_buffer, int size)
{
	for(int i = 0; i < size; ++i) {
		if((unsigned char)file_buffer[i] == (unsigned char)0xff)
			if(((unsigned char)file_buffer[i + 1] & (unsigned char)0xf0) == 0xf0)
				return i;
	}
	return -1;
}

static void get_info(const char *file_buffer, int frame_position)
{
	switch(*(file_buffer + frame_position + 1) & 0x8) {
		case 0:
			printf("MPEG-2\n");
			break;
		case 0x8:
			printf("MPEG-1\n");
			break;
		default: {}
	}

	switch(*(file_buffer + frame_position + 1) & 0x6) {
		case 0:
			printf("Layer not defined.\n");
			break;
		case 0x2:
			printf("Layer III.\n");
			break;
		case 0x4:
			printf("Layer II.\n");
			break;
		case 0x6:
			printf("Layer I.\n");
			break;
		default: {}
	}

	switch(*(file_buffer + frame_position + 1) & 0x1) {
		case 0x0:
			printf("Protected by CRC.\n");
			break;
		case 0x1:
			printf("Not protected.\n");
			break;
		default: {}
	}

	switch(*(file_buffer + frame_position + 2) & 0xc) {
		case 0:
			printf("Bitrate: 44100.\n");
			break;
		case 0x4:
			printf("Bitrate: 48000.\n");
			break;
		case 0x8:
			printf("Bitrate: 32000.\n");
			break;
		default: {}
	}

	switch(*(file_buffer + frame_position + 2) & 0x2) {
		case 0x0:
			printf("Frame not padded.\n");
			break;
		case 0x2:
			printf("Frame padded.\n");
			break;
		default: {}
	}
}


void play_mp3_file(const char *file_name)
{
	struct stat file_stat;
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
	char *file_buffer = malloc(sizeof(char) *file_stat.st_size);
	if(fread((void*)file_buffer, 1, file_stat.st_size, audio_file) 
			!= file_stat.st_size) {
		fprintf(stderr, "File read error.\n");
		return;
	}

	if(strncmp(file_buffer, "ID3", 3) != 0 ) {
		fprintf(stderr, "Unsupported file format.\n");
		return;
	}

	int frame_position = 0, frame_start = 0;
	while((frame_start = search_frame(file_buffer + frame_position + frame_start, 
					file_stat.st_size - frame_position)) > 0) {
		frame_position += frame_start;
		printf("Frame position: %d\n", frame_position);
		get_info(file_buffer, frame_position);
		frame_position += 32;
		frame_start = 0;
	}

	printf("Not implemented.\n");

	free(file_buffer);
	fclose(audio_file);
}
