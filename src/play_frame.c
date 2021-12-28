#include <stdlib.h>
#include <string.h>

#include "play_frame.h"

static const uint8_t slen[2][16] = 
						{{0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
						{0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}};

void play_frame(const char *file_buffer, frame_t *frame_props)
{
	unsigned char *main_data;
	int channels = frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2;

	int header_len = 4 + (channels == 1 ? 17 : 32);
	header_len += frame_props->protection_bit == PROTECTED_BY_CRC ? 2 : 0;

	/* Unpack main data */
	if(frame_props->main_data_begin == 0) {
		/* If main_data_begin = 0, main data started right after heade +
		 * side_info + crc if set */
		main_data = malloc(frame_props->length - header_len);
		memcpy(main_data, file_buffer + frame_props->location + header_len, 
				frame_props->length - header_len);
	} else {
		main_data = malloc(frame_props->length - header_len);
	}

	for(int gr = 0; gr < 2; ++gr)
		for(int ch = 0; ch < channels; ++ch) {
			unsigned int part2_length = 0;
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
		}

	free(main_data);
}
