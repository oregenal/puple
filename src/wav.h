#ifndef WAV_H_
#define WAV_H_

enum format {
	F8BIT = 8,
	F16BIT = 16,
	F24BIT = 24,
	F32BIT = 32,
};

void play_wav_file(const char *file_name);

#endif /* WAV_H_ */
