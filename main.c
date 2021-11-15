#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFFER_SIZE 1024 * 1000

void error_handle(const char *message, int err) {
		fprintf(stderr, "%s: %s\n", message, pa_strerror(err));
}

int main(void)
{
	char buffer[BUFFER_SIZE];
	size_t buffer_size = sizeof(buffer);
	int err;
	pa_simple *s;
	pa_sample_spec ss;

	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 2;
	ss.rate = 44100;

	s = pa_simple_new(NULL, "pupl", PA_STREAM_RECORD,
					NULL, "Music", &ss, NULL, NULL, NULL);
	assert(s && "PulseAudio connection");

	if(pa_simple_read(s, (void*)buffer, buffer_size, &err))
		error_handle("Read fail", err);
	pa_simple_free(s);

	s = pa_simple_new(NULL, "pupl", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, NULL);
	assert(s && "PulseAudio connection");

	printf("size: %zu.\n", buffer_size);

	while(1) {
		if(pa_simple_write(s, (void*)buffer, buffer_size, &err))
			error_handle("Write fail", err);

		if(pa_simple_drain(s, &err))
			error_handle("Drain fail", err);
	}

	pa_simple_free(s);
	printf("OK\n");
	return 0;
}
