/* File: readpng.c */
#ifdef USE_WIN

/*
 * This package provides a routine to read a DIB file and set up the
 * device dependent version of the image.
 *
 * This file has been modified for use with "Angband 2.9.2"
 * This file has been modified for use with "z+Angband 0.3.3"
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

/*
 * This file is a port of readpng.c found in Angband 3.5.1.
 * Note, however, that unlike V, it DOES NOT use libpng,
 * opting for LuPng package instead. Otherwise, the semantics
 * and the return values are the same.
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "../lupng/lupng.h"

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
 * Imports a DIB from a PNG file. Once
 * the DIB is loaded, the function also creates a bitmap
 * and palette out of the DIB for a device-dependent form.
 *
 * Returns TRUE if the DIB is loaded and the bitmap/palette created, in which
 * case, the DIBINIT structure pointed to by pInfo is filled with the appropriate
 * handles, and FALSE if something went wrong.
 */
BOOL ReadDIB2_PNG(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo, DIBINIT *pMask, BOOL premultiply) {
	LuImage *img;
	
	BOOL noerror = TRUE;
	
	HBITMAP hBitmap;
	HPALETTE hPalette, hOldPal;
	BITMAPINFO bi, biSrc;
	HDC hDC;
	
	int color_type;
	int bit_depth;
	int width, height;
	int y;

	BOOL update = FALSE;

	/* open the file and test it for being a png */
	img = luPngReadFile(lpFileName);
	if (!img)
	{
		plog_fmt("Unable to open PNG file.");
		return (FALSE);
	}

	width = img->width;
	height = img->height;
	color_type = img->channels;
	bit_depth = img->depth;

	/* BGR-TO-RGB pass */
	luImageBGR(img);
	/* Darken Alpha pass */
	luImageDarkenAlpha(img);
	/* after these requests, the data should always be RGB or ARGB */

	/* pre multiply the image colors by the alhpa if thats what we want */
	if (premultiply && (color_type == 4)) {
		int x;
		int r,g,b,a;
		char *row;
		/* process the file */
		for (y = 0; y < height; y++) {
			row = &img->data[y * width * 4];
			for (x = 0; x < width; x++) {
				a = *(row + x*4 + 3);
				if (a == 0) {
					/* for every alpha that is fully transparent, make the
					 * corresponding color true black */
					*(row + x*4 + 0) = 0;
					*(row + x*4 + 1) = 0;
					*(row + x*4 + 2) = 0;
				} else
				if (a != 255) {
					float rf,gf,bf,af;
					/* blend the color value based on this value */
					r = *(row + x*4 + 0);
					g = *(row + x*4 + 1);
					b = *(row + x*4 + 2);

					rf = ((float)r) / 255.f;
					gf = ((float)g) / 255.f;
					bf = ((float)b) / 255.f;
					af = ((float)a) / 255.f;
        
					r = (byte)(rf*af*255.f);
					g = (byte)(gf*af*255.f);
					b = (byte)(bf*af*255.f);
        
					*(row + x*4 + 0) = r;
					*(row + x*4 + 1) = g;
					*(row + x*4 + 2) = b;
				}
			}
		}
	}
  
	/* create the DIB */
	bi.bmiHeader.biWidth = (LONG)width;
	bi.bmiHeader.biHeight = -((LONG)height);
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSize = 40; /* the size of the structure */
	bi.bmiHeader.biXPelsPerMeter = 3424; /* just a number I saw when testing this with a sample */
	bi.bmiHeader.biYPelsPerMeter = 3424; /* just a number I saw when testing this with a sample */
	bi.bmiHeader.biSizeImage = width*height*3;
	
	biSrc.bmiHeader.biWidth = (LONG)width;
	biSrc.bmiHeader.biHeight = -((LONG)height);
	biSrc.bmiHeader.biPlanes = 1;
	biSrc.bmiHeader.biClrUsed = 0;
	biSrc.bmiHeader.biClrImportant = 0;
	biSrc.bmiHeader.biCompression = BI_RGB;
	biSrc.bmiHeader.biSize = 40; /* the size of the structure */
	biSrc.bmiHeader.biXPelsPerMeter = 3424; /* just a number I saw when testing this with a sample */
	biSrc.bmiHeader.biYPelsPerMeter = 3424; /* just a number I saw when testing this with a sample */
	
	if (color_type == 4) {
		biSrc.bmiHeader.biBitCount = 32;
		biSrc.bmiHeader.biSizeImage = width*height*4;

		if (!pMask) {
			bi.bmiHeader.biBitCount = 32;
			bi.bmiHeader.biSizeImage = width*height*4;
		}
	} else {
		biSrc.bmiHeader.biBitCount = 24;
		biSrc.bmiHeader.biSizeImage = width*height*3;
	}

	hDC = GetDC(hWnd);
	
	hPalette = GetStockObject(DEFAULT_PALETTE);
	/* Need to realize palette for converting DIB to bitmap. */
	hOldPal = SelectPalette(hDC, hPalette, TRUE);
	RealizePalette(hDC);

	/* copy the data to the DIB */
	hBitmap = CreateDIBitmap(hDC, &(bi.bmiHeader), 0, NULL,
		 &biSrc, DIB_RGB_COLORS);

	if (hBitmap)
	{
		for (y = 0; y < height; y++)
		{
			byte *row = &img->data[y * width * img->channels + 0];
			if (SetDIBits(hDC, hBitmap, height-y-1, 1, row, &biSrc, DIB_RGB_COLORS) != 1)
			{
				/* plog_fmt("Failed to alloc temporary memory for PNG data."); */
				DeleteObject(hBitmap);
				hBitmap = NULL;
				noerror = FALSE;
				break;
			}
		}
	}
	SelectPalette(hDC, hOldPal, TRUE);
	RealizePalette(hDC);
	if (!hBitmap)
	{
		DeleteObject(hPalette);
		noerror = FALSE;
	}
	else
	{
		pInfo->hBitmap = hBitmap;
		pInfo->hPalette = hPalette;
		pInfo->hDIB = 0;
		pInfo->ImageWidth = width;
		pInfo->ImageHeight = height;
	}
	
	if (pMask && ((color_type == 4)||(color_type == 3)))
	{
		byte *pBits, v;
		int x;
		HBITMAP hBitmap2 = NULL;
		HPALETTE hPalette2 = GetStockObject(DEFAULT_PALETTE);
		BOOL have_alpha = FALSE;

		/* Need to realize palette for converting DIB to bitmap. */
		hOldPal = SelectPalette(hDC, hPalette2, TRUE);
		RealizePalette(hDC);
		
		/* allocate the storage space */
		pBits = (byte*)malloc(sizeof(byte)*width*height*3);
		if (!pBits)
		{
			noerror = FALSE;
		}

		if (noerror)
		{
			byte ck_r, ck_g, ck_b;
			if (img->channels == 3) {
				/* Hack -- get colorkey from top left corner */
				byte* top_px = &img->data[0];
				ck_r = *(top_px + 0);
				ck_g = *(top_px + 1);
				ck_b = *(top_px + 2);
			}
			for (y = 0; y < height; y++) {
				for (x = 0; x < width; x++) {
					byte* srcpix = &img->data[y * width * img->channels + x * img->channels + 0];
					if (img->channels == 3)
					{
						/* compare this color to COLOR KEY */
						v = ((*(srcpix + 0) == ck_r)
						 &&  (*(srcpix + 1) == ck_g)
						 &&  (*(srcpix + 2) == ck_b)) ? 255 : 0;
					}
					else
					{
						/* get the alpha byte from the source */
						v = *(srcpix + 3);
						if (v <= 64) v = 0; else v = 255;
						v = 255 - v;
					}
					if (v==255)
					{
						have_alpha = TRUE;
					}
					/* write the alpha byte to the three colors of the storage space */
					*(pBits + (y*width*3) + (x*3)+0) = v;
					*(pBits + (y*width*3) + (x*3)+1) = v;
					*(pBits + (y*width*3) + (x*3)+2) = v;
				}
			}
			/* create the bitmap from the storage space */
			if (have_alpha)
			{
				hBitmap2 = CreateDIBitmap(hDC, &(bi.bmiHeader), CBM_INIT, pBits,
					  &bi, DIB_RGB_COLORS);
			}
			free(pBits);
		}
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		if (!hBitmap2)
		{
			DeleteObject(hPalette2);
			noerror = FALSE;
		}
		else
		{
			pMask->hBitmap = hBitmap2;
			pMask->hPalette = hPalette2;
			pMask->hDIB = 0;
		}
	}
	
	/* release all the the PNG Structures */
	luImageRelease(img, NULL);
	
	ReleaseDC(hWnd,hDC);
	
	if (!noerror)
	{
		return (FALSE);
	}
	return (TRUE);
}

#endif