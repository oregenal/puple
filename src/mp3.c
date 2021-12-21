/* http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm 
 * https://habr.com/ru/post/103635 
 * https://www.fcreyf.com/article/mp3-decoding-in-c++
 * http://gabriel.mp3-tech.org/mp3infotag.html
 * */

#define _DEFAULT_SOURCE

#include "mp3.h"
#include "id3.h"
#include "xing.h"
#include "str_search_ptrn.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <endian.h>

static int search_frame(const char* file_buffer, int size)
{
	for(int i = 0; i < size; ++i) {
		if((unsigned char)file_buffer[i] == 0xff 
			&& (unsigned char)file_buffer[i + 1] >= 0xf0)
				return i;
	}
	return -1;
}

static void read_frame_header(const char *file_buffer, frame_t *frame_props)
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
		frame_props->length = (12 * frame_props->bitrate 
		/ frame_props->samplerate + frame_props->padding_bit) * 4;
	else if(frame_props->layer_discription == LAYER_III 
			|| frame_props->layer_discription == LAYER_II)
		frame_props->length = 144 * frame_props->bitrate 
		/ frame_props->samplerate + frame_props->padding_bit;

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
		frame_props->data = frame_props->location + 32 + 2;
	else
		frame_props->data = frame_props->location + 32;

	int info = str_search_ptrn("Info", 
							   (file_buffer + frame_props->location), 
							   frame_props->length);
	if(info >= 0) {
		frame_props->status = INFO;
		frame_props->data = info;
		read_xing(file_buffer, frame_props);
	}

	info = str_search_ptrn("Xing", 
						   (file_buffer + frame_props->location), 
						   frame_props->length);
	if(info >= 0) {
		frame_props->status = XING;
		frame_props->data = info;
		read_xing(file_buffer, frame_props);
	}
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

static void read_side_info(const char *file_buffer, frame_t *frame_props)
{
	int channels = frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2;

	uint32_t side_info = *(uint32_t *)(file_buffer + frame_props->data);
	side_info = be32toh(side_info);

	/* First 9 bits - negative offset from SYNCWORD to main data. */
	frame_props->main_data_begin = side_info >> 23;

	/* Skip private_bits field. Unused.
	 * 5 - for Single channel mode
	 * 3 - for others */
	uint8_t private_bits = channels == 1 ? 5 : 3;

	/* ScaleFactor Selection Information determines weather the same
	 * scalefactors are transfered for both granules of not.
	 * 4 bits for channel, 1 for each scalefactor band.
	 * if bit is set - scale factor for the first granule reused in the second
	 * if not - then each granule has its own scaling factors. */
	for(int ch = 0; ch < channels; ++ch)
		frame_props->scfsi[ch] = 
			(uint8_t)(side_info << (9 + private_bits + 4 * ch) >> 28);

	/* for each granule in frame */
	for(int gr = 0; gr < 2; ++gr)
		/* For each channel in granule */
		for(int ch = 0; ch < channels; ++ch) {
			/* 12 bits. Length of Scalefactors & Main data in bits. */
			frame_props->part2_3_length[gr][ch] = 0;

			/* 9 bits. Size of big_value partition. */
			frame_props->big_value[gr][ch] = 0;

			/* 8 bits. Quantization step size. */
			frame_props->global_gain[gr][ch] = 0;

			/* 4 bits. Derermine the size of slen1 & slen2. */
			frame_props->scalefac_compress[gr][ch] = 0;

			/* 1 bit. Indicate that not normal window is used. */
			frame_props->windows_switching_flag[gr][ch] = 0;

			if(frame_props->windows_switching_flag[gr][ch]) {
				/* 2 bits. The type of window for particular granule. */
				frame_props->block_type[gr][ch] = 0;

				/* 1 bit. Indicate that two lowest subbands are transformed
				 * using a normal window and remaining 30 are transformed
				 * using the window specified by the block_type variable. */
				frame_props->mixed_block_flag[gr][ch] = 0;

				/* TODO: Proper side info parsing */

				if(frame_props->windows_switching_flag[gr][ch]) {
					/* 10 bits. */
					frame_props->table_select[gr][ch] = 0;
				} else {
					/* 15 bits. */
					frame_props->table_select[gr][ch] = 0;
				}

				if(frame_props->block_type[gr][ch] == 10)
					/* 9 bits.  */
					frame_props->subblock_gain[gr][ch] = 0;
			}
	}

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
