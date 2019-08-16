#ifdef WIN32
/* File: readbits.c */

/*
 * This package provides a routine to read a DIB file and set up the
 * device dependent version of the image.
 *
 * This file has been modified for use with "Angband 2.8.2"
 *
 * COPYRIGHT:
 *
 *   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
 *
 *   You have a royalty-free right to use, modify, reproduce and
 *   distribute the Sample Files (and/or any modified version) in
 *   any way you find useful, provided that you agree that
 *   Microsoft has no warranty obligations or liability for any
 *   Sample Application Files which are modified.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "readdib.h"


/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif

/*
 * Make sure "huge" is legal XXX XXX XXX
 */
#undef huge
#ifdef WIN32
# define huge /* oops */
#endif


/*
 * Number of bytes to be read during each read operation
 */
#define MAXREAD  32768

#define _LREAD(field) \
	if (sizeof(field) != _lread(fh, &field, sizeof(field))) goto ErrExit

/*
 * Private routine to read more than 64K at a time
 *
 * Reads data in steps of 32k till all the data has been read.
 *
 * Returns number of bytes requested, or zero if something went wrong.
 */
static DWORD PASCAL lread(int fh, VOID far *pv, DWORD ul)
{
	DWORD ulT = ul;
	BYTE huge *hp = pv;

	while (ul > (DWORD)MAXREAD)
	{
		if (_lread(fh, (LPSTR)hp, (WORD)MAXREAD) != MAXREAD)
				return 0;
		ul -= MAXREAD;
		hp += MAXREAD;
	}
	if (_lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul)
		return 0;
	return ulT;
}


/*
 * Given a BITMAPINFOHEADER, create a palette based on the color table.
 *
 * Returns the handle of a palette, or zero if something went wrong.
 */
static HPALETTE PASCAL NEAR MakeDIBPalette(LPBITMAPINFOHEADER lpInfo)
{
	NPLOGPALETTE npPal;
	RGBQUAD far *lpRGB;
	HPALETTE hLogPal; 
	WORD i;

	/*
	 * since biClrUsed field was filled during the loading of the DIB,
	 * we know it contains the number of colors in the color table.
	 */
	if (lpInfo->biClrUsed)
	{
		npPal = (NPLOGPALETTE)LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) + 
		                                 (WORD)lpInfo->biClrUsed * sizeof(PALETTEENTRY));
		if (!npPal)
			return(FALSE);

		npPal->palVersion = 0x300;
		npPal->palNumEntries = (WORD)lpInfo->biClrUsed;

		/* get pointer to the color table */
		lpRGB = (RGBQUAD FAR *)((LPSTR)lpInfo + lpInfo->biSize);

		/* copy colors from the color table to the LogPalette structure */
		for (i = 0; i < lpInfo->biClrUsed; i++, lpRGB++)
		{
			npPal->palPalEntry[i].peRed = lpRGB->rgbRed;
			npPal->palPalEntry[i].peGreen = lpRGB->rgbGreen;
			npPal->palPalEntry[i].peBlue = lpRGB->rgbBlue;
			npPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
		}

		hLogPal = CreatePalette((LPLOGPALETTE)npPal);
		LocalFree((HANDLE)npPal);
		return(hLogPal);
	}

	/*
	 * 24-bit DIB with no color table.  return default palette.  Another
	 * option would be to create a 256 color "rainbow" palette to provide
	 * some good color choices.
	 */
	else
	{
		return(GetStockObject(DEFAULT_PALETTE));
	}
}


/*
 * Given a DIB, create a bitmap and corresponding palette to be used for a
 * device-dependent representation of the image.
 *
 * Returns TRUE on success (phPal and phBitmap are filled with appropriate
 * handles.  Caller is responsible for freeing objects) and FALSE on failure
 * (unable to create objects, both pointer are invalid).
 */
static BOOL NEAR PASCAL MakeBitmapAndPalette(HDC hDC, HANDLE hDIB, 
                                             HPALETTE * phPal, HBITMAP * phBitmap)
{
	LPBITMAPINFOHEADER lpInfo;
	BOOL result = FALSE;
	HBITMAP hBitmap;
	HPALETTE hPalette, hOldPal;
	LPSTR lpBits;

	lpInfo = (LPBITMAPINFOHEADER) GlobalLock(hDIB);
	if ((hPalette = MakeDIBPalette(lpInfo)) != 0)
	{
		/* Need to realize palette for converting DIB to bitmap. */
		hOldPal = SelectPalette(hDC, hPalette, TRUE);
		RealizePalette(hDC);

		lpBits = ((LPSTR)lpInfo + (WORD)lpInfo->biSize + 
		          (WORD)lpInfo->biClrUsed * sizeof(RGBQUAD));
		hBitmap = CreateDIBitmap(hDC, lpInfo, CBM_INIT, lpBits, 
		                         (LPBITMAPINFO)lpInfo, DIB_RGB_COLORS);

		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);

		if (!hBitmap)
		{
			DeleteObject(hPalette);
		}
		else
		{
			*phBitmap = hBitmap;
			*phPal = hPalette;
			result = TRUE;
		}
	}
	return(result);
}



/*
* Reads a DIB from a file, obtains a handle to its BITMAPINFO struct, and
* loads the DIB.  Once the DIB is loaded, the function also creates a bitmap
* and palette out of the DIB for a device-dependent form.
*
* Returns TRUE if the DIB is loaded and the bitmap/palette created, in which
* case, the DIBINIT structure pointed to by pInfo is filled with the appropriate
* handles, and FALSE if something went wrong.
*/
BOOL ReadDIB(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo)
{
	HFILE fh;
	LPBITMAPINFOHEADER lpbi;
	OFSTRUCT of;
	BITMAPFILEHEADER bf;
	WORD nNumColors;
	BOOL result = FALSE;
	DWORD offBits, bSize;
	HDC hDC;
	BOOL bCoreHead = FALSE;
	BITMAPCOREHEADER bc;

	/* Open the file and get a handle to its BITMAPINFO */
	fh = OpenFile(lpFileName, &of, OF_READ);
	if (fh == -1) return (FALSE);

	pInfo->hDIB = GlobalAlloc(GHND, (DWORD)(sizeof(BITMAPINFOHEADER) +
		256 * sizeof(RGBQUAD)));

	if (!pInfo->hDIB) return (FALSE);

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(pInfo->hDIB);

	/*
	 * Read the BITMAPFILEHEADER (field by field to avoid structure alignment problems)
	 *
	 * typedef struct tagBITMAPFILEHEADER {
	 *	WORD  bfType;
	 *	DWORD bfSize;
	 *	WORD  bfReserved1;
	 *	WORD  bfReserved2;
	 *	DWORD bfOffBits;
	 * } BITMAPFILEHEADER;
	 */
	_LREAD(bf.bfType);
	_LREAD(bf.bfSize);
	_LREAD(bf.bfReserved1);
	_LREAD(bf.bfReserved2);
	_LREAD(bf.bfOffBits);

	/* 'BM' */
	if (bf.bfType != 0x4d42) goto ErrExit;

	/* Read the size of the bitmap */
	_LREAD(bSize);

	/* Set BITMAPINFOHEADER size */
	lpbi->biSize = sizeof(BITMAPINFOHEADER);

	/*
	 * Read the BITMAPCOREHEADER (field by field to avoid structure alignment problems)
	 *
	 * typedef struct tagBITMAPCOREHEADER {
	 *	DWORD bcSize;
	 *	WORD  bcWidth;
	 *	WORD  bcHeight;
	 *	WORD  bcPlanes;
	 *	WORD  bcBitCount;
	 * } BITMAPCOREHEADER;
	 */

	if (bSize == sizeof(BITMAPCOREHEADER))
	{
		_LREAD(bc.bcWidth);
		_LREAD(bc.bcHeight);
		_LREAD(bc.bcPlanes);
		_LREAD(bc.bcBitCount);

		/* Set BITMAPINFOHEADER values */
		lpbi->biWidth = bc.bcWidth;
		lpbi->biHeight = bc.bcHeight;
		lpbi->biPlanes = bc.bcPlanes;
		lpbi->biBitCount = bc.bcBitCount;
		bCoreHead = TRUE;
	}

	/*
	 * Read the BITMAPINFOHEADER (field by field to avoid structure alignment problems)
	 *
	 * typedef struct tagBITMAPINFOHEADER {
	 *	DWORD biSize;
	 *	LONG  biWidth;
	 *	LONG  biHeight;
	 *	WORD  biPlanes;
	 *	WORD  biBitCount;
	 *	DWORD biCompression;
	 *	DWORD biSizeImage;
	 *	LONG  biXPelsPerMeter;
	 *	LONG  biYPelsPerMeter;
	 *	DWORD biClrUsed;
	 *	DWORD biClrImportant;
	 * } BITMAPINFOHEADER;
	 */

	else if (bSize == sizeof(BITMAPINFOHEADER))
	{
		_LREAD(lpbi->biWidth);
		_LREAD(lpbi->biHeight);
		_LREAD(lpbi->biPlanes);
		_LREAD(lpbi->biBitCount);
		_LREAD(lpbi->biCompression);
		_LREAD(lpbi->biSizeImage);
		_LREAD(lpbi->biXPelsPerMeter);
		_LREAD(lpbi->biYPelsPerMeter);
		_LREAD(lpbi->biClrUsed);
		_LREAD(lpbi->biClrImportant);
	}

	/* Error */
	else goto ErrExit;

	if (!(nNumColors = (WORD)lpbi->biClrUsed))
	{
		/* No color table for 24-bit, default size otherwise */
		if (lpbi->biBitCount != 24) nNumColors = 1 << lpbi->biBitCount;
	}

	/* Fill in some default values if they are zero */
	if (lpbi->biClrUsed == 0) lpbi->biClrUsed = nNumColors;

	if (lpbi->biSizeImage == 0)
		lpbi->biSizeImage = (((((lpbi->biWidth * (DWORD)lpbi->biBitCount) + 31)
			& ~31) >> 3) * lpbi->biHeight);

	/* Otherwise wouldn't work with 16 color bitmaps */
	else if ((nNumColors == 16) && (lpbi->biSizeImage > bf.bfSize))
		lpbi->biSizeImage /= 2;

	/* Get a proper-sized buffer for header, color table and bits */
	GlobalUnlock(pInfo->hDIB);
	pInfo->hDIB = GlobalReAlloc(pInfo->hDIB, lpbi->biSize +
		nNumColors * sizeof(RGBQUAD) + lpbi->biSizeImage, 0);

	/* Can't resize buffer for loading */
	if (!pInfo->hDIB) goto ErrExit2;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(pInfo->hDIB);

	/* Read the color table */
	if (!bCoreHead)
		_lread(fh, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD));
	else
	{
		signed int i;
		RGBQUAD FAR *pQuad;
		RGBTRIPLE FAR *pTriple;

		_lread(fh, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBTRIPLE));

		pQuad = (RGBQUAD FAR *)((LPSTR)lpbi + lpbi->biSize);
		pTriple = (RGBTRIPLE FAR *) pQuad;
		for (i = nNumColors - 1; i >= 0; i--)
		{
			pQuad[i].rgbRed = pTriple[i].rgbtRed;
			pQuad[i].rgbBlue = pTriple[i].rgbtBlue;
			pQuad[i].rgbGreen = pTriple[i].rgbtGreen;
			pQuad[i].rgbReserved = 0;
		}
	}

	/* Offset to the bits from start of DIB header */
	offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

	if (bf.bfOffBits != 0L) _llseek(fh,bf.bfOffBits, SEEK_SET);

	/* Use local version of '_lread()' above */
	if (lpbi->biSizeImage == lread(fh, (LPSTR)lpbi + offBits, lpbi->biSizeImage))
	{
		GlobalUnlock(pInfo->hDIB);

		hDC = GetDC(hWnd);
		if (!MakeBitmapAndPalette(hDC, pInfo->hDIB, &pInfo->hPalette, &pInfo->hBitmap))
		{
			ReleaseDC(hWnd, hDC);
			goto ErrExit2;
		}
		else
		{
			ReleaseDC(hWnd, hDC);
			result = TRUE;
		}
	}
	else
	{
ErrExit:
		GlobalUnlock(pInfo->hDIB);
ErrExit2:
		GlobalFree(pInfo->hDIB);
	}

	_lclose(fh);
	return (result);
}
	
/* Free a DIB */
void FreeDIB(DIBINIT *dib)
{
	/* Free the bitmap stuff */
	if (dib->hPalette) DeleteObject(dib->hPalette);
	if (dib->hBitmap) DeleteObject(dib->hBitmap);
	if (dib->hDIB) GlobalFree(dib->hDIB);

	dib->hPalette = NULL;
	dib->hBitmap = NULL;
	dib->hDIB = NULL;
}

#endif
