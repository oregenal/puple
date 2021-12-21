#define _DEFAULT_SOURCE

#include "xing.h"

#include <stdio.h>
#include <endian.h>

void read_xing(const char *file_buffer, frame_t *frame_props)
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
