/* http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm 
 * https://habr.com/ru/post/103635 
 * https://www.fcreyf.com/article/mp3-decoding-in-c++
 * http://gabriel.mp3-tech.org/mp3infotag.html
 * */

#define _DEFAULT_SOURCE

#include "mp3.h"
#include "str_search_ptrn.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <endian.h>

enum status {
	OK,
	INFO,
	XING,
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
	int data;
	int length;
	int mpeg_id;
	int layer_discription;
	int protection_bit;
	int bitrate;
	int samplerate;
	int padding_bit;
	int channel_mode;
	int intensity_stereo;
	int ms_stereo;

	/* side info */
	uint16_t main_data_begin;
	uint8_t scfsi[2];
	uint32_t part2_3_length[2][2];
	uint16_t big_value[2][2];
	uint8_t global_gain[2][2];
	uint8_t scalefac_compress[2][2];
	uint8_t windows_switching_flag[2][2];
	uint8_t block_type[2][2];
	uint8_t mixed_block_flag[2][2];
	uint8_t subblock_gain[2][2];
} frame_t;

static int search_frame(const char* file_buffer, int size)
{
	for(int i = 0; i < size; ++i) {
		if((unsigned char)file_buffer[i] == 0xff 
			&& (unsigned char)file_buffer[i + 1] >= 0xf0)
				return i;
	}
	return -1;
}

static void read_xing(const char *file_buffer, frame_t *frame_props)
{
	if(frame_props->status == INFO)
		printf("Info Tag found");

	if(frame_props->status == XING)
		printf("Xing Tag found");

	printf(", but proper parser not implemented yet.\n\n");

	printf("Encoder: ");
	fwrite(file_buffer + frame_props->data 
			+ frame_props->location + 120, 1, 9, stdout);
	putchar('.');
	putchar('\n');

	uint8_t low_pass = *(file_buffer + frame_props->data 
							   + frame_props->location + 130);
	printf("Low pass filter: %d.\n", low_pass);

	uint8_t min_bitrate = *(file_buffer + frame_props->data 
								  + frame_props->location + 140);
	printf("Minimal bitrate: %d.\n", min_bitrate);

	uint16_t start_saples = *(uint16_t *)(file_buffer 
										  + frame_props->data 
										  + frame_props->location 
										  + 141);
	start_saples = be16toh(start_saples) >> 4;
	printf("%d samples encoder delay. (samples added at begining)\n", 
			start_saples);

	uint16_t padded_saples = *(uint16_t *)(file_buffer 
										   + frame_props->data 
										   + frame_props->location 
										   + 142);
	padded_saples = be16toh(padded_saples) & 0x0fff;
	printf("%d samples have been padded at the end of the file.\n", 
			padded_saples);

	uint32_t mp3_length = *(uint32_t *)(file_buffer 
										+ frame_props->data 
										+ frame_props->location
										+ 148);
	mp3_length = be32toh(mp3_length);
	printf("Mp3 length: %d bytes.\n", mp3_length);

	uint8_t misc = *(uint8_t *)(file_buffer + frame_props->data
								+ frame_props->location + 144);
	switch(misc & 0xc0) {
		case(0x0):
			printf("Original bitrate: 32kHz or less.\n");
			break;
		case(0x40):
			printf("Original bitrate: 44.1kHz.\n");
			break;
		case(0x80):
			printf("Original bitrate: 48kHz.\n");
			break;
		case(0xc0):
			printf("Original bitrate: hegher than 48kHz.\n");
			break;
		default: {}
	}
	putchar('\n');
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

static int read_id3(const char *file_buffer)
{
	int res = 0;

	if(strncmp("ID3", file_buffer, 3) != 0)
		return -1;

	printf("ID3 version: 2.%d.\n", file_buffer[3]);

	for(int i = 0; i < 4; ++i) {
		res = (res << 7) + ((uint8_t)file_buffer[i + 6] & 0x7f);
	}

	res += 10;

	int unsynchronisation = file_buffer[5] >> 7;
	if(unsynchronisation) {
		fprintf(stderr, "Unsynchronisation not implemented.\n");
		return -1;
	}

	int extended_header = file_buffer[5] << 1 >> 7;
	if(extended_header) {
		fprintf(stderr, "Extended header not implemented.\n");
		return -1;
	}

	int experimental_indicator = file_buffer[5] << 2 >> 7;
	if(experimental_indicator) {
		fprintf(stderr, "Experimental stage not implemented.\n");
		return -1;
	}

	int footer_present = file_buffer[5] << 3 >> 7;
	if(footer_present) {
		fprintf(stderr, "Footer section not implemented.\n");
		return -1;
	}

	printf("Lenght of ID3 header: %d.\n", res);
	putchar('\n');

	return res;
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
