/* http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm */

#include "mp3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum status {
	OK,
	ERROR
};

enum mpeg_id {
	MPEG_v1,
	MPEG_v2
};

enum layer_discription {
	LAYER_III,
	LAYER_II,
	LAYER_I
};

enum protection_bit {
	PROTECTED_BY_CRC,
	NOT_PROTECTED
};
enum samplerate {
	HZ44100,
	HZ48000,
	HZ32000
};
enum padding_bit {
	NOT_PADDED,
	PADDED
};

enum bitrate {
	free_kbps,
	kbps32,
	kbps40,
	kbps48,
	kbps56,
	kbps64,
	kbps80,
	kbps96,
	kbps112,
	kbps128,
	kbps160,
	kbps192,
	kbps224,
	kbps256,
	kbps320,
	bad_kbps
};

typedef struct {
	int status;
	int location;
	int lenght;
	int mpeg_id;
	int layer_discription;
	int protection_bit;
	int bitrate;
	int samplerate;
	int padding_bit;
} frame_t;

static int search_frame(const char* file_buffer, int size)
{
	for(int i = 0; i < size; ++i) {
		if((unsigned char)file_buffer[i] == (unsigned char)0xff)
			if(((unsigned char)file_buffer[i + 1] & (unsigned char)0xf0) == 0xf0)
				return i;
	}
	return -1;
}

static void get_info(const char *file_buffer, 
		int frame_position, 
		frame_t *frame_props)
{
	frame_props->status = OK;

	switch(*(file_buffer + frame_position + 1) & 0x8) {
		case 0:
			frame_props->mpeg_id = MPEG_v2;
			break;
		case 0x8:
			frame_props->mpeg_id = MPEG_v2;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_position + 1) & 0x6) {
		case 0:
			printf("Layer not defined.\n");
			frame_props->status = ERROR;
			break;
		case 0x2:
			printf("Layer III.\n");
			frame_props->layer_discription = LAYER_III;
			break;
		case 0x4:
			printf("Layer II.\n");
			frame_props->layer_discription = LAYER_II;
			break;
		case 0x6:
			printf("Layer I.\n");
			frame_props->layer_discription = LAYER_I;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_position + 1) & 0x1) {
		case 0x0:
			printf("Protected by CRC.\n");
			frame_props->protection_bit = PROTECTED_BY_CRC;
			break;
		case 0x1:
			printf("Not protected.\n");
			frame_props->protection_bit = NOT_PROTECTED;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_position + 2) & 0xc) {
		case 0:
			printf("Samplerate: 44100.\n");
			frame_props->samplerate = HZ44100;
			break;
		case 0x4:
			printf("Samplerate: 48000.\n");
			frame_props->samplerate = HZ48000;
			break;
		case 0x8:
			printf("Samplerate: 32000.\n");
			frame_props->samplerate = HZ32000;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_position + 2) & 0x2) {
		case 0x0:
			printf("Frame not padded.\n");
			frame_props->padding_bit = NOT_PADDED;
			break;
		case 0x2:
			printf("Frame padded.\n");
			frame_props->padding_bit = PADDED;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_position + 2) & 0xf0) {
		case(0x0):
			printf("Bitrate: free.\n");
			frame_props->bitrate = free_kbps;
			break;
		case(0x10):
			printf("Bitrate: 32.\n");
			frame_props->bitrate = kbps32;
			break;
		case(0x20):
			printf("Bitrate: 40.\n");
			frame_props->bitrate = kbps40;
			break;
		case(0x30):
			printf("Bitrate: 48.\n");
			frame_props->bitrate = kbps48;
			break;
		case(0x40):
			printf("Bitrate: 56.\n");
			frame_props->bitrate = kbps56;
			break;
		case(0x50):
			printf("Bitrate: 64.\n");
			frame_props->bitrate = kbps64;
			break;
		case(0x60):
			printf("Bitrate: 80.\n");
			frame_props->bitrate = kbps80;
			break;
		case(0x70):
			printf("Bitrate: 96.\n");
			frame_props->bitrate = kbps96;
			break;
		case(0x80):
			printf("Bitrate: 112.\n");
			frame_props->bitrate = kbps112;
			break;
		case(0x90):
			printf("Bitrate: 128.\n");
			frame_props->bitrate = kbps128;
			break;
		case(0xa0):
			printf("Bitrate: 160.\n");
			frame_props->bitrate = kbps160;
			break;
		case(0xb0):
			printf("Bitrate: 192.\n");
			frame_props->bitrate = kbps192;
			break;
		case(0xc0):
			printf("Bitrate: 224.\n");
			frame_props->bitrate = kbps224;
			break;
		case(0xd0):
			printf("Bitrate: 256.\n");
			frame_props->bitrate = kbps256;
			break;
		case(0xe0):
			printf("Bitrate: 320.\n");
			frame_props->bitrate = kbps320;
			break;
		case(0xf0):
			printf("Bitrate: bad.\n");
			frame_props->bitrate = bad_kbps;
			break;
		default:
			frame_props->status = ERROR;
	}
}

void play_frame(const char *file_buffer, frame_t *frame_props)
{
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

		get_info(file_buffer, frame_position, &frame_props);

		if(frame_props.status == OK)
			play_frame(file_buffer, &frame_props);

		frame_position += 32;
		frame_start = 0;
	}

	printf("Not implemented.\n");

	free(file_buffer);
	fclose(audio_file);
}
