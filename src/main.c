#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <sys/stat.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "str_search_ptrn.h"

enum format {
	F8BIT = 8,
	F16BIT = 16,
	F24BIT = 24,
	F32BIT = 32,
};

void error_handle(const char *message, int err)
{
		fprintf(stderr, "%s: %s\n", message, pa_strerror(err));
}

void play_wav_file(const char *file_name)
{
	struct stat file_stat;
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
	char *file_buffer = malloc(sizeof(char) *file_stat.st_size);
	if(fread((void*)file_buffer, 1, file_stat.st_size, audio_file) 
			!= file_stat.st_size) {
		fprintf(stderr, "File read error.\n");
		return;
	}

	if(strncmp(file_buffer, "RIFF", 4) != 0
		&& strncmp((file_buffer + 8), "WAVE", 4) != 0) {
		fprintf(stderr, "Unsupported file format.\n");
		return;
	}

	uint16_t compression = *(file_buffer + 20);
	if(compression == 0x01 || compression == 0xfffe) {
		printf("Compression type: 0x%04hx.\n", compression);
	} else {
		fprintf(stderr, 
				"Unsupported compression type: 0x%04hx.\n", 
				compression);
		return;
	}

	int audio_data_position = 
		str_search_ptrn("data", file_buffer, file_stat.st_size);

	if(!audio_data_position) {
		fprintf(stderr, "No audio data found.\n");
		return;
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
		case F32BIT:
			ss.format = PA_SAMPLE_FLOAT32LE;
			break;
		default:
			fprintf(stderr, 
					"Unsupported bitrate: %hn\n", 
					(short*)(file_buffer + 34));
			return;
	} 

	ss.channels = *(char*)(file_buffer + 22);
	ss.rate = *(int*)(file_buffer + 24);
	printf("Number of channels: %d, Sample rate %d. Bitrate: %d.\n", 
			ss.channels, ss.rate, *(short*)(file_buffer + 34));

	s = pa_simple_new(NULL, "puple", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, &err);
	if(!s) {
		error_handle("Pulse connectiion error", err);
		return;
	}

	if(pa_simple_write(s, (void*)(file_buffer + audio_data_position + 8), 
				audio_buffer_size, &err))
		error_handle("Write fail", err);

	if(pa_simple_drain(s, &err))
		error_handle("Drain fail", err);

	pa_simple_free(s);
	free(file_buffer);
	fclose(audio_file);
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
		DIR *dir;
		struct dirent *dent;
		dir = opendir(file_name);
		if(!dir) {
			perror(file_name);
			return 1;
		}
		while((dent = readdir(dir)) != NULL) {
			if(str_search_ptrn(".wav", 
								dent->d_name, strlen(dent->d_name)) > 0) {
				printf("%s\n", dent->d_name);
				chdir(file_name);
				play_wav_file(dent->d_name);
			}
		}
		closedir(dir);
	} else if(S_ISREG(file_stat.st_mode)) {
		play_wav_file(file_name);
	} else {
		fprintf(stderr, "Not a regular file");
		exit(EXIT_FAILURE);
	}

	printf("OK\n");
	return 0;
}
