#define _DEFAULT_SOURCE

#include "side.h"

#include <stdio.h>
#include <stdlib.h>
#include <endian.h>

static unsigned int parse_bites(const char *file_buffer, int *index, int amount)
{
	unsigned int first_byte = *index / 8;
	unsigned int first_bit = *index % 8;
	unsigned int last_byte = (*index + amount) / 8;
	unsigned int last_bit = (*index + amount) % 8;

	unsigned int size = last_byte - first_byte + 1;
	if(size > 4) {
		fprintf(stderr, "Error in parsing Side info section");
		exit(EXIT_FAILURE);
	}

	unsigned int res = 0;

	for(unsigned int i = 0; i < size; ++i) {
		res = (res << 8) | (unsigned char)file_buffer[i + first_byte];
	}

	res <<= first_bit + 8 * (4 - size);
	res >>= first_bit + 8 * (4 - size) + 8 - last_bit;

	*index += amount;

	return res;
}

void read_side_info(const char *file_buffer, frame_t *frame_props)
{
	int channels = frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2;
	int bit_counter = 0;

	/* First 9 bits - negative offset from SYNCWORD to main data. */
	frame_props->main_data_begin = parse_bites(file_buffer + frame_props->data, 
											   &bit_counter,
											   9);

	/* Skip private_bits field. Unused.
	 * 5 - for Single channel mode
	 * 3 - for others */
	bit_counter += channels == 1 ? 5 : 3;

	/* ScaleFactor Selection Information determines weather the same
	 * scalefactors are transfered for both granules of not.
	 * 4 bits for channel, 1 for each scalefactor band.
	 * if bit is set - scale factor for the first granule reused in the second
	 * if not - then each granule has its own scaling factors. */
	for(int ch = 0; ch < channels; ++ch)
		frame_props->scfsi[ch] = parse_bites(file_buffer + frame_props->data, 
											 &bit_counter, 
											 4);

	/* for each granule in frame */
	for(int gr = 0; gr < 2; ++gr)
		/* For each channel in granule */
		for(int ch = 0; ch < channels; ++ch) {
			/* 12 bits. Length of Scalefactors & Main data in bits. */
			frame_props->part2_3_length[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 12);

			/* 9 bits. Size of big_value partition. */
			frame_props->big_value[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 9);

			/* 8 bits. Quantization step size. */
			frame_props->global_gain[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 8);

			/* 4 bits. Derermine the size of slen1 & slen2. */
			frame_props->scalefac_compress[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 4);

			/* 1 bit. Indicate that not normal window is used, 
			 * & all info is in region0 & region1, region2 not used. */
			frame_props->windows_switching_flag[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 1);

			if(frame_props->windows_switching_flag[gr][ch]) {
				/* 2 bits. The type of window for particular granule. 
				 * 00 - forbidden.
				 * 01 - start block.
				 * 10 - 3 short windows.
				 * 11 - end block. */
				frame_props->block_type[gr][ch] = 
					parse_bites(file_buffer + frame_props->data, &bit_counter, 2);

				/* 1 bit. Indicate that two lowest subbands are transformed
				 * using a normal window and remaining 30 are transformed
				 * using the window specified by the block_type variable. */
				frame_props->mixed_block_flag[gr][ch] = 
					parse_bites(file_buffer + frame_props->data, &bit_counter, 1);

				for(int region = 0; region < 2; ++region) {
					/* 5 bits. Determine which one of 32 possible Huffman 
					 * tables is in use, for each region/channel/granule. */
					frame_props->table_select[gr][ch][region] = 
						parse_bites(file_buffer + frame_props->data, &bit_counter, 5);
				}

				for(int block = 0; block < 3; ++block)
					/* 3 bits. Indicate gain offset from global_gain 
					 * for each short block.
					 * Used only when block_type is 3 short windows,
					 * butt always transmitted. */
					frame_props->subblock_gain[gr][ch][block] = 
						parse_bites(file_buffer + frame_props->data, &bit_counter, 3);
			} else {
				for(int region = 0; region < 3; ++region) {
					/* 5 bits. Determine which one of 32 possible Huffman 
					 * tables is in use, for each region/channel/granule. */
					frame_props->table_select[gr][ch][region] = 
						parse_bites(file_buffer + frame_props->data, &bit_counter, 5);
				}
			}

			/* 4 bits. One less, then the number of scalefactor bands
			 * in region0. */
			frame_props->region0_count[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 4);

			/* 3 bits. One less, then the number of scalefactor bands
			 * in region1. */
			frame_props->region1_count[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 3);

			/* 1 bit. Additional high frieqensy amplification.
			 * Not used if "3 short windows. */
			frame_props->preflag[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 1);

			/* 1 bit. Quantization step size version. */
			frame_props->scalefac_scale[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 1);

			/* 1bit. Which one of 2 Huffman code tables for count1 region to
			 * apply. */
			frame_props->count1table_select[gr][ch] = 
				parse_bites(file_buffer + frame_props->data, &bit_counter, 1);
	}

}
