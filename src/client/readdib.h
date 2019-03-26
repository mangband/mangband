/* File: readdib.h */

/*
 * This file has been modified for use with "Angband 2.8.2"
 *
 * Copyright 1991 Microsoft Corporation. All rights reserved.
 */

#include "../common/angband.h"

/*
 * Information about a bitmap
 */
typedef struct {
	HANDLE hDIB;
	HBITMAP hBitmap;
	HPALETTE hPalette;
	BYTE   CellWidth;
	BYTE   CellHeight;
} DIBINIT;

/* Read a DIB from a file */
BOOL ReadDIB(HWND, LPSTR, DIBINIT *);

/* Free a DIB */
extern void FreeDIB(DIBINIT *dib);
