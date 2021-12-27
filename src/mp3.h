#ifndef MP3_H_
#define MP3_H_

#include <stdint.h>

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

struct previous_frame_length {
	int length;
	struct previous_frame_length *prev;
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
	struct previous_frame_length *previous_frame;

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
	uint8_t subblock_gain[2][2][3];
	uint16_t table_select[2][2][3];
	uint8_t region0_count[2][2];
	uint8_t region1_count[2][2];
	uint8_t preflag[2][2];
	uint8_t scalefac_scale[2][2];
	uint8_t count1table_select[2][2];
} frame_t;

void play_mp3_file(const char *);

#endif /* MP3_H_ */
