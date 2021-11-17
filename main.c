#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>

#include <pulse/simple.h>
#include <pulse/error.h>

void error_handle(const char *message, int err) {
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

	FILE *audio_file = fopen(file_name, "r");
	if(!audio_file) {
		perror("File open error.");
		exit(EXIT_FAILURE);
	}
	char *file_buffer = malloc(sizeof(char) * file_stat.st_size);
	if(fread((void*)file_buffer, 1, file_stat.st_size, audio_file) != file_stat.st_size) {
		fprintf(stderr, "File read error.\n");
		exit(EXIT_FAILURE);
	};

	if(strncmp(file_buffer, "RIFF", 4) != 0
		&& strncmp((file_buffer + 8), "WAVE", 4) != 0) {
		fprintf(stderr, "Unsuported file format.\n");
		exit(EXIT_FAILURE);
	};

	if(*(file_buffer + 20) != 0x01) {
		fprintf(stderr, "Unsuported compression type.\n");
		exit(EXIT_FAILURE);
	}

	size_t audio_buffer_size = *((int*)(file_buffer + 40));

	int err;
	pa_simple *s;
	pa_sample_spec ss;

	switch(*(short*)(file_buffer + 34)) {
		case 8: 
			ss.format = PA_SAMPLE_U8;
			break;
		case 16:
			ss.format = PA_SAMPLE_S16LE;
			break;
		default:
			fprintf(stderr, "Unsupotred bitrate.\n");
			exit(EXIT_FAILURE);
	} 

	ss.channels = *((char*)(file_buffer + 22));
	ss.rate = *((int*)(file_buffer + 24));
	printf("Number of channels: %d, Sample rate %d. Bitrate: %d\n", 
			ss.channels, ss.rate, *(short*)(file_buffer + 34));

	s = pa_simple_new(NULL, "puple", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, &err);
	if(!s) {
		fprintf(stderr, "Pulse connectiion error: %s.\n", pa_strerror(err));
		exit(EXIT_FAILURE);
	}

	char *audio_data = file_buffer + 44;

	if(pa_simple_write(s, (void*)audio_data, audio_buffer_size, &err))
		error_handle("Write fail", err);

	if(pa_simple_drain(s, &err))
		error_handle("Drain fail", err);

	pa_simple_free(s);
	free(file_buffer);
	fclose(audio_file);

	printf("OK\n");
	return 0;
}
