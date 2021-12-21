#ifndef HEADER_H_
#define HEADER_H_

#include "mp3.h"

void read_frame_header(const char *file_buffer, frame_t *frame_props);

#endif /* HEADER_H_ */
