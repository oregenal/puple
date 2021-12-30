#include <stdlib.h>
#include <string.h>

#include "play_frame.h"

static const uint8_t slen[2][16] = 
						{{0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
						{0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}};

static unsigned char *unpack_main_data(const char *file_buffer, 
									   frame_t *frame_props)
{
	int channels = frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2;

	int header_len = 4 + (channels == 1 ? 17 : 32);
	header_len += frame_props->protection_bit == PROTECTED_BY_CRC ? 2 : 0;

	/* Unpack main data */
	/* If main_data_begin = 0, main data started right after heade +
	 * side_info + crc if set */
	int main_data_size = frame_props->length - header_len;

	unsigned char *main_data = malloc(main_data_size);

	memcpy(main_data, file_buffer + frame_props->location + header_len, 
			main_data_size);

	if(frame_props->main_data_begin != 0) {
		/* If offset is set */
		int offset = frame_props->main_data_begin;
		int buffer_offset = 0;
		struct previous_frame_length *prev_frame = frame_props->previous_frame;
		int prev_main_data_size = main_data_size;

		while(prev_frame->length - header_len < offset) {
			buffer_offset += prev_frame->length;
			main_data_size += prev_frame->length - header_len;

			main_data = realloc(main_data, main_data_size);

			memmove(main_data + prev_frame->length - header_len,
					main_data, prev_main_data_size);

			memcpy(main_data, 
				   file_buffer + frame_props->location 
					   - buffer_offset + header_len, 
				   prev_frame->length - header_len);

			offset -= prev_frame->length - header_len;
			prev_frame = prev_frame->prev;
		}

		main_data_size += offset;
		main_data = realloc(main_data, main_data_size);

		memmove(main_data + offset, main_data, prev_main_data_size);

		memcpy(main_data, 
			   file_buffer + frame_props->location - buffer_offset - offset, 
			   offset);
	}

	return main_data;
}

void get_scalefactors(const unsigned char *main_data, 
					  int gr, int ch, int bit_counter)
{
}

void play_frame(const char *file_buffer, frame_t *frame_props)
{
	unsigned char *main_data = unpack_main_data(file_buffer, frame_props);

	int channels = frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2;

	for(int gr = 0; gr < 2; ++gr)
		for(int ch = 0; ch < channels; ++ch) {
			unsigned int part2_length = 0;
			int bit_counter = 0;
			uint8_t scf_com = frame_props->scalefac_compress[gr][ch];

			/* Decoding scalefactors */
			if((frame_props->block_type[gr][ch] == 0x2) 
					&& (frame_props->mixed_block_flag[gr][ch] == 0))
				part2_length = 18 * slen[1][scf_com] + 18 * slen[2][scf_com];
			else if((frame_props->block_type[gr][ch] == 0x2) 
					&& (frame_props->mixed_block_flag[gr][ch] == 1))
				part2_length = 17 * slen[1][scf_com] + 18 * slen[2][scf_com];
			else 
				part2_length = 11 * slen[1][scf_com] + 10 * slen[2][scf_com];

			/* TODO: remove dummy part2_length */
			(void)part2_length;

			get_scalefactors(main_data, gr, ch, bit_counter);
		}

	free(main_data);
}
