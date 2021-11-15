#include <stdio.h>
#include <stdlib.h>

#include <pulse/simple.h>

int main(void)
{
	const char hello[1024 * 20];
	size_t hello_size = sizeof(hello);
	int err;
	pa_simple *s;
	pa_sample_spec ss;

	ss.format = PA_SAMPLE_S16NE;
	ss.channels = 2;
	ss.rate = 44100;

	s = pa_simple_new(NULL, "pupl", PA_STREAM_PLAYBACK,
					NULL, "Music", &ss, NULL, NULL, NULL);
	if(!s)
		exit(1);

	pa_simple_write(s, (void*)hello, hello_size, &err);
	pa_simple_drain(s, &err);

	printf("size: %zu.\n", hello_size);

	pa_simple_free(s);
	printf("OK\n");
	return 0;
}
