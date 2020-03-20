/*
 * MAngband Data packaging code
 *
 * Copyright (c) 2010 MAngband Project Team.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of the "Angband licence" with an extra clause:
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply. 
 * Clause: You are not allowed to use this code in software which is not
 * distributed in source form.
 *
 */
#include "angband.h"

#define PACK_PTR_8(PT, VAL) \
 * PT ++ = VAL
#define PACK_PTR_16(PT, VAL) \
 * PT ++ = (char)(VAL >> 8), \
 * PT ++ = (char)VAL
#define PACK_PTR_32(PT, VAL) \
 * PT ++ = (char)(VAL >> 24),\
 * PT ++ = (char)(VAL >> 16),\
 * PT ++ = (char)(VAL >> 8), \
 * PT ++ = (char)VAL
#define PACK_PTR_64(PT, VAL) \
 * PT ++ = (char)(VAL >> 56),\
 * PT ++ = (char)(VAL >> 48),\
 * PT ++ = (char)(VAL >> 40),\
 * PT ++ = (char)(VAL >> 32),\
 * PT ++ = (char)(VAL >> 24),\
 * PT ++ = (char)(VAL >> 16),\
 * PT ++ = (char)(VAL >> 8), \
 * PT ++ = (char)VAL
#define PACK_PTR_STR(PT, VAL) while ((* PT ++ = * VAL ++) != '\0')
#define PACK_PTR_NSTR(PT, VAL, SIZE) while (SIZE--) { * PT ++ = * VAL ++ ; }

#define UNPACK_PTR_8(PT, VAL) \
 * PT = * VAL ++
#define UNPACK_PTR_16(PT, VAL)   \
 * PT  = (* VAL ++ & 0xFF) << 8, \
 * PT |= (* VAL ++ & 0xFF)
#define UNPACK_PTR_32(PT, VAL)   \
 * PT  = (* VAL ++ & 0xFF) << 24,\
 * PT |= (* VAL ++ & 0xFF) << 16,\
 * PT |= (* VAL ++ & 0xFF) << 8, \
 * PT |= (* VAL ++ & 0xFF)
#define UNPACK_PTR_64(PT, VAL)     \
 * PT  = (u64b)(* VAL ++ & 0xFFUL) << 56,\
 * PT |= (u64b)(* VAL ++ & 0xFFUL) << 48,\
 * PT |= (u64b)(* VAL ++ & 0xFFUL) << 40,\
 * PT |= (u64b)(* VAL ++ & 0xFFUL) << 32,\
 * PT |= (u64b)(* VAL ++ & 0xFFUL) << 24,\
 * PT |= (u64b)(* VAL ++ & 0xFFUL) << 16,\
 * PT |= (u64b)(* VAL ++ & 0xFFUL) << 8, \
 * PT |= (u64b)(* VAL ++ & 0xFFUL)

#define SOFTER_ERRORS //undefine this for better debug

static const cptr pf_errors[] = {
"", /* 0 */
"Unrecognized format", /* 1 */
"No space in buffer", /* 2 */
"No space in read buffer", /* 3 */
"No space in write buffer", /* 4 */
"Unterminated string", /* 5 */
"String too large for format", /* 6 */
"", /* 7 */
"Cave contains attrs unsuitable for this RLE method", /* 8 */
"Cave contains length that is larger than possible width", /* 9 */
"",
};
#define MAX_CQ_ERRORS 10

/*
 * The macros below WILL define, initialize AND use the following variables.
 * It is imperative you don't interfere.
 */
#define WPTRN wptr
#define WSTRN wstart
#define WENDN wend
#define RPTRN rptr
#define RSTRN rstart
#define RENDN rend

#define PACK_8(VAL) PACK_PTR_8(WPTRN, VAL)
#define PACK_16(VAL) PACK_PTR_16(WPTRN, VAL)
#define PACK_32(VAL) PACK_PTR_32(WPTRN, VAL)
#define PACK_64(VAL) PACK_PTR_64(WPTRN, VAL)
#define PACK_STR(VAL) PACK_PTR_STR(WPTRN, VAL)
#define PACK_NSTR(VAL, SIZE) PACK_PTR_NSTR(WPTRN, VAL, SIZE)

#define PACK_DEF	char * WSTRN, * WPTRN, * WENDN;
#define PACK_INIT(CQ)	WSTRN = WPTRN = &(CQ)->buf[(CQ)->len]; \
						WENDN = &(CQ)->buf[(CQ)->max]
#define PACK_FIN(CQ)	(CQ)->len += (WPTRN - WSTRN)
#define PACK_FIN_R(CQ, R) (R) = (WPTRN - WSTRN); \
						(CQ)->len += (R)

#define UNPACK_8(PT) UNPACK_PTR_8(PT, RPTRN)
#define UNPACK_16(PT) UNPACK_PTR_16(PT, RPTRN)
#define UNPACK_32(PT) UNPACK_PTR_32(PT, RPTRN)
#define UNPACK_64(PT) UNPACK_PTR_64(PT, RPTRN)
#define UNPACK_STR(PT) PACK_PTR_STR(PT, RPTRN)
#define UNPACK_NSTR(PT, SIZE) PACK_PTR_NSTR(PT, RPTRN, SIZE)

#define UNPACK_DEF	char * RSTRN, * RPTRN, * RENDN;
#define UNPACK_INIT(CQ)	RSTRN = RPTRN = &(CQ)->buf[(CQ)->pos]; \
						RENDN = &(CQ)->buf[(CQ)->len]
#define UNPACK_FIN(CQ)	(CQ)->pos += (RPTRN - RSTRN)

#define REPACK_DEF          	UNPACK_DEF		PACK_DEF
#define REPACK_INIT(SRC,DST)	UNPACK_INIT(SRC); PACK_INIT(DST)
#define REPACK_FIN(SRC,DST) 	UNPACK_FIN(SRC); PACK_FIN(DST)

#define REPACK_8 * WPTRN ++ = * RPTRN ++;
#define REPACK_16 REPACK_8; REPACK_8
#define REPACK_32 REPACK_16; REPACK_16
#define REPACK_64 REPACK_32; REPACK_32
#define REPACK_STR PACK_PTR_STR(WPTRN, RPTRN)
#define REPACK_NSTR(SIZE) PACK_PTR_NSTR(WPTRN, RPTRN, SIZE)


int cq_printf(cq *charq, char *str, ...) {
	int error = 0, bytes = 0, str_size = 0;
	va_list marker;

	signed char s8b;
	unsigned char u8b;
	u16b _u16b;
	s16b _s16b;
	u32b _u32b;
	s32b _s32b;
	u64b _u64b;
	s64b _s64b;
	char *text;

	PACK_DEF

	va_start( marker, str );

	PACK_INIT( charq );

#define PF_ERROR_SIZE(SIZE) if (WPTRN + SIZE > WENDN) { error = 2; break; }
#define PF_ERROR_FRMT default: { error = 1; break; }

	while (!error && *str++ == '%') {
		switch(*str++) {
			case 'b': {/* hack, same as '%uc' */
				PF_ERROR_SIZE(1)
				u8b = (unsigned char) va_arg (marker, unsigned int);
				PACK_8(u8b);
				break;}
			case 'c': {
				PF_ERROR_SIZE(1)
				s8b = (signed char) va_arg (marker, signed int);
				PACK_8(s8b);
				break;}
			case 'd': {
				PF_ERROR_SIZE(2)
				_s16b = (s16b) va_arg (marker, signed int);
				PACK_16(_s16b);
				break;}
			case 'l': {
				PF_ERROR_SIZE(4)
				_s32b = (s32b) va_arg (marker, s32b);
				PACK_32(_s32b);
				break;}
			case 'u': { /* unsigned */
				switch (*str++) {
				case 'c': {
					PF_ERROR_SIZE(1)
					u8b = (unsigned char) va_arg (marker, unsigned int);
					PACK_8(u8b);
					break;}
				case 'd': {
					PF_ERROR_SIZE(2)
					_u16b = (u16b) va_arg (marker, unsigned int);
					PACK_16(_u16b);
					break;}
				case 'l': {
					PF_ERROR_SIZE(4)
					_u32b = va_arg (marker, u32b);
					PACK_32(_u32b);
					break;}
				case 'v': { /* variable-length, websocket style */
					_u64b = (u64b) va_arg (marker, u64b);
					if (_u64b > 0xFFFF) {
						PF_ERROR_SIZE(1 + 8)
						u8b = 127;
						PACK_8(u8b);
						PACK_64(_u64b);
					} else if (_u64b >= 126) {
						PF_ERROR_SIZE(1 + 2)
						u8b = 126;
						_u16b = (u16b) _u64b;
						PACK_8(u8b);
						PACK_16(_u16b);
					} else { /* 0 - 125 */
						PF_ERROR_SIZE(1)
						u8b = (unsigned char) _u64b;
						PACK_8(u8b);
					}
					break;}
				PF_ERROR_FRMT
				}
				break;}
			case 'n': {
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text);
				if (str_size > MAX_CHARS - 1)
				{
					plog_fmt("Truncating string '%s', size=%d exceeds MAX_CHARS=%d",text,str_size,MAX_CHARS);
				#ifndef SOFTER_ERRORS
					error = 6;
					break;
				#endif
					str_size = MAX_CHARS - 1;
				}
				PF_ERROR_SIZE(str_size+1);
				PACK_8(str_size);
				PACK_NSTR(text, str_size);
				break;}
			case 'N': {
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text);
				if (str_size > MSG_LEN - 1)
				{
					plog_fmt("Truncating string '%s', size=%d exceeds MSG_LEN=%d",text,str_size,MSG_LEN);
				#ifndef SOFTER_ERRORS
					error = 6;
					break;
				#endif
					str_size = MSG_LEN - 1;
				}
				PF_ERROR_SIZE(str_size+2);
				PACK_16(str_size);
				PACK_NSTR(text, str_size);
				break;}
			case 's': {
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text)+1;
				if (str_size > MAX_CHARS)
				{
					plog_fmt("Truncating string '%s', size=%d exceeds MAX_CHARS=%d",text,str_size,MAX_CHARS);
				#ifndef SOFTER_ERRORS
					error = 6;
					break;
				#endif
					str_size = MAX_CHARS;
				}
				PF_ERROR_SIZE(str_size);
				str_size--;
				PACK_NSTR(text, str_size);
				PACK_8('\0');
				break;}
			case 'S': {
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text)+1;
				if (str_size > MSG_LEN)
				{
					plog_fmt("Truncating string '%s', size=%d exceeds MSG_LEN=%d",text,str_size,MSG_LEN);
				#ifndef SOFTER_ERRORS
					error = 6;
					break;
				#endif
					str_size = MSG_LEN;
				}
				PF_ERROR_SIZE(str_size);
				str_size--;
				PACK_NSTR(text, str_size);
				PACK_8('\0');
				break;}
			case 'T': { /* HACK - unterminated string. NOT equivalent to cq_scanf '%T' !  */
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text)+1;
				if (str_size > MSG_LEN)
				{
					plog_fmt("Truncating string '%s', size=%d exceeds MSG_LEN=%d",text,str_size,MSG_LEN);
				#ifndef SOFTER_ERRORS
					error = 6;
					break;
				#endif
					str_size = MSG_LEN;
				}
				str_size--;
				PF_ERROR_SIZE(str_size);
				PACK_NSTR(text, str_size);
				break;}
			PF_ERROR_FRMT
		}
	}

	charq->err = error;

	if (error) {
		plog_fmt("Error in cq_printf('...%s'): %s [%d.%d]", str, pf_errors[error], str_size, charq->len);
		bytes = 0;
	} else {
		PACK_FIN_R(charq, bytes);
	}

	va_end( marker );
	return bytes;
}

int cq_scanf(cq *charq, char *str, ...) {
	int error = 0, found = 0, str_size = 0;
	va_list marker;

	signed char *s8b;
	unsigned char *u8b;
	u16b *_u16b;
	s16b *_s16b;
	u32b *_u32b;
	s32b *_s32b;
	u64b *_u64b;
	s64b *_s64b;
	char *_text;

	UNPACK_DEF

	va_start( marker, str );

	UNPACK_INIT(charq);

#define SF_BYTES_LEFT (RENDN - RPTRN)
#define SF_ERROR_SIZE_STR(SIZE, WIDE) if (RPTRN + (SIZE) > RENDN) { error = 2; RPTRN -= WIDE; break; }
#define SF_ERROR_SIZE(SIZE) if (RPTRN + SIZE > RENDN) { error = 2; break; }
#define SF_ERROR_STRSIZE(STRSIZE,MAX) if (STRSIZE > MAX) { error = 5; break; }
#define SF_ERROR_NSTRSIZE(STRSIZE,MAX) if (STRSIZE > MAX) { error = 6; break; }
#define SF_ERROR_FRMT default: { error = 1; break; }

	while (!error && *str++ == '%') {
		found++;
		switch(*str++) {
			case 'b': /* hack, same as '%uc' */
				SF_ERROR_SIZE(1)
				u8b = (unsigned char*) va_arg (marker, unsigned int*);
				UNPACK_8(u8b);
			break;
			case 'c': {
				SF_ERROR_SIZE(1)
				s8b = (signed char*) va_arg (marker, signed int*);
				UNPACK_8(s8b);
				break;}
			case 'd': {
				SF_ERROR_SIZE(2)
				_s16b = (s16b*) va_arg (marker, signed int*);
				UNPACK_16(_s16b);
				break;}
			case 'l': {
				SF_ERROR_SIZE(4)
				_s32b = (s32b*) va_arg (marker, s32b*);
				UNPACK_32(_s32b);
				break;}
			case 'u': {/* unsigned */
				switch (*str++) {
					case 'c': {
						SF_ERROR_SIZE(1)
						u8b = (unsigned char*) va_arg (marker, unsigned int*);
						UNPACK_8(u8b);
						break;}
					case 'd': {
						SF_ERROR_SIZE(2)
						_u16b = (u16b*) va_arg (marker, unsigned int*);
						UNPACK_PTR_16(_u16b, rptr);
						break;}
					case 'l': {
						SF_ERROR_SIZE(4)
						_u32b = (u32b*) va_arg (marker, u32b*);
						UNPACK_32(_u32b);
						break;}
					case 'v': { /* variable-length, websocket style */
						byte len8;
						SF_ERROR_SIZE(1);
						_u64b = (u64b*) va_arg (marker, u64b*);
						UNPACK_8(&len8);
						len8 &= 0x7F; /* clear out mask bit */
						if (len8 <= 125) { /* first 8 bits contained len */
							*_u64b = len8;
						} else if (len8 == 126) { /* next 16 bits are len */
							u16b len16 = 0;
							SF_ERROR_SIZE(2);
							UNPACK_16(&len16);
							*_u64b = len16;
						} else if (len8 >= 127) { /* next 64 bits are len */
							SF_ERROR_SIZE(8);
							UNPACK_64(_u64b);
						}
						break;}
					SF_ERROR_FRMT
				}
				break;}
			case 'n': {
				SF_ERROR_SIZE(1)
				_text = (char*) va_arg (marker, char*);
				UNPACK_8(&str_size);
				SF_ERROR_NSTRSIZE(str_size, MAX_CHARS - 1)
				SF_ERROR_SIZE_STR(str_size, 1)
				UNPACK_NSTR(_text, str_size);
				*_text = 0;
				break;}
			case 'N': {
				SF_ERROR_SIZE(2)
				_text = (char*) va_arg (marker, char*);
				UNPACK_16(&str_size);
				SF_ERROR_NSTRSIZE(str_size, MSG_LEN - 1)
				SF_ERROR_SIZE_STR(str_size, 2)
				UNPACK_NSTR(_text, str_size);
				*_text = 0;
				break;}
			case 's': {
				_text = (char*) va_arg (marker, char*);
				//unsigned char str_size;
				SF_ERROR_SIZE(1);//at least 1 more byte should be present
				str_size = strnlen(rptr, MIN(SF_BYTES_LEFT, MAX_CHARS)) + 1;
				SF_ERROR_STRSIZE(str_size, MAX_CHARS)
				SF_ERROR_SIZE(str_size)
				UNPACK_NSTR(_text, str_size);
				break;}
			case 'S': {
				_text = (char*) va_arg (marker, char*);
				//unsigned char str_size;
				SF_ERROR_SIZE(1);//at least 1 more byte should be present
				str_size = strnlen(rptr, MIN(SF_BYTES_LEFT, MSG_LEN)) + 1;
				SF_ERROR_STRSIZE(str_size, MSG_LEN)
				SF_ERROR_SIZE(str_size)
				UNPACK_NSTR(_text, str_size);
				break;}
			case 'T': {/* HACK! \n-terminated string (\r==\n here)*/
				int seen_char = 0, seen_r = 0;
				_text = (char*) va_arg (marker, char*);
				str_size = 0;
				while(str_size++ < MSG_LEN) {
				 SF_ERROR_SIZE(1);
				 if (*rptr == '\r') { seen_r = 1; rptr++; continue; }
				 if (*rptr == '\n') { seen_r = 1; rptr++;
				  if (seen_char) break; else continue; }
				 else { seen_char = 1; }
				 *_text++ = *rptr++;
				}
				if (error == 2) error = 0;
				if (seen_r && !seen_char) *_text++ = '\n';
				 *_text = '\0';
//				if (str_size > 1 && *(_text-1) == '\r') /* Chomp */
//				 *(_text-1) = '\0';
				break;}
			SF_ERROR_FRMT
		}
	}

	charq->err = error;

	if (error) {
		found = 0;
		if (!(error >= 2 && error <= 3)) {/* Hack - do not report "not enough buffer" errors. */
		plog(format("Error in cq_scanf('...%s'): %s [%d]", str, pf_errors[error], str_size));
		}
	} else {
		UNPACK_FIN(charq);
	}

	va_end( marker );
	return found;
}

/* Hack -- a function to apply cq_scanf into cq_printf. 
 * Must be similar to the functions above */
int cq_copyf(cq *src, const char *str, cq *dst) {
	int error = 0, found = 0, str_size = 0;

	REPACK_DEF
	REPACK_INIT(src, dst);

#define CF_ERROR_SIZE(SIZE) \
	if (RPTRN + SIZE > RENDN) { error = 3; break; } \
	if (WPTRN + SIZE > WENDN) { error = 4; break; }
#define CF_ERROR_FRMT default: { error = 1; break; }

	while (!error && *str++ == '%') {
		found++;
		switch(*str++) {
			case 'b': /* hack, same as '%uc' */
				CF_ERROR_SIZE(1)
				REPACK_8
			break;
			case 'c': {
				CF_ERROR_SIZE(1)
				REPACK_8
				break;}
			case 'd': {
				CF_ERROR_SIZE(2)
				REPACK_16
				break;}
			case 'l': {
				CF_ERROR_SIZE(4)
				REPACK_32
				break;}
			case 'u': {/* unsigned */
				switch (*str++) {
					case 'c': {
						CF_ERROR_SIZE(1)
						REPACK_8
						break;}
					case 'd': {
						CF_ERROR_SIZE(2)
						REPACK_16
						break;}
					case 'l': {
						CF_ERROR_SIZE(4)
						REPACK_32
						break;}
					case 'v': {
						byte len8;
						CF_ERROR_SIZE(1)
						UNPACK_8(&len8);
						PACK_8(len8);
						if (len8 <= 125) {
							/* Do nothing */
						} else if (len8 == 126) {
							CF_ERROR_SIZE(2);
							REPACK_16
						} else if (len8 >= 127) {
							CF_ERROR_SIZE(8);
							REPACK_64
						}
						break;}
					CF_ERROR_FRMT
				}
				break;}
			case 'n': {
				CF_ERROR_SIZE(1)
				UNPACK_8(&str_size);
				CF_ERROR_SIZE(str_size)
				PACK_8(str_size);
				REPACK_NSTR(str_size);
				break;}
			case 'N': {
				CF_ERROR_SIZE(2)
				UNPACK_16(&str_size);
				CF_ERROR_SIZE(str_size)
				PACK_16(str_size);
				REPACK_NSTR(str_size);
				break;}
			case 's': {
				CF_ERROR_SIZE(MAX_CHARS)
				REPACK_STR
				break;}
			case 'S': {
				CF_ERROR_SIZE(MSG_LEN)
				REPACK_STR
				break;}
			CF_ERROR_FRMT
		}
	}

#undef CF_ERROR_SIZE
#undef CF_ERROR_FRMT

	src->err = error;
	dst->err = error;

	if (error) {
		found = -1;
		plog(format("Error in cq_copyf('...%s'): %s [%d.%d.%d]", str, pf_errors[error], str_size, src->len, dst->max));
	} else {
		REPACK_FIN(src, dst);
	}

	return found;
}

#define PW_ERROR_SIZE(SIZE) if (WPTRN + SIZE > WENDN) { dst->err = 2; return 0; }
#define PR_ERROR_SIZE(SIZE) if (RPTRN + SIZE > RENDN) { src->err = 2; return 0; }

int cv_encode_none(cave_view_type* src, cq* dst, int len) {
	int i, bytes = 0;
	PACK_DEF
	PACK_INIT(dst);
	PW_ERROR_SIZE(len * 2)
	/* Each column */
	for (i = 0; i < len; i++)
	{
		/* Obtain the char/attr pair */
		char c = (src[i]).c;
		byte a = (src[i]).a;

		/* Write it */
		PACK_PTR_8(wptr, c);
		PACK_PTR_8(wptr, a);
	}
	PACK_FIN_R(dst, bytes);
	return bytes;
}
int cv_decode_none(cave_view_type* dst, cq* src, int len) {
	int	x;
	UNPACK_DEF
	UNPACK_INIT(src);
	PR_ERROR_SIZE(len * 2)	
	for (x = 0; x < len; x++)
	{
		char c;
		byte a;

		/* Read the char/attr pair */
		UNPACK_PTR_8(&c, rptr);
		UNPACK_PTR_8(&a, rptr);

		/* 'Draw' a character n times */
		if (dst)
		{
			/* Memorize */
			dst[x].a = a;
			dst[x].c = c;
		}
	}
	UNPACK_FIN(src);
	return len;
}

int cv_encode_rle1(cave_view_type* src, cq* dst, int len) {
	int i, bytes = 0;
	PACK_DEF
	PACK_INIT(dst);
	/* Each column */
	for (i = 0; i < len; i++)
	{
		int x, n;
		char c;
		byte a;

		/* Obtain the char/attr pair */
		c = (src[i]).c;
		a = (src[i]).a;

		/* Error-check! */
		if (a & 0x40)
		{
			dst->err = 8;
			return 0;
		}

		/* Start with count of 1 */
		n = 1;

		/* Start looking here */
		for (x = i + 1; x < len; x++)
		{
			/* Count repetitions of this grid */
			if (src[x].c != c || src[x].a != a) break;
			/* Increment count */
			n++;
		}

		/* If there are at least 2 similar grids in a row */
		if (n >= 2)
		{
			/* Output the info */
			PW_ERROR_SIZE(3)
			PACK_PTR_8(wptr, c);
			PACK_PTR_8(wptr, (a | 0x40)); /* Set bit 0x40 of a */
			PACK_PTR_8(wptr, (byte)n);

			/* Start again after the run */
			i = x - 1;
		}
		else
		{
			/* Normal, single grid */
			PW_ERROR_SIZE(2)
			PACK_PTR_8(wptr, c);
			PACK_PTR_8(wptr, a);
		}
	}
	PACK_FIN_R(dst, bytes);
	return bytes;
}
int cv_decode_rle1(cave_view_type* dst, cq* src, int len) {
	int	x;
	UNPACK_DEF
	UNPACK_INIT(src);
	for (x = 0; x < len; x++)
	{
		byte n;
		char c;
		byte a;

		/* Read the char/attr pair */
		PR_ERROR_SIZE(2)
		UNPACK_PTR_8(&c, rptr);
		UNPACK_PTR_8(&a, rptr);

		/* Start with count of 1 */
		n = 1;

		/* Check for bit 0x40 on the attribute */
		if (a & 0x40)
		{
			/* First, clear the bit */
			a &= ~(0x40);

			/* Read the number of repetitions */
			PR_ERROR_SIZE(1)
			UNPACK_PTR_8(&n, rptr);

			/* Is it even legal? */
			if (x + n > len)
			{
				src->err = 9;
				return 0;
			}
		}

		/* 'Draw' a character n times */
		if (dst)
		{
			int i;
			for (i = 0; i < n; i++)
			{
				/* Memorize */
				dst[x + i].a = a;
				dst[x + i].c = c;
			}
		}

		/* Reset 'x' to the correct value */
		x += n - 1;
	}
	UNPACK_FIN(src);
	return len;
}

int cv_encode_rle2(cave_view_type* src, cq* dst, int len) {
	int i, bytes = 0;
	PACK_DEF
	PACK_INIT(dst);
	/* Each column */
	for (i = 0; i < len; i++)
	{
		int x, n;
		char c;
		byte a;

		/* Obtain the char/attr pair */
		c = (src[i]).c;
		a = (src[i]).a;

		/* Error-check! */
		if (a == 0xFF)
		{
			dst->err = 8;
			return 0;
		}
		/* Start with count of 1 */
		n = 1;

		/* Start looking here */
		for (x = i + 1; x < len; x++)
		{
			/* Count repetitions of this grid */
			if (src[x].c != c || src[x].a != a) break;
			/* Increment count */
			n++;
		}

		/* If there are at least 2 similar grids in a row */
		if (n >= 2)
		{
			/* Output the info */
			PW_ERROR_SIZE(4)
			PACK_PTR_8(wptr, (byte)n); /* Number of repetitons */
			PACK_PTR_8(wptr, 0xFF); /* 0xFF marks the spot! */
			PACK_PTR_8(wptr, c);
			PACK_PTR_8(wptr, a);

			/* Start again after the run */
			i = x - 1;
		}
		else
		{
			/* Normal, single grid */
			PW_ERROR_SIZE(2)
			PACK_PTR_8(wptr, c);
			PACK_PTR_8(wptr, a);
		}
	}
	PACK_FIN_R(dst, bytes);
	return bytes;
}
int cv_decode_rle2(cave_view_type* dst, cq* src, int len) {
	int	x;
	UNPACK_DEF
	UNPACK_INIT(src);
	for (x = 0; x < len; x++)
	{
		int  n;
		char c;
		byte a;

		/* Read the char/attr pair */
		PR_ERROR_SIZE(2)
		UNPACK_PTR_8(&c, rptr);
		UNPACK_PTR_8(&a, rptr);

		/* Start with count of 1 */
		n = 1;
		/* Check for bit 0xFF on the attribute */
		if (a == 0xFF)
		{
			/* Get the number of repetitions */
			n = (byte)c;

			/* Is it even legal? */
			if (x + n > len)
			{
				src->err = 9;
				return 0;
			}

			/* Read the attr/char pair */
			PR_ERROR_SIZE(2)
			UNPACK_PTR_8(&c, rptr);
			UNPACK_PTR_8(&a, rptr);
		}

		/* 'Draw' a character n times */
		if (dst)
		{
			int i;
			for (i = 0; i < n; i++)
			{
				/* Memorize */
				dst[x + i].a = a;
				dst[x + i].c = c;
			}
		}

		/* Reset 'x' to the correct value */
		x += n - 1;
	}
	UNPACK_FIN(src);
	return len;
}

int cv_encode_rle3(cave_view_type* src, cq* dst, int len) {
	int i, bytes = 0;
	PACK_DEF
	PACK_INIT(dst);
	/* Each column */
	for (i = 0; i < len; i++)
	{
		int x, n;
		byte a;

		/* Obtain the attr */
		a = (src[i]).a;

		/* Error-check! */
		if (a & 0x40)
		{
			dst->err = 8;
			return 0;
		}

		/* Start with count of 1 */
		n = 1;

		/* Start looking here */
		for (x = i + 1; x < len; x++)
		{
			/* Count repetitions of this color */
			if (src[x].a != a) break;
			/* Increment count */
			n++;
		}

		/* If there are at least 3 similar grids in a row */
		if (n >= 3)
		{
			/* Output the info */
			PW_ERROR_SIZE(2 + n)
			PACK_PTR_8(wptr, (a | 0x40)); /* Set bit 0x40 of a */
			PACK_PTR_8(wptr, (byte)n);
			/* Output the chars */
			while (n--) PACK_PTR_8(wptr, (src[i++]).c);
			/* Start again after the run */
			i--;
		}
		else
		{
			/* Normal, single grid */
			PW_ERROR_SIZE(2)
			PACK_PTR_8(wptr, a);
			PACK_PTR_8(wptr, (src[i]).c);
		}
	}
	PACK_FIN_R(dst, bytes);
	return bytes;
}
int cv_decode_rle3(cave_view_type* dst, cq* src, int len) {
	int	x;
	UNPACK_DEF
	UNPACK_INIT(src);
	for (x = 0; x < len; x++)
	{
		byte  i, n;
		byte a;

		/* Read the attr */
		PR_ERROR_SIZE(1)
		UNPACK_PTR_8(&a, rptr);

		/* Start with count of 1 */
		n = 1;

		/* Check for bit 0x40 on the attribute */
		if (a & 0x40)
		{
			/* First, clear the bit */
			a &= ~(0x40);

			/* Read the number of repetitions */
			PR_ERROR_SIZE(1)
			UNPACK_PTR_8(&n, rptr);

			/* Is it even legal? */
			if (x + n > len)
			{
				src->err = 9;
				return 0;
			}
		}

		/* 'Draw' a character n times */
		PR_ERROR_SIZE(n)
		for (i = 0; i < n; i++)
		{
			char c;
			UNPACK_PTR_8(&c, rptr);

			if (dst)
			{
				/* Memorize */
				dst[x + i].a = a;
				dst[x + i].c = c;
			}
		}
		/* Reset 'x' to the correct value */
		x += n - 1;
	}
	UNPACK_FIN(src);
	return len;
}

#define MAX_CAVE_CODECS	4
#define CV_ENCODE 0
#define CV_DECODE 1
typedef int (*cvcb)	(cave_view_type* cv, cq* src, int len); /* "Cave Encoder/Decoder Call-Back" */
cvcb cave_codecs[MAX_CAVE_CODECS][2] = {
	/* RLE_NONE */
	{ cv_encode_none, cv_decode_none },
	/* RLE_CLASSIC */
	{ cv_encode_rle1, cv_decode_rle1 },
	/* RLE_LARGE */
	{ cv_encode_rle2, cv_decode_rle2 },
	/* RLE_COLOR */
	{ cv_encode_rle3, cv_decode_rle3 }
};

int cq_printc(cq *charq, unsigned int mode, cave_view_type *from, int len) {
	int n = 0;
	if (mode < MAX_CAVE_CODECS) 
	{
		n = (cave_codecs[mode][CV_ENCODE]) (from, charq, len);
	}
	return n;
}

int cq_scanc(cq *charq, unsigned int mode, cave_view_type *to, int len) {
	int n = 0;
	if (mode < MAX_CAVE_CODECS) 
	{
		n = (cave_codecs[mode][CV_DECODE]) (to, charq, len);
	}
	return n;
}

/* Sometimes, cave view is stored in a pair of attr/char arrays. 
 * For such cases, we copy them to temporary cave_view_type buffers
 * and call the regular function on them. This is much less optiomal
 * but also much rarer.
 */
int cq_printac(cq *charq, unsigned int mode, byte *a, char *c, int len) {
	cave_view_type buf[PD_SMALL_BUFFER]; 
	int i, n = 0;
	if (len < PD_SMALL_BUFFER)
	{
		if (mode < MAX_CAVE_CODECS)
		{
			for (i = 0; i < len; i++)
			{
				buf[i].a = a[i];
				buf[i].c = c[i];
			}
			n = (cave_codecs[mode][CV_ENCODE]) (&buf[0], charq, len);
		}
	}
	return n;
}

/* Note: pass "NULL" as "a" to discard the result */
int cq_scanac(cq *charq, unsigned int mode, byte *a, char *c, int len) {
	cave_view_type buf[PD_SMALL_BUFFER];
	int i, n = 0;
	if (len < PD_SMALL_BUFFER)
	{
		if (mode < MAX_CAVE_CODECS)
		{
			if ((n = (cave_codecs[mode][CV_DECODE]) (&buf[0], charq, len)) == len)
			{
				if (a != NULL)
				{
					for (i = 0; i < len; i++)
					{
						a[i] = buf[i].a;
						c[i] = buf[i].c;
					}
				}
			}
		}
	}
	return n;
}

const char* cq_error(cq *charq) {
	if (charq->err == 0) return "";
	if (charq->err < MAX_CQ_ERRORS) {
		return pf_errors[charq->err];
	}
	return "";
}

/* Returns true if there was a fatal error in charq.
 * Note: we consider errors 2, 3 and 4 non-fatal. */
bool cq_fatal(cq *charq) {
	if (charq->err == 0) return FALSE;
	if (charq->err >= 2 && charq->err <= 4) return FALSE;
	return TRUE;
}
