/* mini_utf8.h
 *
 * Gunnar Zötl <gz@tset.de> 2014-2016
 * 
 * a tiny library to deal with utf8 encoded strings. Tries to fault
 * invalid unicode codepoints and invalid utf8 sequences. This is just
 * about the en- and decoding, it is not a full featured unicode library.
 * 
 * Stuff starting with _mini_utf8_* is reserved and private. Don't name your
 * identifiers like that, and don't use stuff named like that.
 * 
 * Needed #includes:
 * -----------------
 * 	-
 * 
 * Functions:
 * ----------
 * 
 *	int mini_utf8_check_encoding_f(const char* str, mini_utf8_flags flags)
 *		test all characters in a string for valid utf8 encoding. Returns
 *		0 if the string is valid utf8, 1 if it is pure ASCII, or -1, if
 *		the string is not valid utf8.
 * 
 *	int mini_utf8_check_encoding(const char* str)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	int mini_utf8_decode_f(const char **str, mini_utf8_flags flags)
 *		returns the next valid utf8 character from *str, updating *str
 *		to point behind that char. If *str does not point to a valid
 *		utf8 encoded char, -1 is returned and *str is not updated.
 * 
 *	int mini_utf8_decode(const char **str)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	int mini_utf8_encode_f(int cp, const char* str, int len, mini_utf8_flags flags)
 *		encodes the codepoint cp into an utf8 byte sequence and stores
 *		that into str, where len bytes are available. If that went without
 *		errors, the length of the encoded sequence is returned. If cp is
 *		not a valid code point, -1 is returned, for all other problems,
 *		0 is returned. If cp is 0, it is stored as a single byte 0, unless
 *		the MINI_UTF8_ENC_OVERLONG_0 flag was specified, in which case it
 *		is stored as 0xC0 0x80.
 * 
 *	int mini_utf8_encode(int cp, const char* str, int len)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	int mini_utf8_strlen_f(const char *str, mini_utf8_flags flags)
 *		returns the number of utf8 codepoints in the string str, or -1 if
 *		the string contains invalid utf8 sequences.
 * 
 *	int mini_utf8_strlen(const char *str)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	int mini_utf8_byteoffset_f(const char *str, int cpno, mini_utf8_flags flags)
 *		returns the number of bytes from the start of the string to the
 *		start of codepoint number cpno. Returns >=0 for the offset, or
 *		-1 if the string had less than cpno codepoints, or contained an
 *		invalid utf8 sequence.
 * 
 *	int mini_utf8_byteoffset(const char *str, int cpno)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	int mini_utf8_nextchar_f(const char** str, mini_utf8_flags flags)
 *		advances *str to point to the next char in a string and returns 1
 *		on success, or leaves *str unchanged and returns -1 if there is
 *		no valid utf8 encoded next char. The terminating \0 is considered
 *		valid.
 *
 *	int mini_utf8_nextchar(const char** str)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	int mini_utf8_prevchar_f(const char** str, const char* start, mini_utf8_flags flags)
 *		decreases *str to point to the beginning of the previous char
 *		(i.e. the last char starting before the current position) in a
 *		string and returns 1 on success, or leaves *str unchanged and
 *		returns -1 if there is no valid utf8 encoded char < *str or 0 if
 *		there is no valid utf8 encoded char >= start. start points to the
 *		beginning of the string, and may be NULL if you're reasonably
 *		certain that there is a char before *str. If flags contains
 *		MINI_UTF8_DEC_SURROGATES, then entire surrogate pairs are skipped.
 *
 *	int mini_utf8_prevchar(const char** str, const char* start)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 *	const char* mini_utf8_charstart_f(const char* cp, const char* start, mini_utf8_flags flags)
 *		returns a pointer to the beginning of the char that contains the
 *		byte position cp. start is the beginning of the string to examine.
 *		Returns NULL if either no start of encoding could be found beginning
 *		at or after start, or if the encoding cp points into is invalid.
 *
 *	const char* mini_utf8_charstart(const char* cp, const char* start)
 *		the same with MINI_UTF8_DEFAULT for flags
 *
 * Flags
 * -----
 *
 * these can be combined:
 *
 *	MINI_UTF8_ENC_OVERLONG_0
 *		generate overlong encoding for codepoint 0 (0xC0 0x80)
 *	MINI_UTF8_DEC_OVERLONG_0
 *		decode overlone encoding for codepoint 0
 *	MINI_UTF8_ENC_SURROGATES
 *		encode UTF16 surrogates (U+D800-U+DFFF), see below
 *	MINI_UTF8_DEC_SURROGATES
 *		decode UTF16 surrogates (U+D800-U+DFFF), see below
 *	MINI_UTF8_DEC_UNPAIRED
 *		decode unpaired surrogates. Without this flag, they're treated as errors.
 *		Needed for weird UTF-8 encoding of ancient UCS-2 data.
 *
 * these are predefined combinations:
 *
 *	MINI_UTF8_STRICT
 *		no overlong 0 or surrogates are decoded or generated
 *	MINI_UTF8_DEFAULT
 *		(MINI_UTF8_DEC_OVERLONG_0 | MINI_UTF8_DEC_SURROGATES), reasonably
 *		liberal in what we accept, but rather strict in what we generate
 *	MINI_UTF8_LIBERAL
 *		(MINI_UTF8_DEC_OVERLONG_0 | MINI_UTF8_ENC_SURROGATES | MINI_UTF8_DEC_SURROGATES)
 *	MINI_UTF8_EXTRALIBERAL
 *		all flags except for MINI_UTF8_ENC_OVERLONG_0 enabled
 *
 * UTF8-encoding
 * -------------
 *
 * bits start   end     bytes  encoding
 * 7    U+0000	 U+007F   1     0xxxxxxx
 * 11   U+0080	 U+07FF   2     110xxxxx 10xxxxxx
 * 16   U+0800	 U+FFFF   3     1110xxxx 10xxxxxx 10xxxxxx
 * 21   U+10000  U+10FFFF 4     11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * The only possible invalid range according to RFC 3629 is U+D800-U+DFFF
 * (ED A0 80 - ED BF BF) but you can choose to allow it for reading and
 * writing! However, surrogate pairs will only be written if passed as
 * cp argument to mini_utf8_encode[_f](). If read by mini_utf8_decode[_f]()
 * they will be combined to a single unicode code point. This means, when
 * decoding, a high surrogate that is not followed by a low surrogate, or
 * a low surrogate without preceding high surrogate are invalid even when
 * the MINI_UTF8_DEC_SURROGATES flag is set. Unless, that is, when the
 * MINI_UTF8_DEC_UNPAIRED flag is set. In that case, unpaired surrogates
 * will be decoded as a single codepoint in the range 0xD800-0xDFFF.
 * MINI_UTF8_DEC_UNPAIRED does not imply MINI_UTF8_DEC_SURROGATES. You
 * need to set both flags if you want that behaviour.
 *
 * The only possible valid sequence beginning with 0XC0 is C0 80 which
 * is an overlong encoding of the 0 character, to allow for \0 terminated
 * strings. This is not valid according to RFC 3629, but is used in
 * modified UTF8. You can choose to read or write those.
 *
 * Encoding all chars from the valid range 0x0 - 0x10FFFF yields these
 * patterns:
 *
 * [00-7F] 
 * [C2-DF] [80-BF] 
 * E0 [A0-BF] [80-BF] 
 * [E1-EF] [80-BF] [80-BF] 
 * F0 [90-BF] [80-BF] [80-BF] 
 * [F1-F3] [80-BF] [80-BF] [80-BF] 
 * F4 [80-8F] [80-BF] [80-BF] 
 *
 * Excluding the range 0xD800-0xDFFF yields these patterns:
 *
 * [00-7F] 
 * [C2-DF] [80-BF] 
 * E0 [A0-BF] [80-BF] 
 * [E1-EC] [80-BF] [80-BF] 
 * ED [80-9F] [80-BF] 
 * [EE-EF] [80-BF] [80-BF] 
 * F0 [90-BF] [80-BF] [80-BF] 
 * [F1-F3] [80-BF] [80-BF] [80-BF] 
 * F4 [80-8F] [80-BF] [80-BF] 
 *
 * The utf16 surrogate ranges are
 * High: 0xD800-0xDFFF = ED [A0-BF] [80-BF]
 * Low:  0xDC00-0xDFFF = ED [B0-BF] [80-BF]
 *
 * Example:
 * --------
 * 
	#include <stdio.h>
	#include <stdlib.h>
	#include "mini_utf8.h"

	int main(int argc, char **argv)
	{
		int size = 0x11FFFF;
		int l = size * 4 + 1, i = 0, ok = 1, cp = 0;
		int *ibuf = calloc(size, sizeof(int));
		char *cbuf = calloc(l, sizeof(char));
		char *str = cbuf;
		
		while (cp < size) {
			cp = cp + 1;
			int n = mini_utf8_encode(cp, str, l);
			if (n > 0) {
				l -= n;
				str += n;
				ibuf[i++] = cp;
			}
		}
		*str = 0;
		size = i;
		
		str = cbuf;
		for (i = 0; ok && (i < size); ++i) {
			cp = mini_utf8_decode((const char**)&str);
			ok = (cp == ibuf[i]);
		}

		ok = ok && (mini_utf8_strlen(cbuf) == size);

		printf("Roundtrip test %s.\n", ok ? "succeeded" : "failed");

		ok = mini_utf8_check_encoding(cbuf);

		printf("utf8 check %s.\n", ok >= 0 ? "succeeded" : "failed");

		return ok < 0;
	}
 *
 * License:
 * --------
 * 
 * Copyright (c) 2014-2015 Gunnar Zötl <gz@tset.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _mini_utf8
#define _mini_utf8

typedef enum {
	MINI_UTF8_STRICT = 0,
	MINI_UTF8_ENC_OVERLONG_0 = 1,
	MINI_UTF8_DEC_OVERLONG_0 = 2,
	MINI_UTF8_ENC_SURROGATES = 4,
	MINI_UTF8_DEC_SURROGATES = 8,
	MINI_UTF8_DEC_UNPAIRED = 16,
	MINI_UTF8_DEFAULT = 10, /* (MINI_UTF8_DEC_OVERLONG_0 | MINI_UTF8_DEC_SURROGATES) */
	MINI_UTF8_LIBERAL = 14, /* (MINI_UTF8_DEFAULT | MINI_UTF8_ENC_SURROGATES) */
	MINI_UTF8_EXTRALIBERAL = 30, /* (MINI_UTF8_LIBERAL | MINI_UTF8_DEC_UNPAIRED) */
} mini_utf8_flags;

#define _mini_utf8_in_range(c, s, e) ((s) <= (c) && (c) <= (e))

/* The patterns for the encoding check are derived from the above
 * description.
 */
static inline int mini_utf8_check_encoding_f(const char *str, mini_utf8_flags flags)
{
	const unsigned char *s = (const unsigned char*) str;
	int isu = 1;
	int isa = 1;
	
	while (*s && isu) {

		/* 1 byte */
		if (*s <= 0x7F) {
			s += 1;
			continue;	/* [0x00-0x7F], ASCII */
		}
		isa = 0;		/* if we get here, the file is not pure ASCII */

		/* 2 bytes */
		if ((flags & MINI_UTF8_DEC_OVERLONG_0) && *s == 0xC0 && s[1] == 0x80) {
			s += 2;		/* overlong \0 char */
			continue;
		}
		if (_mini_utf8_in_range(*s, 0xC2, 0xDF) && ((s[1] & 0xC0) == 0x80)) {
			s += 2;		/* [0xC2-0xDF][0x80-0xBF], excluding overlongs */
			continue;
		}

		/* 3 bytes */
		if (*s == 0xE0 && _mini_utf8_in_range(s[1], 0xA0, 0xBF) && ((s[2] & 0xC0) == 0x80)) {
			s += 3;		/* 0xE0[0xA0-0xBF][0x80-0xBF], excluding overlongs */
			continue;
		}
		if ((*s <= 0xEC || *s == 0xEE || *s == 0xEF) && ((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80)) {
			s += 3;		/* [0xE1-0xEC,0xEE,0xEF][0x80-0xBF][0x80-0xBF] */
			continue;
		}
		if (*s == 0xED && _mini_utf8_in_range(s[1], 0x80, 0x9F) && ((s[2] & 0xC0) == 0x80)) {
			s += 3;		/* 0xED[0x80-0x9F][0x80-0xBF], excluding surrogates */
			continue;
		}
		if ((flags & MINI_UTF8_DEC_SURROGATES) && *s == 0xED && _mini_utf8_in_range(s[1], 0xA0, 0xBF) && ((s[2] & 0xC0) == 0x80))
		{
			if (s[3] == 0xED && _mini_utf8_in_range(s[4], 0xB0, 0xBF) && ((s[5] & 0xC0) == 0x80)) {
				s += 6;	/* ED [A0-BF] [80-BF] ED [B0-BF] [80-BF], utf16 high followed by low surrogate */
				continue;
			} else if (flags & MINI_UTF8_DEC_UNPAIRED) {
				s += 3;
				continue;
			}
		}

		/* 4 bytes */
		if (*s == 0xF0 && _mini_utf8_in_range(s[1], 0x90, 0xBF) && ((s[2] & 0xC0) == 0x80) && ((s[3] & 0xC0) == 0x80)) {
			s += 4;		/* 0xF0[0x90-0xBF][0x80-0xBF][0x80-0xBF] */
			continue;
		}
		if (*s <= 0xF3 && ((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80) && ((s[3] & 0xC0) == 0x80)) {
			s += 4; 	/* [0xF1-0xF3][0x80-0xBF][0x80-0xBF][0x80-0xBF] */
			continue;
		}
		if (*s == 0xF4 &&  _mini_utf8_in_range(s[1], 0x80, 0x8F) && ((s[2] & 0xC0) == 0x80) && ((s[3] & 0xC0) == 0x80)) {
			s += 4;		/* 0xF4[0x80-0x8F][0x80-0xBF][0x80-0xBF] */
			continue;
		}
		
		isu = 0;
	}
	
	if (isa && isu) {
		return 1;
	}
	else if (isu) {
		return 0;
	}
	return -1;
}

static inline int mini_utf8_check_encoding(const char* str)
{
	return mini_utf8_check_encoding_f(str, MINI_UTF8_DEFAULT);
}

#undef _mini_utf8_in_range

/* validity checking derived from above patterns
 */
static inline int mini_utf8_decode_f(const char **str, mini_utf8_flags flags)
{
	const unsigned char *s = (const unsigned char*) *str;
	int ret = -1;

	if (*s <= 0x7F) {
		ret = s[0];		/* ASCII */
		*str = (char*) s+1;
		return ret;
	} else if ((flags & MINI_UTF8_DEC_OVERLONG_0) && *s == 0xC0 && s[1] == 0x80) {
		*str = (char*) s+2;
		return 0;
	} else if (*s < 0xC2) {
		return -1;
	} else if (*s<= 0xDF) {
		if ((s[1] & 0xC0) != 0x80) { return -1; }
		ret = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
		*str = (char*) s+2;
		return ret;
	} else if ((flags & MINI_UTF8_DEC_SURROGATES) && *s == 0xED && (s[1] & 0xC0) == 0x80 && s[1] > 0x9F) {
		if ((s[2] & 0xC0) != 0x80) { return -1; }
		if (s[3] == 0xED && (s[4] & 0xC0) == 0x80 && s[4] >= 0xB0 && (s[5] & 0xC0) == 0x80) {
			int hi = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
			int lo = ((s[3] & 0x0F) << 12) | ((s[4] & 0x3F) << 6) | (s[5] & 0x3F);
			ret = ((hi & 0x7FF) << 10) + (lo & 0x3FF);
			*str = (char*) s + 6;
			return ret;
		} else if (flags & MINI_UTF8_DEC_UNPAIRED) {
			ret = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
			return ret;
		}
	} else if (*s <= 0xEF) {
		if ((s[1] & 0xC0) != 0x80) return -1;
		if (*s == 0xE0 && s[1] < 0xA0) return -1;
		if (*s == 0xED && s[1] > 0x9F) return -1;
		if (((s[2] & 0xC0)) != 0x80) return -1;
		ret = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
		*str = (char*) s+3;
		return ret;
	} else if (*s <= 0xF4) {
		if ((s[1] & 0xC0) != 0x80) return -1;
		if (*s == 0xF0 && s[1] < 0x90) return -1;
		if (*s == 0xF4 && s[1] > 0x8F) return -1;
		if ((s[2] & 0xC0) != 0x80) return -1;
		if ((s[3] & 0xC0) != 0x80) return -1;
		ret = ((s[0] & 0x0F) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
		*str = (char*) s+4;
		return ret;
	}
	
	return ret;
}

static inline int mini_utf8_decode(const char **str)
{
	return mini_utf8_decode_f(str, MINI_UTF8_DEFAULT);
}

static inline int mini_utf8_encode_f(int cp, char *str, int len, mini_utf8_flags flags)
{
	unsigned char *s = (unsigned char*) str;
	if ((flags & MINI_UTF8_ENC_OVERLONG_0) && cp == 0) {
		if (len < 2) return 0;
		*s++ = 0xC0;
		*s = 0x80;
		return 2;
	} else if (cp <= 0x7F) {
		if (len < 1) return 0;
		*s = (cp & 0x7F);
		return 1;
	} else if (cp <= 0x7FF) {
		if (len < 2) return 0;
		*s++ = (cp >> 6) | 0xC0;
		*s = (cp & 0x3F) | 0x80;
		return 2;
	} else if (cp <= 0xFFFF) {
		if (!(flags & MINI_UTF8_ENC_SURROGATES) && 0xD800 <= cp && cp <= 0xDFFF) return -1;
		if (len < 3) return 0;
		*s++ = (cp >> 12) | 0xE0;
		*s++ = ((cp >> 6) & 0x3F) | 0x80;
		*s = (cp & 0x3F) | 0x80;
		return 3;
	} else if (cp <= 0x10FFFF) {
		if (len < 4) return 0;
		*s++ =(cp >> 18) | 0xF0;
		*s++ =((cp >> 12) & 0x3F) | 0x80;
		*s++ =((cp >> 6) & 0x3F) | 0x80;
		*s =(cp & 0x3F) | 0x80;
		return 4;
	}
	return -1;
}

static inline int mini_utf8_encode(int cp, char *str, int len)
{
	return mini_utf8_encode_f(cp, str, len, MINI_UTF8_DEFAULT);
}

static inline int mini_utf8_strlen_f(const char *str, mini_utf8_flags flags)
{
	const char *s = str;
	int len = 0;
	int ok = mini_utf8_decode_f(&s, flags);
	while (ok > 0) {
		++len;
		ok = mini_utf8_decode_f(&s, flags);
	}
	if (ok == 0) {
		return len;
	}
	return -1;
}

static inline int mini_utf8_strlen(const char *str)
{
	return mini_utf8_strlen_f(str, MINI_UTF8_DEFAULT);
}

static inline int mini_utf8_byteoffset_f(const char *str, int cpno, mini_utf8_flags flags)
{
	const char *s = str;
	int cnt = 0;
	int ok = 1;
	for (cnt = 0; (cnt < cpno) && (ok > 0); ++cnt) {
		ok = mini_utf8_decode_f(&s, flags);
	}
	if (ok > 0) {
		return (int)(s - str);
	}
	return -1;
}

static inline int mini_utf8_byteoffset(const char* str, int cpno)
{
	return mini_utf8_byteoffset_f(str, cpno, MINI_UTF8_DEFAULT);
}

static inline int mini_utf8_nextchar_f(const char** str, mini_utf8_flags flags)
{
	return mini_utf8_decode_f(str, flags) >= 0;
}

static inline int mini_utf8_nextchar(const char** str)
{
	return mini_utf8_nextchar_f(str, MINI_UTF8_DEFAULT);
}

static inline int mini_utf8_prevchar_f(const char** str, const char* start, mini_utf8_flags flags)
{
	const unsigned char* s1 = (const unsigned char*) *str - 1;
	const char* ss1;
	int again = 0;
	if ((const char*) s1 < start) {
		return 0;
	}
	if (*s1 <= 0x7F) {
		*str = (const char*) s1;
		return 1;
	}
	while ((const char*) s1 > start && *s1 >= 0x80 && *s1 < 0xC0) {
		--s1;
		if (*s1 == 0xED && (flags & MINI_UTF8_DEC_SURROGATES) && !again && (s1[1] & 0xC0) == 0x80 && s1[1] >= 0xB0 && (s1[2] & 0xC0) == 0x80 &&
			(const char*)s1 - 3 >= start && s1[-3] == 0xED && (s1[-2] & 0xC0) == 0x80 && s1[-2] >= 0xA0 && (s1[-1] & 0xC0) == 0x80) {
			--s1;
			again = 1;
		}
	}
	if (!(flags & MINI_UTF8_DEC_SURROGATES) && *str - (const char*)s1 > 4) {
		return -1;
	}
	if ((flags & MINI_UTF8_DEC_SURROGATES) && *str - (const char*)s1 > 6) {
		return -1;
	}
	ss1 = (const char*) s1;
	int ret = mini_utf8_decode_f(&ss1, flags);
	if (ret >= 0) {
		*str = (const char*) s1;
	}
	return ret;
}

static inline int mini_utf8_prevchar(const char** str, const char* start)
{
	return mini_utf8_prevchar_f(str, start, MINI_UTF8_DEFAULT);
}

static inline const char* mini_utf8_charstart_f(const char* cp, const char* start, mini_utf8_flags flags)
{
	const char *str = cp + 1;
	if (mini_utf8_prevchar_f(&str, start, flags) > 0) {
		return str;
	}
	return 0;
}

static inline const char* mini_utf8_charstart(const char* cp, const char* start)
{
	return mini_utf8_charstart_f(cp, start, MINI_UTF8_DEFAULT);
}

#endif /* _mini_utf8 */
