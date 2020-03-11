#include <assert.h>
#include <stdlib.h>
#include "wlhangul.h"

static size_t ucschar_len(ucschar ch) {
	if (ch < 0x80) {
		return 1;
	} else if (ch < 0x800) {
		return 2;
	} else if (ch < 0x10000) {
		return 3;
	} else if (ch < 0x110000) {
		return 4;
	}
	return 0;
}

static size_t ucschar_to_char(char out[static 4], ucschar ch) {
	size_t len = ucschar_len(ch);
	switch (len) {
	case 1:
		out[0] = (char)ch;
		break;
	case 2:
		out[0] = (ch >> 6) | 0xC0;
		out[1] = (ch & 0x3F) | 0x80;
		break;
	case 3:
		out[0] = (ch >> 12) | 0xE0;
		out[1] = ((ch >> 6) & 0x3F) | 0x80;
		out[2] = (ch & 0x3F) | 0x80;
		break;
	case 4:
		out[0] = (ch >> 18) | 0xF0;
		out[1] = ((ch >> 12) & 0x3F) | 0x80;
		out[2] = ((ch >> 6) & 0x3F) | 0x80;
		out[3] = (ch & 0x3F) | 0x80;
		break;
	}
	return len;
}

char *ucsstr_to_str(const ucschar *ucsstr) {
	size_t len = 0;
	for (size_t i = 0; ucsstr[i] != 0; i++) {
		len += ucschar_len(ucsstr[i]);
	}
	char *str = malloc(len + 1);
	size_t n = 0;
	for (size_t i = 0; ucsstr[i] != 0; i++) {
		n += ucschar_to_char(&str[n], ucsstr[i]);
	}
	assert(n == len);
	str[n] = '\0';
	return str;
}
