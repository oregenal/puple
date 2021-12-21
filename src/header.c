#include "header.h"

#include "str_search_ptrn.h"
#include "xing.h"

void read_frame_header(const char *file_buffer, frame_t *frame_props)
{
	frame_props->status = OK;

	switch(*(file_buffer + frame_props->location + 1) & 0x8) {
		case 0:
			frame_props->mpeg_id = MPEG_v2;
			break;
		case 0x8:
			frame_props->mpeg_id = MPEG_v1;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 1) & 0x6) {
		case 0:
			frame_props->status = ERROR;
			break;
		case 0x2:
			frame_props->layer_discription = LAYER_III;
			break;
		case 0x4:
			frame_props->layer_discription = LAYER_II;
			break;
		case 0x6:
			frame_props->layer_discription = LAYER_I;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 1) & 0x1) {
		case 0x0:
			frame_props->protection_bit = PROTECTED_BY_CRC;
			break;
		case 0x1:
			frame_props->protection_bit = NOT_PROTECTED;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 2) & 0xc) {
		case 0:
			if(frame_props->mpeg_id == MPEG_v1)
				frame_props->samplerate = HZ44100;
			else
				frame_props->samplerate = HZ22050;
			break;
		case 0x4:
			if(frame_props->mpeg_id == MPEG_v1)
				frame_props->samplerate = HZ48000;
			else
				frame_props->samplerate = HZ24000;
			break;
		case 0x8:
			if(frame_props->mpeg_id == MPEG_v1)
				frame_props->samplerate = HZ32000;
			else
				frame_props->samplerate = HZ16000;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 2) & 0x2) {
		case 0x0:
			frame_props->padding_bit = NOT_PADDED;
			break;
		case 0x2:
			frame_props->padding_bit = PADDED;
			break;
		default:
			frame_props->status = ERROR;
	}

	switch(*(file_buffer + frame_props->location + 2) & 0xf0) {
		case(0x0):
			frame_props->bitrate = FREE_KBPS;
			frame_props->status = ERROR;
			break;
		case(0x10):
			if(frame_props->mpeg_id == MPEG_v2 
				&& (frame_props->layer_discription == LAYER_II 
				|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS8;
			else
				frame_props->bitrate = KBPS32;
			break;
		case(0x20):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS64;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS40;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS16;
			else
				frame_props->bitrate = KBPS48;
			break;
		case(0x30):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS96;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS48;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS24;
			else
				frame_props->bitrate = KBPS56;
			break;
		case(0x40):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS128;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS56;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS32;
			else
				frame_props->bitrate = KBPS64;
			break;
		case(0x50):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS160;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS64;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS40;
			else
				frame_props->bitrate = KBPS80;
			break;
		case(0x60):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS192;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS80;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS48;
			else
				frame_props->bitrate = KBPS96;
			break;
		case(0x70):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS224;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS96;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS56;
			else
				frame_props->bitrate = KBPS112;
			break;
		case(0x80):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS256;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS112;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& (frame_props->layer_discription == LAYER_II 
					|| frame_props->layer_discription == LAYER_III))
				frame_props->bitrate = KBPS64;
			else
				frame_props->bitrate = KBPS128;
			break;
		case(0x90):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS288;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS160;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS128;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS144;
			else
				frame_props->bitrate = KBPS80;
			break;
		case(0xa0):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS320;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS192;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS160;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS160;
			else
				frame_props->bitrate = KBPS96;
			break;
		case(0xb0):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS352;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS224;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS192;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS176;
			else
				frame_props->bitrate = KBPS112;
			break;
		case(0xc0):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS384;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS256;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS224;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS192;
			else
				frame_props->bitrate = KBPS128;
			break;
		case(0xd0):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS416;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS320;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS256;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS224;
			else
				frame_props->bitrate = KBPS144;
			break;
		case(0xe0):
			if(frame_props->mpeg_id == MPEG_v1 
				&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS448;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_II)
				frame_props->bitrate = KBPS384;
			else if(frame_props->mpeg_id == MPEG_v1 
					&& frame_props->layer_discription == LAYER_III)
				frame_props->bitrate = KBPS320;
			else if(frame_props->mpeg_id == MPEG_v2 
					&& frame_props->layer_discription == LAYER_I)
				frame_props->bitrate = KBPS256;
			else
				frame_props->bitrate = KBPS160;
			break;
		case(0xf0):
			frame_props->bitrate = BAD_KBPS;
			frame_props->status = ERROR;
			break;
		default:
			frame_props->status = ERROR;
	}

	if(frame_props->layer_discription == LAYER_I)
		frame_props->length = (12 * frame_props->bitrate 
		/ frame_props->samplerate + frame_props->padding_bit) * 4;
	else if(frame_props->layer_discription == LAYER_III 
			|| frame_props->layer_discription == LAYER_II)
		frame_props->length = 144 * frame_props->bitrate 
		/ frame_props->samplerate + frame_props->padding_bit;

	switch(*(file_buffer + frame_props->location + 3) & 0xc0) {
		case 0x0:
			frame_props->channel_mode = STEREO;
			break;
		case 0x40:
			frame_props->channel_mode = JOINT_STEREO;
			break;
		case 0x80:
			frame_props->channel_mode = DUAL_CHANNEL;
			break;
		case 0xc0:
			frame_props->channel_mode = SINGLE_CHANNEL;
			break;
		default:
			frame_props->status = ERROR;
	}	

	if(frame_props->channel_mode == JOINT_STEREO) {
		switch(*(file_buffer + frame_props->location + 3) & 0x30) {
			case 0x0:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_OFF;
					frame_props->ms_stereo = MS_OFF;
				} else {
					frame_props->intensity_stereo = BANDS_4_31;
				}
				break;
			case 0x10:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_ON;
					frame_props->ms_stereo = MS_OFF;
				} else {
					frame_props->intensity_stereo = BANDS_8_31;
				}
				break;
			case 0x20:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_OFF;
					frame_props->ms_stereo = MS_ON;
				} else {
					frame_props->intensity_stereo = BANDS_12_31;
				}
				break;
			case 0x30:
				if(frame_props->layer_discription == LAYER_III) {
					frame_props->intensity_stereo = INTENSITY_ON;
					frame_props->ms_stereo = MS_ON;
				} else {
					frame_props->intensity_stereo = BANDS_16_31;
				}
				break;
			default:
				frame_props->status = ERROR;
		}
	}

	if(frame_props->protection_bit == PROTECTED_BY_CRC)
		frame_props->data = frame_props->location + 32 + 2;
	else
		frame_props->data = frame_props->location + 32;

	int info = str_search_ptrn("Info", 
							   (file_buffer + frame_props->location), 
							   frame_props->length);
	if(info >= 0) {
		frame_props->status = INFO;
		frame_props->data = info;
		read_xing(file_buffer, frame_props);
	}

	info = str_search_ptrn("Xing", 
						   (file_buffer + frame_props->location), 
						   frame_props->length);
	if(info >= 0) {
		frame_props->status = XING;
		frame_props->data = info;
		read_xing(file_buffer, frame_props);
	}
}
