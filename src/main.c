#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "str_search_ptrn.h"

enum format {
	F8BIT = 8,
	F16BIT = 16,
	F24BIT = 24,
};

void error_handle(const char *message, int err)
{
		fprintf(stderr, "%s: %s\n", message, pa_strerror(err));
}

int main(int argc, char **argv)
{
	char *file_name = NULL;
	if(argc > 1) {
		++argv;
		file_name = *argv;
	} else {
		fprintf(stderr, "No file specified.\n");
		printf("Usage %s </path/to/file.wav>\n", *argv);
		exit(EXIT_FAILURE);
	}

	struct stat file_stat;
	if(stat(file_name, &file_stat)) {
		perror("File error");
		exit(EXIT_FAILURE);
	}

	if(S_ISDIR(file_stat.st_mode)) {
		/* TODO: Implement dir play */
		fprintf(stderr, "Dir play not implemented yet.\n");
		exit(EXIT_FAILURE);
	} else if(!S_ISREG(file_stat.st_mode)) {
		fprintf(stderr, "Not a regular file");
		exit(EXIT_FAILURE);
	}

	FILE *audio_file = fopen(file_name, "rb");
	if(!audio_file) {
		perror("File open error.");
		exit(EXIT_FAILURE);
	}
	char *file_buffer = malloc(sizeof(char) * file_stat.st_size);
	if(fread((void*)file_buffer, 1, file_stat.st_size, audio_file) 
			!= file_stat.st_size) {
		fprintf(stderr, "File read error.\n");
		exit(EXIT_FAILURE);
	}

	if(strncmp(file_buffer, "RIFF", 4) != 0
		&& strncmp((file_buffer + 8), "WAVE", 4) != 0) {
		fprintf(stderr, "Unsupported file format.\n");
		exit(EXIT_FAILURE);
	}

	if(*(file_buffer + 20) != 0x01) {
		fprintf(stderr, "Unsupported compression type.\n");
		exit(EXIT_FAILURE);
	}

	int audio_data_position = 
		str_search_ptrn("data", file_buffer, file_stat.st_size);

	if(!audio_data_position) {
		fprintf(stderr, "No audio data found.\n");
		exit(EXIT_FAILURE);
	}

	size_t audio_buffer_size = *(int*)(file_buffer + audio_data_position + 4);

	int err;
	pa_simple *s;
	pa_sample_spec ss;

	switch(*(short*)(file_buffer + 34)) {
		case F8BIT: 
			ss.format = PA_SAMPLE_U8;
			break;
		case F16BIT:
			ss.format = PA_SAMPLE_S16LE;
			break;
		case F24BIT:
			ss.format = PA_SAMPLE_S24LE;
			break;
		default:
			fprintf(stderr, "Unsupported bitrate.\n");
			exit(EXIT_FAILURE);
	} 

	ss.channels = *((char*)(file_buffer + 22));
	ss.rate = *((int*)(file_buffer + 24));
	printf("Number of channels: %d, Sample rate %d. Bitrate: %d\n", 
			ss.channels, ss.rate, *(short*)(file_buffer + 34));

	s = pa_simple_new(NULL, "puple", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, &err);
	if(!s) {
		error_handle("Pulse connectiion error", err);
		exit(EXIT_FAILURE);
	}

	if(pa_simple_write(s, (void*)(file_buffer + audio_data_position + 8), 
				audio_buffer_size, &err))
		error_handle("Write fail", err);

	if(pa_simple_drain(s, &err))
		error_handle("Drain fail", err);

	pa_simple_free(s);
	free(file_buffer);
	fclose(audio_file);

	printf("OK\n");
	return 0;
}
