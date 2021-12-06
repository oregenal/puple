#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <sys/stat.h>

#include "str_search_ptrn.h"
#include "wav.h"

void play_mp3_file(const char *file_name)
{
	printf("Not implemented.\n");
}

void play_file(const char *file_name)
{
	if(str_search_ptrn(".wav", file_name, strlen(file_name)) > 0) {
		chdir(file_name);
		play_wav_file(file_name);
	} else if(str_search_ptrn(".mp3", file_name, strlen(file_name)) > 0) {
		play_mp3_file(file_name);
	}
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
			if(dent->d_type == DT_REG) {
				printf("%s\n", dent->d_name);
				chdir(file_name);
				play_file(dent->d_name);
			}
		}
		closedir(dir);
	} else if(S_ISREG(file_stat.st_mode)) {
		play_file(file_name);
	} else {
		fprintf(stderr, "Not a regular file");
		exit(EXIT_FAILURE);
	}

	printf("OK\n");
	return 0;
}
