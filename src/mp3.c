/* https://web.archive.org/web/20070821052201/https://www.id3.org/mp3Frame */

#include "mp3.h"
#include "str_search_ptrn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const unsigned int SYNCWORD = 0xfff0; /* 1111 1111 1111b */

static void play_MPEG_1()
{
	printf("MPEG-1\n");
}

static void play_MPEG_2()
{
	printf("MPEG-2\n");
}

static void get_info(const char *file_buffer, int frame_position)
{
	printf("SYNCWORD: %d.\n", frame_position);

	switch(*(int*)(file_buffer + frame_position + 3) & 0x8) {
		case 0:
			play_MPEG_2();
			break;
		case 0x8:
			play_MPEG_1();
			break;
		default: {}
	}

	switch(*(int*)(file_buffer + frame_position + 3) & 0x6) {
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

	switch(*(int*)(file_buffer + frame_position + 5) & 0xc) {
		case 0:
			printf("22050\n");
			break;
		case 0x4:
			printf("24000\n");
			break;
		case 0x8:
			printf("16000\n");
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
	while((frame_start = str_search_ptrn((char*)&SYNCWORD, 
					file_buffer + frame_position + frame_start, 
					file_stat.st_size - frame_position)) > 0) {
		frame_position += frame_start;
		get_info(file_buffer, frame_position);
		frame_position += 32;
		frame_start = 0;
	}

	printf("Not implemented.\n");

	free(file_buffer);
	fclose(audio_file);
}
