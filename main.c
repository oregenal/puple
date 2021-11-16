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

typedef struct {
	uint32_t riff_heder;
	uint32_t file_size;
	uint32_t wave_header;
	uint32_t fmt__header;
	uint32_t size_of_wave_section_chunck;
	uint16_t wave_type_format;
	uint16_t mono_stereo;
	uint32_t sample_rate;
	uint32_t bytes_sec;
	uint16_t block_aligment;
	uint16_t bit_sample;
	uint32_t data_description_header;
	uint32_t size_of_data_chunck;
} Header;

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

	/*s = pa_simple_new(NULL, "puple", PA_STREAM_RECORD,
					NULL, "Music", &ss, NULL, NULL, NULL);
	assert(s && "PulseAudio connection");

	if(pa_simple_read(s, (void*)audio_buffer, audio_buffer_size, &err))
		error_handle("Read fail", err);
	pa_simple_free(s); */

	s = pa_simple_new(NULL, "puple", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, NULL);
	assert(s && "PulseAudio connection");

	printf("size: %zu.\n", audio_buffer_size);

	char *audio_data = file_buffer + 44;

	if(pa_simple_write(s, (void*)audio_data, audio_buffer_size, &err))
		error_handle("Write fail", err);

	if(pa_simple_drain(s, &err))
		error_handle("Drain fail", err);

	pa_simple_free(s);
	printf("OK\n");
	return 0;
}
