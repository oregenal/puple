#include "play_frame.h"

static const uint8_t slen[2][16] = 
						{{0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
						{0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}};

void play_frame(const char *file_buffer, frame_t *frame_props)
{
	int channels = frame_props->channel_mode == SINGLE_CHANNEL ? 1 : 2;

	for(int gr = 0; gr < 2; ++gr)
		for(int ch = 0; ch < channels; ++ch) {
			unsigned int part2_length = 0;
			uint8_t scf_com = frame_props->scalefac_compress[gr][ch];

			/* Decoding scalefactors */
			if((frame_props->block_type[gr][ch] = 0x2) 
					&& (frame_props->mixed_block_flag[gr][ch] == 0))
				part2_length = 18 * slen[1][scf_com] + 18 * slen[2][scf_com];
			else if((frame_props->block_type[gr][ch] = 0x2) 
					&& (frame_props->mixed_block_flag[gr][ch] == 1))
				part2_length = 17 * slen[1][scf_com] + 18 * slen[2][scf_com];
			else 
				part2_length = 11 * slen[1][scf_com] + 10 * slen[2][scf_com];

			/* TODO: remove dummy part2_length */
			(void)part2_length;
		}
}
