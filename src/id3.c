#include "id3.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

int read_id3(const char *file_buffer)
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
