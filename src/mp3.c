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
	MPEG_v1 = 1,
	MPEG_v2 = 2
};

enum layer_discription {
	LAYER_I = 1,
	LAYER_II = 2,
	LAYER_III = 3
};

enum protection_bit {
	NOT_PROTECTED,
	PROTECTED_BY_CRC
};
enum samplerate {
	HZ16000 = 16000,
	HZ22050 = 22050,
	HZ32000 = 32000,
	HZ24000 = 24000,
	HZ44100 = 44100,
	HZ48000 = 48000
};
enum padding_bit {
	NOT_PADDED = 0,
	PADDED = 1
};

enum bitrate {
	FREE_KBPS = 0,
	KBPS8 = 8000,
	KBPS16 = 16000,
	KBPS24 = 24000,
	KBPS32 = 32000,
	KBPS40 = 40000,
	KBPS48 = 48000,
	KBPS56 = 56000,
	KBPS64 = 64000,
	KBPS80 = 80000,
	KBPS96 = 96000,
	KBPS112 = 112000,
	KBPS128 = 128000,
	KBPS144 = 144000,
	KBPS160 = 160000,
	KBPS176 = 176000,
	KBPS192 = 192000,
	KBPS224 = 224000,
	KBPS256 = 256000,
	KBPS288 = 288000,
	KBPS320 = 320000,
	KBPS352 = 352000,
	KBPS384 = 384000,
	KBPS416 = 416000,
	KBPS448 = 448000,
	BAD_KBPS
};

enum channel_mode {
	STEREO,
	JOINT_STEREO,
	DUAL_CHANNEL,
	SINGLE_CHANNEL
};

enum intensity_stereo {
	INTENSITY_OFF,
	BANDS_4_31,
	BANDS_8_31,
	BANDS_12_31,
	BANDS_16_31,
	INTENSITY_ON
};

enum ms_stereo {
	MS_OFF,
	MS_ON
};

typedef struct {
	int status;
	int location;
	int audio_data;
	int lenght;
	int mpeg_id;
	int layer_discription;
	int protection_bit;
	int bitrate;
	int samplerate;
	int padding_bit;
	int channel_mode;
	int intensity_stereo;
	int ms_stereo;
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

static void get_info(const char *file_buffer, frame_t *frame_props)
{
	frame_props->status = OK;

	switch(*(file_buffer + frame_props->location + 1) & 0x8) {
		case 0:
			frame_props->mpeg_id = MPEG_v2;
			break;
		case 0x8:
			frame_props->mpeg_id = MPEG_v1;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 1) & 0x6) {
		case 0:
			frame_props->status = ERROR;
			break;
		case 0x2:
			frame_props->layer_discription = LAYER_III;
			break;
		case 0x4:
			frame_props->layer_discription = LAYER_II;
			break;
		case 0x6:
			frame_props->layer_discription = LAYER_I;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 1) & 0x1) {
		case 0x0:
			frame_props->protection_bit = PROTECTED_BY_CRC;
			break;
		case 0x1:
			frame_props->protection_bit = NOT_PROTECTED;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 2) & 0xc) {
		case 0:
			if(frame_props->mpeg_id == MPEG_v1)
				frame_props->samplerate = HZ44100;
			else
				frame_props->samplerate = HZ22050;
			break;
		case 0x4:
			if(frame_props->mpeg_id == MPEG_v1)
				frame_props->samplerate = HZ48000;
			else
				frame_props->samplerate = HZ24000;
			break;
		case 0x8:
			if(frame_props->mpeg_id == MPEG_v1)
				frame_props->samplerate = HZ32000;
			else
				frame_props->samplerate = HZ16000;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 2) & 0x2) {
		case 0x0:
			frame_props->padding_bit = NOT_PADDED;
			break;
		case 0x2:
			frame_props->padding_bit = PADDED;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 2) & 0xf0) {
		case(0x0):
			frame_props->bitrate = FREE_KBPS;
			frame_props->status = ERROR;
			break;
		case(0x10):
			if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
						|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS8;
			else
				frame_props->bitrate = KBPS32;
			break;
		case(0x20):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS64;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS40;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS16;
			else
				frame_props->bitrate = KBPS48;
			break;
		case(0x30):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS96;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS48;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS24;
			else
				frame_props->bitrate = KBPS56;
			break;
		case(0x40):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS128;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS56;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS32;
			else
				frame_props->bitrate = KBPS64;
			break;
		case(0x50):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS160;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS64;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS40;
			else
				frame_props->bitrate = KBPS80;
			break;
		case(0x60):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS192;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS80;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS48;
			else
				frame_props->bitrate = KBPS96;
			break;
		case(0x70):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS224;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS96;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS56;
			else
				frame_props->bitrate = KBPS112;
			break;
		case(0x80):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS256;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS112;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS64;
			else
				frame_props->bitrate = KBPS128;
			break;
		case(0x90):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS288;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS160;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS128;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS144;
			else
				frame_props->bitrate = KBPS80;
			break;
		case(0xa0):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS320;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS192;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS160;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS160;
			else
				frame_props->bitrate = KBPS96;
			break;
		case(0xb0):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS352;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS224;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS192;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS176;
			else
				frame_props->bitrate = KBPS112;
			break;
		case(0xc0):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS384;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS256;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS224;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS192;
			else
				frame_props->bitrate = KBPS128;
			break;
		case(0xd0):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS416;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS320;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS256;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS224;
			else
				frame_props->bitrate = KBPS144;
			break;
		case(0xe0):
			if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS448;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS384;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS320;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS256;
			else
				frame_props->bitrate = KBPS160;
			break;
		case(0xf0):
			frame_props->bitrate = BAD_KBPS;
			frame_props->status = ERROR;
			break;
		default:
			frame_props->status = ERROR;
	}

	if(frame_props->layer_discription == LAYER_I)
		frame_props->lenght = (12 * frame_props->bitrate 
				/ frame_props->samplerate + frame_props->padding_bit) * 4;
	else if(frame_props->layer_discription == LAYER_III 
			|| frame_props->layer_discription == LAYER_II)
		frame_props->lenght = 144 * frame_props->bitrate 
			/frame_props->samplerate + frame_props->padding_bit;

	switch(*(file_buffer + frame_props->location + 3) & 0xc0) {
		case 0x0:
			frame_props->channel_mode = STEREO;
			break;
		case 0x40:
			frame_props->channel_mode = JOINT_STEREO;
			break;
		case 0x80:
			frame_props->channel_mode = DUAL_CHANNEL;
			break;
		case 0xc0:
			frame_props->channel_mode = SINGLE_CHANNEL;
			break;
		default:
			frame_props->status = ERROR;
	}	

	if(frame_props->channel_mode == JOINT_STEREO) {
		switch(*(file_buffer + frame_props->location + 3) & 0x30) {
			case 0x0:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_OFF;
					frame_props->ms_stereo = MS_OFF;
				} else {
					frame_props->intensity_stereo = BANDS_4_31;
				}
				break;
			case 0x10:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_ON;
					frame_props->ms_stereo = MS_OFF;
				} else {
					frame_props->intensity_stereo = BANDS_8_31;
				}
				break;
			case 0x20:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_OFF;
					frame_props->ms_stereo = MS_ON;
				} else {
					frame_props->intensity_stereo = BANDS_12_31;
				}
				break;
			case 0x30:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_ON;
					frame_props->ms_stereo = MS_ON;
				} else {
					frame_props->intensity_stereo = BANDS_16_31;
				}
				break;
			default:
				frame_props->status = ERROR;
		}
	}

	if(frame_props->protection_bit == PROTECTED_BY_CRC)
		frame_props->audio_data = frame_props->location + 32 + 2;
	else
		frame_props->audio_data = frame_props->location + 32;
}

void play_frame(const char *file_buffer, frame_t *frame_props)
{
	printf("Frame position: %d\n", frame_props->location);
	printf("Frame lenght: %d.\n", frame_props->lenght);
	printf("Mpeg version: %d.\n", frame_props->mpeg_id);
	printf("Layer version: %d.\n", frame_props->layer_discription);
	printf("Bitrate: %d.\n", frame_props->bitrate);
	printf("Samplerate: %d.\n", frame_props->samplerate);
	printf("Protection: %d.\n", frame_props->protection_bit);
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

	int frame_start = 0;
	frame_props.location = 0;
	while((frame_start = search_frame(file_buffer + frame_props.location + frame_start, 
					file_stat.st_size - frame_props.location)) > 0) {
		frame_props.location += frame_start;

		get_info(file_buffer, &frame_props);

		if(frame_props.status == OK)
			play_frame(file_buffer, &frame_props);

		frame_props.location += 32;
		frame_start = 0;
	}

	printf("Not implemented.\n");

	free(file_buffer);
	fclose(audio_file);
}
