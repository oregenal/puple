#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <sys/stat.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFFER_SIZE 1024 * 1000

void error_handle(const char *message, int err) {
		fprintf(stderr, "%s: %s\n", message, pa_strerror(err));
}

#define FILE_NAME "./speech.wav"

int main(void)
{
	struct stat file_stat;
	if(stat(FILE_NAME, &file_stat)) {
		perror("File error");
		exit(EXIT_FAILURE);
	}

	FILE *audio_file = fopen(FILE_NAME, "r");
	char *file_buffer = malloc(sizeof(char) * file_stat.st_size);

	if(fread((void*)file_buffer, 1, file_stat.st_size, audio_file) != file_stat.st_size) {
		fprintf(stderr, "File read error.\n");
		exit(EXIT_FAILURE);
	};
	printf("Readed from file: %zu.\n", file_stat.st_size);

	size_t audio_buffer_size = *((int*)(file_buffer + 40));
	printf("Audio Data size: %zu.\n", audio_buffer_size);

	int err;
	pa_simple *s;
	pa_sample_spec ss;

	ss.format = PA_SAMPLE_S16LE;
	ss.channels = *((char*)(file_buffer + 22));
	ss.rate = *((int*)(file_buffer + 24));
	printf("Number of channels: %d, Sample rate %d.\n", ss.channels, ss.rate);

	s = pa_simple_new(NULL, "puple", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, NULL);
	assert(s && "PulseAudio connection");

	char *audio_data = file_buffer + 44;

	if(pa_simple_write(s, (void*)audio_data, audio_buffer_size, &err))
		error_handle("Write fail", err);

	if(pa_simple_drain(s, &err))
		error_handle("Drain fail", err);

	pa_simple_free(s);
	printf("OK\n");
	return 0;
}
