#ifndef INCLUDED_CWIN_H
#define INCLUDED_CWIN_H

typedef struct FontSpec {
	char *name;
	int isbold;
	int height;
	int width;
	int charset;
} FontSpec;

/*
* This structure is passed to event handler functions as the `dlg'
* parameter, and hence is passed back to winctrls access functions.
*/
struct dlgparam {
	HWND hwnd;			       /* the hwnd of the dialog box */
	struct winctrls *controltrees[8];  /* can have several of these */
	int nctrltrees;
	char *wintitle;		       /* title of actual window */
	char *errtitle;		       /* title of error sub-messageboxes */
	void *data;			       /* data to pass in refresh events */
	union control *focused, *lastfocused; /* which ctrl has focus now/before */
	char shortcuts[128];	       /* track which shortcuts in use */
	int coloursel_wanted;	       /* has an event handler asked for
								   * a colour selector? */
	struct { unsigned char r, g, b, ok; } coloursel_result;   /* 0-255 */
//	tree234 *privdata;		       /* stores per-control private data */
	int ended, endresult;	       /* has the dialog been ended? */
	int fixed_pitch_fonts;             /* are we constrained to fixed fonts? */
};

#endif