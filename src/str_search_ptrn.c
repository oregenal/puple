#include "str_search_ptrn.h"

static int equal(int i, int j, const char *pattern, const char *str) {
	int f = 0;

	if(pattern[j] == 0)
		return 1;

	if(str[i] == pattern[j])
		f = equal(++i, ++j, pattern, str);

	return f;
}

int str_search_ptrn(const char* pattern, const char *str, int strln) {

	int i;
	
	for(i = 0; i < strln; i++) {
		int f, j = 0; 

		f = equal(i, j, pattern, str);

		if(f)
			break;
	};
	return i;
}
